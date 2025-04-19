#include "bench.h"

using namespace std;
using namespace chrono_literals;

void arro::bench::BenchmarkCtx::start(const string& name) {
	auto now = chrono::high_resolution_clock::now();

	if (_epochs.size() > 0 && _epochs.back().end.count() == -1) {
		auto& lastEpoch = _epochs.back();

		lastEpoch.end = chrono::duration_cast<chrono::milliseconds>(now - _start);
		lastEpoch.delta = lastEpoch.end - lastEpoch.start;
	}

	_epochs.emplace_back(name, chrono::duration_cast<chrono::milliseconds>(now - _start), -1ms, -1ms);
}

void arro::bench::BenchmarkCtx::stop() {
	auto now = chrono::high_resolution_clock::now();

	auto& lastEpoch = _epochs.back();

	lastEpoch.end = chrono::duration_cast<chrono::milliseconds>(now - _start);
	lastEpoch.delta = lastEpoch.end - lastEpoch.start;
}