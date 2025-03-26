#include "JSONStruct.h"

template <JSONObj T>
T arro::JSONStruct::parse(const std::string& file) {
	std::ifstream in(file);
	nlohmann::json obj = nlohmann::json::parse(in);
	in.close();

	return T(obj);
}

template <JSONObj T>
std::vector<T> arro::JSONStruct::parseArray(const std::string& file) {
	using namespace std;

	ifstream in(file);
	nlohmann::json arr = nlohmann::json::parse(in);
	in.close();

	vector<T> out;

	for (auto elem : arr) out.emplace_back(elem);

	return out;
}