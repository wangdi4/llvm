/*
 * IntGenerator.h
 *
 *  Created on: Oct 28, 2009
 *      Author: openCL
 */

#ifndef INTGENERATOR_H_
#define INTGENERATOR_H_

#include <limits>

#include "Generator.h"

using std::numeric_limits;

template<class T>
class IntGenerator: public Generator<T> {
public:
	IntGenerator(int seed, vector<T> edgeCases) :
		Generator<T> (seed, edgeCases) {
	}

	virtual ~IntGenerator() {
	}

	virtual T getRandomNum() {
		return rand();
	}
};

#endif /* INTGENERATOR_H_ */
