#include "routing.h"

#include <iostream>

using namespace std;
using namespace arro;
using namespace arro::algo;
using namespace arro::aviation;
using namespace filesystem;
using json = nlohmann::json;

template <typename T>
using min_pqueue = priority_queue<T, vector<T>, greater<T>>;

data::AirportLatLng::AirportLatLng(const json& obj)
	: id(obj["id"]), city(obj["city"]), type(obj["type"]), lat(obj["lat"]), lng(obj["lng"]), fuel(obj["fuel"]) {}

json data::AirportLatLng::stringify(const AirportLatLng& airport) {
	return {{"id", airport.id}, {"city", airport.city}, {"lat", airport.lat}, {"lng", airport.lng}};
}

data::AirportWithRunways::AirportWithRunways(const json& obj) : AirportLatLng(obj) {
	vector<json> runways = obj["runways"];

	copy(runways.begin(), runways.end(), back_inserter(data::AirportWithRunways::runways));
}

json data::AirportWithRunways::stringify(const AirportWithRunways& airport) {
	return {{"id", airport.id}, {"city", airport.city}, {"lat", airport.lat}, {"lng", airport.lng}};
}

double data::AirwayData::cost(unsigned int passengers) const {
	return (data.minFuel + aviation::FE_PER_PASSENGER * passengers) * fuelPrice;
}

json data::AirwayData::stringify(const AirwayData& airway) {
	return {{"distance", airway.data.totalLateral}};
}

json __routing::PlannedFlight::stringify(const PlannedFlight& flight) {
	return flight.plane;
}

bool __routing::operator>(const RoutePlan& a, const RoutePlan& b) {
	return a.cost > b.cost;
}

bool __routing::operator>(const PotentialFlight& a, const PotentialFlight& b) {
	return a.costApprox > b.costApprox;
}

bool __routing::operator>(const PlaneLoc& a, const PlaneLoc& b) {
	return a.time > b.time;
}

struct __PlaneCity {
	string id;
	double x;
	double y;

	static nlohmann::json stringify(const __PlaneCity& city) { return {{"id", city.id}, {"x", city.x}, {"y", city.y}}; }
};

template <Serializable T>
arro::Graph<__PlaneCity, T> flatten(const arro::Graph<data::AirportLatLng, T>& graph) {
	return graph.template map<__PlaneCity>([](const data::AirportLatLng& city) {
		arro::Vector3D pos = arro::geospatial::mercator(city.lat, city.lng);

		return __PlaneCity{city.id, pos[0], pos[1]};
	});
}

Routing arro::algo::findRoute(const vector<data::CityLatLng>& cities, const vector<data::RouteReq>& requestedRoutes, const vector<Plane>& planes) {
	using ConnGraph = Graph<data::AirportLatLng, data::AirwayData>;
	using CNData = data::AirportLatLng;
	using CLData = data::AirwayData;
	using DbgGraph = Graph<data::AirportLatLng, __routing::PlannedFlight>;
	using ConnNode = ConnGraph::Node;
	using ConnLookup = ConnGraph::LinkLookup;
	using RoutePlan = arro::algo::__routing::RoutePlan;
	using PlaneLoc = arro::algo::__routing::PlaneLoc;

	map<string, ConnGraph> connGraphs;

	map<string, vector<vector<double>>> masterTables;  // master routing table for routing around empty aircraft
	for (auto plane : planes) {
		if (!connGraphs.count(plane.model)) {
			connGraphs.emplace(plane.model, ConnGraph::readFromBinFile(
												filesystem::current_path() / "maps" / (plane.model + ".bing"),
												[](int fd) {
													data::AirportLatLng airport;

													char c;
													do {
														read(fd, &c, 1);
														if (c != '\0') airport.id.push_back(c);
													} while (c != '\0');
													do {
														read(fd, &c, 1);
														if (c != '\0') airport.city.push_back(c);
													} while (c != '\0');
													do {
														read(fd, &c, 1);
														if (c != '\0') airport.type.push_back(c);
													} while (c != '\0');

													read(fd, &airport.lat, sizeof(double));
													read(fd, &airport.lng, sizeof(double));
													read(fd, &airport.fuel, sizeof(double));

													return airport;
												},
												[](int fd) {
													data::AirwayData data;

													read(fd, &data, sizeof(data::AirwayData));

													return data;
												}));

			masterTables.emplace(plane.model, arro::algo::floydWarshall(connGraphs[plane.model], [](const data::AirwayData& airway) {
									 return airway.cost(0);
								 }));  // again, aircraft are empty
		}
	}

	map<string, list<const ConnNode*>> baselineRoute;
	double baselineCost = 0;
	min_pqueue<__routing::PlaneLoc> planeOrder;

	for (auto plane : planes) {
		list<const ConnNode*> route;

		auto start = connGraphs[plane.model][plane.homeBase];
		if (!start) throw invalid_argument("Home base '" + plane.homeBase + "' for plane '" + plane.id + "' not found.");
		route.push_back(start);

		baselineRoute.emplace(plane.id, route);
		planeOrder.emplace(plane, start, 0);
	}

	vector<data::RouteReq> baselineReqRoutes = requestedRoutes;
	while (baselineReqRoutes.size() > 0) {
		if (planeOrder.empty()) throw UnroutableException(baselineReqRoutes);

		auto nextPlane = planeOrder.top();
		planeOrder.pop();
		const auto& connGraph = connGraphs[nextPlane.plane.model];
		const auto& masterTable = masterTables[nextPlane.plane.model];

		const ConnNode* planeLoc = nextPlane.loc;
		size_t fromIdx = planeLoc->idx;

		data::RouteReq closestRoute = baselineReqRoutes[0];
		size_t crIdx = 0;
		for (size_t i = 1; i < baselineReqRoutes.size(); i++) {
			if (connGraph[baselineReqRoutes[i].from]) {
				size_t toIdx = connGraph[baselineReqRoutes[i].from]->idx, oldToIdx = connGraph[closestRoute.from]->idx;

				if (masterTable[fromIdx][toIdx] < masterTable[fromIdx][oldToIdx] && connGraph[baselineReqRoutes[i].to]) {
					closestRoute = baselineReqRoutes[i];
					crIdx = i;
				}
			}
		}

		baselineReqRoutes.erase(baselineReqRoutes.begin() + crIdx);

		const ConnNode *from = connGraph[closestRoute.from], *to = connGraph[closestRoute.to];

		if (!from || !to) continue;	 // plane has no available routes to service

		list<const ConnNode*> path;
		double endTime = nextPlane.time;

		if (from != planeLoc) {
			path = arro::algo::dijkstra<data::AirportLatLng, data::AirwayData>(connGraph, planeLoc, from,
																			   [](const data::AirwayData& airway) { return airway.cost(0); });

			for (auto it = next(path.begin()); it != path.end(); it++) {
				baselineRoute[nextPlane.plane.id].push_back(*it);

				auto airway = connGraph[ConnLookup{*prev(it), *it}]->data();
				baselineCost += airway.cost(0);
				endTime += airway.data.time;
			}
		}

		path =
			arro::algo::dijkstra<data::AirportLatLng, data::AirwayData>(connGraph, from, to, [](const data::AirwayData& airway) { return airway.cost(1); });

		for (auto it = next(path.begin()); it != path.end(); it++) {
			baselineRoute[nextPlane.plane.id].push_back(*it);

			auto airway = connGraph[ConnLookup{*prev(it), *it}]->data();
			baselineCost += airway.cost(1);
			endTime += airway.data.time;
		}

		planeOrder.emplace(nextPlane.plane, to, endTime);
	}

	// DbgGraph dbgRouting;
	// for (auto node : connGraphs[planes[0].model].nodes()) dbgRouting.add(node->data());

	// flatten(dbgRouting).jsonDumpToFile("routing.graph.c.json");

	// for (auto [plane, routing] : baselineRoute) {
	// 	for (auto it = next(routing.begin()); it != routing.end(); it++) {
	// 		dbgRouting.link((*prev(it))->data().id, (*it)->data().id, __routing::PlannedFlight(plane));
	// 	}
	// }

	// flatten(dbgRouting).jsonDumpToFile("routing.graph.b.json");

	vector<RoutePlan> queue(1);
	queue[0].cost = 0;
	copy(requestedRoutes.begin(), requestedRoutes.end(), back_inserter(queue[0].remaining));
	for (auto plane : planes) {
		list<const ConnNode*> path;

		auto base = connGraphs[plane.model][plane.homeBase];
		path.push_back(base);

		queue[0].route.emplace(plane.id, path);

		queue[0].planeOrder.emplace(plane, base, 0);
	}

	while (queue.size() > 0) {
		pop_heap(queue.begin(), queue.end(), std::greater{});

		RoutePlan entry = queue.back();
		queue.pop_back();

		if (entry.remaining.size() == 0) {
			Routing out;
			for (auto [id, routing] : baselineRoute) {
				list<string> route;

				for (auto airport : routing) route.push_back(airport->data().id);

				out.baseline.emplace(id, route);
			}

			for (auto [id, routing] : entry.route) {
				list<string> route;

				for (auto airport : routing) route.push_back(airport->data().id);

				out.route.emplace(id, route);
			}

			out.baselineCost = baselineCost;
			out.routeCost = entry.cost;

			return out;
		}

		queue.reserve(queue.size() + entry.remaining.size());

		min_pqueue<PlaneLoc> planeOrder = entry.planeOrder;
		auto nextPlane = planeOrder.top();
		auto plane = nextPlane.plane;
		planeOrder.pop();

		const auto& connGraph = connGraphs[plane.model];
		const auto& masterTable = masterTables[plane.model];

		bool hasRoute = false;
		for (auto route : entry.remaining) {
			const ConnNode *lastCity = nextPlane.loc, *nextCity = connGraph[route.from], *routeEnd = connGraph[route.to];

			if (nextCity && routeEnd) {
				hasRoute = true;

				double endTime = nextPlane.time;
				vector<data::RouteReq> remaining;
				copy_if(entry.remaining.begin(), entry.remaining.end(), back_inserter(remaining),
						[&route](const data::RouteReq& r) { return !(r.from == route.from && r.to == route.to); });

				RoutePlan newEntry(entry.route, planeOrder, remaining, entry.cost);
				newEntry.cost += masterTable[lastCity->idx][nextCity->idx];

				list<const ConnNode*> path;

				if (lastCity != nextCity) {
					path = dijkstra<CNData, CLData>(connGraph, lastCity, nextCity, [](const data::AirwayData& airway) { return airway.cost(0); });

					for (auto it = next(path.begin()); it != path.end(); it++) {
						newEntry.route[nextPlane.plane.id].push_back(*it);

						auto airway = connGraph[ConnLookup{*prev(it), *it}]->data();
						newEntry.cost += airway.cost(0);
						endTime += airway.data.time;
					}
				}

				path = dijkstra<CNData, CLData>(connGraph, nextCity, routeEnd, [](const data::AirwayData& airway) { return airway.cost(1); });

				for (auto it = next(path.begin()); it != path.end(); it++) {
					newEntry.route[nextPlane.plane.id].push_back(*it);

					auto airway = connGraph[ConnLookup{*prev(it), *it}]->data();
					newEntry.cost += airway.cost(0);
					endTime += airway.data.time;
				}

				newEntry.planeOrder.emplace(plane, routeEnd, endTime);

				if (newEntry.cost < baselineCost) queue.push_back(newEntry);
			}
		}

		if (!hasRoute) {
			if (entry.planeOrder.size() == 1)
				throw UnroutableException(entry.remaining);
			else {
				entry.planeOrder.pop();
				queue.push_back(entry);
			}
		}

		make_heap(queue.begin(), queue.end(), std::greater{});
	}

	throw UnroutableException(requestedRoutes);
}
