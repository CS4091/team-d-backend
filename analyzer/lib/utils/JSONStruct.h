#ifndef JSON_STRUCT_H
#define JSON_STRUCT_H

#include <fstream>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include "concepts.h"

namespace arro {
class JSONStruct;
}  // namespace arro

template <typename T>
concept JSONObj = std::derived_from<T, arro::JSONStruct> && requires(nlohmann::json obj) {
	{ T(obj) } -> std::convertible_to<T>;
};

namespace arro {
class JSONStruct : protected nlohmann::json {
public:
	template <JSONPrimitive T>
	friend class Field;

	template <JSONPrimitive T>
	class Field {
	public:
		Field(const JSONStruct* host, const std::string& name) : _host(host), _name(name) {}

		T operator()() const { return (*_host)[_name]; }
		bool operator==(const Field<T>& other) { return (*this)() == other(); }

	public:
		const JSONStruct* _host;
		std::string _name;
	};

	JSONStruct(const nlohmann::json& obj) : nlohmann::json(obj) {}

	template <JSONObj T>
	static T parse(const std::string& file);

	template <JSONObj T>
	static std::vector<T> parseArray(const std::string& file);
};
}  // namespace arro

#include "JSONStruct.hpp"

#endif