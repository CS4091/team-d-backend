#include "fw.h"

template <arro::UniqueSerializable NodeData, typename LinkData>
	requires arro::Serializable<LinkData> && Weighted<LinkData>
std::vector<std::vector<double>> arro::algo::floydWarshall(const arro::Graph<NodeData, LinkData>& graph) {
	using namespace std;
	using namespace arro;

	vector<vector<double>> out;
	out.resize(graph.size());
	for (size_t i = 0; i < graph.size(); i++) {
		out[i].resize(graph.size());
		for (size_t j = 0; j < graph.size(); j++) out[i][j] = INFINITY;
		out[i][i] = 0;
	}

	for (auto edge : graph.edges()) {
		size_t from = edge->from()->idx, to = edge->to()->idx;

		out[from][to] = edge->data().cost();
	}

	for (size_t k = 0; k < graph.size(); k++) {
		for (size_t i = 0; i < graph.size(); i++) {
			for (size_t j = 0; j < graph.size(); j++) {
				if (out[i][j] > out[i][k] + out[k][j]) {
					out[i][j] = out[i][k] + out[k][j];
				}
			}
		}
	}

	return out;
}