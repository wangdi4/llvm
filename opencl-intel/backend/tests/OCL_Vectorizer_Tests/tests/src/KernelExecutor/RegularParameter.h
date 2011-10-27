/*
 * RegularParameter.h
 *
 *  Created on: Aug 10, 2009
 *      Author: openCL
 */

#ifndef REGULARPARAMETER_H_
#define REGULARPARAMETER_H_

#include <cstddef>
#include <map>
#include <string>

using std::map;
using std::string;

class RegularParameter {
public:

	/**
	 * @throws IllegalParameterException
	 */
	RegularParameter(const string& type);
	RegularParameter(const RegularParameter& paramToCopy);
	RegularParameter& operator=(const RegularParameter& paramToCopy);
	~RegularParameter();

	void *getValue() const {
		return value;
	}

	size_t getSize() const {
		return size;
	}

	static bool isTypeLegal(const string& type) {
		return mapping.count(type) > 0;
	}

private:

	typedef enum {

		BOOL,

		CHAR,

		UCHAR,

		SHORT,

		USHORT,

		INT,

		UINT,

		LONG,

		ULONG,

		FLOAT,

		DOUBLE,

//		PTR,

		PTRDIFF_T

	} ParamType;

	typedef enum {

		NOT_VEC, VEC_2, VEC_4, VEC_8, VEC_16

	} VectorType;

	class Properties {

	public:
		Properties(ParamType paramType = FLOAT, VectorType vecType = NOT_VEC) {
			this->paramType = paramType;
			this->vecType = vecType;
		}

		ParamType getParamType() {
			return paramType;
		}
		VectorType getVectorType() {
			return vecType;
		}

	private:

		ParamType paramType;
		VectorType vecType;

	};

	void* value;
	size_t size;
	ParamType paramType;
	VectorType vecType;

	static map<string, Properties> mapping;

	static map<string, RegularParameter::Properties> intiMapping();

	/**
	 * @throws IllegalParameterException
	 */
	int getNumElements(VectorType vecType);

	/**
	 * @throws IllegalParameterException
	 */
	void initParameter(ParamType paramType, VectorType vecType, void* value);
	void deleteParameter();

};

#endif /* REGULARPARAMETER_H_ */
