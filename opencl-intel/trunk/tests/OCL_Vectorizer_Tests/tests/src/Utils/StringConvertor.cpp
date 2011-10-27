/*
 * ExecutionHelperTypes.cpp
 *
 *  Created on: Jun 11, 2009
 *      Author: openCL
 */

#include "StringConvertor.h"

#include <sstream>

using std::stringstream;


string StringConverter::toString(float array[], int size) {

	stringstream s;

	s << "{";

	for (int i = 0; i < size - 1; i++) {
		s << array[i] << ", ";
	}

	s << array[size - 1] << "}";

	return s.str();
}
