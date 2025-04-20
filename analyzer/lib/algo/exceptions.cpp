#include "exceptions.h"

#include "routing.h"

arro::algo::UnroutableException::UnroutableException(const std::vector<data::RouteError>& errors)
	: std::out_of_range("One or more routes are unsatisfiable with the given planes."), errors(errors) {}