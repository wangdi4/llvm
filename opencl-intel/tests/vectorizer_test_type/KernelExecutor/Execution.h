/*
 * ExecutionType.h
 *
 *  Created on: Aug 9, 2009
 *      Author: openCL
 */

#ifndef EXECUTION_H_
#define EXECUTION_H_

#include <string>

using std::string;

class Execution {
public:

	typedef enum {

		NORMAL,

		VECTORIZED

	} Type;

	static string toString(Type type);
};

#endif /* EXECUTION_H_ */
