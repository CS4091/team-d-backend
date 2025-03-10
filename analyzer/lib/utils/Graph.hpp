#include "Graph.h"

template <UniqueSerializable NodeData, Serializable LinkData>
arro::Graph<NodeData, LinkData>::Graph(const Graph<NodeData, LinkData>& other) : _digraph(other._digraph) {
	_nodes.reserve(other._nodes.size());
	_edges.reserve(other._edges.size());

	for (auto node : other._nodes) _nodes.push_back(new Node(_nodes.size(), node->data()));

	for (auto edge : other._edges) {
		Node *from = nullptr, *to = nullptr;

		for (auto node : other._nodes) {
			if (node->data().id == edge->_from->data().id) from = node;
			if (node->data().id == edge->_to->data().id) to = node;
		}

		if (!from || !to) throw std::invalid_argument("Graph link between nonexistent nodes");

		if (_digraph) {
			_edges.push_back(new Link(from, to, edge->data()));
		} else {
			_edges.push_back(new Link(from, to, edge->data()));
			_edges.push_back(new Link(to, from, edge->data()));
		}
	}
}

template <UniqueSerializable NodeData, Serializable LinkData>
std::size_t arro::Graph<NodeData, LinkData>::indexOf(Node* node) const {
	using namespace std;

	for (size_t i = 0; i < _nodes.size(); i++) {
		if (_nodes[i] == node) return i;
	}

	throw out_of_range("Finding index of node not in graph");
}

template <UniqueSerializable NodeData, Serializable LinkData>
std::size_t arro::Graph<NodeData, LinkData>::indexOf(const decltype(NodeData::id)& id) const {
	using namespace std;

	for (size_t i = 0; i < _nodes.size(); i++) {
		if (_nodes[i]->data().id == id) return i;
	}

	throw out_of_range("Finding index of node not in graph");
}

template <UniqueSerializable NodeData, Serializable LinkData>
arro::Graph<NodeData, LinkData>::Node* arro::Graph<NodeData, LinkData>::add(const NodeData& data) {
	Node* node = new Node(_nodes.size(), data);

	_nodes.push_back(node);

	return node;
}

template <UniqueSerializable NodeData, Serializable LinkData>
arro::Graph<NodeData, LinkData>::Link* arro::Graph<NodeData, LinkData>::link(const decltype(NodeData::id)& from, const decltype(NodeData::id)& to,
																			 const LinkData& data) {
	Node *fromNode, *toNode;

	for (auto node : _nodes) {
		if (node->data().id == from) fromNode = node;
		if (node->data().id == to) toNode = node;
	}

	return link(fromNode, toNode, data);
}

template <UniqueSerializable NodeData, Serializable LinkData>
arro::Graph<NodeData, LinkData>::Link* arro::Graph<NodeData, LinkData>::link(Node* from, Node* to, const LinkData& data) {
	if (_digraph) {
		Link* link = new Link(from, to, data);

		_edges.push_back(link);
		from->_neighbors.push_back(link);

		return link;
	} else {
		Link *link = new Link(from, to, data), *reverse = new Link(to, from, data);

		_edges.push_back(link);
		_edges.push_back(reverse);
		from->_neighbors.push_back(link);
		if (from != to) to->_neighbors.push_back(reverse);	// circular links should not double-count neighbors

		return link;
	}
}

template <UniqueSerializable NodeData, Serializable LinkData>
const arro::Graph<NodeData, LinkData>::Node* arro::Graph<NodeData, LinkData>::operator[](const decltype(NodeData::id)& id) const {
	for (auto node : _nodes) {
		if (node->data().id == id) {
			return node;
		}
	}

	return nullptr;
}

template <UniqueSerializable NodeData, Serializable LinkData>
const arro::Graph<NodeData, LinkData>::Link* arro::Graph<NodeData, LinkData>::operator[](const LinkLookup& lookup) const {
	for (auto edge : _edges) {
		if (edge->_from == lookup.from && edge->_to == lookup.to) {
			return edge;
		}
	}

	return nullptr;
}

template <UniqueSerializable NodeData, Serializable LinkData>
template <UniqueSerializable NewNodeData, Function<NewNodeData, NodeData> Mapper>
arro::Graph<NewNodeData, LinkData> arro::Graph<NodeData, LinkData>::map(const Mapper& fn) const {
	using namespace std;

	using NewNode = Graph<NewNodeData, LinkData>::Node;

	vector<NewNode*> nodes;

	for (auto node : _nodes) nodes.push_back(new NewNode(nodes.size(), fn(node->data())));

	Graph<NewNodeData, LinkData> newGraph(nodes, _digraph);

	for (auto edge : _edges) {
		NewNode *from = nullptr, *to = nullptr;
		for (auto node : nodes) {
			if (node->data().id == edge->_from->data().id) from = node;
			if (node->data().id == edge->_to->data().id) to = node;
		}

		if (!from || !to) throw invalid_argument("Graph link between nonexistent nodes");

		newGraph.link(from, to, edge->data());
	}

	return newGraph;
}

template <UniqueSerializable NodeData, Serializable LinkData>
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

		edges.push_back({{"from", fromIdx}, {"to", toIdx}, {"data", LinkData::stringify(edge->data())}});
	}

	out << std::setw(4) << json{{"nodes", nodes}, {"edges", edges}};

	out.close();
}

template <UniqueSerializable NodeData, Serializable LinkData>
arro::Graph<NodeData, LinkData>::~Graph() {
	_clear();
}

template <UniqueSerializable NodeData, Serializable LinkData>
void arro::Graph<NodeData, LinkData>::_clear() {
	for (auto node : _nodes) delete node;
	for (auto edge : _edges) delete edge;
}

template <UniqueSerializable NodeData, Serializable LinkData>
arro::Graph<NodeData, LinkData> arro::Graph<NodeData, LinkData>::readFrom(std::istream& in) {
	using namespace std;

	bool digraph;
	vector<Node*> nodes;
	vector<Link*> edges;

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

			nodes.push_back(new Node(nodes.size(), data));
		} else if (type == "link") {
			decltype(NodeData::id) fromId, toId;
			in >> fromId >> toId;

			LinkData data;
			in >> data;

			Node *from = nullptr, *to = nullptr;
			for (auto node : nodes) {
				if (node->data().id == fromId) from = node;
				if (node->data().id == toId) to = node;
			}

			if (!from || !to) throw invalid_argument("Graph link between nonexistent nodes");

			if (digraph) {
				edges.push_back(new Link(from, to, data));
			} else {
				edges.push_back(new Link(from, to, data));
				edges.push_back(new Link(to, from, data));
			}
		} else {
			throw invalid_argument("Graph entry must be either 'node' or 'link'");
		}
	}

	return Graph(nodes, edges, digraph);
}

template <UniqueSerializable NodeData, Serializable LinkData>
arro::Graph<NodeData, LinkData> arro::Graph<NodeData, LinkData>::readFromFile(const std::string& path) {
	std::ifstream in(path);

	Graph<NodeData, LinkData> g = Graph<NodeData, LinkData>::readFrom(in);

	in.close();

	return g;
}