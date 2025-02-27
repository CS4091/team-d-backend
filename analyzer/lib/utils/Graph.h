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

// TODO: add template parameter for digraph property & template specialization (?)
template <UniqueSerializable NodeData, typename LinkData>
class Graph {
public:
	struct Link;

	struct Node : public arro::Node<NodeData> {
	public:
		Node(const NodeData& data) : arro::Node<NodeData>(data) {}

		friend class Graph<NodeData, LinkData>;

	private:
		std::vector<Link*> _neighbors;

		~Node() {}
	};

	struct Link {
	public:
		Link(Node* from, Node* to, const LinkData& data) : _data(data), _from(from), _to(to) {}

		const LinkData& data() const { return _data; }

		friend class Graph<NodeData, LinkData>;

	private:
		LinkData _data;
		Node* _from;
		Node* _to;

		~Link() {}
	};

	// these should be sets, but that would be ptr equality (which i dont like)
	Graph(bool digraph = true) : _digraph(digraph) {}
	Graph(const std::vector<Node*>& nodes, bool digraph = false) : _digraph(digraph), _nodes(nodes) {}
	Graph(const std::vector<Node*>& nodes, const std::vector<Link*>& edges, bool digraph = false) : _digraph(digraph), _nodes(nodes), _edges(edges) {}

	Graph(const Graph<NodeData, LinkData>& other);

	const std::vector<Node*>& nodes() const { return _nodes; }

	Node* add(const NodeData& data);

	Link* link(const decltype(NodeData::id)& from, const decltype(NodeData::id)& to, const LinkData& data);
	Link* link(Node* from, Node* to, const LinkData& data);

	template <UniqueSerializable NewNodeData, __mapperfn<NodeData, NewNodeData> Mapper>
	Graph<NewNodeData, LinkData> map(const Mapper& fn) const;

	// [devalue](https://github.com/Rich-Harris/devalue)-inspired graph-to-json serialization
	void jsonDumpToFile(const std::string& path) const;

	~Graph();

	static Graph<NodeData, LinkData> readFrom(std::istream& in);
	static Graph<NodeData, LinkData> readFromFile(const std::string& path);

private:
	bool _digraph;
	std::vector<Node*> _nodes;
	std::vector<Link*> _edges;

	void _clear();
};
}  // namespace arro

#include "Graph.hpp"

#endif
