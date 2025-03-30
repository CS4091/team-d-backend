#include "routing.h"

#include <iostream>

using namespace std;
using namespace arro;
using namespace arro::algo;
using namespace arro::aviation;
using json = nlohmann::json;

template <typename T>
using min_pqueue = priority_queue<T, vector<T>, greater<T>>;

data::AirportLatLng::AirportLatLng(const json& obj)
	: id(obj["id"]), city(obj["city"]), type(obj["type"]), lat(obj["lat"]), lng(obj["lng"]), fuel(obj["fuel"]) {
	vector<json> runways = obj["runways"];

	copy(runways.begin(), runways.end(), back_inserter(data::AirportLatLng::runways));
}

json data::AirportLatLng::stringify(const AirportLatLng& airport) {
	return {{"id", airport.id}, {"city", airport.city}, {"lat", airport.lat}, {"lng", airport.lng}};
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
		if (!masterTables.count(plane.model)) {
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
		auto nextPlane = planeOrder.top();
		planeOrder.pop();
		const auto& connGraph = connGraphs[nextPlane.plane.model];
		const auto& masterTable = masterTables[nextPlane.plane.model];

		const ConnNode* planeLoc = nextPlane.loc;
		size_t fromIdx = planeLoc->idx;

		data::RouteReq closestRoute = baselineReqRoutes[0];
		size_t crIdx = 0;
		for (size_t i = 1; i < baselineReqRoutes.size(); i++) {
			// unsafe dereference but fuck ups should have been validated out earlier
			size_t toIdx = connGraph[baselineReqRoutes[i].from]->idx, oldToIdx = connGraph[closestRoute.from]->idx;

			if (masterTable[fromIdx][toIdx] < masterTable[fromIdx][oldToIdx]) {
				closestRoute = baselineReqRoutes[i];
				crIdx = i;
			}
		}

		baselineReqRoutes.erase(baselineReqRoutes.begin() + crIdx);

		const ConnNode *from = connGraph[closestRoute.from], *to = connGraph[closestRoute.to];

		if (!from) throw runtime_error("Start airport '" + closestRoute.from + "' not found in graph");
		if (!to) throw runtime_error("End airport '" + closestRoute.to + "' not found in graph");

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

		for (auto airport : routing) route.push_back(airport->data().id);

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
		if (airport.type == "large_airport")
			large++;
		else if (airport.type == "medium_airport")
			medium++;
		else if (airport.type == "small_airport")
			small++;
		else
			other++;
	}

	unsigned int lgMaxDegree = log(large) * 3, medMaxDegree = log(medium) * 2, smMaxDegree = log(small), otherMaxDegree = log(other) / 2;

	map<string, ConnGraph> connGraphs;

	for (auto plane : planes) {
		if (!connGraphs.count(plane.model)) {
			ConnGraph graph;
			vector<data::AirportLatLng> availableAirports;
			for (auto airport : airports) {
				unsigned int neighbors = 0;

				bool available = false;
				for (auto runway : airport.runways) {
					if (runway.length >= max(plane.landingRunway, plane.takeoffRunway)) {
						available = true;
						break;
					}
				}

				if (available) availableAirports.push_back(airport);
			}

			for (auto airport : availableAirports) graph.add(airport);

			min_pqueue<PotentialFlight> queue;

			for (auto a : availableAirports) {
				for (auto b : availableAirports) {
					auto data = aviation::planFlight(plane, geospatial::llToRect(a.lat, a.lng), geospatial::llToRect(b.lat, b.lng));

					if (data.has_value()) queue.emplace(*data, a.id, b.id, data->minFuel * a.fuel);
				}
			}

			while (!queue.empty()) {
				auto flight = queue.top();
				queue.pop();

				auto from = graph[flight.from], to = graph[flight.to];

				if (!from || !to) throw runtime_error("Something dun fucked up big");

				unsigned int fromMaxDegree;

				if (from->data().type == "large_airport")
					fromMaxDegree = lgMaxDegree;
				else if (from->data().type == "medium_airport")
					fromMaxDegree = lgMaxDegree;
				else if (from->data().type == "small_airport")
					fromMaxDegree = lgMaxDegree;
				else
					fromMaxDegree = otherMaxDegree;

				if (from->neighbors().size() < fromMaxDegree) graph.link(from, to, data::AirwayData(flight.data, from->data().fuel));
			}

			recluster(graph, plane);

			connGraphs.emplace(plane.model, graph);
		}
	}

	return connGraphs;
}

void arro::algo::recluster(arro::Graph<data::AirportLatLng, data::AirwayData>& graph, const aviation::Plane& plane) {
	using ConnNode = arro::Graph<data::AirportLatLng, data::AirwayData>::Node;

	vector<vector<double>> table = floydWarshall(graph, [](const data::AirwayData& airway) { return airway.cost(0); });

	auto badRow = find_if(table.begin(), table.end(),
						  [](const vector<double>& row) { return find_if(row.begin(), row.end(), [](double val) { return val == INFINITY; }) != row.end(); });
	while (badRow != table.end()) {
		size_t badFrom = badRow - table.begin(),
			   badTo = find_if(badRow->begin(), badRow->end(), [](double val) { return val == INFINITY; }) - badRow->begin();
		auto from = graph.nodes()[badFrom], to = graph.nodes()[badTo];

		ConnNode* nearest = nullptr;
		aviation::FlightData nearestFlight;

		do {
			for (size_t i = 0; i < graph.size(); i++) {
				if (i != badFrom && i != badTo && table[badFrom][i] != INFINITY) {
					auto intermediary = graph.nodes()[i];

					auto flightData = aviation::planFlight(plane, geospatial::llToRect(intermediary->data().lat, intermediary->data().lng),
														   geospatial::llToRect(to->data().lat, to->data().lng));

					if (flightData.has_value() &&
						(!nearest || flightData->minFuel * nearest->data().fuel < nearestFlight.minFuel * intermediary->data().fuel)) {
						nearest = intermediary;
						nearestFlight = flightData.value();
					}
				}
			}

			if (!nearest) {
				auto nextBadTo = find_if(next(badRow->begin(), badTo + 1), badRow->end(), [](double val) { return val == INFINITY; });
				if (nextBadTo == badRow->end()) {
					badRow = find_if(next(badRow), table.end(), [](const vector<double>& row) {
						return find_if(row.begin(), row.end(), [](double val) { return val == INFINITY; }) != row.end();
					});
					if (badRow != table.end()) nextBadTo = badRow->begin();
				}

				if (badRow == table.end()) {
					nearest = from;
					auto flightData = aviation::planFlight(plane, geospatial::llToRect(from->data().lat, from->data().lng),
														   geospatial::llToRect(to->data().lat, to->data().lng));

					cout << from->data().id << ": " << from->neighbors().size() << endl;
					cout << to->data().id << ": " << to->neighbors().size() << endl;
					if (flightData.has_value()) {
						nearestFlight = flightData.value();
						break;
					} else {
						// throw runtime_error("Unable to recluster graph, no intermediary for '" + from->data().id + "' and '" + to->data().id + "'");
						return;
					}
				}

				badFrom = badRow - table.begin();
				badTo = find_if(nextBadTo, badRow->end(), [](double val) { return val == INFINITY; }) - badRow->begin();
				from = graph.nodes()[badFrom];
				to = graph.nodes()[badTo];
			}
		} while (!nearest);

		graph.link(nearest, to, data::AirwayData(nearestFlight, from->data().fuel));
		cout << "fixed " << from->data().id << " to " << to->data().id << " via " << nearest->data().id << endl;

		table = arro::algo::floydWarshall(graph, [](const data::AirwayData& airway) { return airway.cost(0); });
		badRow = find_if(table.begin(), table.end(),
						 [](const vector<double>& row) { return find_if(row.begin(), row.end(), [](double val) { return val == INFINITY; }) != row.end(); });
	}
}