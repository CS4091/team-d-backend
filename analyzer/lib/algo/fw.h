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
	class Row {
	public:
		Row() = delete;
		Row(const BinFWTable* table, std::size_t idx) : _idx(idx), _table(table) {}
		Row(const Row& other) : _idx(other._idx), _table(other._table) {}

		const double& operator[](std::size_t idx) const;

	private:
		std::size_t _idx;
		const BinFWTable* _table;
	};

	BinFWTable() = delete;
	BinFWTable(double* buf, std::size_t sz) : _sz(sz), _buf(buf) {}
	BinFWTable(BinFWTable&& other);

	Row operator[](std::size_t idx) const;

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