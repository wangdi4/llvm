/*
 * KerenelExecutor.cpp
 *
 *  Created on: May 7, 2009
 *      Author: opencl
 */

#include "KerenelExecutor.h"

#include <iostream>
#include <fstream>
#include <assert.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>

#define CHECK_ERRORS(err) if (err != CL_SUCCESS) {cout << "Error (" << err << ") in line " << __LINE__ << "  In file:" << __FILE__ << endl; exit(-1);}

using std::cout;
using std::cin;
using std::cerr;
using std::endl;
using std::ifstream;

bool KerenelExecutor::pauseExecution = false;

void KerenelExecutor::setPauseExecution(bool newVal) {
	pauseExecution = newVal;
}

/**
 * executes the given kernel
 *
 * @param program_source
 * 		path and name of file containing the kernel source
 * @param kernelName
 * 		name of the kernel
 * @param numInstances
 * 		num instances to create while running the kernel
 * @param params
 * 		parameters to pass to the kernel
 * @param numParms
 * 		number of parameters
 * @parm executionType
 * 		in what way to execute the kernel (NORMAL, VECTORIZED)
 *
 * @returns
 * 		message containing the string "ok" if execution succeded or
 *
 */
string KerenelExecutor::execKernel(const string& kernelName,
		const string& program_source, const string& includeFile,
		size_t numInstances, vector<ArrayParameter>& params, list<
				RegularParameter> inputArgs, Execution::Type executionType) {

	cl_context context;
	cl_command_queue cmd_queue;
	cl_device_id *devices;
	cl_program program;
	cl_kernel kernel;
	int numParams = params.size();
	cl_mem *memobjs = new cl_mem[numParams];
	size_t global_work_size[1];
	size_t local_work_size[1];
	size_t cb;
	cl_int err;

	// create the OpenCL context on a CPU device
	context = clCreateContextFromType(0, CL_DEVICE_TYPE_CPU, NULL, NULL, NULL);
	if (context == (cl_context) 0)
		return "could not create the OpenCL context on a CPU device";

	// get the list of GPU devices associated with context
	clGetContextInfo(context, CL_CONTEXT_DEVICES, 0, NULL, &cb);
	devices = (cl_device_id*) malloc(cb);
	clGetContextInfo(context, CL_CONTEXT_DEVICES, cb, devices, NULL);

	// create a command-queue
	cmd_queue = clCreateCommandQueue(context, devices[0], 0, NULL);
	if (cmd_queue == (cl_command_queue) 0) {
		clReleaseContext(context);
		return "could not create a command-queue";
	}
	free(devices);

	// allocate the buffer memory objects
	for (int i = 0; i < numParams; i++) {

		ArrayParameter param = params[i];

		memobjs[i] = clCreateBuffer(context, param.getCLMem(), param.getSize(),
				param.getHostPtr(), NULL);
		if (memobjs[i] == (cl_mem) 0) {
			deleteMemobjs(memobjs, i - 1);
			clReleaseCommandQueue(cmd_queue);
			clReleaseContext(context);
			return "could not allocate the buffer memory objects";
		}
	}

	// create the program
    program = createProgramFromFile(context, program_source, includeFile, executionType);
	// comes instead of this code:
	//	program = clCreateProgramWithSource(context, 1,
	//			(const char**) &program_source, NULL, NULL);
	if (program == (cl_program) 0) {
		deleteMemobjs(memobjs, numParams);
		clReleaseCommandQueue(cmd_queue);
		clReleaseContext(context);
		return "could not create the program";
	}

	// build the program
	err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
	if (err != CL_SUCCESS) {

		char buildLog[1024];

		cl_device_type use_device = CL_DEVICE_TYPE_CPU;
		cl_device_id dev;
		err = clGetDeviceIDs(NULL, use_device, 1, &dev, NULL);
		CHECK_ERRORS(err);
		err = clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG, 1024,
				buildLog, NULL);

		deleteMemobjs(memobjs, numParams);
		clReleaseProgram(program);
		clReleaseCommandQueue(cmd_queue);
		clReleaseContext(context);
		return "could not build the program\nbuildLog:\n" + string(buildLog);
	}

	// create the kernel
    cl_int errcode_ret;
    cl_int *perrcode_ret=&errcode_ret;
	kernel = clCreateKernel(program, kernelName.c_str(), perrcode_ret);
	if (kernel == (cl_kernel) 0) {
		deleteMemobjs(memobjs, numParams);
		clReleaseProgram(program);
		clReleaseCommandQueue(cmd_queue);
		clReleaseContext(context);
		return "could not create the kernel";
	}

	err = 0;
	// set the args values
	for (int i = 0; i < numParams; i++) {
		err |= clSetKernelArg(kernel, i, sizeof(cl_mem), (void *) &memobjs[i]);

		if (err != CL_SUCCESS) {
			deleteMemobjs(memobjs, numParams);
			clReleaseKernel(kernel);
			clReleaseProgram(program);
			clReleaseCommandQueue(cmd_queue);
			clReleaseContext(context);
			return "could not set the array args values";
		}
	}

	list<RegularParameter>::iterator it;
	int i = numParams;
	for (it = inputArgs.begin(); it != inputArgs.end(); it++) {       
		err |= clSetKernelArg(kernel, i, it->getSize(), it->getValue());
			i++;

		if (err != CL_SUCCESS) {
			deleteMemobjs(memobjs, numParams);
			clReleaseKernel(kernel);
			clReleaseProgram(program);
			clReleaseCommandQueue(cmd_queue);
			clReleaseContext(context);
			return "could not set the regular args values";
		}
	}

	// set work-item dimensions
	global_work_size[0] = numInstances;

	//check if need to pause (before kernel execution
	if (pauseExecution) {
		cout << "press any key to begin execution of " << kernelName
				<< " in mode " << Execution::toString(executionType);
		getchar();
	}

	// execute kernel
    //errcode_ret = clFinish(cmd_queue);
	if (executionType == Execution::NORMAL) {
		local_work_size[0] = 1;
		err = clEnqueueNDRangeKernel(cmd_queue, kernel, 1, NULL,
				global_work_size, NULL, 0, NULL, NULL);
	} else {
		err = clEnqueueNDRangeKernel(cmd_queue, kernel, 1, NULL,
				global_work_size, NULL, 0, NULL, NULL);
	}
    //
    //errcode_ret = clFinish(cmd_queue);
	if (err != CL_SUCCESS) {
		deleteMemobjs(memobjs, numParams);
		clReleaseKernel(kernel);
		clReleaseProgram(program);
		clReleaseCommandQueue(cmd_queue);
		clReleaseContext(context);
		return "could not execute kernel";
	}

	//check if need to pause (before) kernel execution
	if (pauseExecution) {
		cout << "press any key to end execution of " << kernelName
				<< " in mode " << Execution::toString(executionType);
		cout << endl;
		getchar();
	}

	// read output image
	for (int i = 0; i < numParams; i++) {
		ArrayParameter param = params[i];
		if (param.needToReturn()) {
            errcode_ret = clFinish(cmd_queue);
			err = clEnqueueReadBuffer(cmd_queue, memobjs[i], CL_TRUE, 0,
					param.getSize(), param.getParam(), 0, NULL, NULL);
			if (err != CL_SUCCESS) {
				deleteMemobjs(memobjs, numParams);
				clReleaseKernel(kernel);
				clReleaseProgram(program);
				clReleaseCommandQueue(cmd_queue);
				clReleaseContext(context);
				return "could not read output image";
			}
		}
	}

	// release kernel, program, and memory objects
	deleteMemobjs(memobjs, numParams);
	clReleaseKernel(kernel);
	clReleaseProgram(program);
	clReleaseCommandQueue(cmd_queue);
	clReleaseContext(context);
	delete memobjs;

	return "ok"; // success...
}

void KerenelExecutor::deleteMemobjs(cl_mem *memobjs, int n) {
	int i;
	for (i = 0; i < n; i++)
		clReleaseMemObject(memobjs[i]);
}


string KerenelExecutor::update_code_vec_type(const string& code ,Execution::Type executionType){

   string updated_code_str;
   updated_code_str = code;

   if (executionType == Execution::NORMAL) {
        size_t k=code.size();
        size_t found;
        found=updated_code_str.find(KERNEL_STRING);
        //assert(found == -1);
        while (found != -1) {
            updated_code_str.insert(found + KERNEL_STRING_SIZE,STRING_DISABLE_VECTORIZER);     
            found=updated_code_str.find(KERNEL_STRING,found+1);
        }
   }
   return updated_code_str;
}


string KerenelExecutor::getFileContent(const string& fileName) {

	// Load file
	struct stat stbuf;
	if (stat(fileName.c_str(), &stbuf) == -1) {
		cerr << "Could not start file '" << fileName << "'\n";
		exit(-1);
	}
	int contentStrSize = stbuf.st_size;
	char *contentStr = new char[contentStrSize + 1];
	assert(contentStr);

	ifstream inFile(fileName.c_str());
	if (!inFile) {
		cerr << "Failed to open '" << fileName << "'\n" << endl;
		exit(-1);
	}
    inFile.read(contentStr, contentStrSize);
    int acRead = inFile.gcount();
    assert(acRead < contentStrSize + 1);
	contentStr[acRead] = '\0';
	inFile.close();

	return contentStr;
}

cl_program KerenelExecutor::createProgramFromFile(cl_context context,
		const string& fileName, const string& includeFile,Execution::Type executionType) {

	// read header file
	string code = KerenelExecutor::getFileContent(includeFile);

	// read kernel file
	code += KerenelExecutor::getFileContent(fileName);

    // add nessesary vec_type_hint to kernel
    string updated_code = KerenelExecutor::update_code_vec_type(code,executionType);

	const char* codeBuff = updated_code.c_str();

	// Load the source into OpenCL driver
	cl_int err;
	cl_program prog = clCreateProgramWithSource(context, 1,
			(const char**) &codeBuff, NULL, &err);
	CHECK_ERRORS(err);

	return prog;
}
