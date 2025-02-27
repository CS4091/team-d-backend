#include "Graph.h"

template <arro::UniqueSerializable NodeData, typename LinkData>
arro::Graph<NodeData, LinkData>::Graph(const Graph<NodeData, LinkData>& other) : _digraph(other._digraph) {
	_nodes.reserve(other._nodes.size());
	_edges.reserve(other._edges.size());

	for (auto node : other._nodes) _nodes.push_back(new GraphNode<NodeData, LinkData>(node->data()));

	for (auto edge : other._edges) {
		GraphNode<NodeData, LinkData>*from = nullptr, *to = nullptr;

		for (auto node : other._nodes) {
			if (node->data().id == edge->_from->data().id) from = node;
			if (node->data().id == edge->_to->data().id) to = node;
		}

		if (!from || !to) throw std::invalid_argument("Graph link between nonexistent nodes");

		if (_digraph) {
			_edges.push_back(new Link<LinkData, NodeData>(from, to, edge->data()));
		} else {
			_edges.push_back(new Link<LinkData, NodeData>(from, to, edge->data()));
			_edges.push_back(new Link<LinkData, NodeData>(to, from, edge->data()));
		}
	}
}

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
template <arro::UniqueSerializable NewNodeData, arro::__mapperfn<NodeData, NewNodeData> Mapper>
arro::Graph<NewNodeData, LinkData> arro::Graph<NodeData, LinkData>::map(const Mapper& fn) const {
	using namespace std;

	vector<GraphNode<NewNodeData, LinkData>*> nodes;
	vector<Link<LinkData, NewNodeData>*> edges;

	for (auto node : _nodes) nodes.push_back(new GraphNode<NewNodeData, LinkData>(fn(node->data())));

	for (auto edge : _edges) {
		GraphNode<NewNodeData, LinkData>*from = nullptr, *to = nullptr;
		for (auto node : nodes) {
			if (node->data().id == edge->_from->data().id) from = node;
			if (node->data().id == edge->_to->data().id) to = node;
		}

		if (!from || !to) throw std::invalid_argument("Graph link between nonexistent nodes");

		if (_digraph) {
			edges.push_back(new Link<LinkData, NewNodeData>(from, to, edge->data()));
		} else {
			edges.push_back(new Link<LinkData, NewNodeData>(from, to, edge->data()));
			edges.push_back(new Link<LinkData, NewNodeData>(to, from, edge->data()));
		}
	}

	return Graph<NewNodeData, LinkData>(nodes, edges, _digraph);
}

template <arro::UniqueSerializable NodeData, typename LinkData>
void arro::Graph<NodeData, LinkData>::jsonDumpToFile(const std::string& path) const {
	using namespace std;
	using json = nlohmann::json;

	ofstream out(path);

	// [devalue](https://github.com/Rich-Harris/devalue)-inspired graph-to-json serialization
	vector<json> nodes, edges;
	nodes.reserve(_nodes.size());
	edges.reserve(_edges.size());

	for (auto node : _nodes) nodes.push_back(NodeData::stringify(node->data()));

	for (auto edge : _edges) {
		size_t fromIdx = (size_t)-1, toIdx = (size_t)-1;

		for (size_t i = 0; i < _nodes.size(); i++) {
			if (_nodes[i]->data().id == edge->_from->data().id) fromIdx = i;
			if (_nodes[i]->data().id == edge->_to->data().id) toIdx = i;
		}

		if (fromIdx == (size_t)-1 || toIdx == (size_t)-1) throw domain_error("Graph state inconsistent");

		edges.push_back({{"from", fromIdx}, {"to", toIdx}, {"data", edge->data()}});
	}

	out << std::setw(4) << json{{"nodes", nodes}, {"edges", edges}};

	out.close();
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

template <arro::UniqueSerializable NodeData, typename LinkData>
arro::Graph<NodeData, LinkData> arro::Graph<NodeData, LinkData>::readFrom(std::istream& in) {
	using namespace std;

	bool digraph;
	vector<GraphNode<NodeData, LinkData>*> nodes;
	vector<Link<LinkData, NodeData>*> edges;

	string type;
	in >> type;

	if (type == "digraph") {
		digraph = true;
	} else if (type == "graph") {
		digraph = false;
	} else {
		throw invalid_argument("Graph type must be either 'digraph' or 'graph'");
	}

	while (in >> type) {
		if (type == "node") {
			NodeData data;
			in >> data;

			nodes.push_back(new GraphNode<NodeData, LinkData>(data));
		} else if (type == "link") {
			decltype(NodeData::id) fromId, toId;
			in >> fromId >> toId;

			LinkData data;
			in >> data;

			GraphNode<NodeData, LinkData>*from = nullptr, *to = nullptr;
			for (auto node : nodes) {
				if (node->data().id == fromId) from = node;
				if (node->data().id == toId) to = node;
			}

			if (!from || !to) throw invalid_argument("Graph link between nonexistent nodes");

			if (digraph) {
				edges.push_back(new Link<LinkData, NodeData>(from, to, data));
			} else {
				edges.push_back(new Link<LinkData, NodeData>(from, to, data));
				edges.push_back(new Link<LinkData, NodeData>(to, from, data));
			}
		} else {
			throw invalid_argument("Graph entry must be either 'node' or 'link'");
		}
	}

	return Graph(nodes, edges, digraph);
}

template <arro::UniqueSerializable NodeData, typename LinkData>
arro::Graph<NodeData, LinkData> arro::Graph<NodeData, LinkData>::readFromFile(const std::string& path) {
	std::ifstream in(path);

	Graph<NodeData, LinkData> g = Graph<NodeData, LinkData>::readFrom(in);

	in.close();

	return g;
}