/*
 * MulVector.cpp
 *
 *  Created on: May 7, 2009
 *      Author: opencl
 */

#include "MulVector.h"

#include <iostream>
#include <string>

#include "../../TestFailedException.h"

using std::cout;
using std::endl;
using std::string;

MulVector::MulVector() {
	// TODO Auto-generated constructor stub

}

MulVector::~MulVector() {
	// TODO Auto-generated destructor stub
}

void MulVector::test() {

	cl_float4 resArray[size] = { { 0.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f,
			0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f },
			{ 0.0f, 0.0f, 0.0f, 0.0f } };
	cl_float4* res = resArray;

	execute(&res, Execution::NORMAL);

	cl_float4 resArray2[size] = { { 0.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f,
			0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f },
			{ 0.0f, 0.0f, 0.0f, 0.0f } };
	cl_float4* res2 = resArray2;

	execute(&res2, Execution::VECTORIZED);

	for (int i = 0; i < size; i++) {
		//	go over cl_float4
		for (int j = 0; j < 4; j++) {
			if (res[i][j] != res2[i][j]) {
				throw TestFailedException("results are different!!!");
			}
		}
	}
}

void MulVector::execute(cl_float4** res, Execution::Type executionType) {

	cl_float4 a[size] = { { 1.0f, 2.0f, 3.0f, 4.0f },
			{ 1.0f, 2.0f, 3.0f, 4.0f }, { 1.0f, 2.0f, 3.0f, 4.0f }, { 1.0f,
					2.0f, 3.0f, 4.0f } };

	Parameter paramSrc(a, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			sizeof(cl_float4) * size, a, false);

	Parameter paramDest(*res, CL_MEM_READ_WRITE, sizeof(cl_float4) * size,
			NULL, true);

	Parameter params[2] = { paramSrc, paramDest };

	string kernelName = "mul_vector";
	string program_source = "src/DifferentTests/MulVector/" + kernelName;

	string executionRes = KerenelExecutor::execKernel(program_source.c_str(),
			kernelName.c_str(), size, params, 2, executionType);

	if (executionRes != "ok") {

		throw TestFailedException("kernel " + kernelName
				+ " didn't execute properly: " + executionRes);
	}
}
