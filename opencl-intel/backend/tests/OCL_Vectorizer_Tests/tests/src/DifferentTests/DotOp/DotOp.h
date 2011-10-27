/*
 * DotProduct.h
 *
 *  Created on: May 7, 2009
 *      Author: opencl
 */

#ifndef DOTOP_H_
#define DOTOP_H_

#include <string>

#include "../../KerenelExecutor.h"

using std::string;

class DotOp {
public:
	DotOp();
	virtual ~DotOp();
	static void runAllTests();

private:
	static void test(string kernelName);
	static void execute(string kernelName, float** res, Execution::Type executionType);
};

#endif /* DOTOP_H_ */
