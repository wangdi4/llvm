/*
 * DotProduct.cpp
 *
 *  Created on: May 7, 2009
 *      Author: opencl
 */

#include "DotOp.h"

#include <iostream>
#include <string>

#include "../../TestFailedException.h"

using std::cout;
using std::endl;
using std::string;

DotOp::DotOp() {
	// TODO Auto-generated constructor stub

}

DotOp::~DotOp() {
	// TODO Auto-generated destructor stub
}

void DotOp::runAllTests() {

	test("process");

	test("process_vec4");

	test("process_vec4_opt");

	test("process_vec8");
}

void DotOp::test(string kernelName) {

	const int SIZE = 8;

	float resArray[SIZE] = { 0, 0, 0, 0 };
	float* res = resArray;

	execute(kernelName, &res, Execution::NORMAL);

	float resArray2[SIZE] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	float* res2 = resArray2;

	execute(kernelName, &res2, Execution::NORMAL);

	for (int i = 0; i < SIZE; i++) {
		if (res[i] != res2[i]) {
			throw TestFailedException("results are different!!!");
		}
	}
}

void DotOp::execute(string kernelName, float** res, Execution::Type executionType) {

	const int SIZE = 8;

	float a[SIZE] = { 1.0f, 2.0f, 3.0f, 4.0f, 1.0f, 2.0f, 3.0f, 4.0f };

	float b[SIZE] = { 1.0f, 2.0f, 3.0f, 4.0f, 1.0f, 2.0f, 3.0f, 4.0f };

	Parameter
			paramSrc1(a, CL_MEM_COPY_HOST_PTR, sizeof(float) * SIZE, a, false);

	Parameter
			paramSrc2(b, CL_MEM_COPY_HOST_PTR, sizeof(float) * SIZE, b, false);

	Parameter paramDest1(*res, CL_MEM_READ_WRITE, sizeof(float) * SIZE, NULL,
			true);

	Parameter params[3] = { paramSrc1, paramSrc2, paramDest1 };

	string program_source = "src/DifferentTests/DotOp/" + kernelName;

	string executionRes = KerenelExecutor::execKernel(program_source.c_str(),
			kernelName.c_str(), SIZE, params, 3, executionType);

	if (executionRes != "ok") {

		throw TestFailedException("kernel " + kernelName
				+ " didn't execute properly: " + executionRes);
	}
}
