/*
 * BoostGenerator.h
 *
 *  Created on: Oct 28, 2009
 *      Author: openCL
 */

#ifndef GENERATOR_H_
#define GENERATOR_H_

#include <vector>
#include <set>
#include <limits>
#include <cstdlib>

using std::vector;
using std::set;
using std::numeric_limits;

template<class T>
class Generator {
public:

	Generator(unsigned int seed, vector<T> edgeCasesArg);

	virtual T getRandomNum() = 0;

	T getNum();

	void getArrayNums(T* array, unsigned int size);

protected:

	vector<T> edgeCases;

	// this value must be between 0.0 and 1.0
	static const float EDGE_CASE_PROBABILITY;

	static const unsigned int EDGE_CASE_THREASHOLD;

};

template<class T>
// TOD0 : add check value is in the right range
// this value must be between 0.0 and 1.0
const float Generator<T>::EDGE_CASE_PROBABILITY = 0.5;

template<class T>
const unsigned int Generator<T>::EDGE_CASE_THREASHOLD = 2;

template<class T>
Generator<T>::Generator(unsigned int seed, vector<T> edgeCasesArg) {

	edgeCases = vector<T> (edgeCasesArg);
}

template<class T>
T Generator<T>::getNum() {

	float edgeCaseProbability = rand() / RAND_MAX;

	if ((edgeCases.size() > 0)
			&& (edgeCaseProbability <= EDGE_CASE_PROBABILITY)) {
		return edgeCases[rand() % edgeCases.size()];
	} else {
		return getRandomNum();
	}
}

template<class T>
void Generator<T>::getArrayNums(T* array, unsigned int size) {

	if (size < EDGE_CASE_THREASHOLD * edgeCases.size()) {
		// don't want to fill small arrays with only edge cases
		for (unsigned int i = 0; i < size; i++) {
			array[i] = getNum();
		}
	} else {

		vector<int> arrayIndices;
		set<int> arrayIndicesSet;

		// choose the array indices where the edge cases will be placed
		int index;
		while (arrayIndices.size() != edgeCases.size()) {
			index = rand() % size;
			if (arrayIndicesSet.count(index) == 0) {
				arrayIndicesSet.insert(index);
				arrayIndices.push_back(index);
			}

		}

		// put the edge cases in the chosen array according to the indices indices
		for (unsigned int i = 0; i < arrayIndices.size(); i++) {
			array[arrayIndices[i]] = edgeCases[i];
		}

		// put values to rest of the array
		for (unsigned int i = 0; i < size; i++) {
			if (arrayIndicesSet.count(i) == 0) {
				array[i] = getNum();
			}
		}

	}

}

#endif /* GENERATOR_H_ */
