#ifndef NODE_H
#define NODE_H

namespace arro {
template <typename Data>
struct Node {
public:
	Node(const Data& data) : _data(data) {}

	const Data& data() const { return _data; }

private:
	Data _data;
};
}  // namespace arro

#endif