/*
 * MyDotProduct.h
 *
 *  Created on: May 7, 2009
 *      Author: opencl
 */

#ifndef MYDOTPRODUCT_H_
#define MYDOTPRODUCT_H_

#include "../../KerenelExecutor.h"

class MyDotProduct {
public:
	MyDotProduct();
	virtual ~MyDotProduct();
	static void test();

private:
	static void execute(float** res, Execution::Type executionType);
};

#endif /* MYDOTPRODUCT_H_ */
