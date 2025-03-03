#ifndef RAND_H
#define RAND_H

#include <fcntl.h>
#include <unistd.h>

#include <stdexcept>

#include "concepts.h"

namespace arro {
namespace rand {
// Fills a T with random bytes and returns it
template <typename T>
T rand();

// enforce max via rejection sampling
template <Comparable T>
T rand(T max);

// specialization for integers
template <typename T>
	requires Comparable<T> && std::integral<T>
T rand(T max);

template <typename E, typename A>
	requires SizedRandomAccess<A, E>
E choice(const A& arr);
}  // namespace rand
}  // namespace arro

#include "rand.hpp"

#endif