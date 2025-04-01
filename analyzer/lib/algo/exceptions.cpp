#include "exceptions.h"

#include "routing.h"

arro::algo::UnroutableException::UnroutableException(const std::vector<data::RouteReq>& routes)
	: std::out_of_range("One or more routes are unsatisfiable with the given planes."), routes(routes) {}