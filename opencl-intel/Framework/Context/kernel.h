// Copyright (c) 2006-2007 Intel Corporation
// All rights reserved.
// 
// WARRANTY DISCLAIMER
// 
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

#pragma once
///////////////////////////////////////////////////////////////////////////////////////////////////
//  kernel.h
//  Implementation of the Kernel class
//  Created on:      10-Dec-2008 2:08:23 PM
//  Original author: ulevy
///////////////////////////////////////////////////////////////////////////////////////////////////

#if !defined(_OCL_KERNEL_H_)
#define _OCL_KERNEL_H_

#include <cl_types.h>
#include <cl_object.h>
#include <logger.h>
#include <cl_device_api.h>
#include <cl_objects_map.h>

namespace Intel { namespace OpenCL { namespace Framework {

	class Program;
	class ProgramBinary;
	class Kernel;
	class Device;

	/**********************************************************************************************
	* Class name:	SKernelPrototype
	*
	* Description:	contains information on kernel prototype
	* Members:		m_psKernelName	- the name of the kernel
	*				m_uiArgsCount	- number of arguments in the kernel
	*				m_pArgs			- list of all kernel's arguments
	* Author:		Uri Levy
	* Date:			January 2008
	**********************************************************************************************/	
	struct SKernelPrototype
	{
		char *					m_psKernelName;
		cl_uint					m_uiArgsCount;
		cl_kernel_arg_type *	m_pArgs;
	};

	/**********************************************************************************************
	* Class name:	DeviceKernel
	*
	* Description:	represents a device kernel object
	* Author:		Uri Levy
	* Date:			January 2008
	**********************************************************************************************/	
	class DeviceKernel
	{
	public:
		
		// Constructor
		DeviceKernel(	Kernel *		pKernel,
						ProgramBinary *	pProgBin, 
						const char *	psKernelName,
						cl_err_code *	pErr );

		// Destructor
		~DeviceKernel();

		// get kernel id
		const cl_dev_kernel GetId(){ return m_clDevKernel; }

		// get kernel prototype
		const SKernelPrototype GetPrototype(){ return m_sKernelPrototype; }

		// compare between kernel's prototypes
		bool CheckKernelDefinition(DeviceKernel * pKernel);


	private:

		// device kernel id
		cl_dev_kernel							m_clDevKernel;

		// kernel prototype
		SKernelPrototype						m_sKernelPrototype;

		// parent kernel
		Kernel *								m_pKernel;
		
		// device to which the device kernel associated
		Device *								m_pDevice;

		// logger client
		Intel::OpenCL::Utils::LoggerClient *	m_pLoggerClient;	// logger client

	};

	/**********************************************************************************************
	* Class name:	KernelArg
	*
	* Inherit:		
	* Description:	represents a kernel argument object
	* Author:		Uri Levy
	* Date:			January 2008
	**********************************************************************************************/	
	class KernelArg
	{
	public:

		KernelArg(cl_uint uiIndex, size_t szSize, void * pValue);

		~KernelArg();

		const cl_uint GetIndex(){ return m_uiIndex; }

		const size_t GetSize(){ return m_szSize; }

		const void * GetValue(){ return m_pValue; }

		const bool IsMemObject(){ return sizeof(cl_mem) == m_szSize; }
		
		const bool IsSampler(){ return sizeof(cl_sampler) == m_szSize; }

	private:

		cl_uint		m_uiIndex;

		size_t		m_szSize;

		void *		m_pValue;
	};

	/**********************************************************************************************
	* Class name:	Kernel
	*
	* Inherit:		OCLObject
	* Description:	represents a kernel object
	* Author:		Uri Levy
	* Date:			January 2008
	**********************************************************************************************/		
	class Kernel : public OCLObject
	{
	public:

		/******************************************************************************************
		* Function: 	Kernel
		* Description:	The Kernel class constructor
		* Arguments:	pProgram [in]		- associated program object
		*				psKernelName [in]	- kernel's name
		* Author:		Uri Levy
		* Date:			January 2008
		******************************************************************************************/		
		Kernel(Program * pProgram, const char * psKernelName);

		/******************************************************************************************
		* Function: 	~Kernel
		* Description:	The Kernel class destructor
		* Arguments:		
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/			
		virtual ~Kernel();

		/******************************************************************************************
		* Function: 	GetInfo    
		* Description:	get object specific information (inharited from OCLObject) the function 
		*				query the desirable parameter value from the device
		* Arguments:	param_name [in]				parameter's name
		*				param_value_size [inout]	parameter's value size (in bytes)
		*				param_value [out]			parameter's value
		*				param_value_size_ret [out]	parameter's value return size
		* Return value:	CL_SUCCESS - operation succeded
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		cl_err_code	GetInfo(cl_int param_name, size_t param_value_size, void * param_value, size_t * param_value_size_ret);

		// relese kernel object
		cl_err_code Release();

		// create device kernels
		cl_err_code CreateDeviceKernels(cl_uint uiBinariesCount, ProgramBinary ** ppBinaries);

		/******************************************************************************************
		* Function: 	AddDeviceKernel
		* Description:	Add new device kernels to the current kernel object. after clCreateKernel
		*				is called the device kerenl id was
		*				taken from the device so we assume that it's already exists.
		*				the funciton check that the device wasn't added before to the kernel and
		*				that its definition fit to the other device kernels in the kerenl object
		* Arguments:	clDeviceKernel [in]	- device kernel id
		*				pProgBin [in]		- program binary
		* Author:		Uri Levy
		* Date:			January 2008
		******************************************************************************************/
		cl_err_code AddDeviceKernel(cl_dev_kernel clDeviceKernel, ProgramBinary *pProgBin);

		// set kernel argument
		cl_err_code SetKernelArg(cl_uint uiIndex, size_t szSize, const void * pValue);

		// get kernel's name
		const char * GetName(){ return m_psKernelName; }

		// get kernel's associated program
		const Program * GetProgram(){ return m_pProgram; }

	private:

		cl_err_code SetKernelPrototype(SKernelPrototype sKernelPrototype);

		// kernel's prototype
		SKernelPrototype						m_sKernelPrototype;

		// kernel name
		char *									m_psKernelName;

		// associated program
		Program *								m_pProgram;

		// list of all device kernels
		OCLObjectsMap *							m_pDeviceKernels;

		// list of kernel arguments
		std::map<cl_uint, KernelArg*>			m_mapKernelArgs;

		// logger client
		Intel::OpenCL::Utils::LoggerClient *	m_pLoggerClient;	// logger client

	};


}}};


#endif //_OCL_PROGRAM_H_