#include "Graph.h"


template <arro::UniqueSerializable NodeData, typename LinkData>
arro::GraphNode<NodeData, LinkData>* arro::Graph<NodeData, LinkData>::add(const NodeData& data) {
	_nodes.push_back(new GraphNode(data));
}

template <arro::UniqueSerializable NodeData, typename LinkData>
arro::Link<LinkData, NodeData>* arro::Graph<NodeData, LinkData>::link(const decltype(NodeData::id)& from, const decltype(NodeData::id)& to,
																	  const LinkData& data) {
	GraphNode<NodeData, LinkData>*fromNode, *toNode;

	for (auto node : _nodes) {
		if (node->data().id == from) fromNode = node;
		if (node->data().id == to) toNode = node;
	}

	return link(fromNode, toNode, data);
}

template <arro::UniqueSerializable NodeData, typename LinkData>
arro::Link<LinkData, NodeData>* arro::Graph<NodeData, LinkData>::link(GraphNode<NodeData, LinkData>* from, GraphNode<NodeData, LinkData>* to,
																	  const LinkData& data) {
	if (_digraph) {
		Link<LinkData, NodeData>* link = new Link(from, to, data);

		_edges.push_back(link);
		from->_neighbors.push_back(link);

		return link;
	} else {
		Link<LinkData, NodeData>*link = new Link(from, to, data), *reverse = new Link(to, from, data);

		_edges.push_back(link);
		_edges.push_back(reverse);
		from->_neighbors.push_back(link);
		if (from != to) to->_neighbors.push_back(reverse);	// circular links should not double-count neighbors

		return link;
	}
}

template <arro::UniqueSerializable NodeData, typename LinkData>
arro::Graph<NodeData, LinkData>::~Graph() {
	_clear();
}

template <arro::UniqueSerializable NodeData, typename LinkData>
void arro::Graph<NodeData, LinkData>::_clear() {
	for (auto node : _nodes) delete node;
	for (auto edge : _edges) delete edge;
}
