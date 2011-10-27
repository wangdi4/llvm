/*
 * IntGenerator.h
 *
 *  Created on: Oct 28, 2009
 *      Author: openCL
 */

#ifndef BOOLGENERATOR_H_
#define BOOLGENERATOR_H_

#include <limits>

#include "Generator.h"

template<class T>
class BoolGenerator: public Generator<T> {
public:
	BoolGenerator(int seed, vector<T> edgeCases) :
		Generator<T> (seed, edgeCases) {
	}

	virtual ~BoolGenerator() {
	}

	virtual T getRandomNum() {
		return (rand() % 2) != 0;
	}
};

#endif /* BOOLGENERATOR_H_ */
