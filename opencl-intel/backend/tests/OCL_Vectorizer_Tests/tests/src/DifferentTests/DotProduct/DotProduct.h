/*
 * DotProduct.h
 *
 *  Created on: May 7, 2009
 *      Author: opencl
 */

#ifndef DOTPRODUCT_H_
#define DOTPRODUCT_H_

#include "../../KerenelExecutor.h"

class DotProduct {
public:
	DotProduct();
	virtual ~DotProduct();
	static void test();

private:
	static void execute(float** res, Execution::Type executionType);
};

#endif /* DOTPRODUCT_H_ */
