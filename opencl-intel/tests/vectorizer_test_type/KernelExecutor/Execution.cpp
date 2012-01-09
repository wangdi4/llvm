/*
 * ExecutionType.cpp
 *
 *  Created on: Aug 9, 2009
 *      Author: openCL
 */

#include "Execution.h"

string Execution::toString(Type type) {

	switch (type) {

	case NORMAL:
		return "NORMAL";

	case VECTORIZED:
		return "VECTORIZED";

	default:
		return "non valid execution type";

	}
}

