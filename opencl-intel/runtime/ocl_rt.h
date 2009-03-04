#pragma once

#include "cl.h"
#include <cl_device_api.h>

#include <intrin.h>
#include <xmmintrin.h>

/*****************************************************************************************************************************
*		Global declarations
*****************************************************************************************************************************/

#define __USING_FIBERS__

#define __kernel	__declspec(dllexport)

#pragma warning(disable:4042) // disable warning caused by static appearance in function parameters
#define __global	static
#define __local		__declspec(thread)

#define MAX_ARG_COUNT		256
#ifndef __USING_FIBERS__
#define	STACK_SEPARATOR	0x0BAADF00D
#endif

// Defines information about working dimentions
struct SWorkDim
{
	unsigned int	iWorkDim;						// Working Dimension
	unsigned int	viOffset[MAX_WORK_DIM];		// Global offset
	unsigned int	viGlobalSize[MAX_WORK_DIM];	// Global size of kernel
	unsigned int	viLocalSize[MAX_WORK_DIM];		// Local size of WG
};

// Defines Work-Group related information
struct SWGinfo
{
	SWorkDim*	pWorkingDim;
	int			viNumGroups[MAX_WORK_DIM];		// Total number of groups
	int			viGroupId[MAX_WORK_DIM];		// Current group ID's
};

// Defines Work-Item related information
struct SWIinfo
{
	int		viLocalId[MAX_WORK_DIM];	// Local idenitfier of the Work-Item
	int		viGlobalId[MAX_WORK_DIM];	// Global idenitfier of the Work-Item
};

// Defines information for Work-Group execution information
struct SKernelInfo
{
	const void*		pfnKernelFunc;			// Pointer to kernel function
	size_t			stParamSize;			// Size of parameter buffer to be passed to the kernel
	void*			pParams;				// Pointer to vector that holds kernel execution parameters
	unsigned int	uiExpLocalCount;		// Number of explicit local memory buffers
	void*			*pLocalPtr;				// Location of local memory sizes to be substituded by
											// local (Work-Group) pointers
	unsigned int	uiImpLocalCount;		// Number of implicit local memory buffers
	bool			bWIInfoSupported;		// WI info structure should be passed to the kernel by parameter
};

// Defines paramters that should be passed to _KernelExecute function
struct	SWIExecutionParam
{
	const SKernelInfo*	psWGExecParam;			// Pointer to a kernel execution information
	const SWGinfo*		psWGInfo;				// Pointer to a WG information
	const SWIinfo*		psWIInfo;				// Pointer to specific WI information
	void*				*pWGFiber;				// Pointer to where Fiber handle will be stored
};

/*****************************************************************************************************************************
*		Work-Item functions
*****************************************************************************************************************************/

#include "wi_functions.h"

/*****************************************************************************************************************************
*		OpenCL specific types decalaration
*****************************************************************************************************************************/

#include "float4.h"

/*****************************************************************************************************************************
*		OpenCL specific types decalaration
*****************************************************************************************************************************/

typedef struct _image2d_t
{
} *image2d_t;