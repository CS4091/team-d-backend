#ifndef SYNCIO_H
#define SYNCIO_H

#include <iostream>
#include <mutex>

namespace arro {
namespace syncio {
class SynchronizedOStream {
public:
	SynchronizedOStream() = delete;
	SynchronizedOStream(std::ostream& stream) : _stream(stream) {}

	template <typename T>
	friend SynchronizedOStream& operator<<(SynchronizedOStream& stream, T item);

private:
	std::mutex _mutex;
	std::ostream& _stream;
};

extern SynchronizedOStream scout;
}  // namespace syncio
}  // namespace arro

#endif