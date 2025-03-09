#ifndef FW_H
#define FW_H

#include <utils/Graph.h>
#include <utils/concepts.h>

#include <vector>

namespace arro {
namespace algo {
template <UniqueSerializable NodeData, typename LinkData>
	requires Serializable<LinkData> && Weighted<LinkData>
std::vector<std::vector<double>> floydWarshall(const Graph<NodeData, LinkData>& graph);
}  // namespace algo
}  // namespace arro

#include "fw.hpp"

#endif