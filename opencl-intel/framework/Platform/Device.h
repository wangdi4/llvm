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
#include <cl_synch_objects.h>
#include <cl_device_api.h>
#include <cl_dynamic_lib.h>
#include <map>

namespace Intel { namespace OpenCL { namespace Framework {

	// Froward declarations
	class IBuildDoneObserver;
	class ICmdStatusChangedObserver;
	class FECompiler;

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
		* Description:	Load OpenCL device library
		* Arguments:	pwcDllPath [in]		full path of device driver's dll
		* Return value:	CL_SUCCESS - operation succeded
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		cl_err_code InitDevice(const char * psDeviceAgentDllPath, ocl_entry_points * pOclEntryPoints);

		/******************************************************************************************
		* Function: 	CreateInstance    
		* Description:	Create a device agent instance
		* Arguments:	
		* Return value:	CL_SUCCESS - operation succeded
		* Author:		Arnon Peleg
		* Date:			June 2009
		******************************************************************************************/
        cl_err_code CreateInstance();

		/******************************************************************************************
		* Function: 	CloseDeviceInstance
		* Description:	Close the instance that was created by CreateInstance.
        *               Local members of the device class may not be valid until next call to CreateInstance
		* Arguments:	
		* Return value:	CL_SUCCESS - operation succeded
		* Author:		Arnon Peleg
		* Date:			June 2009
		******************************************************************************************/
        cl_err_code CloseDeviceInstance();

        /******************************************************************************************
		* Function: 	SetFECompiler    
		* Description:	Set the front-end compiler to the device
		* Arguments:	pFECompiler [in]	pointer to the front-end compiler
		* Return value:	void
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		void SetFECompiler(FECompiler * pFECompiler) { m_pFECompiler = pFECompiler; }

		/******************************************************************************************
		* Function: 	GetFECompiler    
		* Description:	Get the front-end compiler of the device
		* Arguments:	N/A
		* Return value:	pointer to the front-end compiler (null if compiler not initialized)
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		const FECompiler * GetFECompiler(){ return m_pFECompiler; }

		///////////////////////////////////////////////////////////////////////////////////////////
		// Device API functions
		//////////////////////////////////////////////////////////////////////////////////////////

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
									const char *			pcOptions,
									IBuildDoneObserver *	pBuildDoneObserver );

		cl_err_code GetProgramBinary(	cl_dev_program	clDevProg, 
										size_t			szBinSize, 
										void *			pBin,
										size_t *		pszBinSizeRet );

		cl_err_code GetBuildLog(cl_dev_program	clDevProg,
								size_t			szSize,
								char*			psLog,
								size_t*			pszSizeRet);

		cl_err_code GetKernelId(	cl_dev_program	clDevProg,
									const char *	psKernelName,
									cl_dev_kernel *	pclKernel );

		cl_err_code GetProgramKernels(	cl_dev_program	clDevProg,
										cl_uint			uiNumKernels,
										cl_dev_kernel *	pclKernels,
										cl_uint *		puiNumKernelsRet );

		cl_err_code GetKernelInfo(	cl_dev_kernel		clKernel,
									cl_dev_kernel_info	clParam,
									size_t				szValueSize,
									void *				pValue,
									size_t *			pValueSizeRet );

		cl_err_code CreateMemoryObject(	cl_dev_mem_flags		clFlags,
										const cl_image_format*	pclFormat,
										cl_uint					uiDimCount,
										const size_t *			pszDim,
										void *					pBufferPtr,
										const size_t *			pszPitch,
										cl_dev_host_ptr_flags		hstFlags,
										cl_dev_mem *			pMemObj );

		cl_err_code DeleteMemoryObject(cl_dev_mem clMemObj);

		cl_err_code CreateMappedRegion(cl_dev_cmd_param_map * pMapParams);

		cl_err_code ReleaseMappedRegion(cl_dev_cmd_param_map * pMapParams);

		cl_err_code UnloadCompiler(void);

		cl_err_code GetSupportedImageFormats(	cl_dev_mem_flags       clDevFlags,
												cl_dev_mem_object_type clDevImageType,
												cl_uint                uiNumEntries,
												cl_image_format *      pclFormats,
												cl_uint *              puiNumEntriesRet);

		///////////////////////////////////////////////////////////////////////////////////////////
		// Command list methods
		///////////////////////////////////////////////////////////////////////////////////////////
		cl_err_code CreateCommandList(cl_dev_cmd_list_props clDevCmdListProps, cl_dev_cmd_list * pclDevCmdList);
		cl_err_code RetainCommandList(cl_dev_cmd_list clDevCmdList);
		cl_err_code ReleaseCommandList(cl_dev_cmd_list clDevCmdList);
		cl_err_code CommandListExecute(	cl_dev_cmd_list clDevCmdList, 
										cl_dev_cmd_desc * clDevCmdDesc, 
										cl_uint uiCount,
										ICmdStatusChangedObserver ** ppCmdStatusChangedObserver);
        cl_err_code FlushCommandList(cl_dev_cmd_list clDevCmdList);

	private:

		///////////////////////////////////////////////////////////////////////////////////////////
		// callback functions
		///////////////////////////////////////////////////////////////////////////////////////////
		
		// cl_dev_call_backs
		static void BuildStatusUpdate(cl_dev_program clDevProg, void * pData, cl_build_status clBuildStatus);
		static void CmdStatusChanged(cl_dev_cmd_id cmd_id, void * pData, cl_int cmd_status, cl_int status_result, cl_ulong timer);

		// cl_dev_log_descriptor
		static cl_int CreateDeviceLogClient(cl_int device_id, wchar_t* client_name, cl_int * client_id);
		static cl_int ReleaseDeviceLogClient(cl_int client_id);
		static cl_int DeviceAddLogLine(cl_int client_id, cl_int log_level, const wchar_t* IN source_file, const wchar_t* IN function_name, cl_int IN line_num, const wchar_t* IN message, ...);

		///////////////////////////////////////////////////////////////////////////////////////////
		// class private members
		///////////////////////////////////////////////////////////////////////////////////////////

		Utils::OclDynamicLib							m_dlModule;
		// front-end compiler
		FECompiler *							        m_pFECompiler;

		// holds pointer to notification functions for each device program object
		std::map<cl_dev_program, IBuildDoneObserver *>	m_mapBuildDoneObservers;
		
        Intel::OpenCL::Utils::OclMutex          m_muDeviceCloseLock;
		bool                                    m_bIsDeviceOpened;
        
        // Pointer to the device GetInfo function.
        fn_clDevGetDeviceInfo*                  m_pFnClDevGetDeviceInfo;
        
        static cl_int							m_iNextClientId;		// hold the next client logger id

		static std::map<cl_int, Intel::OpenCL::Utils::LoggerClient*>	m_mapDeviceLoggerClinets; // OpenCL device's logger clients

		cl_dev_entry_points						m_clDevEntryPoints;		// device's entry points
		
		cl_dev_call_backs						m_clDevCallBacks;		// device's call backs

		cl_dev_log_descriptor					m_clDevLogDescriptor;	// device's log descriptor

		DECLARE_LOGGER_CLIENT;											// device's class logger client

	};


}}};
#endif //_OCL_DEVICE_H_
