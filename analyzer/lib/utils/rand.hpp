#include "rand.h"

template <typename T>
T arro::rand::rand() {
	int fd = open("/dev/random", O_RDONLY);

	for (int i = 0; i < 10 && fd == -1; i++) {
		fd = open("/dev/random", O_RDONLY);
	}

	if (fd == -1) {
		throw std::runtime_error("Failed to acquire RNG source, errno: " + std::to_string(errno));
	}

	T out;

	int bytesRead = read(fd, &out, sizeof(T));

	close(fd);

	if (bytesRead == -1) {
		throw std::runtime_error("Failed to read bytes, errno: " + std::to_string(errno));
	}

	return out;
}

template <Comparable T>
T arro::rand::rand(T max) {
	T out = rand<T>();

	while (!(out < max)) out = rand<T>();

	return out;
}

template <typename T>
	requires Comparable<T> && std::integral<T>
T arro::rand::rand(T max) {
	int exp = ceil(log2(max));

	T out = rand<T>() % exp;

	while (!(out < max)) out = rand<T>() % exp;

	return out;
}

template <typename E, typename A>
	requires SizedRandomAccess<A, E>
E arro::rand::choice(const A& arr) {
	std::size_t idx = rand(arr.size());

	return arr[idx];
}