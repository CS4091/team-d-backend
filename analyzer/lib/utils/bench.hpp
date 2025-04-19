#include "bench.h"

template <typename T, Function<T, arro::bench::BenchmarkCtx&> BenchFn>
T arro::bench::benchmark(BenchFn fn) {
	using namespace std;

	BenchmarkCtx ctx;

	auto result = fn(ctx);

	for (auto epoch : ctx._epochs) {
		cout << "[" << epoch.name << "]";
		cout << "\n\tDuration: " << epoch.delta.count();
		cout << "\n\tStart: " << epoch.start.count();
		cout << "\n\tEnd: " << epoch.end.count() << endl;
	}

	return result;
}