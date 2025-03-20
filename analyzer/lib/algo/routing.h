#ifndef ROUTING_H
#define ROUTING_H

#include <utils/Graph.h>
#include <utils/concepts.h>

#include <algorithm>
#include <iterator>
#include <list>
#include <string>
#include <vector>

namespace arro {
namespace algo {
namespace __routing {
template <UniqueSerializable CNData, typename CLData, UniqueSerializable DNData, Serializable DLData>
	requires Serializable<CLData> && Weighted<CLData>
struct RoutePlan {
	std::list<const typename arro::Graph<CNData, CLData>::Node*> route;
	std::vector<const typename arro::Graph<DNData, DLData>::Link*> remaining;
	double cost;
};

template <UniqueSerializable CNData, typename CLData, UniqueSerializable DNData, Serializable DLData>
	requires Serializable<CLData> && Weighted<CLData>
bool operator>(const RoutePlan<CNData, CLData, DNData, DLData>& a, const RoutePlan<CNData, CLData, DNData, DLData>& b);
}  // namespace __routing

template <UniqueSerializable CNData, typename CLData, UniqueSerializable DNData, Serializable DLData>
	requires Serializable<CLData> && Weighted<CLData>
struct Routing {
	std::list<const typename arro::Graph<CNData, CLData>::Node*> route;
	std::list<const typename arro::Graph<CNData, CLData>::Node*> baseline;
};

template <UniqueSerializable CNData, typename CLData, UniqueSerializable DNData, Serializable DLData>
	requires Serializable<CLData> && Weighted<CLData>
Routing<CNData, CLData, DNData, DLData> findRoute(const arro::Graph<CNData, CLData>& connGraph, const arro::Graph<DNData, DLData>& demandGraph,
												  const std::string& startCity);
}  // namespace algo
}  // namespace arro

#include "routing.hpp"

#endif