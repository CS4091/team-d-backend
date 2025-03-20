#include "routing.h"

template <UniqueSerializable CNData, typename CLData, UniqueSerializable DNData, Serializable DLData>
	requires Serializable<CLData> && Weighted<CLData>
bool arro::algo::__routing::operator>(const RoutePlan<CNData, CLData, DNData, DLData>& a, const RoutePlan<CNData, CLData, DNData, DLData>& b) {
	return a.cost > b.cost;
}

template <UniqueSerializable CNData, typename CLData, UniqueSerializable DNData, Serializable DLData>
	requires Serializable<CLData> && Weighted<CLData>
arro::algo::Routing<CNData, CLData, DNData, DLData> arro::algo::findRoute(const arro::Graph<CNData, CLData>& connGraph,
																		  const arro::Graph<DNData, DLData>& demandGraph, const std::string& startCity) {
	using namespace std;

	using ConnNode = Graph<CNData, CLData>::Node;
	using ConnLookup = Graph<CNData, CLData>::LinkLookup;
	using DemandNode = Graph<DNData, DLData>::Node;
	using DemandLink = Graph<DNData, DLData>::Link;
	using RoutePlan = arro::algo::__routing::RoutePlan<CNData, CLData, DNData, DLData>;

	vector<vector<double>> masterTable = arro::algo::floydWarshall(connGraph);

	const ConnNode* planeLoc = connGraph[startCity];
	double baselineCost = 0;
	list<const ConnNode*> baselineRoute;
	baselineRoute.push_back(planeLoc);

	vector<DemandLink*> requestedRoutes = demandGraph.edges();

	while (requestedRoutes.size() > 0) {
		size_t fromIdx = planeLoc->idx;
		DemandLink* closestRoute = requestedRoutes[0];

		for (size_t i = 1; i < requestedRoutes.size(); i++) {
			size_t toIdx = requestedRoutes[i]->from()->idx, oldToIdx = closestRoute->from()->idx;

			if (masterTable[fromIdx][toIdx] < masterTable[fromIdx][oldToIdx]) {
				closestRoute = requestedRoutes[i];
			}
		}

		requestedRoutes.erase(find(requestedRoutes.begin(), requestedRoutes.end(), closestRoute));

		const ConnNode *from = connGraph[closestRoute->from()->data().id], *to = connGraph[closestRoute->to()->data().id];

		list<const ConnNode*> path;

		if (from != planeLoc) {
			path = arro::algo::dijkstra<CNData, CLData>(connGraph, planeLoc, from);

			for (auto it = next(path.begin()); it != path.end(); it++) {
				baselineRoute.push_back(*it);
				baselineCost += connGraph[ConnLookup{*prev(it), *it}]->data().cost();
			}

			planeLoc = from;
		}

		path = arro::algo::dijkstra<CNData, CLData>(connGraph, from, to);

		for (auto it = next(path.begin()); it != path.end(); it++) {
			baselineRoute.push_back(*it);
			baselineCost += connGraph[ConnLookup{*prev(it), *it}]->data().cost();
		}

		planeLoc = to;
	}

	vector<RoutePlan> queue(1);
	queue[0].cost = 0;
	copy(demandGraph.edges().begin(), demandGraph.edges().end(), back_inserter(queue[0].remaining));
	queue[0].route.push_back(connGraph[startCity]);

	make_heap(queue.begin(), queue.end(), std::greater{});

	while (queue.size() > 0) {
		pop_heap(queue.begin(), queue.end(), std::greater{});

		RoutePlan entry = queue.back();
		queue.pop_back();

		if (entry.remaining.size() == 0) return {entry.route, baselineRoute};

		queue.reserve(queue.size() + entry.remaining.size());

		for (auto link : entry.remaining) {
			const ConnNode *lastCity = entry.route.back(), *nextCity = connGraph[link->from()->data().id], *routeEnd = connGraph[link->to()->data().id];

			RoutePlan newEntry(entry.route, vector<const DemandLink*>(), entry.cost);
			newEntry.cost += masterTable[lastCity->idx][nextCity->idx];

			list<const ConnNode*> path = arro::algo::dijkstra<CNData, CLData>(connGraph, lastCity, nextCity);
			for (auto it = next(path.begin()); it != path.end(); it++) newEntry.route.push_back(*it);
			path = arro::algo::dijkstra<CNData, CLData>(connGraph, nextCity, routeEnd);
			for (auto it = path.begin(); it != path.end();) {
				const ConnNode* from = *it;

				it++;

				if (it != path.end()) {
					const ConnNode* to = *it;

					newEntry.cost += connGraph[typename arro::Graph<CNData, CLData>::LinkLookup{from, to}]->data().cost();

					if (newEntry.cost < baselineCost) newEntry.route.push_back(to);
				}
			}
			copy_if(entry.remaining.begin(), entry.remaining.end(), back_inserter(newEntry.remaining),
					[link](const DemandLink* route) { return route != link; });

			queue.push_back(newEntry);
		}
	}

	return {baselineRoute, baselineRoute};
}