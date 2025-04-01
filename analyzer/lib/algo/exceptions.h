#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <utils/Graph.h>
#include <utils/concepts.h>

#include <stdexcept>
#include <string>

namespace arro {
namespace algo {
namespace data {
struct RouteReq;

}  // namespace data

template <UniqueSerializable NodeData, Serializable LinkData>
class UnreachableException;

template <UniqueSerializable NodeData, Serializable LinkData>
	requires(!std::convertible_to<decltype(NodeData::id), std::string>)
class UnreachableException<NodeData, LinkData> : public std::out_of_range {
public:
	UnreachableException(const Graph<NodeData, LinkData>::Node* from, const arro::Graph<NodeData, LinkData>::Node* to)
		: std::out_of_range("Node '" + std::to_string(to->data().id) + "' is unreachable from node '" + std::to_string(from->data().id) + "'.") {}
};

template <UniqueSerializable NodeData, Serializable LinkData>
	requires std::convertible_to<decltype(NodeData::id), std::string>
class UnreachableException<NodeData, LinkData> : public std::out_of_range {
public:
	UnreachableException(const Graph<NodeData, LinkData>::Node* from, const arro::Graph<NodeData, LinkData>::Node* to)
		: std::out_of_range("Node '" + std::string(to->data().id) + "' is unreachable from node '" + std::string(from->data().id) + "'.") {}
};

class UnroutableException : public std::out_of_range {
public:
	UnroutableException(const std::vector<data::RouteReq>& routes);

	const std::vector<data::RouteReq> routes;
};
}  // namespace algo
}  // namespace arro

#endif