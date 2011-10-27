/*
 * RandomInputGenerator.cpp
 *
 *  Created on: Oct 22, 2009
 *      Author: openCL
 */

#include "RandomInputGenerator.h"

#include <time.h>
#include <vector>

#include "IntGenerator.h"
#include "RealGenerator.h"
#include "BoolGenerator.h"

using std::vector;

RandomInputGenerator* RandomInputGenerator::randomInputGenerator = 0;

RandomInputGenerator& RandomInputGenerator::getInstance() {
	if (randomInputGenerator == 0) {
		randomInputGenerator = new RandomInputGenerator();
	}

	return *randomInputGenerator;
}

RandomInputGenerator::RandomInputGenerator() {

	// TODO : upgrade edgge cases

	//seed = time(0);
    //seed=1303391365;
    seed = 1299746557;  //seed for test daily
	// init random generators
	srand(seed);

	// the base seeds creator doesn't have age cases
	vector<unsigned int> seedEdgeCases;
	// replace time(0) with some other seed
	IntGenerator<unsigned int> baseSeeds(seed, seedEdgeCases);

	vector<bool> boolEdgeCases;
	boolEdgeCases.push_back(false);
	boolEdgeCases.push_back(true);
	boolGen = new BoolGenerator<bool> (baseSeeds.getNum(), boolEdgeCases);

	vector<int8_t> charEdgeCases;
	charEdgeCases.push_back(0);
	charEdgeCases.push_back(1);
	charEdgeCases.push_back(-1);
	charEdgeCases.push_back(numeric_limits<int8_t>::min());
	charEdgeCases.push_back(numeric_limits<int8_t>::max());
	charGen = new IntGenerator<int8_t> (baseSeeds.getNum(), charEdgeCases);

	vector<uint8_t> ucharEdgeCases;
	ucharEdgeCases.push_back(0);
	ucharEdgeCases.push_back(1);
	// no need for ucharEdgeCases.push_back(numeric_limits<unsigned int8_t>::min()) because it's 0
	ucharEdgeCases.push_back(numeric_limits<uint8_t>::max());
	ucharGen = new IntGenerator<uint8_t> (baseSeeds.getNum(), ucharEdgeCases);

	vector<int16_t> shortEdgeCases;
	shortEdgeCases.push_back(0);
	shortEdgeCases.push_back(1);
	shortEdgeCases.push_back(-1);
	shortEdgeCases.push_back(numeric_limits<int16_t>::min());
	shortEdgeCases.push_back(numeric_limits<int16_t>::max());
	shortGen = new IntGenerator<int16_t> (baseSeeds.getNum(), shortEdgeCases);

	vector<uint16_t> ushortEdgeCases;
	ushortEdgeCases.push_back(0);
	ushortEdgeCases.push_back(1);
	// no need for ushortEdgeCases.push_back(numeric_limits<unsigned int16_t>::min()) because it's 0
	ushortEdgeCases.push_back(numeric_limits<uint16_t>::max());
	ushortGen
			= new IntGenerator<uint16_t> (baseSeeds.getNum(), ushortEdgeCases);

	vector<int32_t> intEdgeCases;
	intEdgeCases.push_back(0);
	intEdgeCases.push_back(1);
	intEdgeCases.push_back(-1);
	intEdgeCases.push_back(numeric_limits<int32_t>::min());
	intEdgeCases.push_back(numeric_limits<int32_t>::max());
	intGen = new IntGenerator<int32_t> (baseSeeds.getNum(), intEdgeCases);

	vector<uint32_t> uintEdgeCases;
	uintEdgeCases.push_back(0);
	uintEdgeCases.push_back(1);
	// no need for uintEdgeCases.push_back(numeric_limits<unsigned int32_t>::min()) because it's 0
	uintEdgeCases.push_back(numeric_limits<uint32_t>::max());
	uintGen = new IntGenerator<uint32_t> (baseSeeds.getNum(), uintEdgeCases);

	vector<int64_t> longEdgeCases;
	longEdgeCases.push_back(0);
	longEdgeCases.push_back(1);
	longEdgeCases.push_back(-1);
	longEdgeCases.push_back(numeric_limits<int64_t>::min());
	longEdgeCases.push_back(numeric_limits<int64_t>::max());
	longGen = new IntGenerator<int64_t> (baseSeeds.getNum(), longEdgeCases);

	vector<uint64_t> ulongEdgeCases;
	ulongEdgeCases.push_back(0);
	ulongEdgeCases.push_back(1);
	// no need for ulongEdgeCases.push_back(numeric_limits<unsigned int64_t>::min()) because it's 0
	ulongEdgeCases.push_back(numeric_limits<uint64_t>::max());
	ulongGen = new IntGenerator<uint64_t> (baseSeeds.getNum(), ulongEdgeCases);

	vector<float> floatEdgeCases;
	floatEdgeCases.push_back(0);
	floatEdgeCases.push_back(-0);
	floatEdgeCases.push_back(1);
	floatEdgeCases.push_back(-1);
	floatEdgeCases.push_back(numeric_limits<float>::min());
	floatEdgeCases.push_back(numeric_limits<float>::max());
	floatGen = new RealGenerator<float> (baseSeeds.getNum(), floatEdgeCases);

	vector<double> doubleEdgeCases;
	doubleEdgeCases.push_back(0);
	doubleEdgeCases.push_back(-0);
	doubleEdgeCases.push_back(1);
	doubleEdgeCases.push_back(-1);
	doubleEdgeCases.push_back(numeric_limits<double>::min());
	doubleEdgeCases.push_back(numeric_limits<double>::max());
	doubleGen = new RealGenerator<double> (baseSeeds.getNum(), doubleEdgeCases);
}

RandomInputGenerator::~RandomInputGenerator() {

	delete boolGen;
	delete charGen;
	delete ucharGen;
	delete shortGen;
	delete ushortGen;
	delete intGen;
	delete uintGen;
	delete longGen;
	delete ulongGen;
	delete floatGen;
	delete doubleGen;
}

