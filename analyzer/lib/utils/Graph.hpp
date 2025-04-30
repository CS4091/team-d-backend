#include "Graph.h"

template <UniqueSerializable NodeData, Serializable LinkData>
arro::Graph<NodeData, LinkData>::Graph(const Graph<NodeData, LinkData>& other) : _digraph(other._digraph) {
	_nodes.reserve(other._nodes.size());
	_edges.reserve(other._edges.size());

	for (const Node* node : other._nodes) _nodes.push_back(new Node(_nodes.size(), node->data()));

	for (const Link* edge : other._edges) {
		link(_nodes[edge->from()->idx], _nodes[edge->to()->idx], edge->data());
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
template <ComparableTo<decltype(NodeData::id)> Search>
arro::Graph<NodeData, LinkData>::Link* arro::Graph<NodeData, LinkData>::link(const Search& from, const Search& to, const LinkData& data) {
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
template <ComparableTo<decltype(NodeData::id)> Search>
const arro::Graph<NodeData, LinkData>::Node* arro::Graph<NodeData, LinkData>::operator[](const Search& id) const {
	for (auto node : _nodes) {
		if (node->data().id == id) {
			return node;
		}
	}

	return nullptr;
}

template <UniqueSerializable NodeData, Serializable LinkData>
template <ComparableTo<decltype(NodeData::id)> Search>
arro::Graph<NodeData, LinkData>::Node* arro::Graph<NodeData, LinkData>::operator[](const Search& id) {
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
		edges.push_back({{"from", edge->_from->idx}, {"to", edge->_to->idx}, {"data", LinkData::stringify(edge->data())}});
	}

	out << std::setw(4) << json{{"nodes", nodes}, {"edges", edges}};

	out.close();
}

template <UniqueSerializable NodeData, Serializable LinkData>
template <Routine<int, NodeData> NodeSerializer, Routine<int, LinkData> EdgeSerializer>
void arro::Graph<NodeData, LinkData>::binDumpToFile(const std::string& path, std::size_t sfieldLen, const NodeSerializer& dumpNode,
													const EdgeSerializer& dumpEdge) const {
	using namespace std;

	int fd = open(path.c_str(), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

	if (fd == -1) throw invalid_argument("Unable to acquire graph file '" + path + "'");
	size_t sz = _nodes.size();

	write(fd, "\x09\x0B", 2);
	write(fd, &_digraph, 1);
	write(fd, &sfieldLen, sizeof(size_t));
	write(fd, &sz, sizeof(size_t));
	sz = _edges.size();
	write(fd, &sz, sizeof(size_t));
	for (auto edge : _edges) {
		size_t endpoints[2] = {edge->_from->idx, edge->_to->idx};
		write(fd, endpoints, sizeof(size_t) * 2);
	}

	for (auto node : _nodes) dumpNode(fd, node->data());
	for (auto edge : _edges) dumpEdge(fd, edge->data());

	close(fd);
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

template <UniqueSerializable NodeData, Serializable LinkData>
template <Function<NodeData, int, std::size_t, char*> NodeSerializer, Function<LinkData, int, std::size_t, char*> EdgeSerializer>
arro::Graph<NodeData, LinkData> arro::Graph<NodeData, LinkData>::readFromBinFile(const std::string& path, const NodeSerializer& readNode,
																				 const EdgeSerializer& readEdge) {
	using namespace std;

	using OutGraph = Graph<NodeData, LinkData>;
	using OutNode = OutGraph::Node;
	using OutLink = OutGraph::Link;

#ifdef BENCHMARK
	return bench::benchmark<OutGraph>([path, readNode, readEdge](bench::BenchmarkCtx& ctx) {
#endif
		int fd = open(path.c_str(), O_RDONLY);

		if (fd == -1) throw invalid_argument("Unable to acquire graph file '" + path + "'");

#ifdef BENCHMARK
		ctx.start("Read Headers");
#endif

		unsigned char buf[2];
		int bytesRead = read(fd, buf, 2);

		if (bytesRead != 2 || buf[0] != 0x09 || buf[1] != 0x0B) throw invalid_argument("Failed to read magic bytes");

		bytesRead = read(fd, buf, 1);
		if (bytesRead != 1) throw invalid_argument("Failed to read digraph bit");

		bool digraph = buf[0];

		size_t sfieldLen;
		bytesRead = read(fd, &sfieldLen, sizeof(size_t));
		if (bytesRead != sizeof(size_t)) throw invalid_argument("Failed to read max str len");
		char* strBuf = new char[sfieldLen + 1];

		size_t numNodes;
		bytesRead = read(fd, &numNodes, sizeof(size_t));
		if (bytesRead != sizeof(size_t)) throw invalid_argument("Failed to read number of nodes");

		size_t numEdges;
		bytesRead = read(fd, &numEdges, sizeof(size_t));
		if (bytesRead != sizeof(size_t)) throw invalid_argument("Failed to read number of edges");

#ifdef BENCHMARK
		ctx.stop();
		ctx.start("Read Connectivity");
#endif

		size_t* connectivity = new size_t[numEdges * 2];
		bytesRead = read(fd, connectivity, sizeof(size_t) * numEdges * 2);
		if (bytesRead != sizeof(size_t) * numEdges * 2) throw invalid_argument("Failed to read edge endpoints list");

#ifdef BENCHMARK
		ctx.stop();
		ctx.start("Read Node Data");
#endif

		vector<OutNode*> nodes;
		nodes.reserve(numNodes);
		for (size_t i = 0; i < numNodes; i++) nodes.push_back(new OutNode(i, readNode(fd, sfieldLen, strBuf)));

#ifdef BENCHMARK
		ctx.stop();
		ctx.start("Read Edge Data");
#endif

		vector<OutLink*> edges;
		edges.reserve(numEdges);
		for (size_t i = 0; i < numEdges; i++)
			edges.push_back(new OutLink(nodes[connectivity[2 * i]], nodes[connectivity[2 * i + 1]], readEdge(fd, sfieldLen, strBuf)));

#ifdef BENCHMARK
		ctx.stop();
#endif

		close(fd);
		delete[] connectivity;
		delete[] strBuf;

		return OutGraph(nodes, edges, digraph);
#ifdef BENCHMARK
	});
#endif
}

template <UniqueSerializable NodeData, Serializable LinkData>
template <Function<NodeData, int, std::size_t, char*> NodeSerializer>
arro::Graph<NodeData, LinkData> arro::Graph<NodeData, LinkData>::readFromBinFile(const std::string& path, const NodeSerializer& readNode) {
	using namespace std;

	using OutGraph = Graph<NodeData, LinkData>;
	using OutNode = OutGraph::Node;
	using OutLink = OutGraph::Link;

#ifdef BENCHMARK
	return bench::benchmark<OutGraph>([path, readNode](bench::BenchmarkCtx& ctx) {
#endif
		int fd = open(path.c_str(), O_RDONLY);

		if (fd == -1) throw invalid_argument("Unable to acquire graph file '" + path + "'");

#ifdef BENCHMARK
		ctx.start("Read Headers");
#endif

		unsigned char buf[2];
		int bytesRead = read(fd, buf, 2);

		if (bytesRead != 2 || buf[0] != 0x09 || buf[1] != 0x0B) throw invalid_argument("Failed to read magic bytes");

		bytesRead = read(fd, buf, 1);
		if (bytesRead != 1) throw invalid_argument("Failed to read digraph bit");

		bool digraph = buf[0];

		size_t sfieldLen;
		bytesRead = read(fd, &sfieldLen, sizeof(size_t));
		if (bytesRead != sizeof(size_t)) throw invalid_argument("Failed to read max str len");
		char* strBuf = new char[sfieldLen + 1];

		size_t numNodes;
		bytesRead = read(fd, &numNodes, sizeof(size_t));
		if (bytesRead != sizeof(size_t)) throw invalid_argument("Failed to read number of nodes");

		size_t numEdges;
		bytesRead = read(fd, &numEdges, sizeof(size_t));
		if (bytesRead != sizeof(size_t)) throw invalid_argument("Failed to read number of edges");

#ifdef BENCHMARK
		ctx.stop();
		ctx.start("Read Connectivity");
#endif

		size_t* connectivity = new size_t[numEdges * 2];
		bytesRead = read(fd, connectivity, sizeof(size_t) * numEdges * 2);
		if (bytesRead != sizeof(size_t) * numEdges * 2) throw invalid_argument("Failed to read edge endpoints list");

#ifdef BENCHMARK
		ctx.stop();
		ctx.start("Read Node Data");
#endif

		vector<OutNode*> nodes;
		nodes.reserve(numNodes);
		for (size_t i = 0; i < numNodes; i++) nodes.push_back(new OutNode(i, readNode(fd, sfieldLen, strBuf)));

#ifdef BENCHMARK
		ctx.stop();
		ctx.start("Read Edge Data");
#endif

		LinkData* linkData = reinterpret_cast<LinkData*>(malloc(sizeof(LinkData) * numEdges));
		bytesRead = read(fd, linkData, sizeof(LinkData) * numEdges);
		if (bytesRead != sizeof(LinkData) * numEdges) throw invalid_argument("Failed to read edge data list");

		vector<OutLink*> edges;
		edges.reserve(numEdges);
		for (size_t i = 0; i < numEdges; i++) edges.push_back(new OutLink(nodes[connectivity[2 * i]], nodes[connectivity[2 * i + 1]], linkData[i]));

#ifdef BENCHMARK
		ctx.stop();
#endif

		close(fd);
		delete[] connectivity;
		delete[] strBuf;
		free(linkData);

		return OutGraph(nodes, edges, digraph);
#ifdef BENCHMARK
	});
#endif
}

template <UniqueSerializable NodeData, Serializable LinkData>
template <Function<LinkData, int, std::size_t, char*> EdgeSerializer>
arro::Graph<NodeData, LinkData> arro::Graph<NodeData, LinkData>::readFromBinFile(const std::string& path, const EdgeSerializer& readEdge) {
	using namespace std;

	using OutGraph = Graph<NodeData, LinkData>;
	using OutNode = OutGraph::Node;
	using OutLink = OutGraph::Link;

#ifdef BENCHMARK
	return bench::benchmark<OutGraph>([path, readEdge](bench::BenchmarkCtx& ctx) {
#endif
		int fd = open(path.c_str(), O_RDONLY);

		if (fd == -1) throw invalid_argument("Unable to acquire graph file '" + path + "'");

#ifdef BENCHMARK
		ctx.start("Read Headers");
#endif

		unsigned char buf[2];
		int bytesRead = read(fd, buf, 2);

		if (bytesRead != 2 || buf[0] != 0x09 || buf[1] != 0x0B) throw invalid_argument("Failed to read magic bytes");

		bytesRead = read(fd, buf, 1);
		if (bytesRead != 1) throw invalid_argument("Failed to read digraph bit");

		bool digraph = buf[0];

		size_t sfieldLen;
		bytesRead = read(fd, &sfieldLen, sizeof(size_t));
		if (bytesRead != sizeof(size_t)) throw invalid_argument("Failed to read max str len");
		char* strBuf = new char[sfieldLen + 1];

		size_t numNodes;
		bytesRead = read(fd, &numNodes, sizeof(size_t));
		if (bytesRead != sizeof(size_t)) throw invalid_argument("Failed to read number of nodes");

		size_t numEdges;
		bytesRead = read(fd, &numEdges, sizeof(size_t));
		if (bytesRead != sizeof(size_t)) throw invalid_argument("Failed to read number of edges");

#ifdef BENCHMARK
		ctx.stop();
		ctx.start("Read Connectivity");
#endif

		size_t* connectivity = new size_t[numEdges * 2];
		bytesRead = read(fd, connectivity, sizeof(size_t) * numEdges * 2);
		if (bytesRead != sizeof(size_t) * numEdges * 2) throw invalid_argument("Failed to read edge endpoints list");

#ifdef BENCHMARK
		ctx.stop();
		ctx.start("Read Node Data");
#endif

		NodeData* nodeData = reinterpret_cast<NodeData*>(malloc(sizeof(NodeData) * numNodes));
		bytesRead = read(fd, nodeData, sizeof(NodeData) * numNodes);
		if (bytesRead != sizeof(NodeData) * numNodes) throw invalid_argument("Failed to read edge data list");

		vector<OutNode*> nodes;
		nodes.reserve(numNodes);
		for (size_t i = 0; i < numNodes; i++) nodes.push_back(new OutNode(i, nodeData[i]));

#ifdef BENCHMARK
		ctx.stop();
		ctx.start("Read Edge Data");
#endif

		vector<OutLink*> edges;
		edges.reserve(numEdges);
		for (size_t i = 0; i < numEdges; i++)
			edges.push_back(new OutLink(nodes[connectivity[2 * i]], nodes[connectivity[2 * i + 1]], readEdge(fd, sfieldLen, strBuf)));

#ifdef BENCHMARK
		ctx.stop();
#endif

		close(fd);
		delete[] connectivity;
		delete[] strBuf;
		free(nodeData);

		return OutGraph(nodes, edges, digraph);
#ifdef BENCHMARK
	});
#endif
}

template <UniqueSerializable NodeData, Serializable LinkData>
arro::Graph<NodeData, LinkData> arro::Graph<NodeData, LinkData>::readFromBinFile(const std::string& path) {
	using namespace std;

	using OutGraph = Graph<NodeData, LinkData>;
	using OutNode = OutGraph::Node;
	using OutLink = OutGraph::Link;

#ifdef BENCHMARK
	return bench::benchmark<OutGraph>([path](bench::BenchmarkCtx& ctx) {
#endif
		int fd = open(path.c_str(), O_RDONLY);

		if (fd == -1) throw invalid_argument("Unable to acquire graph file '" + path + "'");

#ifdef BENCHMARK
		ctx.start("Read Headers");
#endif

		unsigned char buf[2];
		int bytesRead = read(fd, buf, 2);

		if (bytesRead != 2 || buf[0] != 0x09 || buf[1] != 0x0B) throw invalid_argument("Failed to read magic bytes");

		bytesRead = read(fd, buf, 1);
		if (bytesRead != 1) throw invalid_argument("Failed to read digraph bit");

		bool digraph = buf[0];

		size_t sfieldLen;
		bytesRead = read(fd, &sfieldLen, sizeof(size_t));
		if (bytesRead != sizeof(size_t)) throw invalid_argument("Failed to read max str len");
		char* strBuf = new char[sfieldLen + 1];

		size_t numNodes;
		bytesRead = read(fd, &numNodes, sizeof(size_t));
		if (bytesRead != sizeof(size_t)) throw invalid_argument("Failed to read number of nodes");

		size_t numEdges;
		bytesRead = read(fd, &numEdges, sizeof(size_t));
		if (bytesRead != sizeof(size_t)) throw invalid_argument("Failed to read number of edges");

#ifdef BENCHMARK
		ctx.stop();
		ctx.start("Read Connectivity");
#endif

		size_t* connectivity = new size_t[numEdges * 2];
		bytesRead = read(fd, connectivity, sizeof(size_t) * numEdges * 2);
		if (bytesRead != sizeof(size_t) * numEdges * 2) throw invalid_argument("Failed to read edge endpoints list");

#ifdef BENCHMARK
		ctx.stop();
		ctx.start("Read Node Data");
#endif

		NodeData* nodeData = reinterpret_cast<NodeData*>(malloc(sizeof(NodeData) * numNodes));
		bytesRead = read(fd, nodeData, sizeof(NodeData) * numNodes);
		if (bytesRead != sizeof(NodeData) * numNodes) throw invalid_argument("Failed to read edge data list");

		vector<OutNode*> nodes;
		nodes.reserve(numNodes);
		for (size_t i = 0; i < numNodes; i++) nodes.push_back(new OutNode(i, nodeData[i]));

#ifdef BENCHMARK
		ctx.stop();
		ctx.start("Read Edge Data");
#endif

		LinkData* linkData = reinterpret_cast<LinkData*>(malloc(sizeof(LinkData) * numEdges));
		bytesRead = read(fd, linkData, sizeof(LinkData) * numEdges);
		if (bytesRead != sizeof(LinkData) * numEdges) throw invalid_argument("Failed to read edge data list");

		vector<OutLink*> edges;
		edges.reserve(numEdges);
		for (size_t i = 0; i < numEdges; i++) edges.push_back(new OutLink(nodes[connectivity[2 * i]], nodes[connectivity[2 * i + 1]], linkData[i]));

#ifdef BENCHMARK
		ctx.stop();
#endif

		close(fd);
		delete[] connectivity;
		delete[] strBuf;
		free(nodeData);
		free(linkData);

		return OutGraph(nodes, edges, digraph);
#ifdef BENCHMARK
	});
#endif
}