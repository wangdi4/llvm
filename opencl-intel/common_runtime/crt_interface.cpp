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
//
//  Original author: rjiossy
///////////////////////////////////////////////////////////
#include "crt_interface.h"
#include "crt_module.h"
#include "crt_internals.h"
#include <cl_synch_objects.h>
#include <algorithm>
#include <stdio.h>

/// Globally Initialized Variable


namespace OCLCRT
{
	CrtModule crt_ocl_module;
};


	/// Macros
#define isValidPlatform(X) ((X) == OCLCRT::crt_ocl_module.m_crtPlatformId || NULL == (X))

/// Definined CRT CL handles

_cl_platform_id_crt::_cl_platform_id_crt() 
{ 
	dispatch = &OCLCRT::crt_ocl_module.m_icdDispatchMgr.m_crtDispatchTable; 
};

_cl_context_crt::_cl_context_crt() 
{ 
	dispatch = &OCLCRT::crt_ocl_module.m_icdDispatchMgr.m_crtDispatchTable; 
};

_cl_program_crt::_cl_program_crt() 
{ 
	dispatch = &OCLCRT::crt_ocl_module.m_icdDispatchMgr.m_crtDispatchTable; 
};

_cl_kernel_crt::_cl_kernel_crt() 
{ 
	dispatch = &OCLCRT::crt_ocl_module.m_icdDispatchMgr.m_crtDispatchTable; 
};

_cl_command_queue_crt::_cl_command_queue_crt()
{
	dispatch = &OCLCRT::crt_ocl_module.m_icdDispatchMgr.m_crtDispatchTable; 
};

_cl_mem_crt::_cl_mem_crt()
{
	dispatch = &OCLCRT::crt_ocl_module.m_icdDispatchMgr.m_crtDispatchTable; 
};

_cl_event_crt::_cl_event_crt()
{
	dispatch = &OCLCRT::crt_ocl_module.m_icdDispatchMgr.m_crtDispatchTable; 
};

/// Definined CRT CL API


void * CL_API_CALL clGetExtensionFunctionAddress(const char *funcname)
{
	if (funcname && !strcmp(funcname,"clIcdGetPlatformIDsKHR"))
	{
		return ((void*)clGetPlatformIDs);
	}
	if (funcname && !strcmp(funcname,"clCreateSubDevicesEXT"))
	{
		return ((void*)clCreateSubDevicesEXT);
	}
	return NULL;
};
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clGetPlatformIDs(cl_uint num_entries, cl_platform_id * platforms, cl_uint * num_platforms) 
{		
	if ( ((0 == num_entries) && (NULL != platforms)) ||
		 ((NULL == num_platforms) && (NULL == platforms)) )
	{
		return CL_INVALID_VALUE;
	}
	
	if (CRT_FAIL == OCLCRT::crt_ocl_module.Initialize())
	{
		return CL_OUT_OF_HOST_MEMORY;
	}
	
	if (num_platforms != NULL)	
		*num_platforms = 1;

	if (platforms != NULL && num_entries >= 1)
	{
		platforms[0] = OCLCRT::crt_ocl_module.m_crtPlatformId;
	}

	return CL_SUCCESS;
};
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clGetPlatformInfo(
	cl_platform_id		platform,
	cl_platform_info	param_name, 
	size_t				param_value_size, 
	void*				param_value, 
	size_t*				param_value_size_ret)
{
    size_t    RetSize = 0;

    if( ( platform != NULL) && !isValidPlatform( platform ) )
    {
        return CL_INVALID_PLATFORM;
    }

    switch ( param_name )
    {
        case CL_PLATFORM_PROFILE:
            // strnlen_s calculates without null terminator.
            RetSize = strnlen_s( "FULL_PROFILE", MAX_STRLEN );
            RetSize++;    
            if( param_value )
            {
                if( RetSize > param_value_size )
                {
                    return CL_INVALID_VALUE;
                }
                strncpy_s( ( char * )param_value, RetSize, "FULL_PROFILE", RetSize ); 
            }
            break;
        case CL_PLATFORM_VERSION:
            // strnlen_s calculates without null terminator.
            RetSize = strnlen_s( "OpenCL 1.10 ", MAX_STRLEN );
            RetSize++;
            if( param_value )
            {
                if( RetSize > param_value_size )
                {
                    return CL_INVALID_VALUE;
                }
                strncpy_s( ( char * )param_value, RetSize, "OpenCL 1.10 ", RetSize ); 
            }
            break;
        case CL_PLATFORM_NAME:
            // strnlen_s calculates without null terminator.
            RetSize = strnlen_s( INTEL_PLATFORM_NAME, MAX_STRLEN );
            RetSize++;
            if( param_value )
            {
                if( RetSize > param_value_size )
                {
                    return CL_INVALID_VALUE;
                }
                strncpy_s( ( char * )param_value, RetSize, INTEL_PLATFORM_NAME, RetSize ); 
            }
            break;
        case CL_PLATFORM_VENDOR:
            // strnlen_s calculates without null terminator.
            RetSize = strnlen_s( "Intel(R) Corporation", MAX_STRLEN );
            RetSize++;
            if( param_value )
            {
                if( RetSize > param_value_size )
                {
                    return CL_INVALID_VALUE;
                }
                strncpy_s( ( char * )param_value, RetSize, "Intel(R) Corporation", RetSize ); 
            }
            break;
        case CL_PLATFORM_EXTENSIONS:
            // strnlen_s calculates without null terminator.
            RetSize = strnlen_s( PLATFORM_EXT_STRING, MAX_STRLEN );
            RetSize++;
            if( param_value )
            {
                if( RetSize > param_value_size )
                {
                    return CL_INVALID_VALUE;
                }
                strncpy_s( ( char * )param_value, RetSize, PLATFORM_EXT_STRING, RetSize ); 
            }
            break;
        case CL_PLATFORM_ICD_SUFFIX_KHR:
            RetSize = strnlen_s( INTEL_ICD_EXTENSIONS_STRING, MAX_STRLEN );
            RetSize++;
            if( param_value ) 
			{
                if( RetSize > param_value_size )
                {
                    return CL_INVALID_VALUE;
                }
                strncpy_s( ( char * )param_value, RetSize, INTEL_ICD_EXTENSIONS_STRING, RetSize ); 
			}
			break;
		default:
			return CL_INVALID_VALUE;
	}

    if( param_value_size_ret )
    {
        *param_value_size_ret = RetSize;
    }
	return CL_SUCCESS;
}
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clGetDeviceIDs(
	cl_platform_id	platform,
	cl_device_type	device_type, 
	cl_uint			num_entries, 
	cl_device_id*	devices, 
	cl_uint*		num_devices)
{
	if (!isValidPlatform(platform))
		return CL_INVALID_PLATFORM;
		

	if (!(device_type & CL_DEVICE_TYPE_DEFAULT)		&&
		!(device_type & CL_DEVICE_TYPE_CPU)			&&
		!(device_type & CL_DEVICE_TYPE_GPU)			&&
		!(device_type & CL_DEVICE_TYPE_ACCELERATOR)	&&
		!(device_type & CL_DEVICE_TYPE_ALL))
	{
		return CL_INVALID_DEVICE_TYPE;
	}

	if ((NULL != devices && 0 == num_entries) ||
		(NULL == devices && NULL == num_devices))
	{		
		return CL_INVALID_VALUE;
	}
	
	num_entries = (devices == NULL)? 0 : num_entries;
	cl_uint numRet = 0;

	for (OCLCRT::DEV_INFO_MAP::const_iterator itr = OCLCRT::crt_ocl_module.m_deviceInfoMap.begin(); 
		itr != OCLCRT::crt_ocl_module.m_deviceInfoMap.end(); 
		itr++)
	{
		cl_device_type devType;

		const cl_device_id& devIdDEV = itr->first;
		
		CrtDeviceInfo* devInfo = itr->second;		
		if(devInfo->m_isRootDevice == false)
		{
				/// Skip SubDevices; they should not
				/// be returned for this query.
			continue;
		}
		if (!(CL_SUCCESS == devIdDEV->dispatch->clGetDeviceInfo(
										devIdDEV,
										CL_DEVICE_TYPE,
										sizeof(cl_device_type),
										(void*)(&devType),
										NULL)))
		{
			return CL_OUT_OF_HOST_MEMORY;
		}
				
		if (device_type == CL_DEVICE_TYPE_DEFAULT	||
			device_type == CL_DEVICE_TYPE_ALL		||
			devType == device_type)
		{			
			if (devices && numRet < num_entries)
				devices[numRet++] = devIdDEV;
			else 
				numRet++;			
		}		
	}	
	if (num_devices)
	{
		if (device_type == CL_DEVICE_TYPE_DEFAULT && numRet >= 1)
			*num_devices = 1;
		else
			*num_devices = numRet;
	}
	return CL_SUCCESS;
}
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_context CL_API_CALL clCreateContext(
	const cl_context_properties *	properties,
	cl_uint							num_devices,
	const cl_device_id *			devices,
	logging_fn						pfn_notify,
	void *							user_data,
	cl_int *						errcode_ret)
{	
	cl_int errCode = CL_SUCCESS;
	
	if (CL_SUCCESS != OCLCRT::crt_ocl_module.isValidProperties(properties))
		errCode = CL_INVALID_PLATFORM;
	
	if (!devices)
		errCode = CL_INVALID_VALUE;

	if (!pfn_notify && user_data)
		errCode = CL_INVALID_VALUE;
	
	cl_uint numPlatforms = 0;
	cl_platform_id* pPlatformIdDEV = NULL;

	for (cl_uint i=0; i < num_devices; i++)
	{
		const cl_device_id& devIdDEV = devices[i];		

		if (NULL == devIdDEV || NULL == OCLCRT::crt_ocl_module.m_deviceInfoMap[devIdDEV])
			errCode = CL_INVALID_DEVICE;

		if (i==0)
		{
			pPlatformIdDEV = &OCLCRT::crt_ocl_module.m_deviceInfoMap[devIdDEV]->m_platformIdDEV;									
		}
		else
		{
			if (*pPlatformIdDEV ==  OCLCRT::crt_ocl_module.m_deviceInfoMap[devIdDEV]->m_platformIdDEV)
				continue;
		}	
		numPlatforms++;
	}	

	cl_context ctx = NULL;

	if (errCode == CL_SUCCESS && numPlatforms == 1)
	{
			/// Single Platform Context (All devices belong to same underlying platform)
		const KHRicdVendorDispatch* dTable =
			(KHRicdVendorDispatch*)(&OCLCRT::crt_ocl_module.m_deviceInfoMap[devices[0]]->m_origDispatchTable);
		
		cl_context_properties* props;		
		if (CRT_FAIL == OCLCRT::ReplacePlatformId(
							properties, 
							OCLCRT::crt_ocl_module.m_deviceInfoMap[devices[0]]->m_platformIdDEV, 
							&props))
		{
			errCode = CL_OUT_OF_HOST_MEMORY;
		}		
		else
		{													
			ctx =  dTable->clCreateContext(	
				props,
				num_devices, 
				devices, 
				pfn_notify, 
				user_data, 
				&errCode);
			if (ctx != NULL)
			{				
				OCLCRT::crt_ocl_module.PatchClContextID(
					ctx, 
					&OCLCRT::crt_ocl_module.m_deviceInfoMap[devices[0]]->m_origDispatchTable);

				CrtContextInfo* pContextInfo = new CrtContextInfo;
				if (!pContextInfo)
				{
					errCode = CL_OUT_OF_HOST_MEMORY;
				}
				cl_context_properties hGL = NULL, hDC = NULL;
				if (OCLCRT::isGLContext(properties, &hGL, &hDC))
				{
					pContextInfo->m_gLHGLRCHandle	= hGL; 
					pContextInfo->m_gLHDCHandle		= hDC; 	 
				}
				pContextInfo->m_contextType = CrtContextInfo::SinglePlatformContext;
				pContextInfo->m_object = (void*)(&(OCLCRT::crt_ocl_module.m_deviceInfoMap[devices[0]]->m_origDispatchTable));			
								
				OCLCRT::crt_ocl_module.m_contextInfo[ctx] = pContextInfo;				
			}						
		}
		/// free allocated properties memory
		if (props)
		{
			delete props;
			props = NULL;
		}
	}
	else
	{
		ctx = new _cl_context_crt;
		if (!ctx)
		{
			errCode = CL_OUT_OF_HOST_MEMORY;
			goto FINISH;
		}		
			/// Shared Platform Context
		CrtContext* crtContext = new CrtContext(
			ctx,
			properties, 
			num_devices, 
			devices, 
			pfn_notify, 
			user_data, 
			&errCode);
		
		if (NULL == crtContext)
		{
			errCode = CL_OUT_OF_HOST_MEMORY;
			goto FINISH;
		}
							
		((_cl_context_crt*)ctx)->object = (void*)crtContext;

		CrtContextInfo* pContextInfo = new CrtContextInfo;
		if (!pContextInfo)
		{
			errCode = CL_OUT_OF_HOST_MEMORY;
		}
		cl_context_properties hGL = NULL, hDC = NULL;
		if (OCLCRT::isGLContext(properties, &hGL, &hDC))
		{
			pContextInfo->m_gLHGLRCHandle	= hGL; 
			pContextInfo->m_gLHDCHandle		= hDC; 	 
		}
		pContextInfo->m_contextType = CrtContextInfo::SharedPlatformContext;
		pContextInfo->m_object = (void*)crtContext;
		
		OCLCRT::crt_ocl_module.m_contextInfo[ctx] = pContextInfo;
	}	

FINISH:

	if (NULL != errcode_ret)
		*errcode_ret = errCode;
	
	return ctx;
}
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_context CL_API_CALL clCreateContextFromType(
	const cl_context_properties *	properties,
	cl_device_type					device_type,
	logging_fn						pfn_notify,
	void *							user_data,
	cl_int *						errcode_ret)
{		
	cl_int errCode = CL_SUCCESS;
	if (CL_SUCCESS != OCLCRT::crt_ocl_module.isValidProperties(properties))
	{
		errCode = CL_INVALID_PLATFORM;		
	}			

	if (!(device_type & CL_DEVICE_TYPE_DEFAULT)		&&
		!(device_type & CL_DEVICE_TYPE_CPU)			&&
		!(device_type & CL_DEVICE_TYPE_GPU)			&&
		!(device_type & CL_DEVICE_TYPE_ACCELERATOR)	&&
		!(device_type & CL_DEVICE_TYPE_ALL))
	{		
		errCode = CL_INVALID_DEVICE_TYPE;		
	}

	if (!pfn_notify && user_data)
	{
		errCode = CL_INVALID_VALUE;		
	}

	cl_context	ctx = NULL;
	cl_uint numPlatforms = 0;	
	cl_uint numDevices = 0;
	cl_platform_id pId;	
	
	cl_device_id* deviceList = new cl_device_id[OCLCRT::crt_ocl_module.m_deviceInfoMap.size()];
	if (deviceList == NULL)
	{
		errCode = CL_OUT_OF_HOST_MEMORY;		
	}	
	
	if (CL_SUCCESS != errCode)
	{
		if(errcode_ret)
			*errcode_ret = errCode;
		return NULL;
	}
	
		/// In case there is only one underlying device, we pick it as CL_DEFAULT_DEVICE_TYPE
	bool OnlyOneAvailableDevice = (OCLCRT::crt_ocl_module.m_deviceInfoMap.size() == 1);

	for (OCLCRT::DEV_INFO_MAP::iterator itr = OCLCRT::crt_ocl_module.m_deviceInfoMap.begin();
		itr != OCLCRT::crt_ocl_module.m_deviceInfoMap.end();
		itr++)
	{
		CrtDeviceInfo* devInfo = itr->second;
		if (devInfo->m_isRootDevice == false)
		{
				/// Skip Sub-Devices
			continue;
		}
		cl_device_type clDevType;
		errCode = devInfo->m_origDispatchTable.clGetDeviceInfo(	
			itr->first, 
			CL_DEVICE_TYPE, 
			sizeof(cl_device_type), 
			(void*)(&clDevType), 
			NULL);

		if (CL_SUCCESS != errCode) 
		{
			errCode = CL_OUT_OF_RESOURCES;
			break;
		}
		if (device_type == CL_DEVICE_TYPE_DEFAULT && 
				/// In case of two-devices pick the CRT default
			(clDevType == OCLCRT::crt_ocl_module.m_defaultDeviceType || OnlyOneAvailableDevice) )							
		{
			numDevices = 1;
			numPlatforms = 1;
			deviceList[0] = itr->first;
			break;
		}
		if (device_type == clDevType || device_type == CL_DEVICE_TYPE_ALL)
		{
			if (numDevices == 0)
			{
				numPlatforms = 1;					
				pId	=	devInfo->m_platformIdDEV;
			}
			else
			{
				if (pId != devInfo->m_platformIdDEV)
					numPlatforms++;
			}
			deviceList[numDevices++] = itr->first;				
		}
	}		

	if (CL_SUCCESS == errCode)
		if (numDevices == 0)
			errCode = CL_DEVICE_NOT_FOUND;
		else
			ctx =  clCreateContext(properties, numDevices, deviceList, pfn_notify, user_data, &errCode);

	if (errcode_ret)
		*errcode_ret = errCode;

	delete[] deviceList;
	return ctx;
}
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clGetContextInfo(
	cl_context      context,
	cl_context_info param_name,
	size_t          param_value_size,
	void *          param_value,
	size_t *        param_value_size_ret)
{
	cl_int errCode = CL_SUCCESS;

	if (!context || OCLCRT::crt_ocl_module.m_contextInfo.count(context) == 0)
		return CL_INVALID_CONTEXT;

	CrtContextInfo* ctxInfo = OCLCRT::crt_ocl_module.m_contextInfo[context];
	if (ctxInfo->m_contextType == CrtContextInfo::SinglePlatformContext)
	{
			/// Single Platform Context	

		KHRicdVendorDispatch* dTable = 
			(KHRicdVendorDispatch*)(ctxInfo->m_object);

		errCode = dTable->clGetContextInfo(
			context, 
			param_name, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

		if (errCode == CL_SUCCESS && param_name == CL_CONTEXT_PROPERTIES)
		{
				/// replace the underlying platform_id with the CRT platform id
			OCLCRT::ReplacePlatformId(
				(cl_context_properties*)param_value, 
				OCLCRT::crt_ocl_module.m_crtPlatformId, NULL, false);		
		}
	}
	else
	{
		
		if (!param_value_size_ret && !param_value)
			return CL_INVALID_VALUE;
		
			/// Shared Platform Contxt
		CrtContext* ctx = (CrtContext*)(ctxInfo->m_object);
		
		size_t pValueSize = 0;		
		
		switch (param_name)
		{
		case CL_CONTEXT_REFERENCE_COUNT:
			{			
				DEV_CTX_MAP::iterator itr = ctx->m_DeviceToContext.begin();
				errCode = OCLCRT::crt_ocl_module.m_deviceInfoMap[itr->first]->m_origDispatchTable.clGetContextInfo(
					itr->second,
					param_name,
					param_value_size,
					param_value,
					param_value_size_ret);				
				break;
			}
		case CL_CONTEXT_NUM_DEVICES:
			{
				pValueSize = sizeof(cl_uint);				
				
				if (param_value && param_value_size >= pValueSize)
				{
					*((cl_uint*)param_value) = (cl_uint)ctx->m_DeviceToContext.size();
				}
				break;
			}
		case CL_CONTEXT_DEVICES:
			{
				pValueSize = sizeof(cl_device_id)*ctx->m_DeviceToContext.size();
											
				if (param_value && param_value_size >= pValueSize)
				{
					cl_uint i=0;
					DEV_CTX_MAP::iterator itr = ctx->m_DeviceToContext.begin();					
					while( itr != ctx->m_DeviceToContext.end() )
					{
						((cl_device_id*)param_value)[i++] = itr->first;
						itr++;
					}				
				}
				break;
			}
		case CL_CONTEXT_PROPERTIES:
			{
				DEV_CTX_MAP::iterator itr = ctx->m_DeviceToContext.begin();
				errCode = OCLCRT::crt_ocl_module.m_deviceInfoMap[itr->first]->m_origDispatchTable.clGetContextInfo(
					itr->second,
					param_name,
					param_value_size,
					param_value,
					param_value_size_ret);									
				
				if (errCode == CL_SUCCESS)
				{
					/// replace the underlying platform_id with the CRT platform id
					OCLCRT::ReplacePlatformId(
						(cl_context_properties*)param_value, 
						OCLCRT::crt_ocl_module.m_crtPlatformId, NULL, false);		
				}
				break;
			}
		default:
			{
				errCode = CL_INVALID_VALUE;
				break;
			}
		}				

		if( param_value_size_ret )
		{
			*param_value_size_ret = pValueSize;
		}		
		
		if (param_value && param_value_size < pValueSize)
			return CL_INVALID_VALUE;	
	}
	return errCode;
}
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clGetGLContextInfoKHR( const cl_context_properties * properties,
										  cl_gl_context_info            param_name,
										  size_t                        param_value_size,
										  void *                        param_value,
										  size_t *                      param_value_size_ret)				  
{
	cl_int retCode = CL_SUCCESS;

	if (param_value == NULL && param_value_size_ret == NULL)
		return CL_INVALID_VALUE;
	
	if (CL_SUCCESS != (retCode = OCLCRT::crt_ocl_module.isValidProperties(properties)))
		return retCode;
	
	cl_context_properties hGL = NULL, hDC = NULL;
	if (!OCLCRT::isGLContext(properties, &hGL, &hDC))
	{
		return CL_INVALID_VALUE;
	}
	
	switch (param_name)
	{
	case CL_CURRENT_DEVICE_FOR_GL_CONTEXT_KHR:
	{
		
		for (OCLCRT::DEV_INFO_MAP::iterator itr = OCLCRT::crt_ocl_module.m_deviceInfoMap.begin();
			itr != OCLCRT::crt_ocl_module.m_deviceInfoMap.end(); 
			itr++)
		{

			cl_context_properties* props = NULL;
			if (CRT_FAIL == OCLCRT::ReplacePlatformId(properties, itr->second->m_platformIdDEV, &props))
			{
				retCode = CL_OUT_OF_HOST_MEMORY;	
				return retCode;
			}
			retCode = itr->second->m_origDispatchTable.clGetGLContextInfoKHR( props,
						  param_name,
						  param_value_size,
						  param_value,
						  param_value_size_ret);
			if (props)
			{
				delete props;
				props = NULL;
			}
			if (retCode == CL_SUCCESS)		
				return retCode;
		}				
		break;
	}
	case CL_DEVICES_FOR_GL_CONTEXT_KHR:		
	{
		cl_device_id* glDevices = new cl_device_id[ OCLCRT::crt_ocl_module.m_deviceInfoMap.size()];
		if (glDevices == NULL)
		{
			return CL_OUT_OF_HOST_MEMORY;			
		}
		cl_uint numDevices=0;
		for (OCLCRT::DEV_INFO_MAP::iterator itr = OCLCRT::crt_ocl_module.m_deviceInfoMap.begin();
													itr != OCLCRT::crt_ocl_module.m_deviceInfoMap.end(); 
													itr++)
		{
			cl_context_properties* props;			
			
			if (CRT_FAIL == 
				OCLCRT::ReplacePlatformId(properties, itr->second->m_platformIdDEV, &props))
			{
				return CL_OUT_OF_HOST_MEMORY;				
			}					
			cl_device_id devId = NULL;
			if (CL_SUCCESS == 
				(retCode = itr->second->m_origDispatchTable.clGetGLContextInfoKHR( props,
							CL_DEVICES_FOR_GL_CONTEXT_KHR,
							sizeof(cl_device_id),
							devId,
							NULL)))
			{
				glDevices[numDevices++] = devId;
			}			
			if (props)
			{
				delete props;
				props = NULL;
			}
		}
		if (numDevices == 0)
			return CL_INVALID_OPERATION;
		else
		{
			if (param_value != NULL)
				if (param_value_size >= numDevices*sizeof(cl_device_id))
				{
					memcpy_s(param_value, param_value_size, (void*)glDevices,  numDevices*sizeof(cl_device_id));
				}
				else
					return CL_INVALID_VALUE;
			
			if (param_value_size_ret)
				*param_value_size_ret = numDevices*sizeof(cl_device_id);		
		}
		break;	
	}
	default:
		return CL_INVALID_VALUE;
	}	
	return retCode;
}
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clGetDeviceInfo(cl_device_id device,
								   cl_device_info param_name, 
								   size_t param_value_size, 
								   void* param_value,
								   size_t* param_value_size_ret)

{
	cl_int retCode = CL_SUCCESS;

	CrtDeviceInfo* devInfo = OCLCRT::crt_ocl_module.m_deviceInfoMap[device];
	if (NULL == devInfo)
		return CL_INVALID_DEVICE;

	retCode = devInfo->m_origDispatchTable.clGetDeviceInfo( device, 
															param_name,
															param_value_size,
															param_value,
															param_value_size_ret);

	if (retCode == CL_SUCCESS && param_name == CL_DEVICE_PLATFORM)
	{
		memcpy_s(param_value, 
			sizeof(cl_platform_id), 
			&OCLCRT::crt_ocl_module.m_crtPlatformId, 
			sizeof(cl_platform_id));		
	}
	return retCode;
}
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_command_queue CL_API_CALL clCreateCommandQueue(cl_context                  context, 
												  cl_device_id                device, 
												  cl_command_queue_properties properties, 
												  cl_int *                    errcode_ret)
{
	_cl_command_queue_crt* queue_handle = NULL;
	CrtQueue* queue = NULL;
	cl_int errCode = CL_SUCCESS;
	CrtContextInfo* ctxInfo  = NULL;
	
	ctxInfo = OCLCRT::crt_ocl_module.m_contextInfo[context];		
	if (!ctxInfo)
	{
		errCode = CL_INVALID_CONTEXT;
	}	

	if (CL_SUCCESS == errCode)
	{
		CrtContext* ctx = (CrtContext*)(ctxInfo->m_object);
		errCode = ctx->CreateCommandQueue(device, properties, &queue);
	}
	if (CL_SUCCESS == errCode)
	{
		queue_handle = new _cl_command_queue_crt;
		if (!queue_handle)
		{
			// do we need to release anything here?
			errCode = CL_OUT_OF_HOST_MEMORY;			
		}
		else
			queue_handle->object = (void*)queue;
	}
	if (errcode_ret)
		*errcode_ret = errCode;

	return queue_handle;
}
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_mem CL_API_CALL clCreateBuffer(cl_context   context, 
								  cl_mem_flags flags, 
								  size_t       size, 
								  void *       host_ptr, 
								  cl_int *     errcode_ret)
{
	_cl_mem_crt* mem_handle = NULL;
	cl_int errCode = CL_SUCCESS;	
	CrtContextInfo* ctxInfo  = NULL;

	ctxInfo = OCLCRT::crt_ocl_module.m_contextInfo[context];		
	if (!ctxInfo)
	{
		errCode = CL_INVALID_CONTEXT;
	}	
		
	if (CL_SUCCESS == errCode)
	{
		mem_handle = new _cl_mem_crt;
		if (!mem_handle)
		{
			errCode = CL_OUT_OF_HOST_MEMORY;
		}
	}
	if (CL_SUCCESS == errCode)
	{
		CrtContext* ctx = (CrtContext*)(ctxInfo->m_object);
		errCode = ctx->CreateBuffer(			
			flags, 
			size, 
			host_ptr,			
			(CrtMemObject**)(&mem_handle->object));
	}						
	
	if (errcode_ret)
		*errcode_ret = errCode;

	return mem_handle;
}

cl_mem CL_API_CALL clCreateSubBuffer(
	cl_mem					buffer,
	cl_mem_flags			flags,
	cl_buffer_create_type	buffer_create_type,
	const void *            buffer_create_info,
	cl_int *                errcode_ret)
{
	_cl_mem_crt* mem_handle = NULL;
	cl_int errCode = CL_SUCCESS;	

		/// We check this now since we rely on this internally 
		/// to decide if to create a buffer or a sub-buffer
	if (!buffer_create_info)
	{
		errCode = CL_INVALID_VALUE;
		goto FINISH;
	}

	CrtBuffer* crtBuffer = reinterpret_cast<CrtBuffer*>(((_cl_mem_crt*)buffer)->object);	
	if (!crtBuffer)
	{	
		errCode = CL_INVALID_MEM_OBJECT;		
		goto FINISH;
	}			

	if (CL_SUCCESS == errCode)
	{
		mem_handle = new _cl_mem_crt;
		if (!mem_handle)
		{
			errCode = CL_OUT_OF_HOST_MEMORY;
		}
	}

	errCode = crtBuffer->m_pContext->CreateSubBuffer(
		crtBuffer,
		flags,		
		buffer_create_type, 
		buffer_create_info, 
		(CrtMemObject**)(&mem_handle->object));
	

FINISH:
	if (errcode_ret)
		*errcode_ret = errCode;

	return mem_handle;
}
/// ------------------------------------------------------------------------------
///	Commmon Runtime Helper function (Read/Write Buffer)
/// ------------------------------------------------------------------------------						
cl_int CL_API_CALL EnqueueReadWriteBuffer(
	bool				read_command,
	cl_command_queue	command_queue,
	cl_mem				buffer,
	cl_bool				blocking_cmd,
	size_t				offset,
	size_t				cb,
	const void *		ptr,
	cl_uint				num_events_in_wait_list,
	const cl_event *	event_wait_list,
	cl_event *			event)
{
	cl_int errCode = CL_SUCCESS;

	if (command_queue == NULL)
		return CL_INVALID_COMMAND_QUEUE;

	if (buffer == NULL)
		return CL_INVALID_MEM_OBJECT;

	CrtQueue* queue = reinterpret_cast<CrtQueue*>(((_cl_command_queue_crt*)command_queue)->object);
	if (!queue)
		return CL_INVALID_COMMAND_QUEUE;
	
	SyncManager* synchHelper = new SyncManager;
	if (!synchHelper)
		return CL_OUT_OF_HOST_MEMORY;

	cl_event*	outEvents = NULL;
	cl_uint		numOutEvents = 0;
	errCode = synchHelper->PrepareToExecute(
		queue, 
		num_events_in_wait_list, 
		event_wait_list, 
		&numOutEvents, 
		&outEvents, 
		0, 
		NULL);

	if (CL_SUCCESS != errCode)
		goto FINISH;

	CrtEvent* crtEvent = new CrtEvent;	
	if (!crtEvent)
	{
		errCode = CL_OUT_OF_HOST_MEMORY;
		goto FINISH;
	}
	CrtBuffer* crtBuffer = reinterpret_cast<CrtBuffer*>(((_cl_mem_crt*)buffer)->object);	
	if (!crtBuffer)
	{	errCode = CL_INVALID_MEM_OBJECT;
		goto FINISH;
	}		
	
	if (blocking_cmd)
	{
		errCode = queue->m_contextCRT->FlushQueues();
		if (CL_SUCCESS != errCode)
		{
			errCode = CL_OUT_OF_RESOURCES;
			goto FINISH;
		}
	}

	crtEvent->m_queueCRT = queue;
	if (read_command)
		errCode = queue->m_cmdQueueDEV->dispatch->clEnqueueReadBuffer(
			queue->m_cmdQueueDEV,
			crtBuffer->getDeviceMemObj(queue->m_device),
			blocking_cmd,
			offset,
			cb,
			const_cast<void*>(ptr),
			numOutEvents,
			outEvents,
			&crtEvent->m_eventDEV);
	else
		errCode = queue->m_cmdQueueDEV->dispatch->clEnqueueWriteBuffer(
			queue->m_cmdQueueDEV,
			crtBuffer->getDeviceMemObj(queue->m_device),
			blocking_cmd,
			offset,
			cb,
			ptr,
			numOutEvents,
			outEvents,
			&crtEvent->m_eventDEV);
	
	if (errCode == CL_SUCCESS && event)
	{
		_cl_event_crt* event_handle = new _cl_event_crt;
		if (!event_handle)
		{
			delete crtEvent;
			crtEvent = NULL;
			errCode = CL_OUT_OF_HOST_MEMORY;
			goto FINISH;
		}
		event_handle->object = (void*)crtEvent;
		*event = event_handle;
	}
	
FINISH:	
	if (CL_SUCCESS != errCode && crtEvent)
	{
		delete crtEvent;
	}
	if (synchHelper)
		delete synchHelper;	
	return errCode;
}
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------	
cl_int CL_API_CALL clEnqueueReadBuffer(
	cl_command_queue	command_queue,
	cl_mem				buffer,
	cl_bool				blocking_write,
	size_t				offset,
	size_t				cb,
	void *				ptr,
	cl_uint				num_events_in_wait_list,
	const cl_event *	event_wait_list,
	cl_event *			event)
{
	return EnqueueReadWriteBuffer(
		true,
		command_queue,
		buffer,
		blocking_write,
		offset,
		cb,
		ptr,
		num_events_in_wait_list,
		event_wait_list,
		event);
}

cl_int CL_API_CALL clEnqueueWriteBuffer(
	cl_command_queue	command_queue,
	cl_mem				buffer,
	cl_bool				blocking_write,
	size_t				offset,
	size_t				cb,
	const void *		ptr,
	cl_uint				num_events_in_wait_list,
	const cl_event *	event_wait_list,
	cl_event *			event)
{
	return EnqueueReadWriteBuffer(
		false,
		command_queue,
		buffer,
		blocking_write,
		offset,
		cb,
		ptr,
		num_events_in_wait_list,
		event_wait_list,
		event);
}
/// ------------------------------------------------------------------------------
/// Common Runtime Helper function (Read/Write Buffer Rect)
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL EnqueueReadWriteBufferRect(
	bool				read_command,
	cl_command_queue    command_queue,
    cl_mem              buffer,
    cl_bool             blocking_cmd,
    const size_t *      buffer_origin,
    const size_t *      host_origin, 
    const size_t *      region,
    size_t              buffer_row_pitch,
    size_t				buffer_slice_pitch,
    size_t              host_row_pitch,
    size_t              host_slice_pitch,
    const void *        ptr,
    cl_uint             num_events_in_wait_list,
    const cl_event *    event_wait_list,
    cl_event *          event )
{
	cl_int errCode = CL_SUCCESS;

	if (!command_queue)
		return CL_INVALID_COMMAND_QUEUE;

	if (!buffer)
		return CL_INVALID_MEM_OBJECT;

	CrtQueue* queue = reinterpret_cast<CrtQueue*>(((_cl_command_queue_crt*)command_queue)->object);
	if (!queue)
		return CL_INVALID_COMMAND_QUEUE;
	
	SyncManager* synchHelper = new SyncManager;
	if (!synchHelper)
		return CL_OUT_OF_HOST_MEMORY;

	cl_event*	outEvents = NULL;
	cl_uint		numOutEvents = 0;
	errCode = synchHelper->PrepareToExecute(
		queue, 
		num_events_in_wait_list, 
		event_wait_list, 
		&numOutEvents, 
		&outEvents, 
		0, 
		NULL);

	if (CL_SUCCESS != errCode)
		goto FINISH;

	CrtEvent* crtEvent = new CrtEvent;
	if (!crtEvent)
	{
		errCode = CL_OUT_OF_HOST_MEMORY;
		goto FINISH;
	}
	CrtBuffer* crtBuffer = reinterpret_cast<CrtBuffer*>(((_cl_mem_crt*)buffer)->object);
	
	if (!crtBuffer)
	{	
		errCode = CL_INVALID_MEM_OBJECT;
		goto FINISH;
	}		

	if (blocking_cmd)
	{
		errCode = queue->m_contextCRT->FlushQueues();
		if (CL_SUCCESS != errCode)
		{
			errCode = CL_OUT_OF_RESOURCES;
			goto FINISH;
		}
	}

	crtEvent->m_queueCRT = queue;
	if (read_command)
		errCode = queue->m_cmdQueueDEV->dispatch->clEnqueueReadBufferRect(
			queue->m_cmdQueueDEV,
			crtBuffer->getDeviceMemObj(queue->m_device),
			blocking_cmd,
			buffer_origin,
			host_origin,
			region,
			buffer_row_pitch,
			buffer_slice_pitch,
			host_row_pitch,
			host_slice_pitch,
			const_cast<void*>(ptr),
			numOutEvents,
			outEvents,
			&crtEvent->m_eventDEV);
	else
		errCode = queue->m_cmdQueueDEV->dispatch->clEnqueueWriteBufferRect(
			queue->m_cmdQueueDEV,
			crtBuffer->getDeviceMemObj(queue->m_device),
			blocking_cmd,
			buffer_origin,
			host_origin,
			region,
			buffer_row_pitch,
			buffer_slice_pitch,
			host_row_pitch,
			host_slice_pitch,
			ptr,
			numOutEvents,
			outEvents,
			&crtEvent->m_eventDEV);
	
	if (errCode == CL_SUCCESS && event)
	{
		_cl_event_crt* event_handle = new _cl_event_crt;
		if (!event_handle)
		{
			delete crtEvent;
			crtEvent = NULL;
			errCode = CL_OUT_OF_HOST_MEMORY;
			goto FINISH;
		}
		event_handle->object = (void*)crtEvent;
		*event = event_handle;
	}
	
FINISH:
	if (synchHelper)
		delete synchHelper;	
	return errCode;
}
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clEnqueueReadBufferRect(
	cl_command_queue    command_queue,
    cl_mem              buffer,
    cl_bool             blocking_read,
    const size_t *      buffer_origin,
    const size_t *      host_origin, 
    const size_t *      region,
    size_t              buffer_row_pitch,
    size_t              buffer_slice_pitch,
    size_t              host_row_pitch,
    size_t              host_slice_pitch,
    void *				ptr,
    cl_uint             num_events_in_wait_list,
    const cl_event *    event_wait_list,
    cl_event *          event)
{
	return EnqueueReadWriteBufferRect(
		true,
		command_queue, 
		buffer,
		blocking_read,
		buffer_origin,
		host_origin,
		region,		
		buffer_row_pitch,
		buffer_slice_pitch,
		host_row_pitch,
		host_slice_pitch,
		ptr,
		num_events_in_wait_list,
		event_wait_list,
		event);

}

cl_int CL_API_CALL clEnqueueWriteBufferRect(
	cl_command_queue    command_queue,
    cl_mem              buffer,
    cl_bool             blocking_write,
    const size_t *      buffer_origin,
    const size_t *      host_origin, 
    const size_t *      region,
    size_t              buffer_row_pitch,
    size_t              buffer_slice_pitch,
    size_t              host_row_pitch,
    size_t              host_slice_pitch,
    const void *        ptr,
    cl_uint             num_events_in_wait_list,
    const cl_event *    event_wait_list,
    cl_event *          event)
{
	return EnqueueReadWriteBufferRect(
		false,
		command_queue, 
		buffer,
		blocking_write,
		buffer_origin,
		host_origin,
		region,		
		buffer_row_pitch,
		buffer_slice_pitch,
		host_row_pitch,
		host_slice_pitch,
		ptr,
		num_events_in_wait_list,
		event_wait_list,
		event);
}
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clEnqueueCopyBufferRect(
	cl_command_queue    command_queue, 
	cl_mem              src_buffer,
	cl_mem              dst_buffer, 
	const size_t *      src_origin,
	const size_t *      dst_origin,
	const size_t *      region, 
	size_t              src_row_pitch,
	size_t              src_slice_pitch,
	size_t              dst_row_pitch,
	size_t              dst_slice_pitch,
	cl_uint             num_events_in_wait_list,
	const cl_event *    event_wait_list,
	cl_event *          event)
{
		cl_int errCode = CL_SUCCESS;

	if (command_queue == NULL)
		return CL_INVALID_COMMAND_QUEUE;

	if (src_buffer == NULL || dst_buffer == NULL)
		return CL_INVALID_MEM_OBJECT;

	CrtQueue* queue = reinterpret_cast<CrtQueue*>(((_cl_command_queue_crt*)command_queue)->object);
	if (!queue)
		return CL_INVALID_COMMAND_QUEUE;
	
	SyncManager* synchHelper = new SyncManager;
	if (!synchHelper)
		return CL_OUT_OF_HOST_MEMORY;

	cl_event*	outEvents = NULL;
	cl_uint		numOutEvents = 0;
	errCode = synchHelper->PrepareToExecute(
		queue, 
		num_events_in_wait_list, 
		event_wait_list, 
		&numOutEvents, 
		&outEvents, 
		0, 
		NULL);

	if (CL_SUCCESS != errCode)
		goto FINISH;

	CrtEvent* crtEvent = new CrtEvent;
	if (!crtEvent)
	{
		errCode = CL_OUT_OF_HOST_MEMORY;
		goto FINISH;
	}
	CrtBuffer* crtSrcBuffer = reinterpret_cast<CrtBuffer*>(((_cl_mem_crt*)src_buffer)->object);	
	CrtBuffer* crtDstBuffer = reinterpret_cast<CrtBuffer*>(((_cl_mem_crt*)dst_buffer)->object);	
	if (!crtSrcBuffer || !crtDstBuffer)
	{	
		errCode = CL_INVALID_MEM_OBJECT;
		goto FINISH;
	}		

	crtEvent->m_queueCRT = queue;	
	errCode = queue->m_cmdQueueDEV->dispatch->clEnqueueCopyBufferRect(
		queue->m_cmdQueueDEV,
		crtSrcBuffer->getDeviceMemObj(queue->m_device),
		crtDstBuffer->getDeviceMemObj(queue->m_device),
		src_origin,
		dst_origin,
		region,
		src_row_pitch,
		src_slice_pitch,
		dst_row_pitch,
		dst_slice_pitch,		
		numOutEvents,
		outEvents,
		&crtEvent->m_eventDEV);	
	
	if (errCode == CL_SUCCESS && event)
	{
		_cl_event_crt* event_handle = new _cl_event_crt;
		if (!event_handle)
		{
			delete crtEvent;
			crtEvent = NULL;
			errCode = CL_OUT_OF_HOST_MEMORY;
			goto FINISH;
		}
		event_handle->object = (void*)crtEvent;
		*event = event_handle;
	}
	
FINISH:
	if (synchHelper)
		delete synchHelper;	
	return errCode;
}
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clEnqueueCopyBuffer(
	cl_command_queue	command_queue,
	cl_mem				src_buffer,
	cl_mem				dst_buffer,
	size_t				src_offset,
	size_t				dst_offset,
	size_t				cb,
	cl_uint				num_events_in_wait_list,
	const cl_event *	event_wait_list,
	cl_event *			event)
{
		cl_int errCode = CL_SUCCESS;

	if (command_queue == NULL)
		return CL_INVALID_COMMAND_QUEUE;

	if (src_buffer == NULL || dst_buffer == NULL)
		return CL_INVALID_MEM_OBJECT;

	CrtQueue* queue = reinterpret_cast<CrtQueue*>(((_cl_command_queue_crt*)command_queue)->object);
	if (!queue)
		return CL_INVALID_COMMAND_QUEUE;
	
	SyncManager* synchHelper = new SyncManager;
	if (!synchHelper)
		return CL_OUT_OF_HOST_MEMORY;

	cl_event*	outEvents = NULL;
	cl_uint		numOutEvents = 0;
	errCode = synchHelper->PrepareToExecute(
		queue, 
		num_events_in_wait_list, 
		event_wait_list, 
		&numOutEvents, 
		&outEvents, 
		0, 
		NULL);

	if (CL_SUCCESS != errCode)
		goto FINISH;

	CrtEvent* crtEvent = new CrtEvent;
	if (!crtEvent)
	{
		errCode = CL_OUT_OF_HOST_MEMORY;
		goto FINISH;
	}
	CrtBuffer* crtSrcBuffer = reinterpret_cast<CrtBuffer*>(((_cl_mem_crt*)src_buffer)->object);	
	CrtBuffer* crtDstBuffer = reinterpret_cast<CrtBuffer*>(((_cl_mem_crt*)dst_buffer)->object);	
	if (!crtSrcBuffer || !crtDstBuffer)
	{	
		errCode = CL_INVALID_MEM_OBJECT;
		goto FINISH;
	}		

	crtEvent->m_queueCRT = queue;	
	errCode = queue->m_cmdQueueDEV->dispatch->clEnqueueCopyBuffer(
		queue->m_cmdQueueDEV,
		crtSrcBuffer->getDeviceMemObj(queue->m_device),
		crtDstBuffer->getDeviceMemObj(queue->m_device),
		src_offset,
		dst_offset,
		cb,	
		numOutEvents,
		outEvents,
		&crtEvent->m_eventDEV);	
	
	if (errCode == CL_SUCCESS && event)
	{
		_cl_event_crt* event_handle = new _cl_event_crt;
		if (!event_handle)
		{
			delete crtEvent;
			crtEvent = NULL;
			errCode = CL_OUT_OF_HOST_MEMORY;
			goto FINISH;
		}
		event_handle->object = (void*)crtEvent;
		*event = event_handle;
	}
	
FINISH:
	if (synchHelper)
		delete synchHelper;	
	return errCode;
}
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
void * CL_API_CALL clEnqueueMapBuffer(
	cl_command_queue	command_queue,
	cl_mem				buffer,
	cl_bool				blocking_map,
	cl_map_flags		map_flags,
	size_t				offset,
	size_t				cb,
	cl_uint				num_events_in_wait_list,
	const cl_event *	event_wait_list,
	cl_event *			event,
	cl_int *			errcode_ret)
{
	cl_int errCode = CL_SUCCESS;
	void* ptr = NULL;

	if (command_queue == NULL)
	{
		errCode = CL_INVALID_COMMAND_QUEUE;
		return NULL;
	}
	if (buffer == NULL)
	{
		errCode = CL_INVALID_MEM_OBJECT;
		return NULL;
	}
	CrtQueue* queue = reinterpret_cast<CrtQueue*>(((_cl_command_queue_crt*)command_queue)->object);
	if (!queue)
	{
		errCode = CL_INVALID_COMMAND_QUEUE;
		return NULL;
	}	
	SyncManager* synchHelper = new SyncManager;
	if (!synchHelper)
	{
		errCode = CL_OUT_OF_HOST_MEMORY;
		return NULL;
	}
	cl_event*	outEvents = NULL;
	cl_uint		numOutEvents = 0;
	errCode = synchHelper->PrepareToExecute(
		queue, 
		num_events_in_wait_list, 
		event_wait_list, 
		&numOutEvents, 
		&outEvents, 
		0, 
		NULL);

	if (CL_SUCCESS != errCode)
		goto FINISH;

	CrtEvent* crtEvent = new CrtEvent;
	CrtBuffer* crtBuffer = reinterpret_cast<CrtBuffer*>(((_cl_mem_crt*)buffer)->object);
	
	if (!crtBuffer || !crtEvent)
	{	errCode = CL_INVALID_MEM_OBJECT;
		goto FINISH;
	}		
	
	if (blocking_map)
	{
		errCode = queue->m_contextCRT->FlushQueues();
		if (CL_SUCCESS != errCode)
		{
			errCode = CL_OUT_OF_RESOURCES;
			goto FINISH;
		}
	}

	crtEvent->m_queueCRT = queue;	
	ptr = queue->m_cmdQueueDEV->dispatch->clEnqueueMapBuffer(
		queue->m_cmdQueueDEV,
		crtBuffer->getDeviceMemObj(queue->m_device),
		blocking_map,
		map_flags,
		offset,
		cb,		
		numOutEvents,
		outEvents,
		&crtEvent->m_eventDEV,
		&errCode);
	
	if (errCode == CL_SUCCESS && event)
	{
		_cl_event_crt* event_handle = new _cl_event_crt;
		if (!event_handle)
		{
			delete crtEvent;
			crtEvent = NULL;
			errCode = CL_OUT_OF_HOST_MEMORY;
			goto FINISH;
		}
		event_handle->object = (void*)crtEvent;
		*event = event_handle;
	}
	
FINISH:
	if (synchHelper)
		delete synchHelper;	
	
	if (errcode_ret)
		*errcode_ret = errCode;
	return ptr;
}
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clFlush(cl_command_queue command_queue)
{
	if (command_queue == NULL)
		return CL_INVALID_COMMAND_QUEUE;

	CrtQueue* queue = reinterpret_cast<CrtQueue*>(((_cl_command_queue_crt*)command_queue)->object);
	if (!queue)
		return CL_INVALID_COMMAND_QUEUE;
	
	return queue->m_cmdQueueDEV->dispatch->clFlush(queue->m_cmdQueueDEV);		
}

/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clFinish(cl_command_queue command_queue)
{
	cl_int errCode  = CL_SUCCESS;

	if (command_queue == NULL)
		return CL_INVALID_COMMAND_QUEUE;

	CrtQueue* queue = reinterpret_cast<CrtQueue*>(((_cl_command_queue_crt*)command_queue)->object);
	if (!queue)
		return CL_INVALID_COMMAND_QUEUE;	

	return queue->m_cmdQueueDEV->dispatch->clFinish(queue->m_cmdQueueDEV);
}
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clRetainContext(cl_context context)
{
	cl_int errCode = CL_SUCCESS;
	
	CrtContextInfo* ctxInfo = OCLCRT::crt_ocl_module.m_contextInfo[context];		
	if (!ctxInfo)
	{
		errCode = CL_INVALID_CONTEXT;
	}

	if (ctxInfo->m_contextType == CrtContextInfo::SinglePlatformContext)	
	{
			/// Single Platform Context
		KHRicdVendorDispatch* dTable = (KHRicdVendorDispatch*)(ctxInfo->m_object);

		errCode = dTable->clRetainContext(context);
		return errCode;
	}
	else
	{
			/// Shared Platform Context
		CrtContext* ctx = (CrtContext*)(ctxInfo->m_object);	
		atomic_increment(&ctx->m_refCount);		
	}
	return errCode;
}

cl_int CL_API_CALL clReleaseContext(cl_context context)
{
	cl_int errCode = CL_SUCCESS;
		
	OCLCRT::CTX_INFO_MAP::iterator itr = OCLCRT::crt_ocl_module.m_contextInfo.find(context);
		
	if (itr == OCLCRT::crt_ocl_module.m_contextInfo.end())
	{
		return CL_INVALID_CONTEXT;
	}

	cl_uint refCount = 0;
		/// Single Platform Context
	if (itr->second->m_contextType == CrtContextInfo::SinglePlatformContext)	
	{			
		KHRicdVendorDispatch* dTable = (KHRicdVendorDispatch*)(itr->second->m_object);

		/// Get the new reference count		
		errCode = dTable->clGetContextInfo(
			context, 
			CL_CONTEXT_REFERENCE_COUNT, 
			sizeof(cl_uint), 
			&refCount, 
			NULL);

		if (CL_SUCCESS != errCode)
		{
			return CL_OUT_OF_HOST_MEMORY;
		}
		
		errCode = dTable->clReleaseContext(context);
		
		if (refCount == 1)
		{
				/// Deletes CrtContextInfo object
				/// we don't delete context, since it was created by the underlying
				/// platform and not by the CRT.
			delete itr->second;	
				/// Remove the context from the contexts map,
				/// since its not valid any more
			OCLCRT::crt_ocl_module.m_contextInfo.erase(itr);
		}
		return errCode;			
	}
	else
	{
			/// Shared Platform Context
		CrtContext* ctx = (CrtContext*)(itr->second->m_object);	
		
		atomic_decrement(&ctx->m_refCount);
								
		if (refCount == 0)
		{		
			errCode = ctx->Release();
				/// deletes CrtContext object
			delete ctx;
				/// deletes the CrtContextInfo object
			delete itr->second;
				/// deletes the _crt_cl_context handle
			delete context;
				/// Remove the context from the contexts map,
				/// since its not valid any more
			OCLCRT::crt_ocl_module.m_contextInfo.erase(itr);
		}
	}
	return errCode;
}

cl_int CL_API_CALL clRetainCommandQueue(cl_command_queue command_queue)
{
	cl_int errCode = CL_SUCCESS;		
	
	CrtQueue* queue = reinterpret_cast<CrtQueue*>(((_cl_command_queue_crt*)command_queue)->object);
	if (!queue)
	{
		return CL_INVALID_COMMAND_QUEUE;
		
	}
	atomic_increment(&queue->m_refCount);
		
	/// I don't forward retains to the underlying platforms, i only send release when
	/// the CRT ref counter for this queue reaches zero

	return errCode;
}

cl_int CL_API_CALL clReleaseCommandQueue(cl_command_queue command_queue)
{	
	cl_int errCode = CL_SUCCESS;		
	

	CrtQueue* queue = reinterpret_cast<CrtQueue*>(((_cl_command_queue_crt*)command_queue)->object);
	if (!queue)
	{
		return CL_INVALID_COMMAND_QUEUE;
		
	}	
		/// Spec says the clReleaseCommandQueue triggers an implicit flush
	queue->m_contextCRT->FlushQueues();		

		/// Main the self reference counter
	atomic_decrement(&queue->m_refCount);
	
		/// No more references to this queue, it can be deallocated
	if (queue->m_refCount == 0)
	{	
			/// Forward the call to the underlying queue
		errCode = queue->m_cmdQueueDEV->dispatch->clReleaseCommandQueue(queue->m_cmdQueueDEV);
			/// ToDo: Make sure no body is releasing the context at this time we are here
		queue->m_contextCRT->m_commandQueues.remove(queue->m_cmdQueueDEV);

			/// Deletes CrtQueue
		delete queue;

			/// deletes CRT handle _crt_command_queue
		delete command_queue;
	}
	return errCode;	
}


cl_int CL_API_CALL clReleaseMemObject(cl_mem memobj)
{
	cl_int errCode = CL_SUCCESS;
	
	CrtMemObject* crtMemObj = reinterpret_cast<CrtMemObject*>(((_cl_mem_crt*)memobj)->object);		
	atomic_decrement(&crtMemObj->m_refCount);	

	if (crtMemObj->m_refCount == 0)
	{			
		errCode = crtMemObj->Release();	
		/// crtMemObj will be deleted later by the desctructor callback to 
		/// which will also set memobj->object to NULL		
		delete memobj;
	}	
	return errCode;
}

cl_int CL_API_CALL clRetainMemObject(cl_mem memobj)
{
	cl_int errCode = CL_SUCCESS;

	CrtMemObject* crtMemObj = reinterpret_cast<CrtMemObject*>(((_cl_mem_crt*)memobj)->object);			
	atomic_increment(&crtMemObj->m_refCount);	

	/// I don't forward retains to the underlying platforms, i only send release when
	/// the CRT ref counter for this mem objects reaches zero

	return CL_SUCCESS;	
}

/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------

	/// Program Object APIs
cl_program CL_API_CALL clCreateProgramWithSource( cl_context         context ,
                                                  cl_uint            count ,
                                                  const char **      strings ,
                                                  const size_t *     lengths ,
                                                  cl_int *           errcode_ret )
{
    cl_int errCode = CL_SUCCESS;
    _cl_program_crt* SharedProgram = NULL;
    CrtProgram* pgm = NULL;

    CrtContextInfo* ctxInfo = OCLCRT::crt_ocl_module.m_contextInfo[context];
   	if ( !ctxInfo || ctxInfo->m_contextType != CrtContextInfo::SharedPlatformContext )
    {
        errCode = CL_INVALID_CONTEXT;
    }

    if( CL_SUCCESS == errCode ) 
    {
        //Shared context.
        CrtContext* ctx = (CrtContext*)(ctxInfo->m_object);
        errCode = ctx->CreateProgramWithSource( count ,
                                                strings ,
                                                lengths ,
                                                &pgm );
    }

    if( CL_SUCCESS == errCode )
    {
        SharedProgram = new _cl_program_crt;
        if( SharedProgram == NULL )
        {
            delete SharedProgram;
            errCode = CL_OUT_OF_HOST_MEMORY;
        }
        else
        {
            SharedProgram->object = (void *) pgm;
        }
    }

    if( errcode_ret )
    {
        *errcode_ret = errCode;
    }
    return SharedProgram;
}


/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------

cl_int CL_API_CALL clBuildProgram( cl_program            program ,
                                   cl_uint               num_devices ,
                                   const cl_device_id *  device_list ,
                                   const char *          options , 
                                   prog_logging_fn       pfn_notify ,
                                   void *                user_data )
{
    cl_int errCode              = CL_SUCCESS;
    CrtProgram* pgm             = reinterpret_cast<CrtProgram*>(((_cl_program_crt*)program)->object);
    CrtContext* ctx             = pgm->m_contextCRT;
    cl_device_id* outDevices    = NULL;
    CrtBuildMgr* buildMgr       = NULL;

    cl_bool BuildAll            = ( num_devices == 0 ) ? CL_TRUE : CL_FALSE;
    cl_bool BuildAsync          = ( pfn_notify != NULL ) ? CL_TRUE : CL_FALSE;


    if( ( ( num_devices > 0 ) && ( device_list == NULL ) ) ||
        ( ( num_devices == 0 ) && ( device_list != NULL ) ) )
    {
        errCode = CL_INVALID_VALUE;
    }

    if( CL_SUCCESS == errCode )
    {
        outDevices = new cl_device_id[OCLCRT::crt_ocl_module.m_deviceInfoMap.size()];
        if( outDevices == NULL )
        {
            delete outDevices;
            errCode = CL_OUT_OF_HOST_MEMORY;
        }
    }

    if( CL_SUCCESS == errCode )
    {
        //Shared context.
        buildMgr = new CrtBuildMgr;
        if ( buildMgr == NULL )
        {
            errCode = CL_OUT_OF_HOST_MEMORY;
        }
        else
        {
            errCode = buildMgr->Init( program );
            buildMgr->setNumBuild( OCLCRT::crt_ocl_module.m_oclPlatforms.size() );
        }
        void* CrtUserData = ( void* ) buildMgr->setBuildCallBackData( pfn_notify, user_data );

        if( CL_SUCCESS == errCode )
        {
	        for (cl_uint i = 0; i < OCLCRT::crt_ocl_module.m_oclPlatforms.size(); i++)
	        {
		        cl_uint matchDevices = 0;
                ctx->GetDevicesByPlatformId( num_devices, 
								             device_list, 
								             OCLCRT::crt_ocl_module.m_oclPlatforms[i]->m_platformIdDEV, 
								             &matchDevices, 
								             outDevices);
        		
                if( BuildAll || matchDevices !=0 )
                {
                    cl_context currCtx = ctx->GetContextByDeviceID( outDevices[0] );
                    cl_program currPgm = pgm->m_ContextToProgram[ currCtx ];

                    errCode = outDevices[0]->dispatch->clBuildProgram( currPgm,
                                                                       matchDevices,
                                                                       outDevices,
                                                                       options,
                                                                       buildMgr->buildCompleteFn( currPgm, CrtUserData ),
                                                                       CrtUserData );
                    if( errCode != CL_SUCCESS )
                    {
                        goto ERROR_HANDLER;
                    }
                }
                else
                {
                        //We have to decrement even if there is no build required for this platform
                    buildMgr->decNumBuild();
                }
            }

            if( !BuildAsync )
            {
                while( 0 != buildMgr->getNumBuild() )//numbuilds is not zero)
                {
                    SleepEx(1,TRUE);
                }
            }
        }
    }

ERROR_HANDLER:
    if( NULL != outDevices )
        delete outDevices;

    buildMgr->Cleanup();

    if( NULL != buildMgr )
        delete buildMgr;
    return errCode;
}

/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------

cl_int CL_API_CALL clGetProgramBuildInfo( cl_program             program,
                                          cl_device_id           device,
                                          cl_program_build_info  param_name,
                                          size_t                 param_value_size,
                                          void *                 param_value,
                                          size_t *               param_value_size_ret )
{
    cl_int errCode = CL_SUCCESS;
    CrtProgram* pgm = reinterpret_cast<CrtProgram*>(((_cl_program_crt*)program)->object);
    CrtContext* ctx = pgm->m_contextCRT;

    if( CL_SUCCESS == errCode )
    {
        //Shared context.
        cl_program currPgm = pgm->m_ContextToProgram[ ctx->GetContextByDeviceID( device ) ];
        errCode = currPgm->dispatch->clGetProgramBuildInfo( currPgm,
                                                            device,
                                                            param_name,
                                                            param_value_size,
                                                            param_value,
                                                            param_value_size_ret );
    }
    return errCode;
}


/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------

cl_int CL_API_CALL clRetainProgram( cl_program program )
{
	cl_int errCode = CL_SUCCESS;		
	
	CrtProgram* pgm = reinterpret_cast<CrtProgram*>(((_cl_program_crt*)program)->object);
	if (!pgm)
	{
		return CL_INVALID_PROGRAM;
		
	}
    pgm->m_refCount++;
		
	/// I don't forward retains to the underlying platforms, i only send release when
	/// the CRT ref counter for this queue reaches zero

	return errCode;
}

/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------

cl_int CL_API_CALL clReleaseProgram( cl_program program )
{
	cl_int errCode = CL_SUCCESS;		
	
	CrtProgram* pgm = reinterpret_cast<CrtProgram*>(((_cl_program_crt*)program)->object);
	if (!pgm)
	{
		return CL_INVALID_PROGRAM;
		
	}
	pgm->m_refCount--;
	
		/// No more references to this program, it can be deallocated
	if (pgm->m_refCount == 0)
	{	
        CrtContext* ctx = pgm->m_contextCRT;

        SHARED_CTX_DISPATCH::iterator itr = ctx->m_contexts.begin();
        for(;itr != ctx->m_contexts.end(); itr++)		
        {
            cl_context ctxObj = itr->first;
            errCode = ctxObj->dispatch->clReleaseProgram( pgm->m_ContextToProgram[ctxObj] );
        }
        	/// Deletes CrtProgram
        delete pgm;
			/// deletes CRT handle _cl_program_crt
        delete program;

	}
	return errCode;
}

/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------

// Kernel Object APIs
cl_kernel CL_API_CALL clCreateKernel( cl_program       program ,
                                      const char      *kernel_name ,
                                      cl_int          *errcode_ret )
{
    cl_int errCode = CL_SUCCESS;
    _cl_kernel_crt* SharedKernel = NULL;
    CrtKernel* ker = new CrtKernel;
    if( !ker )
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
    }

    CrtProgram* pgm = reinterpret_cast<CrtProgram*>(((_cl_program_crt*)program)->object);
    CrtContext* ctx = pgm->m_contextCRT;

    if( CL_SUCCESS == errCode ) 
    {
        SHARED_CTX_DISPATCH::iterator itr = ctx->m_contexts.begin();
	    for(;itr != ctx->m_contexts.end(); itr++)		
	    {
            cl_context ctxObj = itr->first;
            cl_kernel knlObj = ctxObj->dispatch->clCreateKernel( pgm->m_ContextToProgram[ctxObj],
                                                      kernel_name,
                                                      &errCode );
  		    if (CL_SUCCESS == errCode)
		    {
                ker->m_ContextToKernel[ctxObj] = knlObj;
            }
            else
            {
                break;
            }
        }
    }

	if (CL_SUCCESS == errCode)
	{
        ker->m_programCRT = pgm;
        ker->m_refCount = 1;
    }
    else
    {
            /// Release all previously allocated underlying kernel objects
        if( !ker->m_ContextToKernel.empty() )
        {
            cl_int errCode2 = CL_SUCCESS;
            CrtKernel::CTX_KER_MAP::iterator itr2 = ker->m_ContextToKernel.begin();
            for(;itr2 != ker->m_ContextToKernel.end(); itr2++)
            {
                errCode2 = itr2->first->dispatch->clReleaseKernel(itr2->second);
            }
        }
        delete ker;
    }


    if( CL_SUCCESS == errCode )
    {
        SharedKernel = new _cl_kernel_crt;
        if( SharedKernel == NULL )
        {
            delete SharedKernel;
            errCode = CL_OUT_OF_HOST_MEMORY;
        }
        else
        {
            SharedKernel->object = (void *) ker;
        }
    }

    if( errcode_ret )
    {
        *errcode_ret = errCode;
    }
    return SharedKernel;
}

/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------

cl_int CL_API_CALL clRetainKernel( cl_kernel kernel )
{
	cl_int errCode = CL_SUCCESS;		
	
	CrtKernel* ker = reinterpret_cast<CrtKernel*>(((_cl_kernel_crt*)kernel)->object);
	if (!ker)
	{
		return CL_INVALID_KERNEL;
		
	}
    ker->m_refCount++;
		
	/// I don't forward retains to the underlying platforms, i only send release when
	/// the CRT ref counter for this queue reaches zero

	return errCode;
}

/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------

cl_int CL_API_CALL clReleaseKernel( cl_kernel kernel )
{
    cl_int errCode = CL_SUCCESS;

	CrtKernel* ker = reinterpret_cast<CrtKernel*>(((_cl_kernel_crt*)kernel)->object);
	if (!ker)
	{
		return CL_INVALID_KERNEL;
		
	}
    ker->m_refCount--;

    	/// No more references to this kernel, it can be deallocated
	if (ker->m_refCount == 0)
	{
        CrtContext* ctx = ker->m_programCRT->m_contextCRT;

        SHARED_CTX_DISPATCH::iterator itr = ctx->m_contexts.begin();
	    for(;itr != ctx->m_contexts.end(); itr++)		
	    {
            cl_context ctxObj = itr->first;
            errCode = ctxObj->dispatch->clReleaseKernel( ker->m_ContextToKernel[ctxObj] );
	    }
        	/// Deletes CrtKernel
        delete ker;
			/// deletes CRT handle _cl_kernel_crt
        delete kernel;
    }

    return errCode;
}



/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clCreateSubDevicesEXT(
	cl_device_id								device,
	const cl_device_partition_property_ext*		properties,
	cl_uint										num_entries,
	cl_device_id*								out_devices,
	cl_uint*									num_devices)
{
	cl_int errCode = CL_SUCCESS;	
	CrtDeviceInfo** pDevicesInfo  = NULL;

	CrtDeviceInfo* parentDevInfo = OCLCRT::crt_ocl_module.m_deviceInfoMap[device];
	if (!parentDevInfo)
		return CL_INVALID_DEVICE;

	errCode = parentDevInfo->m_origDispatchTable.clCreateSubDevicesEXT(
				device, 
				properties,
				num_entries,
				out_devices,
				num_devices);
	
	if (CL_SUCCESS == errCode)
	{		
		pDevicesInfo = new CrtDeviceInfo*[*num_devices];
		for (cl_uint i=0; i< *num_devices; i++)
		{
			pDevicesInfo[i] = NULL;
		}
		if (!pDevicesInfo)
		{
			errCode = CL_OUT_OF_HOST_MEMORY;			
			return errCode;
		}
		for (cl_uint i=0; i< *num_devices; i++)
		{
			cl_device_id dev = out_devices[i];			
			pDevicesInfo[i] = new CrtDeviceInfo;
			if (NULL == pDevicesInfo[i])
			{
				errCode = CL_OUT_OF_HOST_MEMORY;
				break;
			}
			*(pDevicesInfo[i]) = *parentDevInfo;
			pDevicesInfo[i]->m_refCount = 1;
				/// This is a sub-device
			pDevicesInfo[i]->m_isRootDevice = false;
				/// Patch new created device IDs. some platforms don't use the same table
				/// for all handles (gpu), so we need to call Patch for each new created handle
			OCLCRT::crt_ocl_module.PatchClDeviceID(dev, NULL);
			OCLCRT::crt_ocl_module.m_deviceInfoMap[dev] = pDevicesInfo[i];
		}
			
		if (CL_SUCCESS != errCode)
		{
			for (cl_uint i=0; i< *num_devices; i++)
			{
				if (pDevicesInfo[i])
					delete pDevicesInfo[i];
			}
		}
	}
	return errCode;
}

/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------

cl_int CL_API_CALL clReleaseDeviceEXT(cl_device_id device)
{
	cl_int errCode = CL_SUCCESS;
			
	CrtDeviceInfo* devInfo = OCLCRT::crt_ocl_module.m_deviceInfoMap[device];

	if (devInfo == NULL)
		return CL_INVALID_DEVICE;
		
		/// No need to do anything for Root devices
	if (devInfo->m_isRootDevice)
		return CL_SUCCESS;
		
	errCode = devInfo->m_origDispatchTable.clReleaseDeviceEXT(device);

	/// If we reached here, then this is a sub-device
	atomic_decrement(&(devInfo->m_refCount));
	if (devInfo->m_refCount == 0)
	{
		OCLCRT::DEV_INFO_MAP::iterator itr = OCLCRT::crt_ocl_module.m_deviceInfoMap.find(device);		
		OCLCRT::crt_ocl_module.m_deviceInfoMap.erase(itr);
		delete devInfo;			
	}
	return errCode;
}

cl_int CL_API_CALL clRetainDeviceEXT(cl_device_id device)
{
	cl_int errCode = CL_SUCCESS;
	
	CrtDeviceInfo* devInfo = OCLCRT::crt_ocl_module.m_deviceInfoMap[device];

	if (devInfo == NULL)
		return CL_INVALID_DEVICE;

		/// No need to do anything for Root devices
	if (devInfo->m_isRootDevice)
		return CL_SUCCESS;
		
	errCode = devInfo->m_origDispatchTable.clRetainDeviceEXT(device);
		/// If we reached here, then this is a sub-device
	atomic_increment(&(devInfo->m_refCount));

	return errCode;
}

/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clSetMemObjectDestructorCallback(
	cl_mem			memObj,
	mem_dtor_fn		pfn_notify,
	void *			pUserData )
{
	cl_int errCode = CL_SUCCESS;

	CrtMemObject* crtMemObj = reinterpret_cast<CrtMemObject*>(((_cl_mem_crt*)memObj)->object);
	errCode = crtMemObj->RegisterDestructorCallback(pfn_notify, pUserData);

	return errCode;
}

/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clRetainEvent(cl_event event)
{	
	CrtEvent* crtEvent = reinterpret_cast<CrtEvent*>(((_cl_event_crt*)event)->object);

	atomic_increment(&crtEvent->m_refCount);		
	return CL_SUCCESS;
}

/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------

cl_int CL_API_CALL clReleaseEvent(cl_event event)
{
	cl_int errCode = CL_SUCCESS;
	CrtEvent* crtEvent = reinterpret_cast<CrtEvent*>(((_cl_event_crt*)event)->object);
	
	atomic_decrement(&crtEvent->m_refCount);
	if (crtEvent->m_refCount == 0)
	{
		errCode = crtEvent->m_eventDEV->dispatch->clReleaseEvent(event);		
		delete crtEvent;
		delete event;
	}
	return errCode;
}

/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------

cl_int CL_API_CALL clWaitForEvents(cl_uint num_events, const cl_event * event_list)
{
	cl_int errCode = CL_SUCCESS;

	//// Implements Option 1 from the Design document

	if (0 == num_events)
	{
		return CL_INVALID_VALUE;
	}		

	CrtContext* crtContext = NULL;	
	for(cl_uint i=0; i < num_events; i++)
	{
		CrtEvent* crtEvent = reinterpret_cast<CrtEvent*>(((_cl_event_crt*)(event_list[i]))->object);
		if (i == 0)
		{
			crtContext = crtEvent->getContext();
			continue;
		}
		if (crtEvent->getContext() != crtContext)
		{
			return CL_INVALID_CONTEXT;
		}		
	}

		/// accumulate events for the same underlying platform
	cl_event* pEvents = new cl_event[num_events];
	if (NULL == pEvents)
	{
		return CL_OUT_OF_HOST_MEMORY;
	}

	cl_uint current = 0;		
	SHARED_CTX_DISPATCH::iterator itr = crtContext->m_contexts.begin();
	for(;itr != crtContext->m_contexts.end(); itr++)		
	{
		for (cl_uint i=0; i < num_events; i++)
		{
			CrtEvent* crtEvent = reinterpret_cast<CrtEvent*>(((_cl_event_crt*)(event_list[i]))->object);
			if (crtEvent->m_isUserEvent)
			{		
					/// Pick the corrsponding queue event on that context
				pEvents[current++] = ((CrtUserEvent*)crtEvent)->m_ContextToEvent[itr->first];
			}
			else
			{					
				pEvents[current++] = crtEvent->m_eventDEV;
			}			
		}
		errCode = pEvents[0]->dispatch->clWaitForEvents(current, pEvents);

		if (CL_SUCCESS != errCode)
			return errCode;			
		current = 0;
	}	
	return errCode;
}
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clGetEventInfo(
	cl_event		event,
	cl_event_info	param_name,
	size_t			param_value_size,
	void *			param_value,
	size_t *		param_value_size_ret)
{
	cl_int errCode = CL_SUCCESS;

	CrtEvent* crtEvent = reinterpret_cast<CrtEvent*>(((_cl_event_crt*)event)->object);
	
	cl_event eventDEV = NULL;
	CrtContext* crtContext = NULL;
	if (crtEvent->m_isUserEvent)
	{	
		eventDEV = ((CrtUserEvent*)crtEvent)->m_ContextToEvent.begin()->second;
		crtContext = ((CrtUserEvent*)crtEvent)->m_pContext;
	}
	else
	{
		eventDEV = crtEvent->m_eventDEV;
		crtContext = crtEvent->m_queueCRT->m_contextCRT;

	}
	errCode = eventDEV->dispatch->clGetEventInfo( eventDEV, 
												param_name,
												param_value_size,
												param_value,
												param_value_size_ret);

	if (errCode == CL_SUCCESS && param_value)
	{
		if (param_name == CL_EVENT_CONTEXT)
		{
				/// since the app receives a context handle from the CRT, we need to assure
				/// we are returning the CRT context and not the underlying context.
			memcpy_s(param_value, 
				sizeof(cl_context),
				&(crtContext->m_context_handle),
				sizeof(cl_context));
		}
		else if (param_name == CL_EVENT_REFERENCE_COUNT)
		{
				/// The CRT maintains the correct reference count, so we need to return
				/// it from the CRT; recall CRT doesn't forward retain calls to the underlying
				/// platforms.
			*((cl_uint*)param_value) = crtEvent->m_refCount;
		}
	}		
	return errCode;
}

/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_event CL_API_CALL clCreateUserEvent(
	cl_context    context,
	cl_int *      errcode_ret)
{
	cl_int errCode = CL_SUCCESS;
	_cl_event_crt* event_handle = NULL;

	CrtContext* crtContext = reinterpret_cast<CrtContext*>(((_cl_context_crt*)context)->object);

	CrtUserEvent* crtUserEvent = new CrtUserEvent;
	if (NULL == crtUserEvent)
	{
		errCode = CL_OUT_OF_HOST_MEMORY;
		goto FINISH;
	}
	else
	{
		crtUserEvent->m_pContext = crtContext;
		SHARED_CTX_DISPATCH::iterator itr = crtContext->m_contexts.begin();	
		for(;itr != crtContext->m_contexts.end(); itr++)		
		{		
			cl_event eventDEV = itr->second->clCreateUserEvent(itr->first, &errCode);
			if (CL_SUCCESS != errCode)
			{
				goto FINISH;
			}
			crtUserEvent->m_ContextToEvent[itr->first] = eventDEV;
		}
	}

FINISH:
	
	if (CL_SUCCESS != errCode)
	{
		if (crtUserEvent)
			delete crtUserEvent;
	}
	else
	{		
		event_handle = new _cl_event_crt;
		if (!event_handle)
		{
			delete crtUserEvent;			
			errCode = CL_OUT_OF_HOST_MEMORY;			
		}
		else
		{
			event_handle->object = (void*)crtUserEvent;			
		}
		
	}
	if (errcode_ret)
		*errcode_ret = errCode;

	return event_handle;
}

/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clSetUserEventStatus(cl_event event, cl_int execution_status)
{
	cl_int errCode = CL_SUCCESS;

	CrtEvent* crtEvent = reinterpret_cast<CrtEvent*>(((_cl_event_crt*)event)->object);
	if (!crtEvent || false == crtEvent->m_isUserEvent)
	{
		return CL_INVALID_EVENT;
	}
	else
	{		
		CrtUserEvent* crtUserEVent = (CrtUserEvent*)crtEvent;
		std::map<cl_context, cl_event>::iterator itr = crtUserEVent->m_ContextToEvent.begin();	
		for(;itr != crtUserEVent->m_ContextToEvent.end(); itr++)		
		{		
			errCode = itr->second->dispatch->clSetUserEventStatus(itr->second, execution_status);
			if (CL_SUCCESS != errCode)
			{
				return errCode;
			}
		}
	}
	return errCode;
}

/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clSetEventCallback( 
	cl_event		event,
	cl_int			command_exec_callback_type,
	pfn_notify		notify_callback,
	void *			user_data)
{
	cl_int errCode = CL_SUCCESS;
	CrtEvent* crtEvent = reinterpret_cast<CrtEvent*>(((_cl_event_crt*)event)->object);
	if (!crtEvent)
	{
		return CL_INVALID_EVENT;
	}
	else
	{
		if (false == crtEvent->m_isUserEvent)
		{
			errCode = crtEvent->m_eventDEV->dispatch->clSetEventCallback(
				crtEvent->m_eventDEV,
				command_exec_callback_type,
				notify_callback,
				user_data);
		}
		else
		{
			CrtUserEvent* crtUserEVent = (CrtUserEvent*)crtEvent;
			std::map<cl_context, cl_event>::iterator itr = crtUserEVent->m_ContextToEvent.begin();	
				/// No point on registering the callback notification on all 
				/// underlying platforms, so we pick randomly the first one.
			errCode = itr->second->dispatch->clSetEventCallback(
				itr->second,
				command_exec_callback_type,
				notify_callback,
				user_data);
		}
	}
	return errCode;
}
// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clEnqueueNDRangeKernel(
	cl_command_queue	command_queue,
	cl_kernel			kernel,
	cl_uint				work_dim,
	const size_t *		global_work_offset,
	const size_t *		global_work_size,
	const size_t *		local_work_size,
	cl_uint				num_events_in_wait_list,
	const cl_event *	event_wait_list,
	cl_event *			event)
{
	cl_int errCode = CL_SUCCESS;

	if (command_queue == NULL)
		return CL_INVALID_COMMAND_QUEUE;

	if (kernel == NULL)
		return CL_INVALID_KERNEL;

	CrtQueue* queue = reinterpret_cast<CrtQueue*>(((_cl_command_queue_crt*)command_queue)->object);
	if (!queue)
		return CL_INVALID_COMMAND_QUEUE;
	
	SyncManager* synchHelper = new SyncManager;
	if (!synchHelper)
		return CL_OUT_OF_HOST_MEMORY;

	cl_event*	outEvents = NULL;
	cl_uint		numOutEvents = 0;
	errCode = synchHelper->PrepareToExecute(
		queue, 
		num_events_in_wait_list, 
		event_wait_list, 
		&numOutEvents, 
		&outEvents, 
		0, 
		NULL);

	if (CL_SUCCESS != errCode)
		goto FINISH;

	CrtEvent* crtEvent = new CrtEvent;	
	if (!crtEvent)
	{
		errCode = CL_OUT_OF_HOST_MEMORY;
		goto FINISH;
	}
	cl_context& targetContext = queue->m_contextCRT->m_DeviceToContext[queue->m_device];
	
	CrtKernel* crtKernel = reinterpret_cast<CrtKernel*>(((_cl_kernel_crt*)kernel)->object);	
	if (!crtKernel)
	{	
		errCode = CL_INVALID_KERNEL;
		goto FINISH;
	}		
	crtEvent->m_queueCRT = queue;	
	errCode = queue->m_cmdQueueDEV->dispatch->clEnqueueNDRangeKernel(
		queue->m_cmdQueueDEV,
		crtKernel->m_ContextToKernel[targetContext],
		work_dim,
		global_work_offset,
		global_work_size,
		local_work_size,
		numOutEvents,
		outEvents,
		&crtEvent->m_eventDEV);	
	
	if (errCode == CL_SUCCESS && event)
	{
		_cl_event_crt* event_handle = new _cl_event_crt;
		if (!event_handle)
		{
			delete crtEvent;
			crtEvent = NULL;
			errCode = CL_OUT_OF_HOST_MEMORY;
			goto FINISH;
		}
		event_handle->object = (void*)crtEvent;
		*event = event_handle;
	}
	
FINISH:
	if (CL_SUCCESS != errCode && crtEvent)
	{
		delete crtEvent;
	}
	if (synchHelper)
		delete synchHelper;	
	return errCode;
}
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clEnqueueTask(
	cl_command_queue	command_queue,
	cl_kernel			kernel,
	cl_uint				num_events_in_wait_list,
	const cl_event *	event_wait_list,
	cl_event *			event)
{
	cl_int errCode = CL_SUCCESS;

	if (command_queue == NULL)
		return CL_INVALID_COMMAND_QUEUE;

	if (kernel == NULL)
		return CL_INVALID_KERNEL;

	CrtQueue* queue = reinterpret_cast<CrtQueue*>(((_cl_command_queue_crt*)command_queue)->object);
	if (!queue)
		return CL_INVALID_COMMAND_QUEUE;
	
	SyncManager* synchHelper = new SyncManager;
	if (!synchHelper)
		return CL_OUT_OF_HOST_MEMORY;

	cl_event*	outEvents = NULL;
	cl_uint		numOutEvents = 0;
	errCode = synchHelper->PrepareToExecute(
		queue, 
		num_events_in_wait_list, 
		event_wait_list, 
		&numOutEvents, 
		&outEvents, 
		0, 
		NULL);

	if (CL_SUCCESS != errCode)
		goto FINISH;

	CrtEvent* crtEvent = new CrtEvent;	
	if (!crtEvent)
	{
		errCode = CL_OUT_OF_HOST_MEMORY;
		goto FINISH;
	}
	CrtKernel* crtKernel = reinterpret_cast<CrtKernel*>(((_cl_kernel_crt*)kernel)->object);	
	if (!crtKernel)
	{	
		errCode = CL_INVALID_KERNEL;
		goto FINISH;
	}		
	crtEvent->m_queueCRT = queue;	
	errCode = queue->m_cmdQueueDEV->dispatch->clEnqueueTask(
		queue->m_cmdQueueDEV,
		NULL,		
		numOutEvents,
		outEvents,
		&crtEvent->m_eventDEV);	
	
	if (errCode == CL_SUCCESS && event)
	{
		_cl_event_crt* event_handle = new _cl_event_crt;
		if (!event_handle)
		{
			delete crtEvent;
			crtEvent = NULL;
			errCode = CL_OUT_OF_HOST_MEMORY;
			goto FINISH;
		}
		event_handle->object = (void*)crtEvent;
		*event = event_handle;
	}
	
FINISH:
	if (CL_SUCCESS != errCode && crtEvent)
	{
		delete crtEvent;
	}
	if (synchHelper)
		delete synchHelper;	
	return errCode;
}
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clEnqueueMarker(cl_command_queue command_queue, cl_event * event)
{
	cl_int errCode = CL_SUCCESS;

	CrtQueue* queue = reinterpret_cast<CrtQueue*>(((_cl_command_queue_crt*)command_queue)->object);
	if (!queue)
	{
		return CL_INVALID_COMMAND_QUEUE;
	}
	CrtEvent* crtEvent = new CrtEvent;	
	if (!crtEvent)
	{
		return CL_OUT_OF_HOST_MEMORY;		
	}

	_cl_event_crt* event_handle = new _cl_event_crt;
	if (!event_handle)
	{
		delete crtEvent;
		return CL_OUT_OF_HOST_MEMORY;		
	}	

	cl_event eventDEV = NULL;
	errCode = queue->m_cmdQueueDEV->dispatch->clEnqueueMarker(queue->m_cmdQueueDEV, &eventDEV);
	if (CL_SUCCESS != errCode)
	{
		delete crtEvent;
		delete event_handle;
		return errCode;
	}

	crtEvent->m_eventDEV = eventDEV;
	crtEvent->m_queueCRT = queue;	
	event_handle->object = (void*)crtEvent;
	
	*event = event_handle;

	return errCode;
}
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------

cl_int CL_API_CALL clEnqueueBarrier(cl_command_queue command_queue)
{
	cl_int errCode = CL_SUCCESS;

	CrtQueue* queue = reinterpret_cast<CrtQueue*>(((_cl_command_queue_crt*)command_queue)->object);
	if (!queue)
	{
		return CL_INVALID_COMMAND_QUEUE;
	}
	errCode = queue->m_cmdQueueDEV->dispatch->clEnqueueBarrier(queue->m_cmdQueueDEV);
	return errCode;	
}
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clEnqueueWaitForEvents(
	cl_command_queue	command_queue,
	cl_uint				num_events_in_wait_list,
	const cl_event *	event_wait_list)
{
	cl_int errCode = CL_SUCCESS;

	if (command_queue == NULL)
		return CL_INVALID_COMMAND_QUEUE;
	
	CrtQueue* queue = reinterpret_cast<CrtQueue*>(((_cl_command_queue_crt*)command_queue)->object);
	if (!queue)
		return CL_INVALID_COMMAND_QUEUE;
	
	SyncManager* synchHelper = new SyncManager;
	if (!synchHelper)
		return CL_OUT_OF_HOST_MEMORY;

	cl_event*	outEvents = NULL;
	cl_uint		numOutEvents = 0;
	errCode = synchHelper->PrepareToExecute(
		queue, 
		num_events_in_wait_list, 
		event_wait_list, 
		&numOutEvents, 
		&outEvents, 
		0, 
		NULL);

	if (CL_SUCCESS != errCode)
		goto FINISH;
	
	errCode = queue->m_cmdQueueDEV->dispatch->clEnqueueWaitForEvents(
		queue->m_cmdQueueDEV,	
		numOutEvents,
		outEvents);
				
FINISH:
	if (synchHelper)
		delete synchHelper;	
	return errCode;
}
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clEnqueueNativeKernel(
	cl_command_queue	command_queue,
	user_func			user_native_func,
	void *				args,
	size_t				cb_args, 
	cl_uint				num_mem_objects,
	const cl_mem *		mem_list,
	const void **		args_mem_loc,
	cl_uint				num_events_in_wait_list,
	const cl_event *	event_wait_list,
	cl_event *			event)
{
	cl_int errCode = CL_SUCCESS;

	if (command_queue == NULL)
		return CL_INVALID_COMMAND_QUEUE;

	if (    ( NULL == user_native_func)                                             ||
            ( NULL == args && ((cb_args > 0) || num_mem_objects > 0 ))				||
            ( NULL != args && 0 == cb_args)											||
            ( (num_mem_objects >  0) && ( NULL == mem_list || NULL == args_mem_loc))||
            ( (0 == num_mem_objects) && ( NULL != mem_list || NULL != args_mem_loc)) )
    {
        return CL_INVALID_VALUE;
    }

	CrtQueue* queue = reinterpret_cast<CrtQueue*>(((_cl_command_queue_crt*)command_queue)->object);
	if (!queue)
		return CL_INVALID_COMMAND_QUEUE;

		/// Check if device support Native Kernels in reported device capabilities
	CrtDeviceInfo * devInfo = OCLCRT::crt_ocl_module.m_deviceInfoMap[queue->m_device];
	if (!(devInfo->m_deviceCapabilities & CL_EXEC_NATIVE_KERNEL)) 
	{        
		return CL_INVALID_OPERATION;
    }  

	SyncManager* synchHelper = new SyncManager;
	if (!synchHelper)
		return CL_OUT_OF_HOST_MEMORY;

	cl_event*	outEvents = NULL;
	cl_uint		numOutEvents = 0;
	errCode = synchHelper->PrepareToExecute(
		queue, 
		num_events_in_wait_list, 
		event_wait_list, 
		&numOutEvents, 
		&outEvents, 
		0, 
		NULL);

	if (CL_SUCCESS != errCode)
		goto FINISH;

	CrtEvent* crtEvent = new CrtEvent;	
	if (!crtEvent)
	{
		errCode = CL_OUT_OF_HOST_MEMORY;
		goto FINISH;
	}
	cl_mem* crt_mem_list = NULL;

	if (num_mem_objects > 0)
	{
		crt_mem_list = new cl_mem[num_mem_objects];
		if (!crt_mem_list)
		{
			delete crtEvent;
			errCode = CL_OUT_OF_HOST_MEMORY;
			goto FINISH;
		}
			///	Translate memory object handles
		for (cl_uint i=0; i < num_mem_objects; i++)
		{
			_cl_mem_crt * crtMemHandle = (_cl_mem_crt *)(mem_list[i]);
			CrtMemObject* crtMemObj = ((CrtMemObject*)crtMemHandle->object);
			crt_mem_list[i] = crtMemObj->getDeviceMemObj(queue->m_device);
		}
	}
	crtEvent->m_queueCRT = queue;	
	errCode = queue->m_cmdQueueDEV->dispatch->clEnqueueNativeKernel(
		queue->m_cmdQueueDEV,
		user_native_func,
		args,
		cb_args,
		num_mem_objects,
		crt_mem_list,
		args_mem_loc,
		numOutEvents,
		outEvents,
		&crtEvent->m_eventDEV);	
	
	if (errCode == CL_SUCCESS && event)
	{
		_cl_event_crt* event_handle = new _cl_event_crt;
		if (!event_handle)
		{
			delete crtEvent;
			crtEvent = NULL;
			errCode = CL_OUT_OF_HOST_MEMORY;
			goto FINISH;
		}
		event_handle->object = (void*)crtEvent;
		*event = event_handle;
	}
	
FINISH:	
	if (crt_mem_list)
		delete crt_mem_list;

	if (synchHelper)
		delete synchHelper;	
	return errCode;
}
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_mem CL_API_CALL clCreateImage2D(
	cl_context              context,
	cl_mem_flags            flags,
	const cl_image_format * image_format,
	size_t                  image_width,
	size_t                  image_height,
	size_t                  image_row_pitch,
	void *                  host_ptr,
	cl_int *                errcode_ret)
{
	_cl_mem_crt* mem_handle = NULL;
	cl_int errCode = CL_SUCCESS;	
	CrtContextInfo* ctxInfo  = NULL;

	ctxInfo = OCLCRT::crt_ocl_module.m_contextInfo[context];		
	if (!ctxInfo)
	{
		errCode = CL_INVALID_CONTEXT;
	}	
		
	if (CL_SUCCESS == errCode)
	{
		mem_handle = new _cl_mem_crt;
		if (!mem_handle)
		{
			errCode = CL_OUT_OF_HOST_MEMORY;
		}
	}
	if (CL_SUCCESS == errCode)
	{
		CrtContext* ctx = (CrtContext*)(ctxInfo->m_object);
		errCode = ctx->CreateImage(
			CL_MEM_OBJECT_IMAGE2D,
			flags,
			image_format,
			image_width,
			image_height,
			1,	// Depth
			image_row_pitch,
			0,	// Slice Pitch
			host_ptr,			
			(CrtMemObject**)(&mem_handle->object));
	}						
	
	if (errcode_ret)
		*errcode_ret = errCode;

	return mem_handle;
};
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_mem CL_API_CALL clCreateImage3D(
	cl_context              context,
	cl_mem_flags            flags,
	const cl_image_format * image_format,
	size_t                  image_width,
	size_t                  image_height,
	size_t                  image_depth,
	size_t                  image_row_pitch,
	size_t                  image_slice_pitch,
	void *                  host_ptr,
	cl_int *                errcode_ret)
{
	_cl_mem_crt* mem_handle = NULL;
	cl_int errCode = CL_SUCCESS;	
	CrtContextInfo* ctxInfo  = NULL;

	ctxInfo = OCLCRT::crt_ocl_module.m_contextInfo[context];		
	if (!ctxInfo)
	{
		errCode = CL_INVALID_CONTEXT;
	}	
		
	if (CL_SUCCESS == errCode)
	{
		mem_handle = new _cl_mem_crt;
		if (!mem_handle)
		{
			errCode = CL_OUT_OF_HOST_MEMORY;
		}
	}
	if (CL_SUCCESS == errCode)
	{
		CrtContext* ctx = (CrtContext*)(ctxInfo->m_object);
		errCode = ctx->CreateImage(
			CL_MEM_OBJECT_IMAGE3D,
			flags,
			image_format,
			image_width,
			image_height,
			image_depth,
			image_row_pitch,
			image_slice_pitch,
			host_ptr,			
			(CrtMemObject**)(&mem_handle->object));
	}						
	
	if (errcode_ret)
		*errcode_ret = errCode;

	return mem_handle;
}

/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL EnqueueReadWriteImage(
	bool				read_command,
	cl_command_queue	command_queue,
	cl_mem				image,
	cl_bool				blocking_cmd, 
	const size_t *		origin,
	const size_t *		region,
	size_t				row_pitch,
	size_t				slice_pitch, 
	const void *		ptr,
	cl_uint				num_events_in_wait_list,
	const cl_event *	event_wait_list,
	cl_event *			event)
{
	cl_int errCode = CL_SUCCESS;

	if (command_queue == NULL)
		return CL_INVALID_COMMAND_QUEUE;

	if (image == NULL)
		return CL_INVALID_MEM_OBJECT;

	CrtQueue* queue = reinterpret_cast<CrtQueue*>(((_cl_command_queue_crt*)command_queue)->object);
	if (!queue)
		return CL_INVALID_COMMAND_QUEUE;
	
	SyncManager* synchHelper = new SyncManager;
	if (!synchHelper)
		return CL_OUT_OF_HOST_MEMORY;

	cl_event*	outEvents = NULL;
	cl_uint		numOutEvents = 0;
	errCode = synchHelper->PrepareToExecute(
		queue, 
		num_events_in_wait_list, 
		event_wait_list, 
		&numOutEvents, 
		&outEvents, 
		0, 
		NULL);

	if (CL_SUCCESS != errCode)
		goto FINISH;

	CrtEvent* crtEvent = new CrtEvent;	
	if (!crtEvent)
	{
		errCode = CL_OUT_OF_HOST_MEMORY;
		goto FINISH;
	}
	CrtImage* crtImage = reinterpret_cast<CrtImage*>(((_cl_mem_crt*)image)->object);	
	if (!crtImage)
	{	errCode = CL_INVALID_MEM_OBJECT;
		goto FINISH;
	}		
	
	if (blocking_cmd)
	{
		errCode = queue->m_contextCRT->FlushQueues();
		if (CL_SUCCESS != errCode)
		{
			errCode = CL_OUT_OF_RESOURCES;
			goto FINISH;
		}
	}

	crtEvent->m_queueCRT = queue;
	if (read_command)
		errCode = queue->m_cmdQueueDEV->dispatch->clEnqueueReadImage(
			queue->m_cmdQueueDEV,
			crtImage->getDeviceMemObj(queue->m_device),
			blocking_cmd,
			origin,
			region,
			row_pitch,
			slice_pitch,
			const_cast<void*>(ptr),
			numOutEvents,
			outEvents,
			&crtEvent->m_eventDEV);
	else
		errCode = queue->m_cmdQueueDEV->dispatch->clEnqueueWriteImage(
			queue->m_cmdQueueDEV,
			crtImage->getDeviceMemObj(queue->m_device),
			blocking_cmd,
			origin,
			region,
			row_pitch,
			slice_pitch,
			ptr,
			numOutEvents,
			outEvents,
			&crtEvent->m_eventDEV);
	
	if (errCode == CL_SUCCESS && event)
	{
		_cl_event_crt* event_handle = new _cl_event_crt;
		if (!event_handle)
		{
			delete crtEvent;
			crtEvent = NULL;
			errCode = CL_OUT_OF_HOST_MEMORY;
			goto FINISH;
		}
		event_handle->object = (void*)crtEvent;
		*event = event_handle;
	}
	
FINISH:	
	if (CL_SUCCESS != errCode && crtEvent)
	{
		delete crtEvent;
	}
	if (synchHelper)
		delete synchHelper;	
	return errCode;
}
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clEnqueueWriteImage(
	cl_command_queue	command_queue,
	cl_mem				image,
	cl_bool				blocking_read, 
	const size_t *		origin,
	const size_t *		region,
	size_t				row_pitch,
	size_t				slice_pitch, 
	const void *		ptr,
	cl_uint				num_events_in_wait_list,
	const cl_event *	event_wait_list,
	cl_event *			event)
{
	return EnqueueReadWriteImage(
		false,
		command_queue,
		image,
		blocking_read,
		origin,
		region,
		row_pitch,
		slice_pitch,
		ptr,
		num_events_in_wait_list,
		event_wait_list,
		event);
};
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clEnqueueReadImage(cl_command_queue command_queue,
	cl_mem				image,
	cl_bool				blocking_read, 
	const size_t *		origin,
	const size_t *		region,
	size_t				row_pitch,
	size_t				slice_pitch, 
	void *				ptr,
	cl_uint				num_events_in_wait_list,
	const cl_event *	event_wait_list,
	cl_event *			event)
{
	return EnqueueReadWriteImage(
		true,
		command_queue,
		image,
		blocking_read,
		origin,
		region,
		row_pitch,
		slice_pitch,
		ptr,
		num_events_in_wait_list,
		event_wait_list,
		event);
};
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clEnqueueCopyImage(
	cl_command_queue	command_queue,
	cl_mem				src_image,
	cl_mem				dst_image, 
	const size_t * 		src_origin,
	const size_t * 		dst_origin,
	const size_t *		region, 
	cl_uint				num_events_in_wait_list,
	const cl_event *	event_wait_list,
	cl_event *			event)
{
	cl_int errCode = CL_SUCCESS;

	if (command_queue == NULL)
		return CL_INVALID_COMMAND_QUEUE;

	if (src_image == NULL || dst_image == NULL)
		return CL_INVALID_MEM_OBJECT;

	CrtQueue* queue = reinterpret_cast<CrtQueue*>(((_cl_command_queue_crt*)command_queue)->object);
	if (!queue)
		return CL_INVALID_COMMAND_QUEUE;
	
	SyncManager* synchHelper = new SyncManager;
	if (!synchHelper)
		return CL_OUT_OF_HOST_MEMORY;

	cl_event*	outEvents = NULL;
	cl_uint		numOutEvents = 0;
	errCode = synchHelper->PrepareToExecute(
		queue, 
		num_events_in_wait_list, 
		event_wait_list, 
		&numOutEvents, 
		&outEvents, 
		0, 
		NULL);

	if (CL_SUCCESS != errCode)
		goto FINISH;

	CrtEvent* crtEvent = new CrtEvent;
	if (!crtEvent)
	{
		errCode = CL_OUT_OF_HOST_MEMORY;
		goto FINISH;
	}
	CrtImage* crtSrcImage = reinterpret_cast<CrtImage*>(((_cl_mem_crt*)src_image)->object);	
	CrtImage* crtDstImage = reinterpret_cast<CrtImage*>(((_cl_mem_crt*)dst_image)->object);	
	if (!crtSrcImage || !crtDstImage)
	{	
		errCode = CL_INVALID_MEM_OBJECT;
		goto FINISH;
	}		

	crtEvent->m_queueCRT = queue;	
	errCode = queue->m_cmdQueueDEV->dispatch->clEnqueueCopyImage(
		queue->m_cmdQueueDEV,
		crtSrcImage->getDeviceMemObj(queue->m_device),
		crtDstImage->getDeviceMemObj(queue->m_device),
		src_origin,
		dst_origin,
		region,
		numOutEvents,
		outEvents,
		&crtEvent->m_eventDEV);	
	
	if (errCode == CL_SUCCESS && event)
	{
		_cl_event_crt* event_handle = new _cl_event_crt;
		if (!event_handle)
		{
			delete crtEvent;
			crtEvent = NULL;
			errCode = CL_OUT_OF_HOST_MEMORY;
			goto FINISH;
		}
		event_handle->object = (void*)crtEvent;
		*event = event_handle;
	}
	
FINISH:
	if (synchHelper)
		delete synchHelper;	
	return errCode;
}

/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clEnqueueCopyImageToBuffer(
	cl_command_queue	command_queue,
	cl_mem				src_image,
	cl_mem				dst_buffer,
	const size_t * 		src_origin,
	const size_t *  	region,
	size_t				dst_offset,
	cl_uint				num_events_in_wait_list,
	const cl_event *	event_wait_list,
	cl_event *			event)
{
	cl_int errCode = CL_SUCCESS;

	if (command_queue == NULL)
		return CL_INVALID_COMMAND_QUEUE;

	if (src_image == NULL || dst_buffer == NULL)
		return CL_INVALID_MEM_OBJECT;

	CrtQueue* queue = reinterpret_cast<CrtQueue*>(((_cl_command_queue_crt*)command_queue)->object);
	if (!queue)
		return CL_INVALID_COMMAND_QUEUE;
	
	SyncManager* synchHelper = new SyncManager;
	if (!synchHelper)
		return CL_OUT_OF_HOST_MEMORY;

	cl_event*	outEvents = NULL;
	cl_uint		numOutEvents = 0;
	errCode = synchHelper->PrepareToExecute(
		queue, 
		num_events_in_wait_list, 
		event_wait_list, 
		&numOutEvents, 
		&outEvents, 
		0, 
		NULL);

	if (CL_SUCCESS != errCode)
		goto FINISH;

	CrtEvent* crtEvent = new CrtEvent;
	if (!crtEvent)
	{
		errCode = CL_OUT_OF_HOST_MEMORY;
		goto FINISH;
	}
	CrtImage* crtSrcImage = reinterpret_cast<CrtImage*>(((_cl_mem_crt*)src_image)->object);	
	CrtBuffer* crtDstBuffer = reinterpret_cast<CrtBuffer*>(((_cl_mem_crt*)dst_buffer)->object);	
	if (!crtSrcImage || !crtDstBuffer)
	{	
		errCode = CL_INVALID_MEM_OBJECT;
		goto FINISH;
	}		

	crtEvent->m_queueCRT = queue;	
	errCode = queue->m_cmdQueueDEV->dispatch->clEnqueueCopyImageToBuffer(
		queue->m_cmdQueueDEV,
		crtSrcImage->getDeviceMemObj(queue->m_device),
		crtDstBuffer->getDeviceMemObj(queue->m_device),
		src_origin,
		region,
		dst_offset,		
		numOutEvents,
		outEvents,
		&crtEvent->m_eventDEV);	
	
	if (errCode == CL_SUCCESS && event)
	{
		_cl_event_crt* event_handle = new _cl_event_crt;
		if (!event_handle)
		{
			delete crtEvent;
			crtEvent = NULL;
			errCode = CL_OUT_OF_HOST_MEMORY;
			goto FINISH;
		}
		event_handle->object = (void*)crtEvent;
		*event = event_handle;
	}
	
FINISH:
	if (synchHelper)
		delete synchHelper;	
	return errCode;
}
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clEnqueueCopyBufferToImage(
	cl_command_queue	command_queue,
	cl_mem				src_buffer,
	cl_mem				dst_image,
	size_t				src_offset,
	const size_t * 		dst_origin,
	const size_t *  	region, 
	cl_uint				num_events_in_wait_list,
	const cl_event *	event_wait_list,
	cl_event *			event)
{
	cl_int errCode = CL_SUCCESS;

	if (command_queue == NULL)
		return CL_INVALID_COMMAND_QUEUE;

	if (src_buffer == NULL || dst_image == NULL)
		return CL_INVALID_MEM_OBJECT;

	CrtQueue* queue = reinterpret_cast<CrtQueue*>(((_cl_command_queue_crt*)command_queue)->object);
	if (!queue)
		return CL_INVALID_COMMAND_QUEUE;
	
	SyncManager* synchHelper = new SyncManager;
	if (!synchHelper)
		return CL_OUT_OF_HOST_MEMORY;

	cl_event*	outEvents = NULL;
	cl_uint		numOutEvents = 0;
	errCode = synchHelper->PrepareToExecute(
		queue, 
		num_events_in_wait_list, 
		event_wait_list, 
		&numOutEvents, 
		&outEvents, 
		0, 
		NULL);

	if (CL_SUCCESS != errCode)
		goto FINISH;

	CrtEvent* crtEvent = new CrtEvent;
	if (!crtEvent)
	{
		errCode = CL_OUT_OF_HOST_MEMORY;
		goto FINISH;
	}
	CrtBuffer* crtSrcBuffer = reinterpret_cast<CrtBuffer*>(((_cl_mem_crt*)src_buffer)->object);	
	CrtImage*  crtDstImage = reinterpret_cast<CrtImage*>(((_cl_mem_crt*)dst_image)->object);	
	if (!crtSrcBuffer || !crtDstImage)
	{	
		errCode = CL_INVALID_MEM_OBJECT;
		goto FINISH;
	}		

	crtEvent->m_queueCRT = queue;	
	errCode = queue->m_cmdQueueDEV->dispatch->clEnqueueCopyBufferToImage(
		queue->m_cmdQueueDEV,
		crtSrcBuffer->getDeviceMemObj(queue->m_device),
		crtDstImage->getDeviceMemObj(queue->m_device),
		src_offset,
		dst_origin,
		region,		
		numOutEvents,
		outEvents,
		&crtEvent->m_eventDEV);	
	
	if (errCode == CL_SUCCESS && event)
	{
		_cl_event_crt* event_handle = new _cl_event_crt;
		if (!event_handle)
		{
			delete crtEvent;
			crtEvent = NULL;
			errCode = CL_OUT_OF_HOST_MEMORY;
			goto FINISH;
		}
		event_handle->object = (void*)crtEvent;
		*event = event_handle;
	}
	
FINISH:
	if (synchHelper)
		delete synchHelper;	
	return errCode;
}
