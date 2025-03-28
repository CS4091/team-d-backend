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

const double EARTH_R = 6.378e6;

struct ArrivalTime {
	int time;

	static json stringify(const ArrivalTime& city);
	static ArrivalTime parse(const string& str);
};

struct RouteData {
	double distance;
	double duration;

	double cost() const { return duration; }

	static json stringify(const RouteData& city);
	static RouteData parse(const string& str);
};

struct RouteOrder {
	int order;

	static json stringify(const RouteOrder& city);
	static RouteOrder parse(const string& str);
};

struct City {
	string id;
};

struct MapCity : public City {
	double lat;
	double lng;

	static json stringify(const MapCity& city);
	static MapCity parse(const string& str);
};

struct GeoCity : public City {
	arro::Vector3D pos;

	static json stringify(const GeoCity& city);
	static GeoCity parse(const string& str);
};

struct GraphCity : public City {
	double x;
	double y;

	static json stringify(const GraphCity& city);
	static GraphCity parse(const string& str);
};

istream& operator>>(istream& in, ArrivalTime& city);

istream& operator>>(istream& in, MapCity& city);

struct NeighborEntry {
	arro::Graph<GeoCity, RouteData>::Node* node;
	double distance;
};

bool operator>(const NeighborEntry& a, const NeighborEntry& b);

template <Serializable T>
arro::Graph<GraphCity, T> flatten(const arro::Graph<GeoCity, T>& graph) {
	arro::Vector3D avg(0, 0, 0);
	for (auto node : graph.nodes()) avg += node->data().pos;

	// avg is now the normal vector to the "average plane" of the city points (and also the center of the points)
	avg /= graph.nodes().size();

	arro::Vector3D north = arro::Vector3D(0, 0, 1), east = arro::Vector3D(1, 0, 0);
	// project north and east into the plane (will serve as y and x respectively)
	north = north - (north % avg);
	east = east - (east % avg);

	return graph.template map<GraphCity>([&avg, &north, &east](const GeoCity& city) {
		arro::Vector3D planeVec = city.pos - (city.pos % avg);

		double x = planeVec[east], y = planeVec[north];

		return GraphCity{city.id, x, y};
	});
}

int main(int argc, char* argv[]) {
	// string path = argc > 1 ? argv[1] : "test.graph";

	// arro::Graph<MapCity, ArrivalTime> demandGraph = arro::Graph<MapCity, ArrivalTime>::readFromFile(path);
	// arro::Graph<GeoCity, RouteData> connectivityGraph;

	// for (auto node : demandGraph.nodes()) connectivityGraph.add(GeoCity{node->data().id, arro::geospatial::llToRect(node->data().lat, node->data().lng)});

	// int k = sqrt(connectivityGraph.size());

	// // perform knn
	// for (auto node : connectivityGraph.nodes()) {
	// 	NeighborEntry farthestNeighbor = {nullptr, -INFINITY};
	// 	priority_queue<NeighborEntry, std::vector<NeighborEntry>, std::greater<NeighborEntry>> queue;

	// 	for (auto other : connectivityGraph.nodes()) {
	// 		if (node != other) {
	// 			auto fromCity = node->data(), toCity = other->data();

	// 			double distance = arro::aviation::flightDistance(fromCity.pos, toCity.pos);
	// 			queue.emplace(other, distance);

	// 			if (distance > farthestNeighbor.distance) {
	// 				farthestNeighbor.node = other;
	// 				farthestNeighbor.distance = distance;
	// 			}
	// 		}
	// 	}

	// 	for (int i = 0; i < k; i++) {
	// 		auto entry = queue.top();
	// 		queue.pop();
	// 		auto fromCity = node->data(), toCity = entry.node->data();

	// 		double distance = arro::aviation::flightDistance(fromCity.pos, toCity.pos);
	// 		double duration = distance / 901'000;  // 901km/h (Boing 737 cruise speed)

	// 		connectivityGraph.link(node, entry.node, RouteData{distance, duration});
	// 	}

	// 	connectivityGraph.link(node, farthestNeighbor.node, RouteData{farthestNeighbor.distance, farthestNeighbor.distance / 901'000});
	// }

	// flatten(connectivityGraph).jsonDumpToFile(path + ".c.json");
	// flatten(demandGraph.map<GeoCity>([](const MapCity& city) {
	// 	return GeoCity{city.id, arro::geospatial::llToRect(city.lat, city.lng)};
	// })).jsonDumpToFile(path + ".d.json");

	// ifstream assets("test.assets");
	// string startCity;
	// assets >> startCity;
	// assets.close();

	// try {
	// 	auto route = arro::algo::findRoute(connectivityGraph, demandGraph, startCity);

	// 	arro::Graph<GeoCity, RouteOrder> routeGraph;

	// 	for (auto node : connectivityGraph.nodes()) routeGraph.add(node->data());

	// 	int i = 1;
	// 	for (auto it = route.route.begin(); it != route.route.end();) {
	// 		const arro::Graph<GeoCity, RouteData>::Node* from = *it;

	// 		it++;

	// 		if (it != route.route.end()) {
	// 			const arro::Graph<GeoCity, RouteData>::Node* to = *it;

	// 			routeGraph.link(from->data().id, to->data().id, RouteOrder{i++});
	// 		}
	// 	}

	// 	flatten(routeGraph).jsonDumpToFile(path + ".p.json");

	// 	arro::Graph<GeoCity, RouteOrder> baselineGraph;

	// 	for (auto node : connectivityGraph.nodes()) baselineGraph.add(node->data());

	// 	i = 1;
	// 	for (auto it = route.baseline.begin(); it != route.baseline.end();) {
	// 		const arro::Graph<GeoCity, RouteData>::Node* from = *it;

	// 		it++;

	// 		if (it != route.baseline.end()) {
	// 			const arro::Graph<GeoCity, RouteData>::Node* to = *it;

	// 			baselineGraph.link(from->data().id, to->data().id, RouteOrder{i++});
	// 		}
	// 	}

	// 	flatten(baselineGraph).jsonDumpToFile(path + ".b.json");

	// 	auto it = route.route.begin();
	// 	cout << (*it++)->data().id;
	// 	for (; it != route.route.end(); it++) {
	// 		cout << " -> " << (*it)->data().id;
	// 	}
	// 	cout << endl;
	// } catch (exception& e) {
	// 	cerr << "Warning: " << e.what() << endl;

	// 	return 1;
	// } catch (...) {
	// 	cerr << "Unknown error" << endl;

	// 	return 1;
	// }

	return 0;
}

istream& operator>>(istream& in, ArrivalTime& time) {
	in >> time.time;

	return in;
}

istream& operator>>(istream& in, MapCity& city) {
	in >> city.id >> city.lat >> city.lng;

	return in;
}

json ArrivalTime::stringify(const ArrivalTime& time) {
	return time.time;
}

ArrivalTime ArrivalTime::parse(const string& str) {
	json parsed = json::parse(str);

	if (!parsed.is_number_integer()) throw invalid_argument("Invalid ArrivalTime json");

	return ArrivalTime{parsed.get<int>()};
}

json RouteData::stringify(const RouteData& data) {
	return {{"distance", data.distance}, {"duration", data.duration}};
}

RouteData RouteData::parse(const string& str) {
	json parsed = json::parse(str);

	if (!parsed["distance"].is_number_float() || !parsed["duration"].is_number_float()) throw invalid_argument("Invalid RouteData json");

	return RouteData{parsed["distance"].get<double>(), parsed["duration"].get<double>()};
}

json RouteOrder::stringify(const RouteOrder& data) {
	return data.order;
}

RouteOrder RouteOrder::parse(const string& str) {
	json parsed = json::parse(str);

	if (!parsed.is_number_integer()) throw invalid_argument("Invalid RouteOrder json");

	return RouteOrder{parsed.get<int>()};
}

json MapCity::stringify(const MapCity& city) {
	return {{"id", city.id}, {"lat", city.lat}, {"lng", city.lng}};
}

MapCity MapCity::parse(const string& str) {
	json parsed = json::parse(str);

	if (!parsed["id"].is_string() || !parsed["lat"].is_number_float() || !parsed["lng"].is_number_float()) throw invalid_argument("Invalid MapCity json");

	return MapCity{parsed["id"].get<string>(), parsed["lat"].get<double>(), parsed["lng"].get<double>()};
}

json GeoCity::stringify(const GeoCity& city) {
	return {{"id", city.id}, {"pos", json::array({city.pos[0], city.pos[1], city.pos[2]})}};
}

GeoCity GeoCity::parse(const string& str) {
	json parsed = json::parse(str), pos = parsed["pos"];

	if (!parsed["id"].is_string() || !pos.is_array() || !pos[0].is_number_float() || !pos[1].is_number_float() || !pos[2].is_number_float())
		throw invalid_argument("Invalid GeoCity json");

	return GeoCity{parsed["id"].get<string>(), arro::Vector3D(pos[0].get<double>(), pos[1].get<double>(), pos[2].get<double>())};
}

json GraphCity::stringify(const GraphCity& city) {
	return {{"id", city.id}, {"x", city.x}, {"y", city.y}};
}

GraphCity GraphCity::parse(const string& str) {
	json parsed = json::parse(str);

	if (!parsed["id"].is_string() || !parsed["x"].is_number_float() || !parsed["y"].is_number_float()) throw invalid_argument("Invalid GraphCity json");

	return GraphCity{parsed["id"].get<string>(), parsed["x"].get<double>(), parsed["y"].get<double>()};
}

bool operator>(const NeighborEntry& a, const NeighborEntry& b) {
	return a.distance > b.distance;
}
