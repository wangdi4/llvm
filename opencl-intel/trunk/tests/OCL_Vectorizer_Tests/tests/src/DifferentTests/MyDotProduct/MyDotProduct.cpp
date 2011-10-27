/*
 * MyDotProduct.cpp
 *
 *  Created on: May 7, 2009
 *      Author: opencl
 */

#include "MyDotProduct.h"

#include <iostream>
#include <string>

#include "../../TestFailedException.h"

using std::cout;
using std::endl;
using std::string;

MyDotProduct::MyDotProduct() {
	// TODO Auto-generated constructor stub

}

MyDotProduct::~MyDotProduct() {
	// TODO Auto-generated destructor stub
}

void MyDotProduct::test() {

	const int SIZE = 4;

	float resArray[4] = { 0, 0, 0, 0 };
	float* res = resArray;

	execute(&res, Execution::NORMAL);

	float resArray2[4] = { 0, 0, 0, 0 };
	float* res2 = resArray2;

	execute(&res2, Execution::VECTORIZED);

	for (int i = 0; i < SIZE; i++) {
		if (res[i] != res2[i]) {
			throw TestFailedException("results are different!!!");
		}
	}
}

void MyDotProduct::execute(float** res, Execution::Type executionType) {

	const int SIZE = 4;

	cl_float4 a[SIZE] = { { 1.0f, 2.0f, 3.0f, 4.0f },
			{ 1.0f, 2.0f, 3.0f, 4.0f }, { 1.0f, 2.0f, 3.0f, 4.0f }, { 1.0f,
					2.0f, 3.0f, 4.0f } };

	cl_float4 b[SIZE] = { { 1.0f, 2.0f, 3.0f, 4.0f },
			{ 1.0f, 2.0f, 3.0f, 4.0f }, { 1.0f, 2.0f, 3.0f, 4.0f }, { 1.0f,
					2.0f, 3.0f, 4.0f } };

	Parameter paramSrc1(a, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			sizeof(cl_float4) * SIZE, a, false);

	Parameter paramSrc2(b, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			sizeof(cl_float4) * SIZE, b, false);

	Parameter paramDest1(*res, CL_MEM_READ_WRITE, sizeof(float) * SIZE, NULL,
			true);

	Parameter params[3] = { paramSrc1, paramSrc2, paramDest1 };

	string kernelName = "my_dot_product";
	string program_source = "src/DifferentTests/MyDotProduct/" + kernelName;

	string executionRes = KerenelExecutor::execKernel(program_source.c_str(),
			kernelName.c_str(), SIZE, params, 3, executionType);

	if (executionRes != "ok") {

		throw TestFailedException("kernel " + kernelName
				+ " didn't execute properly: " + executionRes);
	}
}
