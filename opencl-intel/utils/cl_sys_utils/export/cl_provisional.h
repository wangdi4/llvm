#ifndef __CL_PROVISIONAL_H__
#define __CL_PROVISIONAL_H__

#include "CL/cl.h"

#include "provisional_malloc.h"

template <>
inline void provisionalDeleteObjectByType<cl_mem>(cl_mem data)
{
    PROV_DEBUG_PRINT("deleteObjectByType<cl_mem> calling clReleaseMemObject\n");
    clReleaseMemObject(data);
}

template <>
inline void provisionalDeleteObjectByType<cl_context>(cl_context data)
{
	PROV_DEBUG_PRINT("deleteObjectByType<cl_context> calling clReleaseContext\n");
    clReleaseContext(data);
}

template <>
inline void provisionalDeleteObjectByType<cl_command_queue>(cl_command_queue data)
{
	PROV_DEBUG_PRINT("deleteObjectByType<cl_command_queue> calling clReleaseCommandQueue\n");
    clReleaseCommandQueue(data);
}

template <>
inline void provisionalDeleteObjectByType<cl_program>(cl_program data)
{
	PROV_DEBUG_PRINT("deleteObjectByType<cl_program> calling clReleaseProgram\n");
    clReleaseProgram(data);
}

template <>
inline void provisionalDeleteObjectByType<cl_kernel>(cl_kernel data)
{
	PROV_DEBUG_PRINT("deleteObjectByType<cl_kernel> calling clReleaseKernel\n");
    clReleaseKernel(data);
}

#endif // __CL_PROVISIONAL_H__

