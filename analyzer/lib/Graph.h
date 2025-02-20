#ifndef GRAPH_H
#define GRAPH_H

#include <fstream>
#include <iomanip>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>
#include <vector>

#include "Node.h"

namespace arro {
template <typename T>
concept IDAble = requires(T elem) {
	elem.id;
	elem.id == elem.id;
};

template <typename T>
concept UniqueSerializable = std::copy_constructible<T> && IDAble<T> && requires(T elem, std::string str) {
	{ T::stringify(elem) } -> std::convertible_to<nlohmann::json>;
	{ T::parse(str) } -> std::convertible_to<T>;
};

template <typename T, typename Arg, typename Ret>
concept __mapperfn = std::copy_constructible<T> && requires(T fn, Arg arg) {
	{ fn(arg) } -> std::convertible_to<Ret>;
};

template <UniqueSerializable NodeData, typename LinkData>
class Graph;

template <typename Data, UniqueSerializable NodeData>
struct Link;

template <UniqueSerializable Data, typename LinkData>
struct GraphNode : public Node<Data> {
public:
	GraphNode(const Data& data) : Node<Data>(data) {}

	friend class Graph<Data, LinkData>;

private:
	std::vector<Link<LinkData, Data>*> _neighbors;

	~GraphNode() {}
};

template <typename Data, UniqueSerializable NodeData>
struct Link {
public:
	Link(GraphNode<NodeData, Data>* from, GraphNode<NodeData, Data>* to, const Data& data) : _data(data), _from(from), _to(to) {}

	const Data& data() const { return _data; }

	friend class Graph<NodeData, Data>;

private:
	Data _data;
	GraphNode<NodeData, Data>* _from;
	GraphNode<NodeData, Data>* _to;

	~Link() {}
};

// TODO: add template parameter for digraph property & template specialization (?)
template <UniqueSerializable NodeData, typename LinkData>
class Graph {
public:
	// these should be sets, but that would be ptr equality (which i dont like)
	Graph(bool digraph = true) : _digraph(digraph) {}
	Graph(const std::vector<GraphNode<NodeData, LinkData>*>& nodes, bool digraph = false) : _digraph(digraph), _nodes(nodes) {}
	Graph(const std::vector<GraphNode<NodeData, LinkData>*>& nodes, const std::vector<Link<LinkData, NodeData>*>& edges, bool digraph = false)
		: _digraph(digraph), _nodes(nodes), _edges(edges) {}

	Graph(const Graph<NodeData, LinkData>& other);

	const std::vector<GraphNode<NodeData, LinkData>*>& nodes() const { return _nodes; }

	GraphNode<NodeData, LinkData>* add(const NodeData& node);

	Link<LinkData, NodeData>* link(const decltype(NodeData::id)& from, const decltype(NodeData::id)& to, const LinkData& data);
	Link<LinkData, NodeData>* link(GraphNode<NodeData, LinkData>* from, GraphNode<NodeData, LinkData>* to, const LinkData& data);

	template <UniqueSerializable NewNodeData, __mapperfn<NodeData, NewNodeData> Mapper>
	Graph<NewNodeData, LinkData> map(const Mapper& fn) const;

	void jsonDumpToFile(const std::string& path) const;

	~Graph();

	static Graph<NodeData, LinkData> readFrom(std::istream& in);
	static Graph<NodeData, LinkData> readFromFile(const std::string& path);

private:
	bool _digraph;
	std::vector<GraphNode<NodeData, LinkData>*> _nodes;
	std::vector<Link<LinkData, NodeData>*> _edges;

	void _clear();
};
}  // namespace arro

#endif