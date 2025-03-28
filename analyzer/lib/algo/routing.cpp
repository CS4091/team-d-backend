#include "routing.h"

using namespace std;
using namespace arro;
using namespace arro::algo;
using namespace arro::aviation;
using json = nlohmann::json;

template <typename T>
using min_pqueue = priority_queue<T, vector<T>, greater<T>>;

json data::AirportLatLng::stringify(const AirportLatLng& airport) {
	return {{"id", airport.id()}, {"city", airport.city()}, {"lat", airport.lat()}, {"lng", airport.lng()}};
}

double data::AirwayData::cost(unsigned int passengers) const {
	return (data.minFuel + aviation::FE_PER_PASSENGER * passengers) * fuelPrice;
}

json data::AirwayData::stringify(const AirwayData& airway) {
	return {{"distance", airway.data.totalLateral}};
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

Routing arro::algo::findRoute(const vector<data::AirportLatLng>& airports, const vector<data::CityLatLng>& cities,
							  const vector<data::RouteReq>& requestedRoutes, const vector<Plane>& planes) {
	using ConnGraph = Graph<data::AirportLatLng, data::AirwayData>;
	using ConnNode = ConnGraph::Node;
	using ConnLookup = ConnGraph::LinkLookup;
	using RoutePlan = arro::algo::__routing::RoutePlan;

	map<string, ConnGraph> connGraphs = mapFlights(airports, planes);

	map<string, vector<vector<double>>> masterTables;  // master routing table for routing around empty aircraft
	for (auto plane : planes) {
		string model = plane.model();

		if (!masterTables.count(model)) {
			masterTables.emplace(model, arro::algo::floydWarshall(
											connGraphs[model], [](const data::AirwayData& airway) { return airway.cost(0); }));	 // again, aircraft are empty
		}
	}

	map<string, list<const ConnNode*>> baselineRoute;
	double baselineCost = 0;
	min_pqueue<__routing::PlaneLoc> planeOrder;

	for (auto plane : planes) {
		list<const ConnNode*> route;

		auto start = connGraphs[plane.model()][plane.homeBase];
		if (!start) throw invalid_argument("Home base '" + plane.homeBase() + "' for plane '" + plane.id() + "' not found.");
		route.push_back(start);

		baselineRoute.emplace(plane.id(), route);
		planeOrder.emplace(plane, start, 0);
	}

	vector<data::RouteReq> baselineReqRoutes = requestedRoutes;
	while (baselineReqRoutes.size() > 0) {
		auto nextPlane = planeOrder.top();
		planeOrder.pop();
		const auto& connGraph = connGraphs[nextPlane.plane.model()];
		const auto& masterTable = masterTables[nextPlane.plane.model()];

		const ConnNode* planeLoc = nextPlane.loc;
		size_t fromIdx = planeLoc->idx;

		data::RouteReq closestRoute = baselineReqRoutes[0];
		for (size_t i = 1; i < baselineReqRoutes.size(); i++) {
			// unsafe dereference but fuck ups should have been validated out earlier
			size_t toIdx = connGraph[baselineReqRoutes[i].from]->idx, oldToIdx = connGraph[closestRoute.from]->idx;

			if (masterTable[fromIdx][toIdx] < masterTable[fromIdx][oldToIdx]) {
				closestRoute = baselineReqRoutes[i];
			}
		}

		baselineReqRoutes.erase(find_if(baselineReqRoutes.begin(), baselineReqRoutes.end(),
										[&closestRoute](data::RouteReq req) { return req.from == closestRoute.from && req.to == closestRoute.to; }));

		const ConnNode *from = connGraph[closestRoute.from], *to = connGraph[closestRoute.to];

		list<const ConnNode*> path;
		double endTime = nextPlane.time;

		if (from != planeLoc) {
			path = arro::algo::dijkstra<data::AirportLatLng, data::AirwayData>(connGraph, planeLoc, from,
																			   [](const data::AirwayData& airway) { return airway.cost(0); });

			for (auto it = next(path.begin()); it != path.end(); it++) {
				baselineRoute[nextPlane.plane.id()].push_back(*it);

				auto airway = connGraph[ConnLookup{*prev(it), *it}]->data();
				baselineCost += airway.cost(0);
				endTime += airway.data.time;
			}
		}

		path =
			arro::algo::dijkstra<data::AirportLatLng, data::AirwayData>(connGraph, from, to, [](const data::AirwayData& airway) { return airway.cost(1); });

		for (auto it = next(path.begin()); it != path.end(); it++) {
			baselineRoute[nextPlane.plane.id()].push_back(*it);

			auto airway = connGraph[ConnLookup{*prev(it), *it}]->data();
			baselineCost += airway.cost(1);
			endTime += airway.data.time;
		}

		planeOrder.emplace(nextPlane.plane, to, endTime);
	}

	// vector<RoutePlan> queue(1);
	// queue[0].cost = 0;
	// copy(demandGraph.edges().begin(), demandGraph.edges().end(), back_inserter(queue[0].remaining));
	// queue[0].route.push_back(connGraph[startCity]);

	// make_heap(queue.begin(), queue.end(), std::greater{});

	// while (queue.size() > 0) {
	// 	pop_heap(queue.begin(), queue.end(), std::greater{});

	// 	RoutePlan entry = queue.back();
	// 	queue.pop_back();

	// 	if (entry.remaining.size() == 0) return {entry.route, baselineRoute};

	// 	queue.reserve(queue.size() + entry.remaining.size());

	// 	for (auto link : entry.remaining) {
	// 		const ConnNode *lastCity = entry.route.back(), *nextCity = connGraph[link->from()->data().id], *routeEnd = connGraph[link->to()->data().id];

	// 		RoutePlan newEntry(entry.route, vector<const DemandLink*>(), entry.cost);
	// 		newEntry.cost += masterTable[lastCity->idx][nextCity->idx];

	// 		list<const ConnNode*> path = arro::algo::dijkstra<CNData, CLData>(connGraph, lastCity, nextCity);
	// 		for (auto it = next(path.begin()); it != path.end(); it++) newEntry.route.push_back(*it);
	// 		path = arro::algo::dijkstra<CNData, CLData>(connGraph, nextCity, routeEnd);
	// 		for (auto it = path.begin(); it != path.end();) {
	// 			const ConnNode* from = *it;

	// 			it++;

	// 			if (it != path.end()) {
	// 				const ConnNode* to = *it;

	// 				newEntry.cost += connGraph[typename arro::Graph<CNData, CLData>::LinkLookup{from, to}]->data().cost();

	// 				if (newEntry.cost < baselineCost) newEntry.route.push_back(to);
	// 			}
	// 		}
	// 		copy_if(entry.remaining.begin(), entry.remaining.end(), back_inserter(newEntry.remaining),
	// 				[link](const DemandLink* route) { return route != link; });

	// 		queue.push_back(newEntry);
	// 	}
	// }

	Routing out;
	for (auto [id, routing] : baselineRoute) {
		list<string> route;

		for (auto airport : routing) route.push_back(airport->data().id());

		out.route.emplace(id, route);
		out.baseline.emplace(id, route);
	}

	return out;
}

map<string, Graph<data::AirportLatLng, data::AirwayData>> arro::algo::mapFlights(const vector<data::AirportLatLng>& airports,
																				 const vector<aviation::Plane>& planes) {
	using ConnGraph = Graph<data::AirportLatLng, data::AirwayData>;
	using PotentialFlight = __routing::PotentialFlight;

	// collect some general stats first (determines degree of nodes in graph)
	unsigned int large = 0, medium = 0, small = 0, other = 0;
	for (auto airport : airports) {
		if (airport.type() == "large_airport")
			large++;
		else if (airport.type() == "medium_airport")
			medium++;
		else if (airport.type() == "small_airport")
			small++;
		else
			other++;
	}

	unsigned int lgMaxDegree = log(large) * 2, medMaxDegree = log(medium), smMaxDegree = log(small) / 2, otherMaxDegree = log(other) / 2;

	map<string, ConnGraph> connGraphs;

	for (auto plane : planes) {
		if (!connGraphs.count(plane.model())) {
			ConnGraph graph;

			for (auto airport : airports) graph.add(airport);

			min_pqueue<PotentialFlight> queue;

			for (auto a : airports) {
				for (auto b : airports) {
					if (a.id != b.id) {
						auto data = aviation::planFlight(plane, geospatial::llToRect(a.lat(), a.lng()), geospatial::llToRect(b.lat(), b.lng()));

						if (data.has_value()) queue.emplace(*data, a.id(), b.id(), data->minFuel * a.fuel());
					}
				}
			}

			while (!queue.empty()) {
				auto flight = queue.top();
				queue.pop();

				auto from = graph[flight.from], to = graph[flight.to];

				if (!from || !to) throw runtime_error("Something dun fucked up big");

				unsigned int fromMaxDegree, toMaxDegree;

				if (from->data().type() == "large_airport")
					fromMaxDegree = lgMaxDegree;
				else if (from->data().type() == "medium_airport")
					fromMaxDegree = lgMaxDegree;
				else if (from->data().type() == "small_airport")
					fromMaxDegree = lgMaxDegree;
				else
					fromMaxDegree = otherMaxDegree;

				if (to->data().type() == "large_airport")
					toMaxDegree = lgMaxDegree;
				else if (to->data().type() == "medium_airport")
					toMaxDegree = lgMaxDegree;
				else if (to->data().type() == "small_airport")
					toMaxDegree = lgMaxDegree;
				else
					toMaxDegree = otherMaxDegree;

				if (from->neighbors().size() < fromMaxDegree && to->neighbors().size() < toMaxDegree)
					graph.link(from, to, data::AirwayData(flight.data, from->data().fuel()));
			}

			connGraphs.emplace(plane.model(), graph);
		}
	}

	return connGraphs;
}