#ifndef BENCH_H
#define BENCH_H

#include <chrono>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "concepts.h"

namespace arro {
namespace bench {
class BenchmarkCtx {
public:
	struct Epoch {
		std::string name;
		std::chrono::milliseconds start;
		std::chrono::milliseconds end;
		std::chrono::milliseconds delta;
	};

	BenchmarkCtx() : _start(std::chrono::high_resolution_clock::now()) {}

	void start(const std::string& name);
	void stop();

	template <Routine<BenchmarkCtx&> BenchFn>
	friend void benchmark(BenchFn fn);

	template <typename T, Function<T, BenchmarkCtx&> BenchFn>
	friend T benchmark(BenchFn fn);

private:
	std::chrono::high_resolution_clock::time_point _start;
	std::vector<Epoch> _epochs;
};

template <Routine<BenchmarkCtx&> BenchFn>
void benchmark(BenchFn fn);

template <typename T, Function<T, BenchmarkCtx&> BenchFn>
T benchmark(BenchFn fn);
}  // namespace bench
}  // namespace arro

#include "bench.hpp"

#endif