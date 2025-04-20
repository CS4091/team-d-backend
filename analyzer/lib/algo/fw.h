#ifndef FW_H
#define FW_H

#include <fcntl.h>
#include <utils/Graph.h>
#include <utils/concepts.h>

#include <string>
#include <vector>

namespace arro {
namespace algo {
template <UniqueSerializable NodeData, Serializable LinkData, Function<double, LinkData> WeightOp>
std::vector<std::vector<double>> floydWarshall(const Graph<NodeData, LinkData>& graph, WeightOp weight);

class BinFWTable {
public:
	BinFWTable() = delete;
	BinFWTable(double* buf, std::size_t sz) : _sz(sz), _buf(buf) {}
	BinFWTable(BinFWTable&& other);

	// Use operator() since syntax highlighting for operator[] with multiple arguments doesn't work,
	// and workarounds incur runtime costs
	const double& operator()(std::size_t i, std::size_t j) const;

	~BinFWTable();

	static void binDumpToFile(const std::string& path, const std::vector<std::vector<double>>& table);

	static BinFWTable readFromBinFile(const std::string& path);

private:
	std::size_t _sz;
	double* _buf;
};
}  // namespace algo
}  // namespace arro

#include "fw.hpp"

#endif