/*
 * ArrayParameter.h
 *
 *  Created on: Aug 9, 2009
 *      Author: openCL
 */

#ifndef ARRAYPARAMETER_H_
#define ARRAYPARAMETER_H_

#include <cstddef>

#ifdef WINDOWS
#include <CL/cl.h>
#else
#include <opencl.h>
#endif

class ArrayParameter {

public:

	typedef enum {

		READ_ONLY,

		READ_WRITE

	} Access;

	ArrayParameter(void* param, size_t size, Access access);

	void *getParam() const;

	size_t getSize() const;

	cl_mem_flags getCLMem() const;

	void *getHostPtr() const;

	bool needToReturn() const;

private:

	void *param;
	cl_mem_flags memFlags;
	size_t size;
	void *hostPtr;
	bool returnParam;
};

#endif /* ARRAYPARAMETER_H_ */
