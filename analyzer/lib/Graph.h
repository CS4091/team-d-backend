#ifndef GRAPH_H
#define GRAPH_H

#include <vector>

namespace arro {
template <typename T>
class Node {
public:
	Node(const T& data) : _data(data) {}

	const T& data() const { return _data; }

private:
	T _data;
};

template <typename T>
class Link {
public:
private:
};

template <typename T>
class Graph {
public:
private:
};
}  // namespace arro

#endif