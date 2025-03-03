#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include <utils/Graph.h>
#include <utils/concepts.h>

#include <algorithm>
#include <concepts>
#include <list>
#include <map>

namespace arro {
namespace algo {
namespace __dijkstra {
template <UniqueSerializable NodeData, typename LinkData>
	requires Serializable<LinkData> && Weighted<LinkData>
struct NodeEntry {
	const Graph<NodeData, LinkData>::Node* node;
	const Graph<NodeData, LinkData>::Node* parent;
	double cost;
};

template <UniqueSerializable NodeData, typename LinkData>
	requires Serializable<LinkData> && Weighted<LinkData>
bool operator>(const NodeEntry<NodeData, LinkData>& a, const NodeEntry<NodeData, LinkData>& b);
}  // namespace __dijkstra

template <UniqueSerializable NodeData, typename LinkData>
	requires Serializable<LinkData> && Weighted<LinkData>
std::list<const typename Graph<NodeData, LinkData>::Node*> dijkstra(const Graph<NodeData, LinkData>& graph,
																	const typename Graph<NodeData, LinkData>::Node* start,
																	const typename Graph<NodeData, LinkData>::Node* end);
}  // namespace algo
}  // namespace arro

#include "dijkstra.hpp"

#endif