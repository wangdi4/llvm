/*
 * ArrayParameter.cpp
 *
 *  Created on: Aug 9, 2009
 *      Author: openCL
 */

#include "ArrayParameter.h"


ArrayParameter::ArrayParameter(void* param, size_t size, Access access) {

		this->param = param;
		this->size = size;

		switch (access) {

		case READ_ONLY:
			this->memFlags = CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR;
			this->returnParam = false;
			this->hostPtr = param;
			break;

		case READ_WRITE:
			this->memFlags = CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR;
			this->returnParam = true;
			this->hostPtr = param;
			break;

		}

	}

	void *ArrayParameter::getParam() const {
		return param;
	}

	size_t ArrayParameter::getSize() const {
		return size;
	}

	cl_mem_flags ArrayParameter::getCLMem() const {
		return memFlags;
	}

	void *ArrayParameter::getHostPtr() const {
		return hostPtr;
	}

	bool ArrayParameter::needToReturn() const {
		return returnParam;
	}
