/*
 * RegularParameter.cpp
 *
 *  Created on: Aug 10, 2009
 *      Author: openCL
 */

#include "RegularParameter.h"

#include "IllegalParameterException.h"
#include "../RandomInputGenerator/RandomInputGenerator.h"

#include <iostream>

map<string, RegularParameter::Properties> RegularParameter::mapping =
		RegularParameter::intiMapping();

/**
 * @throws IllegalParameterException
 */
RegularParameter::RegularParameter(const string& type) {

	Properties props = mapping.find(type)->second;
	initParameter(props.getParamType(), props.getVectorType(), NULL);
}

map<string, RegularParameter::Properties> RegularParameter::intiMapping() {

	map<string, RegularParameter::Properties> mapping;

	mapping["bool"] = Properties(BOOL, NOT_VEC);

	mapping["char"] = Properties(CHAR, NOT_VEC);
	mapping["char2"] = Properties(CHAR, VEC_2);
	mapping["char4"] = Properties(CHAR, VEC_4);
	mapping["char8"] = Properties(CHAR, VEC_8);
	mapping["char16"] = Properties(CHAR, VEC_16);

	mapping["uchar"] = Properties(UCHAR, NOT_VEC);
	mapping["uchar2"] = Properties(UCHAR, VEC_2);
	mapping["uchar4"] = Properties(UCHAR, VEC_4);
	mapping["uchar8"] = Properties(UCHAR, VEC_8);
	mapping["uchar16"] = Properties(UCHAR, VEC_16);

	mapping["short"] = Properties(SHORT, NOT_VEC);
	mapping["short2"] = Properties(SHORT, VEC_2);
	mapping["short4"] = Properties(SHORT, VEC_4);
	mapping["short8"] = Properties(SHORT, VEC_8);
	mapping["short16"] = Properties(SHORT, VEC_16);

	mapping["ushort"] = Properties(USHORT, NOT_VEC);
	mapping["ushort2"] = Properties(USHORT, VEC_2);
	mapping["ushort4"] = Properties(USHORT, VEC_4);
	mapping["ushort8"] = Properties(USHORT, VEC_8);
	mapping["ushort16"] = Properties(USHORT, VEC_16);

	mapping["int"] = Properties(INT, NOT_VEC);
	mapping["int2"] = Properties(INT, VEC_2);
	mapping["int4"] = Properties(INT, VEC_4);
	mapping["int8"] = Properties(INT, VEC_8);
	mapping["int16"] = Properties(INT, VEC_16);

	mapping["uint"] = Properties(UINT, NOT_VEC);
	mapping["uint2"] = Properties(UINT, VEC_2);
	mapping["uint4"] = Properties(UINT, VEC_4);
	mapping["uint8"] = Properties(UINT, VEC_8);
	mapping["uint16"] = Properties(UINT, VEC_16);

	mapping["long"] = Properties(LONG, NOT_VEC);
	mapping["long2"] = Properties(LONG, VEC_2);
	mapping["long4"] = Properties(LONG, VEC_4);
	mapping["long8"] = Properties(LONG, VEC_8);
	mapping["long16"] = Properties(LONG, VEC_16);

	mapping["ulong"] = Properties(ULONG, NOT_VEC);
	mapping["ulong2"] = Properties(ULONG, VEC_2);
	mapping["ulong4"] = Properties(ULONG, VEC_4);
	mapping["ulong8"] = Properties(ULONG, VEC_8);
	mapping["ulong16"] = Properties(ULONG, VEC_16);

	mapping["float"] = Properties(FLOAT, NOT_VEC);
	mapping["float2"] = Properties(FLOAT, VEC_2);
	mapping["float4"] = Properties(FLOAT, VEC_4);
	mapping["float8"] = Properties(FLOAT, VEC_8);
	mapping["float16"] = Properties(FLOAT, VEC_16);

	mapping["double"] = Properties(DOUBLE, NOT_VEC);
	mapping["double2"] = Properties(DOUBLE, VEC_2);
	mapping["double4"] = Properties(DOUBLE, VEC_4);
	mapping["double8"] = Properties(DOUBLE, VEC_8);
	mapping["double16"] = Properties(DOUBLE, VEC_16);

	mapping["ptrdiff_t"] = Properties(PTRDIFF_T, NOT_VEC);

	//	mapping["ptr"] = Properties(PTR, NOT_VEC);

	// TODO : do we need to add the scalar built-in data type half? we don't know it's size
	// TODO : do we need to add the built-in scalar types size_t, intptr_t, intptr_t, void?
	// TODO : do we need to add the other built-in types image, sampler, event and reserved data type?

	return mapping;
}

RegularParameter::RegularParameter(const RegularParameter& paramToCopy) {

	initParameter(paramToCopy.paramType, paramToCopy.vecType, paramToCopy.value);
}

RegularParameter& RegularParameter::operator=(
		const RegularParameter& paramToCopy) {

	deleteParameter();
	initParameter(paramToCopy.paramType, paramToCopy.vecType, paramToCopy.value);
	return *this;

}

RegularParameter::~RegularParameter() {
	deleteParameter();
}

/**
 * @throws IllegalParameterException
 */
int RegularParameter::getNumElements(VectorType vecType) {
	switch (vecType) {

	case NOT_VEC:
		return 1;

	case VEC_2:
		return 2;

	case VEC_4:
		return 4;

	case VEC_8:
		return 8;

	case VEC_16:
		return 16;

	default:
		throw IllegalParameterException("unrecognized vector type: " + vecType);
	}
}

/**
 * @throws IllegalParameterException
 */
void RegularParameter::initParameter(ParamType paramType, VectorType vecType,
		void* value) {

	this->paramType = paramType;
	this->vecType = vecType;

	int numElements = getNumElements(vecType);

	switch (paramType) {

	case BOOL:

		this->size = sizeof(bool) * numElements;
		this->value = new bool[numElements];
		if (value != NULL) {
			memcpy(this->value, value, this->size);
		} else {
			for (int i = 0; i < numElements; i++) {
				((bool*) (this->value))[i]
						= RandomInputGenerator::getInstance().getBoolGen().getNum();

				//std::cout << "bool: " << ((bool*) (this->value))[i] << std::endl;
			}
		}
		break;

	case CHAR:

		this->size = sizeof(int8_t) * numElements;
		this->value = new int8_t[numElements];
		if (value != NULL) {
			memcpy(this->value, value, this->size);
		} else {
			for (int i = 0; i < numElements; i++) {
				((int8_t*) (this->value))[i]
						= RandomInputGenerator::getInstance().getCharGen().getNum();

//				std::cout << "char: " << ((int8_t*) (this->value))[i] << std::endl;
			}
		}
		break;

	case UCHAR:

		this->size = sizeof(uint8_t) * numElements;
		this->value = new uint8_t[numElements];
		if (value != NULL) {
			memcpy(this->value, value, this->size);
		} else {
			for (int i = 0; i < numElements; i++) {
				((uint8_t*) (this->value))[i]
						= RandomInputGenerator::getInstance().getUcharGen().getNum();

//				std::cout << "uchar: " << ((uint8_t*) (this->value))[i] << std::endl;
			}
		}
		break;

	case SHORT:

		this->size = sizeof(int16_t) * numElements;
		this->value = new int16_t[numElements];
		if (value != NULL) {
			memcpy(this->value, value, this->size);
		} else {
			for (int i = 0; i < numElements; i++) {
				((int16_t*) (this->value))[i] = RandomInputGenerator::getInstance().getShortGen().getNum();

//				std::cout << "short: " << ((int16_t*) (this->value))[i] << std::endl;
			}
		}
		break;

	case USHORT:

		this->size = sizeof(uint16_t) * numElements;
		this->value = new uint16_t[numElements];
		if (value != NULL) {
			memcpy(this->value, value, this->size);
		} else {
			for (int i = 0; i < numElements; i++) {
				((uint16_t*) (this->value))[i] = RandomInputGenerator::getInstance().getUshortGen().getNum();

//				std::cout << "ushort: " << ((uint16_t*) (this->value))[i] << std::endl;
			}
		}
		break;

	case INT:

		this->size = sizeof(int32_t) * numElements;
		this->value = new int32_t[numElements];
		if (value != NULL) {
			memcpy(this->value, value, this->size);
		} else {
			for (int i = 0; i < numElements; i++) {
				((int32_t*) (this->value))[i] = RandomInputGenerator::getInstance().getIntGen().getNum();

//				std::cout << "int: " << ((int32_t*) (this->value))[i] << std::endl;
			}
		}
		break;

	case UINT:

		this->size = sizeof(uint32_t) * numElements;
		this->value = new uint32_t[numElements];
		if (value != NULL) {
			memcpy(this->value, value, this->size);
		} else {
			for (int i = 0; i < numElements; i++) {
				((uint32_t*) (this->value))[i] = RandomInputGenerator::getInstance().getUintGen().getNum();

//				std::cout << "uint: " << ((uint32_t*) (this->value))[i] << std::endl;
			}
		}
		break;

	case LONG:

		this->size = sizeof(int64_t) * numElements;
		this->value = new int64_t[numElements];
		if (value != NULL) {
			memcpy(this->value, value, this->size);
		} else {
			for (int i = 0; i < numElements; i++) {
				((int64_t*) (this->value))[i] = RandomInputGenerator::getInstance().getLongGen().getNum();

//				std::cout << "long: " << ((int64_t*) (this->value))[i] << std::endl;
			}
		}
		break;

	case ULONG:

		this->size = sizeof(uint64_t) * numElements;
		this->value = new uint64_t[numElements];
		if (value != NULL) {
			memcpy(this->value, value, this->size);
		} else {
			for (int i = 0; i < numElements; i++) {
				((uint64_t*) (this->value))[i] = RandomInputGenerator::getInstance().getUlongGen().getNum();

//				std::cout << "ulong: " << ((uint64_t*) (this->value))[i] << std::endl;
			}
		}
		break;

	case FLOAT:

		this->size = sizeof(float) * numElements;
		this->value = new float[numElements];
		if (value != NULL) {
			memcpy(this->value, value, this->size);
		} else {
			for (int i = 0; i < numElements; i++) {
				((float*) (this->value))[i] =  RandomInputGenerator::getInstance().getFloatGen().getNum();

//				std::cout << "float: " << ((float*) (this->value))[i] << std::endl;
			}
		}
		break;

	case DOUBLE:

		this->size = sizeof(double) * numElements;
		this->value = new double[numElements];
		if (value != NULL) {
			memcpy(this->value, value, this->size);
		} else {
			for (int i = 0; i < numElements; i++) {
				((double*) (this->value))[i] =  RandomInputGenerator::getInstance().getDoubleGen().getNum();

//				std::cout << "double: " << ((double*) (this->value))[i] << std::endl;
			}
		}
		break;

		//	case PTR:
		//
		//		this->size = sizeof(double*) * numElements;
		//		this->value = new double*[numElements];
		//		if (value != NULL) {
		//			for (int i = 0; i < numElements; i++) {
		//				((double**) (this->value))[i] = new double;
		//				*(((double**) (this->value))[i]) = *(((double**) value)[i]);
		//			}
		//		} else {
		//			for (int i = 0; i < numElements; i++) {
		//				((double**) (this->value))[i] = new double;
		//				*(((double**) (this->value))[i]) = (double) (5 + i);
		//			}
		//		}
		//		break;

	case PTRDIFF_T:

		this->size = sizeof(int*) * numElements;
		this->value = new int*[numElements];
		if (value != NULL) {
			memcpy(this->value, value, this->size);
		} else {
			for (int i = 0; i < numElements; i++) {
				// TODO : is the casting uint into int* ok?
				((int**) (this->value))[i] = (int*)RandomInputGenerator::getInstance().getUintGen().getNum();

//				std::cout << "ptrdiff: " << ((int*) (this->value))[i] << std::endl;
			}
		}
		break;

	default:
		throw IllegalParameterException("unrecognized parameter type: "
				+ paramType);
	}
}

void RegularParameter::deleteParameter() {

	switch (paramType) {

	case BOOL:
		delete[] ((bool*) this->value);
		break;

	case CHAR:
		delete[] ((char*) this->value);
		break;

	case UCHAR:
		delete[] ((unsigned char*) this->value);
		break;

	case SHORT:
		delete[] ((short*) this->value);
		break;

	case USHORT:
		delete[] ((unsigned short*) this->value);
		break;

	case INT:
		delete[] ((int*) this->value);
		break;

	case UINT:
		delete[] ((unsigned int*) this->value);
		break;

	case LONG:
		delete[] ((long*) this->value);
		break;

	case ULONG:
		delete[] ((unsigned long*) this->value);
		break;

	case FLOAT:
		delete[] ((float*) this->value);
		break;

	case DOUBLE:
		delete[] ((double*) this->value);
		break;

		//	case PTR:
		//		for (int i = 0; i < numElements; i++) {
		//			delete (((double**) (this->value))[i]);
		//		}
		//		delete[] ((double**) this->value);
		//		break;

	case PTRDIFF_T:
		delete[] ((int**) this->value);
		break;

	default:
		// nothing to do
		return;
	}

}
