#include "syncio.h"

template <typename T>
arro::syncio::SynchronizedOStream& arro::syncio::operator<<(arro::syncio::SynchronizedOStream& stream, T item) {
	std::lock_guard lock(stream._mutex);

	stream._stream << item;

	return stream;
}