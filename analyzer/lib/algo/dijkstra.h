#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include <utils/Graph.h>
#include <utils/concepts.h>

#include <algorithm>
#include <concepts>
#include <list>
#include <map>

#include "exceptions.h"

namespace arro {
namespace algo {
namespace __dijkstra {
template <UniqueSerializable NodeData, Serializable LinkData>
struct NodeEntry {
	const Graph<NodeData, LinkData>::Node* node;
	const Graph<NodeData, LinkData>::Node* parent;
	double cost;
};

template <UniqueSerializable NodeData, Serializable LinkData>
bool operator>(const NodeEntry<NodeData, LinkData>& a, const NodeEntry<NodeData, LinkData>& b);
}  // namespace __dijkstra

template <UniqueSerializable NodeData, Serializable LinkData, Function<bool, LinkData> CostFn>
std::list<const typename Graph<NodeData, LinkData>::Node*> dijkstra(const Graph<NodeData, LinkData>& graph,
																	const typename Graph<NodeData, LinkData>::Node* start,
																	const typename Graph<NodeData, LinkData>::Node* end, CostFn costFn);
}  // namespace algo
}  // namespace arro

#include "dijkstra.hpp"

#endif