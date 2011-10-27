/*
 * MulVector.h
 *
 *  Created on: May 7, 2009
 *      Author: opencl
 */

#ifndef INVERTCOLORS_H_
#define INVERTCOLORS_H_

#include "../../KerenelExecutor.h"

class InvertColors {
public:
	InvertColors();
	virtual ~InvertColors();
	static void test();

private:
	static void execute(cl_float4** res, Execution::Type executionType);
	static const int size = 4;
};

#endif /* INVERTCOLORS_H_ */
