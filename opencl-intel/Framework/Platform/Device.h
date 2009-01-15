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
//  Device.h
//  Implementation of the Class Device
//  Created on:      10-Dec-2008 2:08:23 PM
//  Original author: ulevy
///////////////////////////////////////////////////////////////////////////////////////////////////

#if !defined(_OCL_DEVICE_H_)
#define _OCL_DEVICE_H_

#include <cl_types.h>
#include <cl_object.h>
#include <logger.h>
#include <cl_device_api.h>
#include <map>

namespace Intel { namespace OpenCL { namespace Framework {

	// Froward declarations
	class IBuildDoneObserver;

	/**********************************************************************************************
	* Class name:	Device
	*
	* Inherit:		OCLObject
	* Description:	This object is a gate from the framework into the openCL device driver that is 
	*				implemented by a seperated library.
	* Author:		Uri Levy
	* Date:			December 2008
	**********************************************************************************************/		
	class Device : public OCLObject
	{
	public:

		/******************************************************************************************
		* Function: 	Device
		* Description:	The Device class constructor
		* Arguments:	
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/		
		Device();

		/******************************************************************************************
		* Function: 	~Device
		* Description:	The OCLObject class destructor
		* Arguments:		
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/			
		virtual ~Device();

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
		cl_err_code	GetInfo(cl_int		param_name, 
							size_t		param_value_size, 
							void *		param_value, 
							size_t *	param_value_size_ret);

		/******************************************************************************************
		* Function: 	InitDevice    
		* Description:	Initialize OpenCL device
		* Arguments:	pwcDllPath [in]		full path of device driver's dll
		* Return value:	CL_SUCCESS - operation succeded
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		cl_err_code InitDevice(const wchar_t * pwcDllPath);

		/******************************************************************************************
		* Function: 	CheckProgramBinary    
		* Description:	Check if the binary is a valid binary
		* Arguments:	szBinSize [in] 	- binary size (in bytes)
		*				pBinData [in]	- pointer to the binary's data
		* Return value:	CL_SUCCESS - operation succeded, the binary is valid
		* Author:		Uri Levy
		* Date:			January 2009
		******************************************************************************************/
		cl_err_code CheckProgramBinary(size_t szBinSize, const void* pBinData);

		cl_err_code CreateProgram(	size_t				szBinSize,
									const void*			pBinData,
									cl_dev_binary_prop	clBinProp,
									cl_dev_program *	clProg);

		cl_err_code BuildProgram(	cl_dev_program			clProg,
									const cl_char *			pcOptions,
									IBuildDoneObserver *	pBuildDoneObserver );

		cl_err_code GetProgramBinary(	cl_dev_program	clDevProg, 
										size_t			szBinSize, 
										void *			pBin,
										size_t *		pszBinSizeRet );

	private:

		///////////////////////////////////////////////////////////////////////////////////////////
		// callback functions
		//////////////////////////////////////////////////////////////////////////////////////////
		
		// cl_dev_call_backs
		static void BuildStatusUpdate(cl_dev_program clDevProg, void * pData, cl_build_status clBuildStatus);
		static void CmdStatusChanged(cl_dev_cmd_id cmd_id, cl_int cmd_status, cl_int status_result);

		// cl_dev_log_descriptor
		static cl_int CreateDeviceLogClient(cl_int device_id, wchar_t* client_name, cl_int * client_id);
		static cl_int ReleaseDeviceLogClient(cl_int client_id);
		static cl_int DeviceAddLogLine(cl_int client_id, cl_int log_level, const wchar_t* IN source_file, const wchar_t* IN function_name, cl_int IN line_num, const wchar_t* IN message, ...);

		///////////////////////////////////////////////////////////////////////////////////////////
		// class private members
		///////////////////////////////////////////////////////////////////////////////////////////

		std::map<cl_dev_program, IBuildDoneObserver *>	m_mapBuildDoneObservers;	// holds pointer to notification functions
																					// for each device program object
		
		static cl_int							m_iNextClientId;		// hold the next client logger id

		static std::map<cl_int, Intel::OpenCL::Utils::LoggerClient*>	m_mapDeviceLoggerClinets; // OpenCL device's logger clients

		Intel::OpenCL::Utils::LoggerClient *	m_pLoggerClient;		// device's class logger client

		cl_dev_entry_points						m_clDevEntryPoints;		// device's entry points
		
		cl_dev_call_backs						m_clDevCallBacks;		// device's call backs

		cl_dev_log_descriptor					m_clDevLogDescriptor;	// device's log descriptor
	};


}}};


#endif //_OCL_DEVICE_H_