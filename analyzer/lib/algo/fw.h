#ifndef FW_H
#define FW_H

#include <utils/Graph.h>
#include <utils/concepts.h>

#include <vector>

namespace arro {
namespace algo {
template <UniqueSerializable NodeData, Serializable LinkData, Function<double, LinkData> WeightOp>
std::vector<std::vector<double>> floydWarshall(const Graph<NodeData, LinkData>& graph, WeightOp weight);
}  // namespace algo
}  // namespace arro

#include "fw.hpp"

#endif