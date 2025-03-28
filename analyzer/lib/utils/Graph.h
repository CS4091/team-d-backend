#ifndef GRAPH_H
#define GRAPH_H

#include <fstream>
#include <iomanip>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>
#include <vector>

#include "Node.h"
#include "concepts.h"

namespace arro {
// TODO: add template parameter for digraph property & template specialization (?)
template <UniqueSerializable NodeData, Serializable LinkData>
class Graph {
public:
	struct Link;

	struct Node : public arro::Node<NodeData> {
	public:
		const std::size_t idx;	// NOTE: relies on grow-only nature of graph

		Node(std::size_t idx, const NodeData& data) : arro::Node<NodeData>(data), idx(idx) {}

		const std::vector<Link*>& neighbors() const { return _neighbors; }

		friend class Graph<NodeData, LinkData>;

	private:
		std::vector<Link*> _neighbors;

		~Node() {}
	};

	struct Link {
	public:
		Link(Node* from, Node* to, const LinkData& data) : _data(data), _from(from), _to(to) {}

		const LinkData& data() const { return _data; }

		const Node* from() const { return _from; }
		const Node* to() const { return _to; }

		friend class Graph<NodeData, LinkData>;

	private:
		LinkData _data;
		Node* _from;
		Node* _to;

		~Link() {}
	};

	struct LinkLookup {
		const Node* from;
		const Node* to;
	};

	// these should be sets, but that would be ptr equality (which i dont like)
	Graph(bool digraph = true) : _digraph(digraph) {}
	Graph(const std::vector<Node*>& nodes, bool digraph = false) : _digraph(digraph), _nodes(nodes) {}
	Graph(const std::vector<Node*>& nodes, const std::vector<Link*>& edges, bool digraph = false) : _digraph(digraph), _nodes(nodes), _edges(edges) {}

	Graph(const Graph<NodeData, LinkData>& other);

	const std::vector<Node*>& nodes() const { return _nodes; }
	const std::vector<Link*>& edges() const { return _edges; }

	std::size_t size() const { return _nodes.size(); }
	std::size_t indexOf(Node* node) const;
	std::size_t indexOf(const decltype(NodeData::id)& id) const;

	Node* add(const NodeData& data);

	template <ComparableTo<decltype(NodeData::id)> Search>
	Link* link(const Search& from, const Search& to, const LinkData& data);
	Link* link(Node* from, Node* to, const LinkData& data);

	template <ComparableTo<decltype(NodeData::id)> Search>
	const Node* operator[](const Search& id) const;
	template <ComparableTo<decltype(NodeData::id)> Search>
	Node* operator[](const Search& id);
	const Link* operator[](const LinkLookup& lookup) const;

	template <UniqueSerializable NewNodeData, Function<NewNodeData, NodeData> Mapper>
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
