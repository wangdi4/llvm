/*
 * RealGenerator.h
 *
 *  Created on: Oct 28, 2009
 *      Author: openCL
 */

#ifndef REALGENERATOR_H_
#define REALGENERATOR_H_

#include <limits>

#include "Generator.h"

using std::numeric_limits;

template<class T>
class RealGenerator: public Generator<T> {
public:
	// TODO : set other range values, maybe use different random isEdgeCaseGenerator
	RealGenerator(int seed, vector<T> edgeCases) :
		Generator<T> (seed, edgeCases) {
	}

	virtual ~RealGenerator() {
	}

	virtual T getRandomNum() {
		return rand() + (rand() / RAND_MAX);
	}
};

#endif /* REALGENERATOR_H_ */
