#include "dijkstra.h"

template <UniqueSerializable NodeData, Serializable LinkData>
bool arro::algo::__dijkstra::operator>(const NodeEntry<NodeData, LinkData>& a, const NodeEntry<NodeData, LinkData>& b) {
	return a.cost > b.cost;
}

template <UniqueSerializable NodeData, Serializable LinkData, Function<bool, LinkData> CostFn>
std::list<const typename arro::Graph<NodeData, LinkData>::Node*> arro::algo::dijkstra(const arro::Graph<NodeData, LinkData>& graph,
																					  const typename arro::Graph<NodeData, LinkData>::Node* start,
																					  const typename arro::Graph<NodeData, LinkData>::Node* end,
																					  CostFn costFn) {
	using namespace std;
	using Graph = arro::Graph<NodeData, LinkData>;
	using Node = Graph::Node;
	using Link = Graph::Link;
	using NodeEntry = arro::algo::__dijkstra::NodeEntry<NodeData, LinkData>;

	map<const Node*, NodeEntry> costs;
	vector<NodeEntry> queue;
	queue.reserve(graph.size());

	bool foundStart = false, foundEnd = false;

	for (auto node : graph.nodes()) {
		if (node == start) {
			queue.emplace_back(node, nullptr, 0);
			foundStart = true;
		} else {
			queue.emplace_back(node, nullptr, INFINITY);
			if (node == end) foundEnd = true;
		}
	}

	if (!foundStart) {
		throw invalid_argument("Start node '" + std::string(start->data().id) + "' not found in graph");
	}
	if (!foundEnd) {
		throw invalid_argument("End node '" + std::string(end->data().id) + "' not found in graph");
	}

	make_heap(queue.begin(), queue.end(), std::greater{});

	while (queue.size() > 0) {
		pop_heap(queue.begin(), queue.end(), std::greater{});

		NodeEntry entry = queue.back();
		queue.pop_back();

		double cost = entry.cost;
		for (auto link : entry.node->neighbors()) {
			bool inQueue = false;

			for (NodeEntry& frontierEntry : queue) {
				if (frontierEntry.node == link->to()) {
					if (cost + costFn(link->data()) < frontierEntry.cost) {
						frontierEntry.cost = cost + costFn(link->data());
						frontierEntry.parent = entry.node;
					}

					inQueue = true;
					break;
				}
			}

			if (!inQueue) {
				for (auto& [_, oldEntry] : costs) {
					if (oldEntry.node == link->to()) {
						if (cost + costFn(link->data()) < oldEntry.cost) {
							oldEntry.cost = cost + costFn(link->data());
							oldEntry.parent = entry.node;
						}

						break;
					}
				}
			}
		}

		costs.emplace(entry.node, entry);

		if (queue.size() > 0) make_heap(queue.begin(), queue.end(), std::greater{});
	}

	NodeEntry endEntry = costs.at(end);

	if (endEntry.cost == INFINITY) throw algo::UnreachableException<NodeData, LinkData>(start, end);

	list<const Node*> path;
	path.push_front(end);

	const Node* next = endEntry.parent;
	do {
		NodeEntry entry = costs.at(next);
		path.push_front(entry.node);

		next = entry.parent;
	} while (next != nullptr);

	return path;
}