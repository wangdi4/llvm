/*
 * RandomInputGenerator.h
 *
 *  Created on: Oct 22, 2009
 *      Author: openCL
 */

#ifndef RANDOMINPUTGENERATOR_H_
#define RANDOMINPUTGENERATOR_H_

#include "Generator.h"


typedef char int8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned int uint32_t;
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;

class RandomInputGenerator {
public:

	static RandomInputGenerator& getInstance();

	virtual ~RandomInputGenerator();

	// getters

	unsigned int getSeed()										{	return seed; 		}

	// generator getters

	Generator<bool>& getBoolGen() const 						{	return *boolGen;	}

	Generator<int8_t>& getCharGen() const 						{	return *charGen;	}

	Generator<uint8_t>& getUcharGen() const 					{	return *ucharGen;	}

	Generator<int16_t>& getShortGen() const 					{	return *shortGen;	}

	Generator<uint16_t>& getUshortGen() const 					{	return *ushortGen;	}

	Generator<int32_t>& getIntGen() const 						{	return *intGen;		}

	Generator<uint32_t>& getUintGen() const 					{	return *uintGen;	}

	Generator<int64_t>& getLongGen() const 						{	return *longGen;	}

	Generator<uint64_t>& getUlongGen() const 					{	return *ulongGen;	}

	Generator<float>& getFloatGen() const 						{	return *floatGen;	}

	Generator<double>& getDoubleGen() const 					{	return *doubleGen;	}

private:

	unsigned int seed;

	RandomInputGenerator();

	static RandomInputGenerator* randomInputGenerator;

	Generator<bool>* boolGen;
	Generator<int8_t>* charGen;
	Generator<uint8_t>* ucharGen;
	Generator<int16_t>* shortGen;
	Generator<uint16_t>* ushortGen;
	Generator<int32_t>* intGen;
	Generator<uint32_t>* uintGen;
	Generator<int64_t>* longGen;
	Generator<uint64_t>* ulongGen;
	Generator<float>* floatGen;
	Generator<double>* doubleGen;
};

#endif /* RANDOMINPUTGENERATOR_H_ */
