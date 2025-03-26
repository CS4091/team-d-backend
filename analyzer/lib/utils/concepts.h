#ifndef CONCEPTS_H
#define CONCEPTS_H

#include <concepts>
#include <nlohmann/json.hpp>
#include <string>

template <typename T>
concept Comparable = requires(T a, T b) {
	{ a < b } -> std::convertible_to<bool>;
};

template <typename T, typename... A>
concept Callable = requires(T fn, A... args) { fn(args...); };

template <typename T, typename R, typename... A>
concept Function = Callable<T, A...> && requires(T fn, A... args) {
	{ fn(args...) } -> std::convertible_to<R>;
};

template <typename T, typename... A>
concept Routine = Callable<T, A...> && requires(T fn, A... args) {
	{ fn(args...) } -> std::convertible_to<void>;
};

template <typename T>
concept Weighted = requires(T t) {
	{ t.cost() } -> std::convertible_to<double>;
};

template <typename T, typename E>
concept SizedRandomAccess = requires(T arr, std::size_t i) {
	{ arr.size() } -> std::convertible_to<std::size_t>;
	{ arr[i] } -> std::convertible_to<E>;
};

template <typename T>
concept IDAble = requires(T elem) {
	elem.id;
	elem.id == elem.id;
};

template <typename T>
concept Serializable = std::copy_constructible<T> && requires(T elem, std::string str) {
	{ T::stringify(elem) } -> std::convertible_to<nlohmann::json>;
	{ T::parse(str) } -> std::convertible_to<T>;
};

template <typename T>
concept UniqueSerializable = Serializable<T> && IDAble<T>;

template <typename T>
concept JSONPrimitive = requires(nlohmann::json json, T elem) { json = elem; };

#endif