#include <algo/fw.h>
#include <algo/routing.h>
#include <utils/Graph.h>

#include <iostream>
#include <string>

using namespace std;

int main(int argv, char** argc) {
	using CNData = arro::algo::data::AirportLatLng;
	using CLData = arro::algo::data::ReducedAirwayData;
	using ConnGraph = arro::Graph<CNData, CLData>;

	if (argv < 2) {
		cerr << "Missing map file path" << endl;
		return 1;
	}

	string path = argc[1];

	arro::bench::benchmark([path](arro::bench::BenchmarkCtx& ctx) {
		ctx.start("Read");

		ConnGraph graph = ConnGraph::readFromBinFile(path, [](int fd, size_t maxStrLen, char* buf) {
			arro::algo::data::AirportLatLng airport;

			memset(buf, '\0', maxStrLen + 1);
			read(fd, buf, maxStrLen);
			airport.id = string(buf);

			memset(buf, '\0', maxStrLen + 1);
			read(fd, buf, maxStrLen);
			airport.city = string(buf);

			read(fd, &airport.lat, sizeof(double));
			read(fd, &airport.lng, sizeof(double));
			read(fd, &airport.fuel, sizeof(double));

			return airport;
		});

		ctx.stop();
		ctx.start("Floyd Warshall");

		arro::algo::floydWarshall(graph, [](const arro::algo::data::ReducedAirwayData& airway) { return airway.cost(0); });

		ctx.stop();

		cout << graph.size() << endl;
	});
}