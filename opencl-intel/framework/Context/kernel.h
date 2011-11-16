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

#include <cl_types.h>
#include <cl_object.h>
#include <Logger.h>
#include <cl_device_api.h>
#include <cl_objects_map.h>
#include "Device.h"
#include "cl_sys_defines.h"


namespace Intel { namespace OpenCL { namespace Framework {

	class Program;
	class DeviceProgram;
	class Kernel;
	class Context;

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
		cl_kernel_argument*		m_pArgs;
	};

	/**********************************************************************************************
	* Class name:	DeviceKernel
	*
	* Description:	represents a device kernel object
	* Author:		Uri Levy
	* Date:			January 2008
	**********************************************************************************************/	
    class DeviceKernel : public OCLObjectBase
	{
	public:
		
		// Constructor
		DeviceKernel(	Kernel *		pKernel,
						FissionableDevice *        pDevice,
						cl_dev_program  devProgramId,
						const char *	psKernelName,
						Intel::OpenCL::Utils::LoggerClient *	pLoggerClient,
						cl_err_code *	pErr );

		// Destructor
		~DeviceKernel();

		// get kernel id
		cl_dev_kernel GetId(){ return m_clDevKernel; }

		// Get device ID
		cl_int        GetDeviceId() const { return m_pDevice->GetId(); }

		// get kernel prototype
		const SKernelPrototype GetPrototype(){ return m_sKernelPrototype; }

		// compare between kernel's prototypes
		bool CheckKernelDefinition(DeviceKernel * pKernel);

	private:

		// device kernel id
		cl_dev_kernel	m_clDevKernel;

		// kernel prototype
		SKernelPrototype						m_sKernelPrototype;

		// parent kernel
		Kernel *								m_pKernel;
		
		// device to which the device kernel is associated
		FissionableDevice *						m_pDevice;

		// logger client
		DECLARE_LOGGER_CLIENT;
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

		KernelArg(	cl_uint uiIndex, 
					size_t szSize, 
					void * pValue, 
					cl_kernel_argument clKernelArgType);

		~KernelArg();

		cl_uint GetIndex() const { return m_uiIndex; }

        // return the size (in bytes) of the kernel arg's value
        // if Buffer / Image / ... returns sizeof(MemoryObject*)
		size_t GetSize() const { return m_szSize; }

        // returns the value of the kernel argument
		void * GetValue() const { return m_pValue; }

		bool	IsMemObject() const;

		//bool IsBuffer() const;
        //bool IsImage() const;

        bool IsSampler() const;

		bool IsLocalPtr() const;
		void ModifyValue(size_t szSize, void * pValue)
		{ 
			if ((m_clKernelArgType.type <= CL_KRNL_ARG_VECTOR) || (m_clKernelArgType.type == CL_KRNL_ARG_COMPOSITE))
				MEMCPY_S(m_pValue, m_szSize, pValue, szSize);
			else if (m_clKernelArgType.type == CL_KRNL_ARG_PTR_LOCAL)
				*((void**)m_pValue) = (void*)szSize;
		}

	private:

		// index of kernel argument
		cl_uint		m_uiIndex;

		// size (in bytes) of kernel argument
		size_t		m_szSize;

		// value of kernel argument
		void *		m_pValue;

		// type of kernel argument
		cl_kernel_argument	m_clKernelArgType;
	};

	/**********************************************************************************************
	* Class name:	Kernel
	*
	* Inherit:		OCLObject
	* Description:	represents a kernel object
	* Author:		Uri Levy
	* Date:			January 2008
	**********************************************************************************************/		
	class Kernel : public OCLObject<_cl_kernel_int>
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
		Kernel(Program * pProgram, const char * psKernelName, size_t szNumDevices, ocl_entry_points * pOclEntryPoints);

		/******************************************************************************************
		* Function: 	GetInfo    
		* Description:	get object specific information (inherited from OCLObject) the function 
		*				query the desirable parameter value from the device
		* Arguments:	iParamName [in]				parameter's name
		*				szParamValueSize [in out]	parameter's value size (in bytes)
		*				pParamValue [out]			parameter's value
		*				pszParamValueSizeRet [out]	parameter's value return size
		* Return value:	CL_SUCCESS - operation succeeded
		*				CL_INVALID_VALUE - if iParamName is not valid, or if size in bytes 
		*								   specified by szParamValueSize is < size of return type 
		*									and pParamValue is not NULL
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		cl_err_code	GetInfo(cl_int iParamName, 
							size_t szParamValueSize, 
							void * pParamValue, 
							size_t * pszParamValueSizeRet);


		/******************************************************************************************
		* Function: 	GetWorkGroupInfo    
		* Description:	returns information about the kernel object that may be specific to a 
		*				device
		* Arguments:	clDevice [in]				identifies a specific device in the list of 
		*											devices associated with
		*				iParamName [in]				parameter's name
		*				szParamValueSize [inout]	parameter's value size (in bytes)
		*				pParamValue [out]			parameter's value
		*				pszParamValueSizeRet [out]	parameter's value return size
		* Return value:	CL_INVALID_DEVICE - ckDevice is not valid device
		*				CL_INVALID_VALUE - iParamName is not valid parameter
		*				CL_SUCCESS - operation succeeded
		* Author:		Uri Levy
		* Date:			April 2008
		******************************************************************************************/
		cl_err_code	GetWorkGroupInfo(	FissionableDevice*		pDevice,
										cl_int      iParamName, 
										size_t      szParamValueSize,
										void *      pParamValue,
										size_t *    pszParamValueSizeRet);

		// create device kernels
		cl_err_code CreateDeviceKernels(DeviceProgram ** pDevicePrograms);

		// set kernel argument
		cl_err_code SetKernelArg(cl_uint uiIndex, size_t szSize, const void * pValue);

		// returns the number of arguments in the kernel
		size_t GetKernelArgsCount() const;

        // Return true if all kernel arguments were specified
        bool IsValidKernelArgs() const;

        // Return true if there is a successfully built program executable available for device
        bool IsValidExecutable(const FissionableDevice* pDevice) const;

		// get pointer to the kernel argument object of the uiIndex. if no available returns NULL;
		const KernelArg * GetKernelArg(size_t uiIndex) const;

        // Returns non zero handle.
        cl_dev_kernel GetDeviceKernelId(FissionableDevice* pDevice) const;

		// get kernel's name
		const char * GetName() const { return m_sKernelPrototype.m_psKernelName; }

		// get kernel's associated program
		const Program * GetProgram() const { return m_pProgram; }

		const Context * GetContext() const;

		///////////////////////////////////////////////////////////////////////////////////////////
		// OpenCL 1.2 functions
		///////////////////////////////////////////////////////////////////////////////////////////

		/******************************************************************************************
		* Function: 	clGetKernelArgInfo    
		* Description:	returns information about the arguments of a kernel.
		* Arguments:	argIndx [in]				is the argument index
		*				paramName [in]				specifies the argument information to query
		*				szParamValueSize [inout]	parameter's value size (in bytes)
		*				pParamValue [out]			parameter's value
		*				pszParamValueSizeRet [out]	parameter's value return size
		* Return value:	CL_INVALID_ARG_INDEX	- if arg_indx is not a valid argument index and param_name is
		*											not CL_KERNEL_ATTRIBUTES.
		*				CL_INVALID_VALUE		- if param_name is not valid, or if size in bytes specified by
		*											param_value size is < size of return type as described in
		*											table 5.17 and param_value is not NULL.
		*				CL_KERNEL_ARG_INFO_NOT_AVAILABLE - if the argument information is not available
		*											for kernel.
		* Author:		Evgeny Fiksman
		* Date:			August 2011
		******************************************************************************************/		        
		cl_err_code GetKernelArgInfo (	cl_uint argIndx,
										cl_kernel_arg_info paramName,
										size_t      szParamValueSize,
										void *      pParamValue,
										size_t *    pszParamValueSizeRet);

	protected:
		/******************************************************************************************
		* Function: 	~Kernel
		* Description:	The Kernel class destructor
		* Arguments:		
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/			
		virtual ~Kernel();

		DeviceKernel* GetDeviceKernel(const FissionableDevice* pDevice) const;


		//Kernel prototype
		cl_err_code SetKernelPrototype(SKernelPrototype sKernelPrototype);
		SKernelPrototype						m_sKernelPrototype;

		Program *								m_pProgram;
		size_t                                  m_szAssociatedDevices;

		// Kernel arguments
		KernelArg**                             m_ppArgs;

		// Per-device kernels
		DeviceKernel**                          m_ppDeviceKernels;

		// To ensure all args have been set
		Intel::OpenCL::Utils::AtomicCounter     m_numValidArgs;

		// For OpenCL 1.2
		static const char*						g_szArgTypeNames[];

	};


}}}

