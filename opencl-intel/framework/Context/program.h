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
//  program.h
//  Implementation of the Program class
//  Created on:      10-Dec-2008 2:08:23 PM
//  Original author: ulevy
///////////////////////////////////////////////////////////////////////////////////////////////////

#if !defined(_OCL_PROGRAM_H_)
#define _OCL_PROGRAM_H_

#include <cl_types.h>
#include <cl_object.h>
#include <observer.h>
#include <logger.h>
#include <cl_synch_objects.h>
#include <map>

namespace Intel { namespace OpenCL { namespace Framework {

	class Context;
	class ProgramBinary;
	class Device;
	class Kernel;
	class OCLObjectsMap;

	/**********************************************************************************************
	* Enumaration:	EProgramBuiltState
	*
	* Description:	define the current state of the program
	*
	* clCreateProgramWithSource                    clCreateProgramWithBinaries
	*             [PST_NONE]                                 [PST_NONE]
	* (1) [SOURC] [PST_SOURCE_CODE]                (1) [IR]  [PST_INTERMIDIATE]
	* (2) [IR]    [PST_INTERMIDIATE_FROM_SOURCE]   (2) [BIN] [PST_BINARY]
	* (3) [BIN]   [PST_BINARY_FROM_SOURCE]
	*
	*
	* Values:		PST_NONE		 - Empty program
	*				PST_SOURCE_CODE	 - Create program with source
	*				PST_INTERMIDIATE - Device intermidiates are ready
	*				PST_BINARY		 - Program binaries are ready
	**********************************************************************************************/	
	enum EProgramBuiltState
	{
		PST_NONE = 0,					// initial state
		PST_SOURCE_CODE,				// program created with source (before build)
		PST_INTERMIDIATE_FROM_SOURCE,	// after front-end compiler
		PST_INTERMIDIATE,				// program created with binary (before build)
		PST_BINARY_FROM_SOURCE,			// after build from source
		PST_BINARY						// after builf from binary
	};

	/**********************************************************************************************
	* Class name:	Program
	*
	* Inherit:		OCLObject
	* Description:	represents a program class
	* Author:		Uri Levy
	* Date:			December 2008
	**********************************************************************************************/		
	class Program : public OCLObject, IBuildDoneObserver, IFrontendBuildDoneObserver
	{
	public:

		/******************************************************************************************
		* Function: 	Program
		* Description:	The Program class constructor
		* Arguments:	pContext [in]	holds the parent conetxt
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/		
		Program(Context * pContext);

		/******************************************************************************************
		* Function: 	~Program
		* Description:	The Program class destructor
		* Arguments:		
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/			
		virtual ~Program();

		// return the context to which the program belongs
		const Context * GetContext() const { return m_pContext; }

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

		cl_err_code GetBuildInfo(	cl_device_id clDevice, 
									cl_program_build_info clParamName, 
									size_t szParamValueSize, 
									void * pParamValue, 
									size_t * pzsParamValueSizeRet);

		// relase the program object and its reosurces
		cl_err_code Release();

		// add source code to the program object
		cl_err_code AddSource(cl_uint uiCount, const char ** ppcStrings, const size_t * pszLengths);

		// add binary to the program object, each binary is assign to specific device
		cl_err_code AddBinaries(cl_uint					uiNumDevices, 
								Device **				ppDevices, 
								const size_t *			pszBinariesSize, 
								const unsigned char **	ppBinariesData,
								cl_int *				piBinaryStatus);

		// build program
		cl_err_code Build(	cl_uint					uiNumDevices,
							const cl_device_id *	pclDeviceList,
							const char *			pcOptions,
							void (*pfnNotify)(cl_program clProgram, void * pUserData),
							void *					pUserData );

		// create new kernel object
		cl_err_code CreateKernel(const char * pscKernelName, Kernel ** ppKernel);

		// create all kernels from the program object
		cl_err_code CreateAllKernels(cl_uint uiNumKernels, cl_kernel * pclKernels, cl_uint * puiNumKernelsRet);

		// get the kernels associated to the program
		cl_err_code GetKernels(cl_uint uiNumKernels, Kernel ** ppKernels, cl_uint * puiNumKernelsRet);

		// remove kernel from program
		cl_err_code RemoveKernel(cl_kernel clKernel);

		// IBuildDoneObserver
		virtual cl_err_code NotifyBuildDone(cl_device_id device, cl_build_status build_status);
		
		// IFrontendBuildDoneObserver
		virtual cl_err_code NotifyFEBuildDone(cl_device_id device, size_t szBinSize, void * pBinData, const char* pLogStr);

	private:

		// check if the devices associated to the program and if thier binaries are valid binaries
		cl_err_code CheckBinaries(cl_uint uiNumDevices, const cl_device_id * pclDevices);

		// build binaries
		cl_err_code BuildBinaries(	cl_uint					uiNumDevices,
									const cl_device_id *	pclDevices, 
									const char *			pcOptions);

		// build from source
		cl_err_code BuildSource(	cl_uint					uiNumDevices,
									const cl_device_id *	pclDevices, 
									const char *			pcOptions);

		// check that the front-end compiler is available for all devices
		cl_err_code CheckFECompilers(cl_uint uiNumDevices, const cl_device_id * pclDevices);

		// check if the kernel with name psKernelName already attached to the program
		// if ppKernel != null return the kernel
		bool IsKernelExists(const char * psKernelName, Kernel ** ppKernelRet);
								
		//user notification callback for build completion
		void									(*m_pfnNotify)(cl_program clProgram, void * pUserData);
		
		void *									m_pUserData;
		
		EProgramBuiltState						m_eProgramState;	// program state

		Context *								m_pContext;			// parent context

		///////////////////////////////////////////////////////////////////////////////////////////
		// Build varaibles
		///////////////////////////////////////////////////////////////////////////////////////////
	
		char *									m_pBuildOptions;

		cl_uint									m_uiFeBuildLogLength;

		char *									m_pcFeBuildLog;

		///////////////////////////////////////////////////////////////////////////////////////////
		// Source code varaibles
		///////////////////////////////////////////////////////////////////////////////////////////
		
		cl_uint									m_uiStcStrCount;	// number of strings in the source code
		
		const char **							m_ppcSrcStrArr;		// source code - set of strings
		
		size_t *								m_pszSrcStrLengths;	// for each string - the lengths of the string

		cl_build_status							m_clSourceBuildStatus; // front-end build status

		///////////////////////////////////////////////////////////////////////////////////////////

		OCLObjectsMap *							m_pKernels;			// associated kernels

		// ------------------------------------- DEVICES -------------------------------------
		
		std::map<cl_device_id, Device*>			m_mapDevices;		// associated devices

		cl_uint									m_uiDevicesCount;

		Device **								m_ppDevices;

		cl_device_id *							m_pDeviceIds;

		// -----------------------------------------------------------------------------------
		
		std::map<cl_device_id, ProgramBinary*>	m_mapIntermidiates;	// program's IRs

        Intel::OpenCL::Utils::OclMutex			m_csIntermidateMap;	// protect m_mapIntermidiates
	
		Intel::OpenCL::Utils::OclMutex			m_BuildNotifyCS;	// Used with the Build wait condition

        Intel::OpenCL::Utils::OclCondition	    m_BuildWaitCond;	// Used by host thread to wait for build done if pfn_notify is NULL

		bool									m_bBuildFinished;


	};


}}};


#endif //_OCL_PROGRAM_H_