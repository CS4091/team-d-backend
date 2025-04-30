#include "routing.h"

#include <iostream>

#include "dijkstra.h"
#include "exceptions.h"
#include "fw.h"

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

double data::ReducedAirwayData::cost(unsigned int passengers) const {
	return (minFuel + aviation::FE_PER_PASSENGER * passengers * lateral) * fuelPrice;
}

json data::ReducedAirwayData::stringify(const ReducedAirwayData& airway) {
	return {{"minFuel", airway.minFuel}};
}

json __routing::PlannedFlight::stringify(const PlannedFlight& flight) {
	return flight.plane;
}

bool __routing::operator>(const RoutePlan& a, const RoutePlan& b) {
	return a.cost + a.reqCost > b.cost + b.reqCost;
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
	using CNData = data::AirportLatLng;
	using CLData = data::ReducedAirwayData;
	using ConnGraph = Graph<CNData, CLData>;
	using DbgGraph = Graph<data::AirportLatLng, __routing::PlannedFlight>;
	using ConnNode = ConnGraph::Node;
	using ConnLookup = ConnGraph::LinkLookup;
	using RoutePlan = arro::algo::__routing::RoutePlan;
	using PlaneLoc = arro::algo::__routing::PlaneLoc;

#ifdef BENCHMARK
	return bench::benchmark<Routing>([cities, requestedRoutes, planes](bench::BenchmarkCtx& ctx) {
#endif
		map<string, ConnGraph> connGraphs;

#ifdef BENCHMARK
		ctx.start("Read Graphs");
#endif

		map<string, BinFWTable> masterTables;  // master routing table for routing around empty aircraft
		for (auto plane : planes) {
			if (!connGraphs.count(plane.model)) {
				connGraphs.emplace(plane.model, ConnGraph::readFromBinFile(filesystem::current_path() / "maps" / (plane.model + ".bing"),
																		   [](int fd, size_t maxStrLen, char* buf) {
																			   data::AirportLatLng airport;

																			   memset(buf, '\0', maxStrLen + 1);
																			   read(fd, buf, maxStrLen);
																			   airport.id = string(buf);

																			   memset(buf, '\0', maxStrLen + 1);
																			   read(fd, buf, maxStrLen);
																			   airport.city = string(buf);

																			   read(fd, &airport.lat, sizeof(double));
																			   read(fd, &airport.lng, sizeof(double));
																			   read(fd, &airport.fuel, sizeof(double));

																			   return airport;
																		   }));

				masterTables.emplace(plane.model, arro::algo::BinFWTable::readFromBinFile(filesystem::current_path() / "maps" /
																						  (plane.model + ".binmrt")));	// again, aircraft are empty
			}
		}

#ifdef BENCHMARK
		ctx.stop();
#endif

		vector<data::RouteError> errors;
		for (auto req : requestedRoutes) {
			data::RouteError err(req);

			for (auto plane : planes) {
				const auto& graph = connGraphs.at(plane.model);

				auto home = graph[plane.homeBase], from = graph[req.from], to = graph[req.to];

				if (!home) err.reasons.push_back(data::PlaneError(plane, "Plane's home base is impossible (too short runways)."));
				if (!from) err.reasons.push_back(data::PlaneError(plane, "Plane cannot reach start of route."));
				if (!to) err.reasons.push_back(data::PlaneError(plane, "Plane cannot reach start of route."));

				if (!home || !from || !to) continue;

				if (home != from) {
					try {
						dijkstra(graph, home, from, [](const CLData& airway) { return airway.cost(0); });
					} catch (...) {
						err.reasons.push_back(data::PlaneError(plane, "Plane cannot reach '" + req.from + "' from its home base '" + plane.homeBase + "'."));
						continue;
					}
				}

				try {
					dijkstra(graph, from, to, [](const CLData& airway) { return airway.cost(0); });
				} catch (...) {
					err.reasons.push_back(data::PlaneError(plane, "Plane cannot reach '" + req.to + "' from '" + req.from + "'."));
					continue;
				}
			}

			if (err.reasons.size() == planes.size()) errors.push_back(err);
		}

		if (errors.size() > 0) throw UnroutableException(errors);

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

#ifdef BENCHMARK
		ctx.start("Generating Baseline Route");
#endif

		vector<data::RouteReq> baselineReqRoutes = requestedRoutes;
		while (baselineReqRoutes.size() > 0) {
			if (planeOrder.empty()) throw runtime_error("Empty baseline plane order.");

			auto nextPlane = planeOrder.top();
			planeOrder.pop();
			const auto& connGraph = connGraphs[nextPlane.plane.model];
			const auto& masterTable = masterTables.at(nextPlane.plane.model);

			const ConnNode* planeLoc = nextPlane.loc;
			size_t fromIdx = planeLoc->idx;

			data::RouteReq closestRoute = baselineReqRoutes[0];
			size_t crIdx = 0;
			for (size_t i = 1; i < baselineReqRoutes.size(); i++) {
				if (connGraph[baselineReqRoutes[i].from]) {
					size_t toIdx = connGraph[baselineReqRoutes[i].from]->idx, oldToIdx = connGraph[closestRoute.from]->idx;

					if (masterTable(fromIdx, toIdx) < masterTable(fromIdx, oldToIdx) && connGraph[baselineReqRoutes[i].to]) {
						closestRoute = baselineReqRoutes[i];
						crIdx = i;
					}
				}
			}

			const ConnNode *from = connGraph[closestRoute.from], *to = connGraph[closestRoute.to];

			if (!from || !to) continue;	 // plane has no available routes to service

			if (closestRoute.passengers <= nextPlane.plane.passengers) {
				baselineReqRoutes.erase(baselineReqRoutes.begin() + crIdx);
			} else {
				baselineReqRoutes[crIdx].passengers -= nextPlane.plane.passengers;
				baselineReqRoutes[crIdx].approxCost -= nextPlane.plane.passengers * FE_PER_PASSENGER * from->data().fuel;
			}

			unsigned int passengersTaken = min(closestRoute.passengers, nextPlane.plane.passengers);

			list<const ConnNode*> path;
			double endTime = nextPlane.time;

			if (from != planeLoc) {
				path = arro::algo::dijkstra<CNData, CLData>(connGraph, planeLoc, from, [](const data::ReducedAirwayData& airway) { return airway.cost(0); });

				for (auto it = next(path.begin()); it != path.end(); it++) {
					baselineRoute[nextPlane.plane.id].push_back(*it);

					auto airway = connGraph[ConnLookup{*prev(it), *it}]->data();
					baselineCost += airway.cost(0);
					endTime += airway.time;
				}
			}

			path = arro::algo::dijkstra<CNData, CLData>(connGraph, from, to,
														[passengersTaken](const data::ReducedAirwayData& airway) { return airway.cost(passengersTaken); });

			for (auto it = next(path.begin()); it != path.end(); it++) {
				baselineRoute[nextPlane.plane.id].push_back(*it);

				auto airway = connGraph[ConnLookup{*prev(it), *it}]->data();
				baselineCost += airway.cost(passengersTaken);
				endTime += airway.time;
			}

			planeOrder.emplace(nextPlane.plane, to, endTime);
		}

#ifdef BENCHMARK
		ctx.stop();
#endif

		// DbgGraph dbgRouting;
		// for (auto node : connGraphs[planes[0].model].nodes()) dbgRouting.add(node->data());

		// flatten(dbgRouting).jsonDumpToFile("routing.graph.c.json");

		// for (auto [plane, routing] : baselineRoute) {
		// 	for (auto it = next(routing.begin()); it != routing.end(); it++) {
		// 		dbgRouting.link((*prev(it))->data().id, (*it)->data().id, __routing::PlannedFlight(plane));
		// 	}
		// }

		// flatten(dbgRouting).jsonDumpToFile("routing.graph.b.json");

#ifdef BENCHMARK
		ctx.start("Route Optimization");
#endif

		vector<RoutePlan> queue(1);
		queue[0].cost = 0;
		queue[0].reqCost = 0;
		copy(requestedRoutes.begin(), requestedRoutes.end(), back_inserter(queue[0].remaining));

		for (auto req : requestedRoutes) queue[0].reqCost += req.approxCost;
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

#ifdef BENCHMARK
				ctx.stop();
#endif

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
			const auto& masterTable = masterTables.at(plane.model);

			bool hasRoute = false;
			for (auto route : entry.remaining) {
				const ConnNode *lastCity = nextPlane.loc, *nextCity = connGraph[route.from], *routeEnd = connGraph[route.to];

				if (nextCity && routeEnd) {
					hasRoute = true;

					double endTime = nextPlane.time;

					RoutePlan newEntry(entry.route, planeOrder, vector<data::RouteReq>(), entry.cost, entry.reqCost - route.approxCost);
					copy_if(entry.remaining.begin(), entry.remaining.end(), back_inserter(newEntry.remaining),
							[&route](const data::RouteReq& r) { return !(r.from == route.from && r.to == route.to); });
					if (plane.passengers < route.passengers) {
						entry.remaining.push_back(route);
						entry.remaining.back().passengers -= plane.passengers;
						entry.remaining.back().approxCost -= plane.passengers * FE_PER_PASSENGER * nextCity->data().fuel;
					}
					newEntry.cost += masterTable(lastCity->idx, nextCity->idx);

					unsigned int passengersTaken = min(route.passengers, plane.passengers);

					list<const ConnNode*> path;

					if (lastCity != nextCity) {
						path = dijkstra<CNData, CLData>(connGraph, lastCity, nextCity, [](const data::ReducedAirwayData& airway) { return airway.cost(0); });

						for (auto it = next(path.begin()); it != path.end(); it++) {
							newEntry.route[nextPlane.plane.id].push_back(*it);

							auto airway = connGraph[ConnLookup{*prev(it), *it}]->data();
							endTime += airway.time;
						}
					}

					path = dijkstra<CNData, CLData>(connGraph, nextCity, routeEnd,
													[passengersTaken](const data::ReducedAirwayData& airway) { return airway.cost(passengersTaken); });

					for (auto it = next(path.begin()); it != path.end(); it++) {
						newEntry.route[nextPlane.plane.id].push_back(*it);

						auto airway = connGraph[ConnLookup{*prev(it), *it}]->data();
						newEntry.cost += airway.cost(passengersTaken);
						endTime += airway.time;
					}

					newEntry.planeOrder.emplace(plane, routeEnd, endTime);

					if (newEntry.cost <= baselineCost) queue.push_back(newEntry);
				}
			}

			if (!hasRoute) {
				if (entry.planeOrder.size() == 1)
					throw runtime_error("Undetected unroutability");
				else {
					entry.planeOrder.pop();
					queue.push_back(entry);
				}
			}

			make_heap(queue.begin(), queue.end(), std::greater{});
		}

		throw runtime_error("Undetected unroutability");

#ifdef BENCHMARK
	});
#endif
}
