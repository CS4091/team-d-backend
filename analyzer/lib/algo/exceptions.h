#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <utils/Graph.h>
#include <utils/concepts.h>

#include <stdexcept>
#include <string>

namespace arro {
namespace algo {
template <UniqueSerializable NodeData, typename LinkData>
	requires Serializable<LinkData> && Weighted<LinkData>
class UnreachableException;

template <UniqueSerializable NodeData, typename LinkData>
	requires Serializable<LinkData> && Weighted<LinkData> && (!std::same_as<decltype(NodeData::id), std::string>)
class UnreachableException<NodeData, LinkData> : public std::out_of_range {
public:
	UnreachableException(const Graph<NodeData, LinkData>::Node* from, const arro::Graph<NodeData, LinkData>::Node* to)
		: std::out_of_range("Node '" + std::to_string(to->data().id) + "' is unreachable from node '" + std::to_string(from->data().id) + "'.") {}
};

template <UniqueSerializable NodeData, typename LinkData>
	requires Serializable<LinkData> && Weighted<LinkData> && std::same_as<decltype(NodeData::id), std::string>
class UnreachableException<NodeData, LinkData> : public std::out_of_range {
public:
	UnreachableException(const Graph<NodeData, LinkData>::Node* from, const arro::Graph<NodeData, LinkData>::Node* to)
		: std::out_of_range("Node '" + to->data().id + "' is unreachable from node '" + from->data().id + "'.") {}
};
}  // namespace algo
}  // namespace arro

#endif