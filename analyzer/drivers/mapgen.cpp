#include <algo/dijkstra.h>
#include <algo/fw.h>
#include <algo/routing.h>
#include <math/Vector3D.h>
#include <math/aviation.h>
#include <math/geospatial.h>
#include <utils/Graph.h>
#include <utils/rand.h>

#include <cmath>
#include <fstream>
#include <iostream>
#include <list>
#include <numbers>
#include <queue>
#include <string>
#include <vector>

using namespace std;
using namespace numbers;
using json = nlohmann::json;

template <typename T>
using min_pqueue = priority_queue<T, vector<T>, greater<T>>;

using ConnGraph = arro::Graph<arro::algo::data::AirportWithRunways, arro::algo::data::AirwayData>;
using ConnNode = ConnGraph::Node;

struct PotentialFlight {
	arro::aviation::FlightData data;
	ConnNode* from;
	ConnNode* to;
	double costApprox;
};

bool operator>(const PotentialFlight& a, const PotentialFlight& b);

ConnGraph mapFlights(const vector<arro::algo::data::AirportWithRunways>& airports, const arro::aviation::Plane& plane);
void recluster(ConnGraph& graph, const arro::aviation::Plane& plane);

template <JSONConstructable T>
vector<T> readArray(const string& path) {
	ifstream in(path);
	vector<json> arr = json::parse(in);
	in.close();

	vector<T> out;
	copy(arr.begin(), arr.end(), back_inserter(out));

	return out;
}

struct __PlaneCity {
	string id;
	double x;
	double y;

	static nlohmann::json stringify(const __PlaneCity& city) { return {{"id", city.id}, {"x", city.x}, {"y", city.y}}; }
};

arro::Graph<__PlaneCity, arro::algo::data::AirwayData> flatten(const ConnGraph& graph) {
	return graph.template map<__PlaneCity>([](const arro::algo::data::AirportWithRunways& city) {
		arro::Vector3D pos = arro::geospatial::mercator(city.lat, city.lng);

		return __PlaneCity{city.id, pos[0], pos[1]};
	});
}

int main(int argc, char* argv[]) {
	vector<arro::algo::data::AirportWithRunways> airports = readArray<arro::algo::data::AirportWithRunways>("airports.json");
	vector<arro::aviation::Plane> planes = readArray<arro::aviation::Plane>("planes.json");

	for (auto plane : planes) {
		auto graph = mapFlights(airports, plane);

		size_t maxStrLen = airports[0].id.length();
		for (auto airport : airports) {
			if (airport.id.length() > maxStrLen) maxStrLen = airport.id.length();
			if (airport.city.length() > maxStrLen) maxStrLen = airport.city.length();
		}

		graph.binDumpToFile(
			plane.model + ".bing", maxStrLen,
			[maxStrLen](int fd, const arro::algo::data::AirportWithRunways& airport) {
				char* buf = new char[maxStrLen];

				memset(buf, '\0', maxStrLen);
				memcpy(buf, airport.id.c_str(), airport.id.length());
				write(fd, buf, maxStrLen);

				memset(buf, '\0', maxStrLen);
				memcpy(buf, airport.city.c_str(), airport.city.length());
				write(fd, buf, maxStrLen);

				delete[] buf;

				write(fd, &airport.lat, sizeof(double));
				write(fd, &airport.lng, sizeof(double));
				write(fd, &airport.fuel, sizeof(double));
			},
			[](int fd, const arro::algo::data::AirwayData& airway) {
				arro::algo::data::ReducedAirwayData data{airway.data.minFuel, airway.fuelPrice, airway.data.time};

				write(fd, &data, sizeof(arro::algo::data::ReducedAirwayData));
			});
		flatten(graph).jsonDumpToFile(plane.model + ".graph.json");
	}

	return 0;
}

bool operator>(const PotentialFlight& a, const PotentialFlight& b) {
	return a.costApprox > b.costApprox;
}

ConnGraph mapFlights(const vector<arro::algo::data::AirportWithRunways>& airports, const arro::aviation::Plane& plane) {
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

	ConnGraph graph;
	vector<arro::algo::data::AirportWithRunways> availableAirports;
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
			auto data = arro::aviation::planFlight(plane, arro::geospatial::llToRect(a.lat, a.lng), arro::geospatial::llToRect(b.lat, b.lng));

			if (data.has_value()) queue.emplace(*data, graph[a.id], graph[b.id], data->minFuel * a.fuel);
		}
	}

	while (!queue.empty()) {
		auto flight = queue.top();
		queue.pop();

		auto from = flight.from, to = flight.to;

		unsigned int fromMaxDegree;

		if (from->data().type == "large_airport")
			fromMaxDegree = lgMaxDegree;
		else if (from->data().type == "medium_airport")
			fromMaxDegree = lgMaxDegree;
		else if (from->data().type == "small_airport")
			fromMaxDegree = lgMaxDegree;
		else
			fromMaxDegree = otherMaxDegree;

		if (from->neighbors().size() < fromMaxDegree) graph.link(from, to, arro::algo::data::AirwayData(flight.data, from->data().fuel));
	}

	recluster(graph, plane);

	return graph;
}

void recluster(ConnGraph& graph, const arro::aviation::Plane& plane) {
	vector<vector<double>> table = arro::algo::floydWarshall(graph, [](const arro::algo::data::AirwayData& airway) { return airway.cost(0); });

	auto badRow = find_if(table.begin(), table.end(),
						  [](const vector<double>& row) { return find_if(row.begin(), row.end(), [](double val) { return val == INFINITY; }) != row.end(); });
	while (badRow != table.end()) {
		size_t badFrom = badRow - table.begin(),
			   badTo = find_if(badRow->begin(), badRow->end(), [](double val) { return val == INFINITY; }) - badRow->begin();
		auto from = graph.nodes()[badFrom], to = graph.nodes()[badTo];

		ConnNode* nearest = nullptr;
		arro::aviation::FlightData nearestFlight;

		do {
			for (size_t i = 0; i < graph.size(); i++) {
				if (i != badFrom && i != badTo && table[badFrom][i] != INFINITY) {
					auto intermediary = graph.nodes()[i];

					auto flightData = arro::aviation::planFlight(plane, arro::geospatial::llToRect(intermediary->data().lat, intermediary->data().lng),
																 arro::geospatial::llToRect(to->data().lat, to->data().lng));

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
					auto flightData = arro::aviation::planFlight(plane, arro::geospatial::llToRect(from->data().lat, from->data().lng),
																 arro::geospatial::llToRect(to->data().lat, to->data().lng));

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

		graph.link(nearest, to, arro::algo::data::AirwayData(nearestFlight, from->data().fuel));
		cout << "fixed " << from->data().id << " to " << to->data().id << " via " << nearest->data().id << endl;

		table = arro::algo::floydWarshall(graph, [](const arro::algo::data::AirwayData& airway) { return airway.cost(0); });
		badRow = find_if(table.begin(), table.end(),
						 [](const vector<double>& row) { return find_if(row.begin(), row.end(), [](double val) { return val == INFINITY; }) != row.end(); });
	}
}