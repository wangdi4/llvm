/*
 * MulVector.h
 *
 *  Created on: May 7, 2009
 *      Author: opencl
 */

#ifndef MULVECTOR_H_
#define MULVECTOR_H_

#include "../../KerenelExecutor.h"

class MulVector {
public:
	MulVector();
	virtual ~MulVector();
	static void test();

private:
	static void execute(cl_float4** res, Execution::Type executionType);
	static const int size = 4;
};

#endif /* MULVECTOR_H_ */
