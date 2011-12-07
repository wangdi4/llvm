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
#include <string>
#include <numeric>
#include <vector>
#include <sstream>
#include <stdio.h>


namespace OCLCRT
{
    CrtModule crt_ocl_module;

        /// Globally Initialized Variable
    char* CrtModule::m_common_extensions = NULL;
};


    /// Macros
#define isValidPlatform(X) ((X) == OCLCRT::crt_ocl_module.m_crtPlatformId || NULL == (X))

/// Defined CRT CL handles

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

_cl_sampler_crt::_cl_sampler_crt()
{
    dispatch = &OCLCRT::crt_ocl_module.m_icdDispatchMgr.m_crtDispatchTable;
};


/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clGetPlatformIDs(
    cl_uint           num_entries,
    cl_platform_id *  platforms,
    cl_uint *         num_platforms )
{
    if( ( ( 0 == num_entries ) && ( NULL != platforms ) )||
        ( ( NULL == num_platforms ) && ( NULL == platforms ) ) )
    {
        return CL_INVALID_VALUE;
    }

    if( CRT_FAIL == OCLCRT::crt_ocl_module.Initialize() )
    {
        return CL_OUT_OF_HOST_MEMORY;
    }

    if( num_platforms != NULL )
    {
        *num_platforms = 1;
    }

    if( ( platforms != NULL ) && ( num_entries >= 1 ) )
    {
        platforms[0] = OCLCRT::crt_ocl_module.m_crtPlatformId;
    }

    return CL_SUCCESS;
};

    /// Helper function for 'UpdatePlatformExtensions'
inline std::string concat_spaced_strings(std::string a, std::string b)
{
    if (a.compare(""))
    {
        return a+" "+b;
    }
    else
    {
        return b;
    }
};

    /// Detects what extensions are in common for all the
    /// the managed platforms.
void UpdatePlatformExtensions()
{
    if (NULL == OCLCRT::crt_ocl_module.m_common_extensions)
    {
            /// common extensions initialization needs to be thread-safe
        OCLCRT::Utils::OclAutoMutex CS(&OCLCRT::crt_ocl_module.m_mutex);
            /// Check if any other thread has done the initialization for us
        if (NULL == OCLCRT::crt_ocl_module.m_common_extensions)
        {
            std::vector<std::string> common_exts;
            for (cl_uint i=0; i < OCLCRT::crt_ocl_module.m_oclPlatforms.size(); i++)
            {
                CrtPlatform* crtPlatform = OCLCRT::crt_ocl_module.m_oclPlatforms[i];
                std::stringstream platExt(crtPlatform->m_supportedExtensionsStr);
                std::istream_iterator<std::string> platExt_it(platExt);
                std::istream_iterator<std::string> platExt_end;
                if (i==0)
                {
                        /// start with all extensions from the first platform
                    common_exts.assign(platExt_it, platExt_end);
                    continue;
                }
                std::vector<std::string> platExt_vec(platExt_it, platExt_end);
                    /// sort the vectors, required by set_intersection
                std::sort(common_exts.begin(),common_exts.end());
                std::sort(platExt_vec.begin(),platExt_vec.end());
                    /// Do the intersection in-place to common_exts
                std::vector<std::string>::iterator it = std::set_intersection( common_exts.begin(),
                                                                               common_exts.end(),
                                                                               platExt_vec.begin(),
                                                                               platExt_vec.end(),
                                                                               common_exts.begin() );

                common_exts.erase(it,common_exts.end());
            }
            std::string ret = std::accumulate(common_exts.begin(), common_exts.end(), std::string(""), concat_spaced_strings);
            OCLCRT::crt_ocl_module.m_common_extensions = new char[ret.size()+1];
            if (OCLCRT::crt_ocl_module.m_common_extensions != NULL)
            {
                strcpy_s(OCLCRT::crt_ocl_module.m_common_extensions, ret.size()+1, ret.c_str());
            }
        }
    }
}

/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clGetPlatformInfo(
    cl_platform_id      platform,
    cl_platform_info    param_name,
    size_t              param_value_size,
    void*               param_value,
    size_t*             param_value_size_ret )
{
    size_t    RetSize = 0;

    if( ( platform != NULL) && !( isValidPlatform( platform ) ) )
    {
        return CL_INVALID_PLATFORM;
    }

    switch( param_name )
    {
        case CL_PLATFORM_PROFILE:
            // strnlen_s calculates without null terminator.
            RetSize = strnlen_s( INTEL_PLATFORM_PROFILE, MAX_STRLEN );
            RetSize++;
            if( param_value )
            {
                if( RetSize > param_value_size )
                {
                    return CL_INVALID_VALUE;
                }
                strncpy_s( ( char * )param_value, RetSize, INTEL_PLATFORM_PROFILE, RetSize );
            }
            break;
        case CL_PLATFORM_VERSION:
            // strnlen_s calculates without null terminator.
            RetSize = strnlen_s( INTEL_PLATFORM_VERSION, MAX_STRLEN );
            RetSize++;
            if( param_value )
            {
                if( RetSize > param_value_size )
                {
                    return CL_INVALID_VALUE;
                }
                strncpy_s( ( char * )param_value, RetSize, INTEL_PLATFORM_VERSION, RetSize );
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
            RetSize = strnlen_s( INTEL_PLATFORM_VENDOR, MAX_STRLEN );
            RetSize++;
            if( param_value )
            {
                if( RetSize > param_value_size )
                {
                    return CL_INVALID_VALUE;
                }
                strncpy_s( ( char * )param_value, RetSize, INTEL_PLATFORM_VENDOR, RetSize );
            }
            break;
        case CL_PLATFORM_EXTENSIONS:

            UpdatePlatformExtensions();
                /// there was an allocation failure in UpdatePlatformExtensions
            if (NULL == OCLCRT::crt_ocl_module.m_common_extensions)
            {
                return CL_OUT_OF_HOST_MEMORY;
            }
            // strnlen_s calculates without null terminator.
            RetSize = strnlen_s(OCLCRT::crt_ocl_module.m_common_extensions, MAX_STRLEN );
            RetSize++;
            if( param_value )
            {
                if( RetSize > param_value_size )
                {
                    return CL_INVALID_VALUE;
                }
                strncpy_s( ( char * )param_value, RetSize, OCLCRT::crt_ocl_module.m_common_extensions, RetSize );
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
    cl_platform_id  platform,
    cl_device_type  device_type,
    cl_uint         num_entries,
    cl_device_id*   devices,
    cl_uint*        num_devices )
{

    if( !isValidPlatform( platform ) )
    {
        return CL_INVALID_PLATFORM;
    }


    if( !( device_type & CL_DEVICE_TYPE_DEFAULT )     &&
        !( device_type & CL_DEVICE_TYPE_CPU )         &&
        !( device_type & CL_DEVICE_TYPE_GPU )         &&
        !( device_type & CL_DEVICE_TYPE_ACCELERATOR ) )
    {
        return CL_INVALID_DEVICE_TYPE;
    }

    if( ( ( NULL != devices ) && ( 0 == num_entries ) ) ||
        ( ( NULL == devices ) && ( NULL == num_devices ) ) )
    {
        return CL_INVALID_VALUE;
    }

    num_entries = ( devices == NULL ) ? 0 : num_entries;
    cl_uint numRet = 0;

    cl_int errCode = CL_SUCCESS;

    // Lock Devices Map, devices might be concurrently modifed
    OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.Lock();

    for( OCLCRT::DEV_INFO_MAP::const_iterator itr = OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.get().begin();
         itr != OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.get().end();
         itr++ )
    {
        cl_device_type devType;

        const cl_device_id& devIdDEV = itr->first;

        CrtDeviceInfo* devInfo = itr->second;
        if( devInfo->m_isRootDevice == false )
        {
            // Skip SubDevices; they should not be returned for this query.
            continue;
        }
        if( !( CL_SUCCESS == devIdDEV->dispatch->clGetDeviceInfo(
                                        devIdDEV,
                                        CL_DEVICE_TYPE,
                                        sizeof( cl_device_type ),
                                        (void*)( &devType ),
                                        NULL ) ) )
        {
            errCode = CL_OUT_OF_HOST_MEMORY;
            goto FINISH;
        }

        if( ( device_type == CL_DEVICE_TYPE_DEFAULT )   ||
            ( device_type == CL_DEVICE_TYPE_ALL )       ||
            ( devType == device_type ) )
        {
            if( devices && ( numRet < num_entries ) )
            {
                devices[numRet++] = devIdDEV;
            }
            else
            {
                numRet++;
            }
        }
    }

    if( num_devices )
    {
        if( ( device_type == CL_DEVICE_TYPE_DEFAULT ) && ( numRet >= 1 ) )
        {
            *num_devices = 1;
        }
        else
        {
            *num_devices = numRet;
        }
    }
FINISH:
    // Unlock Devices Map, done with it
    OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.Release();
    return errCode;
}


/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_context CL_API_CALL clCreateContext(
    const cl_context_properties *   properties,
    cl_uint                         num_devices,
    const cl_device_id *            devices,
    logging_fn                      pfn_notify,
    void *                          user_data,
    cl_int *                        errcode_ret )
{
    cl_int errCode = CL_SUCCESS;
    cl_context ctx = NULL;

    errCode = OCLCRT::crt_ocl_module.isValidProperties( properties );
    if( CL_SUCCESS != errCode )
    {
        goto FINISH;
    }

    if( !devices )
    {
        errCode = CL_INVALID_VALUE;
        goto FINISH;
    }

    if( !pfn_notify && user_data )
    {
        errCode = CL_INVALID_VALUE;
        goto FINISH;
    }

    cl_uint numPlatforms = 0;
    cl_platform_id* pPlatformIdDEV = NULL;

    for( cl_uint i=0; i < num_devices; i++ )
    {
        const cl_device_id& devIdDEV = devices[i];

        if( ( NULL == devIdDEV ) ||
            ( NULL == OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.GetValue( devIdDEV ) ) )
        {
            errCode = CL_INVALID_DEVICE;
            goto FINISH;
        }

        if( i==0 )
        {
            pPlatformIdDEV = &OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.GetValue(devIdDEV)->m_platformIdDEV;
        }
        else
        {
            if( *pPlatformIdDEV ==  OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.GetValue(devIdDEV)->m_platformIdDEV)
            {
                continue;
            }
        }
        numPlatforms++;
    }


    if( ( errCode == CL_SUCCESS ) && ( numPlatforms == 1 ) )
    {
        // Single Platform Context (All devices belong to same underlying platform)
        const KHRicdVendorDispatch* dTable =
            (KHRicdVendorDispatch*)( &OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.GetValue( devices[0] )->m_origDispatchTable );

        cl_context_properties* props;
        if( CRT_FAIL == OCLCRT::ReplacePlatformId(
                            properties,
                            OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.GetValue( devices[0] )->m_platformIdDEV,
                            &props ) )
        {
            errCode = CL_OUT_OF_HOST_MEMORY;
        }
        else
        {
            CrtContextInfo* pContextInfo = new CrtContextInfo;
            if( !pContextInfo )
            {
                errCode = CL_OUT_OF_HOST_MEMORY;
                goto FINISH;
            }
            ctx =  dTable->clCreateContext(
                props,
                num_devices,
                devices,
                pfn_notify,
                user_data,
                &errCode );
            if( ctx != NULL )
            {
                OCLCRT::crt_ocl_module.PatchClContextID(
                    ctx,
                    &OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.GetValue( devices[0])->m_origDispatchTable );

                pContextInfo->m_contextType = CrtContextInfo::SinglePlatformContext;
                pContextInfo->m_platformId = OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.GetValue(devices[0])->m_platformIdDEV;
                pContextInfo->m_object = (void*)( &( OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.GetValue( devices[0] )->m_origDispatchTable ) );
                OCLCRT::crt_ocl_module.m_contextInfoGuard.Add( ctx, pContextInfo );
            }
            else
            {
                delete pContextInfo;
                goto FINISH;
            }
        }
        // free allocated properties memory
        if( props )
        {
            delete props;
            props = NULL;
        }
    }
    else
    {
        // Shared Platform Context

        // We don't support GL shared context now!
        if( OCLCRT::isGLContext( properties ) )
        {
            errCode = CL_INVALID_DEVICE;
            goto FINISH;
        }

        ctx = new _cl_context_crt;
        if( !ctx )
        {
            errCode = CL_OUT_OF_HOST_MEMORY;
            goto FINISH;
        }

        CrtContext* crtContext = new CrtContext(
            ctx,
            properties,
            num_devices,
            devices,
            pfn_notify,
            user_data,
            &errCode );

        if (NULL == crtContext)
        {
            errCode = CL_OUT_OF_HOST_MEMORY;
            goto FINISH;
        }
        else if (CL_SUCCESS != errCode)
        {
            delete ctx;
            crtContext->Release();
            crtContext->DecPendencyCnt();
            goto FINISH;
        }

        ((_cl_context_crt*)ctx)->object = (void*)crtContext;

        CrtContextInfo* pContextInfo = new CrtContextInfo;
        if( !pContextInfo )
        {
            errCode = CL_OUT_OF_HOST_MEMORY;
            delete ctx;

            crtContext->Release();
            crtContext->DecPendencyCnt();
            goto FINISH;
        }
        pContextInfo->m_contextType = CrtContextInfo::SharedPlatformContext;
        pContextInfo->m_object = (void*)crtContext;

        OCLCRT::crt_ocl_module.m_contextInfoGuard.Add( ctx,pContextInfo );
    }
FINISH:

    if( NULL != errcode_ret )
    {
        *errcode_ret = errCode;
    }

    return ctx;
}


/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_context CL_API_CALL clCreateContextFromType(
    const cl_context_properties *   properties,
    cl_device_type                  device_type,
    logging_fn                      pfn_notify,
    void *                          user_data,
    cl_int *                        errcode_ret)
{
    cl_int errCode = CL_SUCCESS;

    errCode = OCLCRT::crt_ocl_module.isValidProperties( properties );
    if( CL_SUCCESS != errCode )
    {
        if(errcode_ret)
        {
            *errcode_ret = errCode;
        }
        return NULL;
    }

    if (!(device_type & CL_DEVICE_TYPE_DEFAULT)     &&
        !(device_type & CL_DEVICE_TYPE_CPU)         &&
        !(device_type & CL_DEVICE_TYPE_GPU)         &&
        !(device_type & CL_DEVICE_TYPE_ACCELERATOR))
    {
        errCode = CL_INVALID_DEVICE_TYPE;
    }

    if (!pfn_notify && user_data)
    {
        errCode = CL_INVALID_VALUE;
    }

    cl_context  ctx = NULL;
    cl_uint numPlatforms = 0;
    cl_uint numDevices = 0;
    cl_platform_id pId;
    cl_device_id* deviceList = NULL;

    size_t numPlatformDevices = 0;
    numPlatformDevices = OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.size();

    deviceList = new cl_device_id[numPlatformDevices];
    if (deviceList == NULL)
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
    }
    if (CL_SUCCESS != errCode)
    {
        if(errcode_ret)
        {
            *errcode_ret = errCode;
        }

        return NULL;
    }

    OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.Lock();

    // In case there is only one underlying device, we pick it as CL_DEFAULT_DEVICE_TYPE
    bool OnlyOneAvailableDevice = (numPlatformDevices == 1);

    for (OCLCRT::DEV_INFO_MAP::iterator itr = OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.get().begin();
        itr != OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.get().end();
        itr++)
    {
        CrtDeviceInfo* devInfo = itr->second;
        if (devInfo->m_isRootDevice == false)
        {
            // Skip Sub-Devices
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
            // In case of two-devices pick the CRT default
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
                pId =   devInfo->m_platformIdDEV;
            }
            else
            {
                if (pId != devInfo->m_platformIdDEV)
                {
                    numPlatforms++;
                }
            }
            deviceList[numDevices++] = itr->first;
        }
    }
    OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.Release();

    if (CL_SUCCESS == errCode)
    {
        if (numDevices == 0)
        {
            errCode = CL_DEVICE_NOT_FOUND;
        }
        else
        {
            ctx = clCreateContext(properties, numDevices, deviceList, pfn_notify, user_data, &errCode);
        }
    }

    if (errcode_ret)
    {
        *errcode_ret = errCode;
    }

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

    CrtContextInfo* ctxInfo = OCLCRT::crt_ocl_module.m_contextInfoGuard.GetValue(context);
    if (ctxInfo->m_contextType == CrtContextInfo::SinglePlatformContext)
    {
        // Single Platform Context

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
            // replace the underlying platform_id with the CRT platform id
            OCLCRT::ReplacePlatformId(
                (cl_context_properties*)param_value,
                OCLCRT::crt_ocl_module.m_crtPlatformId, NULL, false);
        }
    }
    else
    {

        if (!param_value_size_ret && !param_value)
        {
            return CL_INVALID_VALUE;
        }

        // Shared Platform Contxt
        CrtContext* ctx = (CrtContext*)(ctxInfo->m_object);

        size_t pValueSize = 0;

        switch (param_name)
        {
        case CL_CONTEXT_REFERENCE_COUNT:
            {
                DEV_CTX_MAP::iterator itr = ctx->m_DeviceToContext.begin();
                errCode = OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.GetValue(itr->first)->m_origDispatchTable.clGetContextInfo(
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
                errCode = OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.GetValue(itr->first)->m_origDispatchTable.clGetContextInfo(
                    itr->second,
                    param_name,
                    param_value_size,
                    param_value,
                    param_value_size_ret);

                if (errCode == CL_SUCCESS)
                {
                    // replace the underlying platform_id with the CRT platform id
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
        {
            return CL_INVALID_VALUE;
        }
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

        /// No supported for GL in a shared Context yet!
    cl_int retCode = CL_SUCCESS;

    if (param_value == NULL && param_value_size_ret == NULL)
    {
        return CL_INVALID_VALUE;
    }

    if (CL_SUCCESS != (retCode = OCLCRT::crt_ocl_module.isValidProperties(properties)))
    {
        return retCode;
    }

    switch (param_name)
    {
        case CL_CURRENT_DEVICE_FOR_GL_CONTEXT_KHR:
        case CL_DEVICES_FOR_GL_CONTEXT_KHR:
            {
                cl_uint numDevices=0;
                for (cl_uint i = 0; i < OCLCRT::crt_ocl_module.m_oclPlatforms.size(); i++)
                {
                    CrtPlatform* crtPlatform = OCLCRT::crt_ocl_module.m_oclPlatforms[i];

                    cl_context_properties* props = NULL;
                    if (CRT_FAIL == OCLCRT::ReplacePlatformId(properties, crtPlatform->m_platformIdDEV, &props))
                    {
                        retCode = CL_OUT_OF_HOST_MEMORY;
                        goto FINISH;
                    }
                    retCode = crtPlatform->m_platformIdDEV->dispatch->clGetGLContextInfoKHR( props,
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
                    {
                        goto FINISH;
                    }
                }
                break;
            }
        default:
            {
                retCode = CL_INVALID_VALUE;
                goto FINISH;
            }
    }
FINISH:
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

    CrtDeviceInfo* devInfo = OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.GetValue(device);
    if (NULL == devInfo)
    {
        return CL_INVALID_DEVICE;
    }

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

    ctxInfo = OCLCRT::crt_ocl_module.m_contextInfoGuard.GetValue(context);
    if (!ctxInfo)
    {
        errCode = CL_INVALID_CONTEXT;
        goto FINISH;
    }

    CrtContext* ctx = (CrtContext*)(ctxInfo->m_object);
    errCode = ctx->CreateCommandQueue(device, properties, &queue);

    if (CL_SUCCESS != errCode)
    {
        goto FINISH;
    }

    queue_handle = new _cl_command_queue_crt;
    if (!queue_handle)
    {
        queue->Release();
        queue->DecPendencyCnt();
        errCode = CL_OUT_OF_HOST_MEMORY;
    }
    else
    {
        queue_handle->object = (void*)queue;
    }

FINISH:
    if (errcode_ret)
        *errcode_ret = errCode;

    return queue_handle;
}
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clGetCommandQueueInfo(
    cl_command_queue      command_queue,
    cl_command_queue_info param_name,
    size_t                param_value_size,
    void *                param_value,
    size_t *              param_value_size_ret)
{
    cl_int errCode = CL_SUCCESS;
    CrtQueue* crtQueue = reinterpret_cast<CrtQueue*>(((_cl_command_queue_crt*)command_queue)->object);

    size_t  pValueSize = 0;
    switch(param_name)
    {
    case CL_QUEUE_REFERENCE_COUNT:
        {
            pValueSize = sizeof(crtQueue->m_refCount);
            if (param_value && param_value_size >= pValueSize)
            {
                *((cl_uint*)param_value) = crtQueue->m_refCount;
            }
        }
        break;
    case CL_QUEUE_CONTEXT:
        {
            pValueSize = sizeof(cl_context);
            if (param_value && param_value_size >= pValueSize)
            {
                *((cl_context*)param_value) = crtQueue->m_contextCRT->m_context_handle;
            }

            if( param_value_size_ret )
            {
                *param_value_size_ret = pValueSize;
            }
        }
        break;
    default:
        {
            errCode = crtQueue->m_cmdQueueDEV->dispatch->clGetCommandQueueInfo(
                crtQueue->m_cmdQueueDEV,
                param_name,
                param_value_size,
                param_value,
                param_value_size_ret);
        }
        break;
    }
    return errCode;
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

    ctxInfo = OCLCRT::crt_ocl_module.m_contextInfoGuard.GetValue(context);
    if (!ctxInfo)
    {
        errCode = CL_INVALID_CONTEXT;
        goto FINISH;
    }

    mem_handle = new _cl_mem_crt;
    if (!mem_handle)
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }

    CrtContext* ctx = (CrtContext*)(ctxInfo->m_object);
    errCode = ctx->CreateBuffer(
        flags,
        size,
        host_ptr,
        (CrtMemObject**)(&mem_handle->object));

FINISH:
    if (CL_SUCCESS != errCode)
    {
        delete mem_handle;
        mem_handle = NULL;
    }
    if (errcode_ret)
    {
        *errcode_ret = errCode;
    }

    return mem_handle;
}


/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_mem CL_API_CALL clCreateSubBuffer(
    cl_mem                  buffer,
    cl_mem_flags            flags,
    cl_buffer_create_type   buffer_create_type,
    const void *            buffer_create_info,
    cl_int *                errcode_ret)
{
    _cl_mem_crt* mem_handle = NULL;
    cl_int errCode = CL_SUCCESS;

    // We check this now since we rely on this internally
    // to decide if to create a buffer or a sub-buffer
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

    mem_handle = new _cl_mem_crt;
    if (!mem_handle)
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }

    errCode = crtBuffer->m_pContext->CreateSubBuffer(
        (_cl_mem_crt*)buffer,
        flags,
        buffer_create_type,
        buffer_create_info,
        (CrtMemObject**)(&mem_handle->object));


FINISH:
    if (CL_SUCCESS != errCode)
    {
        delete mem_handle;
        mem_handle = NULL;
    }
    if (errcode_ret)
    {
        *errcode_ret = errCode;
    }

    return mem_handle;
}


/// ------------------------------------------------------------------------------
/// Commmon Runtime Helper function (Read/Write Buffer)
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL EnqueueReadWriteBuffer(
    bool                read_command,
    cl_command_queue    command_queue,
    cl_mem              buffer,
    cl_bool             blocking_cmd,
    size_t              offset,
    size_t              cb,
    const void *        ptr,
    cl_uint             num_events_in_wait_list,
    const cl_event *    event_wait_list,
    cl_event *          event)
{
    cl_int errCode = CL_SUCCESS;
    CrtEvent* crtEvent = NULL;

    if (command_queue == NULL)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    if (buffer == NULL)
    {
        return CL_INVALID_MEM_OBJECT;
    }

    CrtQueue* queue = reinterpret_cast<CrtQueue*>(((_cl_command_queue_crt*)command_queue)->object);
    if (!queue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    SyncManager* synchHelper = new SyncManager;
    if (!synchHelper)
    {
        return CL_OUT_OF_HOST_MEMORY;
    }

    cl_event*   outEvents = NULL;
    cl_uint     numOutEvents = 0;
    errCode = synchHelper->PrepareToExecute(
        queue,
        num_events_in_wait_list,
        event_wait_list,
        &numOutEvents,
        &outEvents,
        0,
        NULL);

    if (CL_SUCCESS != errCode)
    {
        goto FINISH;
    }

    crtEvent = new CrtEvent(queue);
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

	if (read_command)
	{
		if ( crtBuffer->m_flags & (CL_MEM_HOST_NO_ACCESS | CL_MEM_HOST_WRITE_ONLY) )
		{
	        errCode = CL_INVALID_OPERATION;
			goto FINISH;
		}
	} else {
		if ( crtBuffer->m_flags & (CL_MEM_HOST_NO_ACCESS | CL_MEM_HOST_READ_ONLY) )
		{
	        errCode = CL_INVALID_OPERATION;
			goto FINISH;
		}
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

    if (read_command)
    {
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
    }
    else
    {
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
    }

    if (errCode == CL_SUCCESS && event)
    {
        _cl_event_crt* event_handle = new _cl_event_crt;
        if (!event_handle)
        {
            errCode = CL_OUT_OF_HOST_MEMORY;
            goto FINISH;
        }
        event_handle->object = (void*)crtEvent;
        *event = event_handle;
    }

FINISH:
    if (crtEvent && (!event || (CL_SUCCESS != errCode)))
    {
        crtEvent->Release();
        crtEvent->DecPendencyCnt();
    }
    if (synchHelper)
    {
        synchHelper->Release(errCode);
        delete synchHelper;
    }
    return errCode;
}


/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
inline cl_int CL_API_CALL clEnqueueReadBuffer(
    cl_command_queue    command_queue,
    cl_mem              buffer,
    cl_bool             blocking_write,
    size_t              offset,
    size_t              cb,
    void *              ptr,
    cl_uint             num_events_in_wait_list,
    const cl_event *    event_wait_list,
    cl_event *          event)
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


/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
inline cl_int CL_API_CALL clEnqueueWriteBuffer(
    cl_command_queue    command_queue,
    cl_mem              buffer,
    cl_bool             blocking_write,
    size_t              offset,
    size_t              cb,
    const void *        ptr,
    cl_uint             num_events_in_wait_list,
    const cl_event *    event_wait_list,
    cl_event *          event)
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
    bool                read_command,
    cl_command_queue    command_queue,
    cl_mem              buffer,
    cl_bool             blocking_cmd,
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
    cl_event *          event )
{
    cl_int errCode = CL_SUCCESS;
    CrtEvent* crtEvent = NULL;

    if (!command_queue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    if (!buffer)
    {
        return CL_INVALID_MEM_OBJECT;
    }

    CrtQueue* queue = reinterpret_cast<CrtQueue*>(((_cl_command_queue_crt*)command_queue)->object);
    if (!queue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    SyncManager* synchHelper = new SyncManager;
    if (!synchHelper)
    {
        return CL_OUT_OF_HOST_MEMORY;
    }

    cl_event*   outEvents = NULL;
    cl_uint     numOutEvents = 0;
    errCode = synchHelper->PrepareToExecute(
        queue,
        num_events_in_wait_list,
        event_wait_list,
        &numOutEvents,
        &outEvents,
        0,
        NULL);

    if (CL_SUCCESS != errCode)
    {
        goto FINISH;
    }

    crtEvent = new CrtEvent(queue);
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

    if (read_command)
    {
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
    }
    else
    {
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
    }

    if (errCode == CL_SUCCESS && event)
    {
        _cl_event_crt* event_handle = new _cl_event_crt;
        if (!event_handle)
        {
            errCode = CL_OUT_OF_HOST_MEMORY;
            goto FINISH;
        }
        event_handle->object = (void*)crtEvent;
        *event = event_handle;
    }

FINISH:
    if (crtEvent && (!event || (CL_SUCCESS != errCode)))
    {
        crtEvent->Release();
        crtEvent->DecPendencyCnt();
    }
    if (synchHelper)
    {
        synchHelper->Release(errCode);
        delete synchHelper;
    }
    return errCode;
}


/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
inline cl_int CL_API_CALL clEnqueueReadBufferRect(
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
    void *              ptr,
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


/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
inline cl_int CL_API_CALL clEnqueueWriteBufferRect(
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
    CrtEvent* crtEvent = NULL;

    if (command_queue == NULL)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    if (src_buffer == NULL || dst_buffer == NULL)
    {
        return CL_INVALID_MEM_OBJECT;
    }

    CrtQueue* queue = reinterpret_cast<CrtQueue*>(((_cl_command_queue_crt*)command_queue)->object);
    if (!queue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    SyncManager* synchHelper = new SyncManager;
    if (!synchHelper)
    {
        return CL_OUT_OF_HOST_MEMORY;
    }

    cl_event*   outEvents = NULL;
    cl_uint     numOutEvents = 0;
    errCode = synchHelper->PrepareToExecute(
        queue,
        num_events_in_wait_list,
        event_wait_list,
        &numOutEvents,
        &outEvents,
        0,
        NULL);

    if (CL_SUCCESS != errCode)
    {
        goto FINISH;
    }

    crtEvent = new CrtEvent(queue);
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
            errCode = CL_OUT_OF_HOST_MEMORY;
            goto FINISH;
        }
        event_handle->object = (void*)crtEvent;
        *event = event_handle;
    }

FINISH:
    if (crtEvent && (!event || (CL_SUCCESS != errCode)))
    {
        crtEvent->Release();
        crtEvent->DecPendencyCnt();
    }
    if (synchHelper)
    {
        synchHelper->Release(errCode);
        delete synchHelper;
    }
    return errCode;
}


/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clEnqueueCopyBuffer(
    cl_command_queue    command_queue,
    cl_mem              src_buffer,
    cl_mem              dst_buffer,
    size_t              src_offset,
    size_t              dst_offset,
    size_t              cb,
    cl_uint             num_events_in_wait_list,
    const cl_event *    event_wait_list,
    cl_event *          event)
{
    cl_int errCode = CL_SUCCESS;
    CrtEvent* crtEvent = NULL;

    if (command_queue == NULL)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    if (src_buffer == NULL || dst_buffer == NULL)
    {
        return CL_INVALID_MEM_OBJECT;
    }

    CrtQueue* queue = reinterpret_cast<CrtQueue*>(((_cl_command_queue_crt*)command_queue)->object);
    if (!queue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    SyncManager* synchHelper = new SyncManager;
    if (!synchHelper)
    {
        return CL_OUT_OF_HOST_MEMORY;
    }

    cl_event*   outEvents = NULL;
    cl_uint     numOutEvents = 0;
    errCode = synchHelper->PrepareToExecute(
        queue,
        num_events_in_wait_list,
        event_wait_list,
        &numOutEvents,
        &outEvents,
        0,
        NULL);

    if (CL_SUCCESS != errCode)
    {
        goto FINISH;
    }

    crtEvent = new CrtEvent(queue);
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
            errCode = CL_OUT_OF_HOST_MEMORY;
            goto FINISH;
        }
        event_handle->object = (void*)crtEvent;
        *event = event_handle;
    }

FINISH:
    if (crtEvent && (!event || (CL_SUCCESS != errCode)))
    {
        crtEvent->Release();
        crtEvent->DecPendencyCnt();
    }
    if (synchHelper)
    {
        synchHelper->Release(errCode);
        delete synchHelper;
    }
    return errCode;
}


/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
void * CL_API_CALL clEnqueueMapBuffer(
    cl_command_queue    command_queue,
    cl_mem              buffer,
    cl_bool             blocking_map,
    cl_map_flags        map_flags,
    size_t              offset,
    size_t              cb,
    cl_uint             num_events_in_wait_list,
    const cl_event *    event_wait_list,
    cl_event *          event,
    cl_int *            errcode_ret)
{
    cl_int errCode = CL_SUCCESS;
    CrtEvent* crtMapEvent = NULL;
    CrtEvent* crtEvent = NULL;
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
    cl_event*   outEvents = NULL;
    cl_uint     numOutEvents = 0;
    errCode = synchHelper->PrepareToExecute(
        queue,
        num_events_in_wait_list,
        event_wait_list,
        &numOutEvents,
        &outEvents,
        0,
        NULL);

    if (CL_SUCCESS != errCode)
    {
        goto FINISH;
    }

    crtMapEvent = new CrtEvent(queue);
    if (!crtMapEvent)
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }
    CrtBuffer* crtBuffer = reinterpret_cast<CrtBuffer*>(((_cl_mem_crt*)buffer)->object);
    if (!crtBuffer)
    {   errCode = CL_INVALID_MEM_OBJECT;
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

    ptr = queue->m_cmdQueueDEV->dispatch->clEnqueueMapBuffer(
            queue->m_cmdQueueDEV,
            crtBuffer->getDeviceMemObj(queue->m_device),
            blocking_map,
            map_flags,
            offset,
            cb,
            numOutEvents,
            outEvents,
            &crtMapEvent->m_eventDEV,
            &errCode);

    if (crtBuffer->HasPrivateCopy())
    {
        // This can be extended to include the region
        // of memory being mapped, so we won't need
        // to copy all memory.
        crtEvent = new CrtEvent(queue);
        if (NULL == crtEvent)
        {
            errCode = CL_OUT_OF_HOST_MEMORY;
            goto FINISH;
        }

        errCode = synchHelper->PostExecuteMemSync(
                SyncManager::SYNC_FROM_BACKING_STORE,
                queue,
                crtBuffer,
                crtMapEvent,
                &crtEvent);
        
        crtEvent->IncPendencyCnt();

        crtMapEvent->Release();
        crtMapEvent->DecPendencyCnt();
    }
    else
    {
        crtEvent = crtMapEvent;
    }

    if (errCode == CL_SUCCESS && event)
    {
        _cl_event_crt* event_handle = new _cl_event_crt;
        if (!event_handle)
        {
            errCode = CL_OUT_OF_HOST_MEMORY;
            goto FINISH;
        }
        event_handle->object = (void*)crtEvent;
        *event = event_handle;
    }
    else
    {
        crtEvent->Release();
        crtEvent->DecPendencyCnt();
    }

FINISH:

    if (CL_SUCCESS != errCode)
    {
        if (crtMapEvent)
        {
            crtMapEvent->Release();
            crtMapEvent->DecPendencyCnt();
        }
        if (crtEvent != crtMapEvent)
        {
            crtEvent->Release();
            crtEvent->DecPendencyCnt();
        }
    }
    if (synchHelper)
    {
        synchHelper->Release(errCode);
        delete synchHelper;
    }
    if (errcode_ret)
    {
        *errcode_ret = errCode;
    }

    return ptr;
}


/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
void * CL_API_CALL clEnqueueMapImage(
    cl_command_queue    command_queue,
    cl_mem              image,
    cl_bool             blocking_map,
    cl_map_flags        map_flags,
    const size_t *      origin,
    const size_t *      region,
    size_t *            image_row_pitch,
    size_t *            image_slice_pitch,
    cl_uint             num_events_in_wait_list,
    const cl_event *    event_wait_list,
    cl_event *          event,
    cl_int *            errcode_ret)
{
    cl_int errCode = CL_SUCCESS;
    CrtEvent* crtMapEvent = NULL;
    CrtEvent* crtEvent = NULL;

    void* ptr = NULL;

    if (command_queue == NULL)
    {
        errCode = CL_INVALID_COMMAND_QUEUE;
        return NULL;
    }
    if (image == NULL)
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
    cl_event*   outEvents = NULL;
    cl_uint     numOutEvents = 0;
    errCode = synchHelper->PrepareToExecute(
        queue,
        num_events_in_wait_list,
        event_wait_list,
        &numOutEvents,
        &outEvents,
        0,
        NULL);

    if (CL_SUCCESS != errCode)
    {
        goto FINISH;
    }
    crtMapEvent = new CrtEvent(queue);
    if (!crtMapEvent)
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }

    CrtImage* crtImage = reinterpret_cast<CrtImage*>(((_cl_mem_crt*)image)->object);
    if (!crtImage)
    {
        errCode = CL_INVALID_MEM_OBJECT;
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

    ptr = queue->m_cmdQueueDEV->dispatch->clEnqueueMapImage(
        queue->m_cmdQueueDEV,
        crtImage->getDeviceMemObj(queue->m_device),
        blocking_map,
        map_flags,
        origin,
        region,
        image_row_pitch,
        image_slice_pitch,
        numOutEvents,
        outEvents,
        &crtMapEvent->m_eventDEV,
        &errCode);

    if (crtImage->HasPrivateCopy())
    {
        // This can be extended to include the region
        // of memory being mapped, so we won't need
        // to copy all memory.
        crtEvent = new CrtEvent(queue);
        if (NULL == crtEvent)
        {
            errCode = CL_OUT_OF_HOST_MEMORY;
            goto FINISH;
        }

        errCode = synchHelper->PostExecuteMemSync(
                SyncManager::SYNC_FROM_BACKING_STORE,
                queue,
                crtImage,
                crtMapEvent,
                &crtEvent);
        
        crtEvent->IncPendencyCnt();

        crtMapEvent->Release();
        crtMapEvent->DecPendencyCnt();
    }
    else
    {
        crtEvent = crtMapEvent;
    }

    if (errCode == CL_SUCCESS && event)
    {
        _cl_event_crt* event_handle = new _cl_event_crt;
        if (!event_handle)
        {
            errCode = CL_OUT_OF_HOST_MEMORY;
            goto FINISH;
        }
        event_handle->object = (void*)crtEvent;
        *event = event_handle;
    }
    else
    {
        crtEvent->Release();
        crtEvent->DecPendencyCnt();
    }

FINISH:
    if (CL_SUCCESS != errCode)
    {
        if (crtMapEvent)
        {
            crtMapEvent->Release();
            crtMapEvent->DecPendencyCnt();
        }
        if (crtEvent)
        {
            crtEvent->Release();
            crtEvent->DecPendencyCnt();
        }
    }
    if (synchHelper)
    {
        synchHelper->Release(errCode);
        delete synchHelper;
    }
    if (errcode_ret)
        *errcode_ret = errCode;

    return ptr;
}


/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clEnqueueUnmapMemObject(
    cl_command_queue    command_queue,
    cl_mem              memobj,
    void *              mapped_ptr,
    cl_uint             num_events_in_wait_list,
    const cl_event *    event_wait_list,
    cl_event *          event)
{
    cl_int errCode = CL_SUCCESS;
    CrtEvent* crtUnmapEvent = NULL;
    CrtEvent* crtEvent = NULL;

    void* ptr = NULL;

    if (command_queue == NULL)
    {
        errCode = CL_INVALID_COMMAND_QUEUE;
        goto FINISH;
    }
    if (memobj == NULL)
    {
        errCode = CL_INVALID_MEM_OBJECT;
        goto FINISH;
    }
    CrtQueue* queue = reinterpret_cast<CrtQueue*>(((_cl_command_queue_crt*)command_queue)->object);
    if (!queue)
    {
        errCode = CL_INVALID_COMMAND_QUEUE;
        goto FINISH;
    }

    SyncManager* synchHelper = new SyncManager;
    if (!synchHelper)
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        return NULL;
    }

    CrtMemObject* crtMemObj = reinterpret_cast<CrtMemObject*>(((_cl_mem_crt*)memobj)->object);
    if (!crtMemObj)
    {
        errCode = CL_INVALID_MEM_OBJECT;
        goto FINISH;
    }

    cl_event*   outEvents = NULL;   
    cl_uint     numOutEvents = 0;
    
    errCode = synchHelper->PrepareToExecute(
        queue,
        num_events_in_wait_list,
        event_wait_list,
        &numOutEvents,
        &outEvents,
        0,
        NULL);

    if (CL_SUCCESS != errCode)
    {
        goto FINISH;
    }    
        
    crtUnmapEvent = new CrtEvent(queue);
    if (!crtUnmapEvent)
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }
 
    errCode = queue->m_cmdQueueDEV->dispatch->clEnqueueUnmapMemObject(
        queue->m_cmdQueueDEV,
        crtMemObj->getDeviceMemObj(queue->m_device),
        mapped_ptr,
        numOutEvents,
        outEvents,
        &crtUnmapEvent->m_eventDEV);        
    
    if (CL_SUCCESS != errCode)
    {
        goto FINISH;
    }
        /// Theoretically we should sync the surface before the UnMap command
        /// however, since we already know that UnMap on the devices will do
        /// nothing besides reference counting the map/unmap operations...
        /// we choose to follow the Map sequence implementation for simplicity.
    if (crtMemObj->HasPrivateCopy())
    {
        // This can be extended to include the region
        // of memory being mapped, so we won't need
        // to copy all memory.
        crtEvent = new CrtEvent(queue);
        if (NULL == crtEvent)
        {
            errCode = CL_OUT_OF_HOST_MEMORY;
            goto FINISH;
        }

        errCode = synchHelper->PostExecuteMemSync(
                SyncManager::SYNC_TO_BACKING_STORE,
                queue,
                crtMemObj,
                crtUnmapEvent,
                &crtEvent); 
        
        crtEvent->IncPendencyCnt();

        crtUnmapEvent->Release();
        crtUnmapEvent->DecPendencyCnt();
    }
    else
    {
        crtEvent = crtUnmapEvent;
    }

    if (errCode == CL_SUCCESS && event)
    {
        _cl_event_crt* event_handle = new _cl_event_crt;
        if (!event_handle)
        {
            errCode = CL_OUT_OF_HOST_MEMORY;
            goto FINISH;
        }
        event_handle->object = (void*)crtEvent;
        *event = event_handle;
    }
    else
    {
        crtEvent->Release();
        crtEvent->DecPendencyCnt();
    }

FINISH:
   
    if (CL_SUCCESS != errCode)
    {
        if (crtUnmapEvent)
        {
            crtUnmapEvent->Release();            
            crtUnmapEvent->DecPendencyCnt();            
        }    
        if (crtEvent != crtUnmapEvent)
        {
            crtEvent->Release();
            crtEvent->DecPendencyCnt();
        }
    }
    if (synchHelper)
    {
        synchHelper->Release(errCode);
        delete synchHelper;
    }

    return errCode;
}


/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clFlush(cl_command_queue command_queue)
{
    if (command_queue == NULL)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }
    CrtQueue* queue = reinterpret_cast<CrtQueue*>(((_cl_command_queue_crt*)command_queue)->object);
    if (!queue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }
    return queue->m_cmdQueueDEV->dispatch->clFlush(queue->m_cmdQueueDEV);
}


/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clFinish(cl_command_queue command_queue)
{
    cl_int errCode  = CL_SUCCESS;

    if (command_queue == NULL)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    CrtQueue* queue = reinterpret_cast<CrtQueue*>(((_cl_command_queue_crt*)command_queue)->object);
    if (!queue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    return queue->m_cmdQueueDEV->dispatch->clFinish(queue->m_cmdQueueDEV);
}


/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clRetainContext(cl_context context)
{
    cl_int errCode = CL_SUCCESS;

    CrtContextInfo* ctxInfo = OCLCRT::crt_ocl_module.m_contextInfoGuard.GetValue(context);
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
        ctx->IncRefCnt();
    }
    return errCode;
}


/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clReleaseContext(cl_context context)
{
    cl_int errCode = CL_SUCCESS;

    CrtContextInfo* ctxInfo = OCLCRT::crt_ocl_module.m_contextInfoGuard.GetValue(context);
    if (NULL == ctxInfo)
    {
        return CL_INVALID_CONTEXT;
    }

    long refCount = 0;
    // Single Platform Context
    if (ctxInfo->m_contextType == CrtContextInfo::SinglePlatformContext)
    {
        KHRicdVendorDispatch* dTable = (KHRicdVendorDispatch*)(ctxInfo->m_object);

        // Get the new reference count
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
            // Deletes CrtContextInfo object
            // we don't delete context, since it was created by the underlying
            // platform and not by the CRT.
            delete ctxInfo;
            // Remove the context from the contexts map,
            // since its not valid any more
            OCLCRT::crt_ocl_module.m_contextInfoGuard.Remove(context);
        }
        return errCode;
    }
    else
    {
        // Shared Platform Context
        CrtContext* ctx = (CrtContext*)(ctxInfo->m_object);

        ctx->IncPendencyCnt();
        refCount = ctx->DecRefCnt();
        if (refCount == 0)
        {
            errCode = ctx->Release();
            // deletes the CrtContextInfo object
            delete ctxInfo;
            // Remove the context from the contexts map,
            // since its not valid any more
            OCLCRT::crt_ocl_module.m_contextInfoGuard.Remove(context);
            // deletes the _crt_cl_context handle
            delete context;
        }
        ctx->DecPendencyCnt();
    }
    return errCode;
}


/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clRetainCommandQueue(cl_command_queue command_queue)
{
    cl_int errCode = CL_SUCCESS;

    CrtQueue* queue = reinterpret_cast<CrtQueue*>(((_cl_command_queue_crt*)command_queue)->object);
    if (!queue)
    {
        return CL_INVALID_COMMAND_QUEUE;

    }
    queue->IncRefCnt();
    // I don't forward retains to the underlying platforms, i only send release when
    // the CRT ref counter for this queue reaches zero
    return errCode;
}


/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clReleaseCommandQueue(cl_command_queue command_queue)
{
    cl_int errCode = CL_SUCCESS;


    CrtQueue* queue = reinterpret_cast<CrtQueue*>(((_cl_command_queue_crt*)command_queue)->object);
    if (!queue)
    {
        return CL_INVALID_COMMAND_QUEUE;

    }
    // Spec says the clReleaseCommandQueue triggers an implicit flush
    queue->m_contextCRT->FlushQueues();

    // Main the self reference counter
    queue->IncPendencyCnt();
    long refCount = queue->DecRefCnt();
    // No more references to this queue, it can be deallocated
    if (refCount == 0)
    {
        // Forward the call to the underlying queue
        errCode = queue->Release();
        // deletes CRT handle _crt_command_queue
        delete command_queue;
    }
    queue->DecPendencyCnt();
    return errCode;
}


/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clReleaseMemObject(cl_mem memobj)
{
    cl_int errCode = CL_SUCCESS;

    CrtMemObject* crtMemObj = reinterpret_cast<CrtMemObject*>(((_cl_mem_crt*)memobj)->object);

    crtMemObj->IncPendencyCnt();

    long refCount = crtMemObj->DecRefCnt();
    if (refCount == 0)
    {
        errCode = crtMemObj->Release();
        // crtMemObj will be deleted later by the desctructor callback to
        // which will also set memobj->object to NULL
        delete ((_cl_mem_crt*)memobj);

        // when RefCount==0 then Pendency is decreased too by 1, so we should
        // return here since if we call DecPendencyCnt() again, the crtMemObj
        // will be deleted now, while it should be deleted by it's callback
        return errCode;
    }

    crtMemObj->DecPendencyCnt();
    return errCode;
}


/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clRetainMemObject(cl_mem memobj)
{
    cl_int errCode = CL_SUCCESS;

    CrtMemObject* crtMemObj = reinterpret_cast<CrtMemObject*>(((_cl_mem_crt*)memobj)->object);

    crtMemObj->IncRefCnt();
    // I don't forward retains to the underlying platforms, i only send release when
    // the CRT ref counter for this mem objects reaches zero
    return CL_SUCCESS;
}


/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_program CL_API_CALL clCreateProgramWithSource( cl_context         context ,
                                                  cl_uint            count ,
                                                  const char **      strings ,
                                                  const size_t *     lengths ,
                                                  cl_int *           errcode_ret )
{
    cl_int errCode = CL_SUCCESS;
    _cl_program_crt* SharedProgram = NULL;
    CrtProgram* pgm = NULL;

    CrtContext* ctx = ((CrtContext*)((_cl_context_crt*)context)->object);

    //Shared context.
    errCode = ctx->CreateProgramWithSource( count ,
                                            strings ,
                                            lengths ,
                                            &pgm );
    if( CL_SUCCESS == errCode )
    {
        SharedProgram = new _cl_program_crt;
        if( SharedProgram == NULL )
        {
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
cl_program CL_API_CALL clCreateProgramWithBinary(
    cl_context              context,
    cl_uint                 num_devices,
    const cl_device_id *    device_list,
    const size_t *          lengths,
    const unsigned char **  binaries,
    cl_int *                binary_status,
    cl_int *                errcode_ret)
{
    cl_int errCode = CL_SUCCESS;

    if (NULL == device_list || 0 == num_devices || NULL == lengths || NULL == binaries)
    {
        errCode = CL_INVALID_VALUE;
        goto FINISH;
    }

    _cl_program_crt* SharedProgram = NULL;
    CrtProgram* pgm = NULL;

    CrtContext* ctx = ((CrtContext*)((_cl_context_crt*)context)->object);

    errCode = ctx->CreateProgramWithBinary(
        num_devices,
        device_list,
        lengths,
        binaries,
        binary_status,
        &pgm);

    if( CL_SUCCESS == errCode )
    {
        SharedProgram = new _cl_program_crt;
        if( SharedProgram == NULL )
        {
            errCode = CL_OUT_OF_HOST_MEMORY;
        }
        else
        {
            SharedProgram->object = (void *) pgm;
        }
    }

FINISH:

    if( errcode_ret )
    {
        *errcode_ret = errCode;
    }

    return SharedProgram;
}


/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clBuildProgram(
    cl_program            program ,
    cl_uint               num_devices ,
    const cl_device_id *  device_list ,
    const char *          options ,
    prog_logging_fn       pfn_notify ,
    void *                user_data )
{
    cl_int errCode = CL_SUCCESS;
    CrtProgram* crtProg = reinterpret_cast<CrtProgram*>(((_cl_program_crt*)program)->object);

    if( ( ( num_devices > 0 ) && ( device_list == NULL ) ) ||
        ( ( num_devices == 0 ) && ( device_list != NULL ) ) )
    {
        errCode = CL_INVALID_VALUE;
    }

    CrtBuildCallBackData* crtData = new CrtBuildCallBackData;
    if (NULL == crtData)
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }
    else
    {
        crtData->m_clProgramHandle = program;
        crtData->m_pfnNotify = pfn_notify;
        crtData->m_userData = user_data;
    }

    cl_device_id* deviceList = NULL;

    if (device_list == NULL)
    {
        num_devices = (cl_uint)crtProg->m_contextCRT->m_DeviceToContext.size();
        deviceList = new cl_device_id[num_devices];
        if (!deviceList)
        {
            errCode = CL_OUT_OF_HOST_MEMORY;
            goto FINISH;
        }
        cl_uint i = 0;
        DEV_CTX_MAP::iterator itr = crtProg->m_contextCRT->m_DeviceToContext.begin();
        for (;itr != crtProg->m_contextCRT->m_DeviceToContext.end(); itr++)
        {
            deviceList[i++] = itr->first;
        }
    }
    else
    {
        deviceList = new cl_device_id[num_devices];
        if (!deviceList)
        {
            errCode = CL_OUT_OF_HOST_MEMORY;
            goto FINISH;
        }
        for (cl_uint i=0; i < num_devices; i++)
        {
            deviceList[i] = device_list[i];
        }
    }
    // Perform build on specific list of devices
    cl_uint numRelevantContexts = 0;
    for (cl_uint i = 0; i < OCLCRT::crt_ocl_module.m_oclPlatforms.size(); i++)
    {
        cl_uint matchDevices = 0;
        crtProg->m_contextCRT->GetDevicesByPlatformId(
            num_devices,
            deviceList,
            OCLCRT::crt_ocl_module.m_oclPlatforms[i]->m_platformIdDEV,
            &matchDevices,
            NULL);
        numRelevantContexts += 1;
    }

    crtData->m_numBuild = numRelevantContexts;

    cl_int numRequested = 0;
    for (cl_uint i = 0; i < OCLCRT::crt_ocl_module.m_oclPlatforms.size(); i++)
    {
        cl_device_id* outDevices = new cl_device_id[num_devices];
        if (NULL == outDevices)
        {
            errCode = CL_OUT_OF_HOST_MEMORY;
            goto FINISH;
        }

        cl_uint matchDevices = 0;
        crtProg->m_contextCRT->GetDevicesByPlatformId(
            num_devices,
            deviceList,
            OCLCRT::crt_ocl_module.m_oclPlatforms[i]->m_platformIdDEV,
            &matchDevices,
            outDevices);

        if (matchDevices == 0)
        {
            continue;
        }

        cl_context ctx = crtProg->m_contextCRT->m_DeviceToContext[outDevices[0]];
        cl_program prog = crtProg->m_ContextToProgram[ctx];

        errCode = prog->dispatch->clBuildProgram(
            prog,
            matchDevices,
            outDevices,
            options,
            buildCompleteFn,
            crtData);

        if (CL_SUCCESS != errCode)
        {
            goto FINISH;
        }

        numRequested++;
    }

    if (!pfn_notify)
    {
            /// The Build is a blocking command in this case
        crtData->m_lock.Wait();

            /// According to the spec we must return the build status on returning
            /// from a blocking build request
        for (cl_uint i=0; i < num_devices; i++)
        {
            cl_build_status build_status;
            cl_context ctx = crtProg->m_contextCRT->m_DeviceToContext[deviceList[i]];

            deviceList[i]->dispatch->clGetProgramBuildInfo(
                crtProg->m_ContextToProgram[ctx],
                deviceList[i],
                CL_PROGRAM_BUILD_STATUS,
                sizeof(build_status),
                &build_status,
                NULL);

            if (build_status != CL_BUILD_SUCCESS)
            {
                errCode = CL_BUILD_PROGRAM_FAILURE;
                goto FINISH;
            }
        }
    }
FINISH:
    if (device_list == NULL && deviceList)
    {
        delete[] deviceList;
    }

    if (CL_SUCCESS != errCode && crtData)
    {
        // We set crtData->m_numBuild to numRelevantContexts; however, we succeeded to
        // send build requests on numRequsted, since the others has fails or not
        // submitted; hence, we need to reduce this diff from the m_numBuild.
        cl_int notExecutedCount = numRequested - numRelevantContexts;
        if (0 == (atomic_add_ret_prev(&crtData->m_numBuild, notExecutedCount)+notExecutedCount))
        {
            delete crtData;
        }
    }
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

    CrtProgram* crtProgram = reinterpret_cast<CrtProgram*>(((_cl_program_crt*)program)->object);
    if (!crtProgram)
    {
        return CL_INVALID_PROGRAM;

    }
    crtProgram->IncRefCnt();
    // I don't forward retains to the underlying platforms, i only send release when
    // the CRT ref counter for this queue reaches zero

    return errCode;
}


/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clReleaseProgram( cl_program program )
{
    cl_int errCode = CL_SUCCESS;

    CrtProgram* crtProgram = reinterpret_cast<CrtProgram*>(((_cl_program_crt*)program)->object);
    if (!crtProgram)
    {
        return CL_INVALID_PROGRAM;

    }
    crtProgram->IncPendencyCnt();
    long refCount = crtProgram->DecRefCnt();

    if (refCount == 0)
    {
        crtProgram->Release();
        // deletes CRT handle _cl_program_crt
        delete program;
    }
    crtProgram->DecPendencyCnt();
    return errCode;
}


/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_kernel CL_API_CALL clCreateKernel( cl_program       program ,
                                      const char      *kernel_name ,
                                      cl_int          *errcode_ret )
{
    cl_int errCode = CL_SUCCESS;
    _cl_kernel_crt* SharedKernel = NULL;

    CrtProgram* pgm = reinterpret_cast<CrtProgram*>(((_cl_program_crt*)program)->object);

    CrtKernel* crtKernel = new CrtKernel(pgm);
    if(NULL == crtKernel )
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }
    CrtContext* ctx = pgm->m_contextCRT;

    for( SHARED_CTX_DISPATCH::iterator itr = ctx->m_contexts.begin();
        itr != ctx->m_contexts.end();
        itr++)
    {
        cl_context ctxObj = itr->first;
        cl_kernel knlObj = ctxObj->dispatch->clCreateKernel( pgm->m_ContextToProgram[ctxObj],
                                                  kernel_name,
                                                  &errCode );
        if (CL_SUCCESS == errCode)
        {
            crtKernel->m_ContextToKernel[ctxObj] = knlObj;
        }
        else
        {
            goto FINISH;
        }
    }

    SharedKernel = new _cl_kernel_crt;
    if( NULL == SharedKernel )
    {
        delete SharedKernel;
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }
    else
    {
        SharedKernel->object = (void *)crtKernel;
    }

FINISH:

    if (CL_SUCCESS != errCode)
    {
        if (NULL != crtKernel)
        {
            crtKernel->Release();
            crtKernel->DecPendencyCnt();
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
cl_int CL_API_CALL clGetKernelInfo(
    cl_kernel           kernel,
    cl_sampler_info     param_name,
    size_t              param_value_size,
    void *              param_value,
    size_t *            param_value_size_ret)
{
    cl_int retCode = CL_SUCCESS;
    CrtKernel* crtKernel = reinterpret_cast<CrtKernel*>(((_cl_kernel_crt*)kernel)->object);
        /// Pick any of the underlying created sampler objects
    cl_kernel kernelObj = crtKernel->m_ContextToKernel.begin()->second;
        /// Forward call
    retCode = kernelObj->dispatch->clGetKernelInfo(
                        kernelObj,
                        param_name,
                        param_value_size,
                        param_value,
                        param_value_size_ret);
    return retCode;
}
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clSetKernelArg(
    cl_kernel    kernel,
    cl_uint      arg_index,
    size_t       arg_size,
    const void * arg_value)
{
    cl_int errCode = CL_SUCCESS;
    CrtKernel* crtKernel = reinterpret_cast<CrtKernel*>(((_cl_kernel_crt*)kernel)->object);

    CrtContext* kernelCrtCtx = crtKernel->m_programCRT->m_contextCRT;
    cl_device_id devId = kernelCrtCtx->GetKernelReflectionDevice();

    size_t size = 0;
    ((CrtKHRicdVendorDispatch*)devId->dispatch)->clGetKernelArgInfo(
        crtKernel->m_ContextToKernel[kernelCrtCtx->m_DeviceToContext[devId]],
        arg_index,
        CL_KERNEL_ARG_TYPE_NAME,
        0,
        NULL,
        &size);
    char* paramType = new char[size];
    ((CrtKHRicdVendorDispatch*)devId->dispatch)->clGetKernelArgInfo(
        crtKernel->m_ContextToKernel[kernelCrtCtx->m_DeviceToContext[devId]],
        arg_index,
        CL_KERNEL_ARG_TYPE_NAME,
        size,
        paramType,
        NULL);



    std::string paramT(paramType);

    bool isImage = !(paramT.compare(0, strlen("__image"),"__image"));
    bool isBuffer = !(strcmp(&paramType[strlen(paramType)-1],"*"));
    bool isSampler = !(paramT.compare(0, strlen("sampler"),"sampler"));

    const CrtObject* crtObject = NULL;
    CrtSampler* samplerObj = NULL;
    CrtMemObject* memObj = NULL;

    if (isImage || isBuffer)
    {
        _cl_object* ClObject = *((_cl_object**)(arg_value));
        if (NULL == ClObject)
        {
            return CL_INVALID_KERNEL_ARGS;
        }
        crtObject = reinterpret_cast<const CrtObject*>(ClObject->object);
        if (NULL == crtObject)
        {
            return CL_INVALID_KERNEL_ARGS;
        }
        memObj = (CrtMemObject*)crtObject;
    }
    else if (isSampler)
    {
        samplerObj = (CrtSampler*)crtObject;
    }

    CTX_KRN_MAP::iterator itr = crtKernel->m_ContextToKernel.begin();
    for (;itr != crtKernel->m_ContextToKernel.end(); itr++)
    {
        void* devObject = NULL;
        if (isImage || isBuffer)
        {
            devObject = (void*)(&(memObj->m_ContextToMemObj[itr->first]));
        }
        else if (isSampler)
        {

            devObject = (void*)(&(samplerObj->m_ContextToSampler[itr->first]));
        }

        if (devObject == NULL)
        {
            errCode = itr->second->dispatch->clSetKernelArg(
                itr->second,
                arg_index,
                arg_size,
                arg_value);
        }
        else
        {
            errCode = itr->second->dispatch->clSetKernelArg(
                itr->second,
                arg_index,
                arg_size,
                devObject);
        }
        if (CL_SUCCESS != errCode)
            break;
    }
    return errCode;
}


/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clGetKernelWorkGroupInfo(
    cl_kernel                 kernel,
    cl_device_id              device,
    cl_kernel_work_group_info param_name,
    size_t                    param_value_size,
    void *                    param_value,
    size_t *                  param_value_size_ret)
{
    cl_int errCode = CL_SUCCESS;

    CrtKernel* crtKernel = reinterpret_cast<CrtKernel*>(((_cl_kernel_crt*)kernel)->object);

    cl_context devCtx = crtKernel->m_programCRT->m_contextCRT->m_DeviceToContext[device];
    if (NULL == devCtx)
    {
        return CL_INVALID_DEVICE;
    }
    errCode = device->dispatch->clGetKernelWorkGroupInfo(
        crtKernel->m_ContextToKernel[devCtx],
        device,
        param_name,
        param_value_size,
        param_value,
        param_value_size_ret);

    return errCode;
}


/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clRetainKernel( cl_kernel kernel )
{
    cl_int errCode = CL_SUCCESS;

    CrtKernel* crtKernel = reinterpret_cast<CrtKernel*>(((_cl_kernel_crt*)kernel)->object);
    if (!crtKernel)
    {
        return CL_INVALID_KERNEL;
    }
    crtKernel->IncRefCnt();
    // I don't forward retains to the underlying platforms, i only send release when
    // the CRT ref counter for this queue reaches zero
    return errCode;
}


/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clReleaseKernel( cl_kernel kernel )
{
    cl_int errCode = CL_SUCCESS;

    CrtKernel* crtKernel = reinterpret_cast<CrtKernel*>(((_cl_kernel_crt*)kernel)->object);
    if (!crtKernel)
    {
        return CL_INVALID_KERNEL;

    }
    crtKernel->IncPendencyCnt();
    // No more references to this kernel, it can be released and deallocated
    long refCount = crtKernel->DecRefCnt();
    if (refCount == 0)
    {
        crtKernel->Release();
            /// deletes CRT handle _cl_kernel_crt
        delete kernel;
    }
    crtKernel->DecPendencyCnt();
    return errCode;
}


/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clCreateSubDevicesEXT(
    cl_device_id                                device,
    const cl_device_partition_property_ext*     properties,
    cl_uint                                     num_entries,
    cl_device_id*                               out_devices,
    cl_uint*                                    num_devices)
{
    cl_int errCode = CL_SUCCESS;
    CrtDeviceInfo** pDevicesInfo  = NULL;

    CrtDeviceInfo* parentDevInfo = OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.GetValue(device);
    if (!parentDevInfo)
    {
        return CL_INVALID_DEVICE;
    }

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
            OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.Add(dev,pDevicesInfo[i]);
        }

        if (CL_SUCCESS != errCode)
        {
            for (cl_uint i=0; i< *num_devices; i++)
            {
                if (pDevicesInfo[i])
                {
                    delete pDevicesInfo[i];
                }
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

    CrtDeviceInfo* devInfo = OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.GetValue(device);

    if (devInfo == NULL)
    {
        return CL_INVALID_DEVICE;
    }

    // No need to do anything for Root devices
    if (devInfo->m_isRootDevice)
    {
        return CL_SUCCESS;
    }

        /// If we reached here, then this is a sub-device
    long refCount = atomic_decrement(&(devInfo->m_refCount));
    if (refCount == 0)
    {
        errCode = devInfo->m_origDispatchTable.clReleaseDeviceEXT(device);

        OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.Remove(device);

        delete devInfo;
    }
    return errCode;
}


/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clRetainDeviceEXT(cl_device_id device)
{
    cl_int errCode = CL_SUCCESS;

    CrtDeviceInfo* devInfo = OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.GetValue(device);

    if (devInfo == NULL)
    {
        return CL_INVALID_DEVICE;
    }

    // No need to do anything for Root devices
    if (devInfo->m_isRootDevice)
    {
        return CL_SUCCESS;
    }
        /// If we reached here, then this is a sub-device
        /// No need to forward the retain request, just incremement
        /// the reference counter
    atomic_increment(&(devInfo->m_refCount));

    return errCode;
}


/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clSetMemObjectDestructorCallback(
    cl_mem          memObj,
    mem_dtor_fn     pfn_notify,
    void *          pUserData )
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

    crtEvent->IncRefCnt();

    return CL_SUCCESS;
}


/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clReleaseEvent(cl_event event)
{
    cl_int errCode = CL_SUCCESS;
    CrtEvent* crtEvent = reinterpret_cast<CrtEvent*>(((_cl_event_crt*)event)->object);

    crtEvent->IncPendencyCnt();
    long refCount = crtEvent->DecRefCnt();
    if (refCount == 0)
    {
        crtEvent->Release();
        delete event;
    }
    crtEvent->DecPendencyCnt();
    return errCode;
}


/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clWaitForEvents(cl_uint num_events, const cl_event * event_list)
{
    cl_int errCode = CL_SUCCESS;
    CrtContext* crtContext = NULL;

    //// Implements Option 1 from the Design document

    if (0 == num_events)
    {
        return CL_INVALID_VALUE;
    }
    else     
    {
        CrtEvent* crtEvent = reinterpret_cast<CrtEvent*>(((_cl_event_crt*)(event_list[0]))->object);
        crtContext = crtEvent->getContext();
        for(cl_uint i=1; i < num_events; i++)
        {
            if (crtEvent->getContext() != crtContext)
            {
                return CL_INVALID_CONTEXT;
            }        
        }
        crtEvent->getContext()->FlushQueues();
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

            if (itr->first == crtEvent->getContext()->m_DeviceToContext[crtEvent->m_queueCRT->m_device])
            {
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
        }
        if (current > 0)
        {
            errCode = pEvents[0]->dispatch->clWaitForEvents(current, pEvents);
        }

        if (CL_SUCCESS != errCode)
        {
            return errCode;
        }

        current = 0;
    }
    return errCode;
}


/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clGetEventInfo(
    cl_event        event,
    cl_event_info   param_name,
    size_t          param_value_size,
    void *          param_value,
    size_t *        param_value_size_ret)
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
cl_int CL_API_CALL clGetProgramInfo(
    cl_program      program,
    cl_program_info param_name,
    size_t          param_value_size,
    void *          param_value,
    size_t *        param_value_size_ret)
{
    cl_int errCode = CL_SUCCESS;
    cl_device_id* outDevices = NULL;

    CrtProgram* crtProgram= reinterpret_cast<CrtProgram*>(((_cl_program_crt*)program)->object);

    size_t  pValueSize = 0;
    switch(param_name)
    {
        case CL_PROGRAM_REFERENCE_COUNT:
            {
                pValueSize = sizeof(crtProgram->m_refCount);
                if (param_value && param_value_size >= pValueSize)
                {
                    *((cl_uint*)param_value) = crtProgram->m_refCount;
                }
            }
            break;
        case CL_PROGRAM_CONTEXT:
            {
                pValueSize = sizeof(cl_context);
                if (param_value && param_value_size >= pValueSize)
                {
                    *((cl_context*)param_value) = crtProgram->m_contextCRT->m_context_handle;
                }

                if( param_value_size_ret )
                {
                    *param_value_size_ret = pValueSize;
                }
            }
            break;
        case CL_PROGRAM_NUM_DEVICES:
            {

                pValueSize = sizeof(cl_uint);
                cl_uint numDevices = 0;
                CTX_PGM_MAP::iterator itr = crtProgram->m_ContextToProgram.begin();
                for (;itr != crtProgram->m_ContextToProgram.end(); itr++)
                {
                    cl_uint ctxNumDevices = 0;
                    errCode = itr->second->dispatch->clGetProgramInfo(
                        itr->second,
                        CL_PROGRAM_NUM_DEVICES,
                        sizeof(cl_uint),
                        &ctxNumDevices,
                        NULL);

                    numDevices += ctxNumDevices;
                }
                if (param_value && param_value_size >= pValueSize)
                {
                    *((cl_uint*)param_value) = numDevices;
                }
            }
            break;
        case CL_PROGRAM_DEVICES:
            {
                //pValueSize = sizeof(cl_device);
                cl_uint maxNumDevices = (cl_uint)crtProgram->m_contextCRT->m_DeviceToContext.size();
                outDevices = new cl_device_id[maxNumDevices];
                if (CL_SUCCESS != errCode)
                {
                    errCode = CL_OUT_OF_HOST_MEMORY;
                    goto FINISH;
                }
                cl_uint numDevices = 0;
                CTX_PGM_MAP::iterator itr = crtProgram->m_ContextToProgram.begin();
                for (;itr != crtProgram->m_ContextToProgram.end(); itr++)
                {
                    size_t sizeRetDevices = 0;
                    errCode = itr->second->dispatch->clGetProgramInfo(
                        itr->second,
                        CL_PROGRAM_DEVICES,
                        sizeof(cl_device_id)*maxNumDevices,
                        outDevices + numDevices,
                        &sizeRetDevices);
                    numDevices += (cl_uint)(sizeRetDevices / sizeof(cl_device_id));

                    if (CL_SUCCESS != errCode)
                    {
                        goto FINISH;
                    }
                }
                pValueSize = numDevices * sizeof(cl_device_id);
                if (param_value && param_value_size >= pValueSize)
                {
                    for (cl_uint i=0; i < numDevices; i++)
                    {
                        ((cl_device_id*)param_value)[i] = outDevices[i];
                    }
                }
            }
            break;
        default:
            {
                CTX_PGM_MAP::iterator itr = crtProgram->m_ContextToProgram.begin();
                errCode = itr->second->dispatch->clGetProgramInfo(
                    itr->second,
                    param_name,
                    param_value_size,
                    param_value,
                    param_value_size_ret);
            }
            break;
    }
FINISH:
    if (outDevices)
    {
        delete[] outDevices;
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

    CrtUserEvent* crtUserEvent = new CrtUserEvent(crtContext);
    if (NULL == crtUserEvent)
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }

    for(SHARED_CTX_DISPATCH::iterator itr = crtContext->m_contexts.begin();
        itr != crtContext->m_contexts.end();
        itr++)
    {
        cl_event eventDEV = itr->second->clCreateUserEvent(itr->first, &errCode);
        if (CL_SUCCESS != errCode)
        {
            goto FINISH;
        }
        crtUserEvent->m_ContextToEvent[itr->first] = eventDEV;
    }

    event_handle = new _cl_event_crt;
    if (!event_handle)
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
    }
    else
    {
        event_handle->object = (void*)crtUserEvent;
    }

FINISH:
    if (CL_SUCCESS != errCode && crtUserEvent)
    {
        crtUserEvent->Release();
        crtUserEvent->DecPendencyCnt();
    }
    if (errcode_ret)
    {
        *errcode_ret = errCode;
    }

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
    cl_event        event,
    cl_int          command_exec_callback_type,
    pfn_notify      notify_callback,
    void *          user_data)
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


/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clEnqueueNDRangeKernel(
    cl_command_queue    command_queue,
    cl_kernel           kernel,
    cl_uint             work_dim,
    const size_t *      global_work_offset,
    const size_t *      global_work_size,
    const size_t *      local_work_size,
    cl_uint             num_events_in_wait_list,
    const cl_event *    event_wait_list,
    cl_event *          event)
{
    cl_int errCode = CL_SUCCESS;

    if (command_queue == NULL)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    if (kernel == NULL)
    {
        return CL_INVALID_KERNEL;
    }

    CrtQueue* queue = reinterpret_cast<CrtQueue*>(((_cl_command_queue_crt*)command_queue)->object);
    if (!queue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    SyncManager* synchHelper = new SyncManager;
    if (!synchHelper)
    {
        return CL_OUT_OF_HOST_MEMORY;
    }

    cl_event*   outEvents = NULL;
    cl_uint     numOutEvents = 0;
    errCode = synchHelper->PrepareToExecute(
        queue,
        num_events_in_wait_list,
        event_wait_list,
        &numOutEvents,
        &outEvents,
        0,
        NULL);

    if (CL_SUCCESS != errCode)
    {
        goto FINISH;
    }

    CrtEvent* crtEvent = new CrtEvent(queue);
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
    if (crtEvent && (!event || (CL_SUCCESS != errCode)))
    {
        crtEvent->Release();
        crtEvent->DecPendencyCnt();
    }
    if (synchHelper)
    {
        synchHelper->Release(errCode);
        delete synchHelper;
    }
    return errCode;
}


/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clEnqueueTask(
    cl_command_queue    command_queue,
    cl_kernel           kernel,
    cl_uint             num_events_in_wait_list,
    const cl_event *    event_wait_list,
    cl_event *          event)
{
    cl_int errCode = CL_SUCCESS;

    if (command_queue == NULL)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    if (kernel == NULL)
    {
        return CL_INVALID_KERNEL;
    }

    CrtQueue* queue = reinterpret_cast<CrtQueue*>(((_cl_command_queue_crt*)command_queue)->object);
    if (!queue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    SyncManager* synchHelper = new SyncManager;
    if (!synchHelper)
    {
        return CL_OUT_OF_HOST_MEMORY;
    }

    cl_event*   outEvents = NULL;
    cl_uint     numOutEvents = 0;
    errCode = synchHelper->PrepareToExecute(
        queue,
        num_events_in_wait_list,
        event_wait_list,
        &numOutEvents,
        &outEvents,
        0,
        NULL);

    if (CL_SUCCESS != errCode)
    {
        goto FINISH;
    }

    CrtEvent* crtEvent = new CrtEvent(queue);
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
            errCode = CL_OUT_OF_HOST_MEMORY;
            goto FINISH;
        }
        event_handle->object = (void*)crtEvent;
        *event = event_handle;
    }

FINISH:
    if (crtEvent && (!event || (CL_SUCCESS != errCode)))
    {
        crtEvent->Release();
        crtEvent->DecPendencyCnt();
    }
    if (synchHelper)
    {
        synchHelper->Release(errCode);
        delete synchHelper;
    }
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
    CrtEvent* crtEvent = new CrtEvent(queue);
    if (!crtEvent)
    {
        return CL_OUT_OF_HOST_MEMORY;
    }

    _cl_event_crt* event_handle = new _cl_event_crt;
    if (!event_handle)
    {
        crtEvent->DecPendencyCnt();
        return CL_OUT_OF_HOST_MEMORY;
    }

    cl_event eventDEV = NULL;
    errCode = queue->m_cmdQueueDEV->dispatch->clEnqueueMarker(queue->m_cmdQueueDEV, &eventDEV);
    if (CL_SUCCESS != errCode)
    {
        crtEvent->DecPendencyCnt();
        delete event_handle;
        return errCode;
    }

    crtEvent->m_eventDEV = eventDEV;
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
    cl_command_queue    command_queue,
    cl_uint             num_events_in_wait_list,
    const cl_event *    event_wait_list)
{
    cl_int errCode = CL_SUCCESS;

    if (command_queue == NULL)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    CrtQueue* queue = reinterpret_cast<CrtQueue*>(((_cl_command_queue_crt*)command_queue)->object);
    if (!queue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    SyncManager* synchHelper = new SyncManager;
    if (!synchHelper)
    {
        return CL_OUT_OF_HOST_MEMORY;
    }

    cl_event*   outEvents = NULL;
    cl_uint     numOutEvents = 0;
    errCode = synchHelper->PrepareToExecute(
        queue,
        num_events_in_wait_list,
        event_wait_list,
        &numOutEvents,
        &outEvents,
        0,
        NULL);

    if (CL_SUCCESS != errCode)
    {
        goto FINISH;
    }

    errCode = queue->m_cmdQueueDEV->dispatch->clEnqueueWaitForEvents(
        queue->m_cmdQueueDEV,
        numOutEvents,
        outEvents);

FINISH:
    if (synchHelper)
    {
        synchHelper->Release(errCode);
        delete synchHelper;
    }
    return errCode;
}


/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clEnqueueNativeKernel(
    cl_command_queue    command_queue,
    user_func           user_native_func,
    void *              args,
    size_t              cb_args,
    cl_uint             num_mem_objects,
    const cl_mem *      mem_list,
    const void **       args_mem_loc,
    cl_uint             num_events_in_wait_list,
    const cl_event *    event_wait_list,
    cl_event *          event)
{
    cl_int errCode = CL_SUCCESS;
    CrtEvent* crtEvent = NULL;

    if (command_queue == NULL)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    if (    ( NULL == user_native_func)                                             ||
            ( NULL == args && ((cb_args > 0) || num_mem_objects > 0 ))              ||
            ( NULL != args && 0 == cb_args)                                         ||
            ( (num_mem_objects >  0) && ( NULL == mem_list || NULL == args_mem_loc))||
            ( (0 == num_mem_objects) && ( NULL != mem_list || NULL != args_mem_loc)) )
    {
        return CL_INVALID_VALUE;
    }

    CrtQueue* queue = reinterpret_cast<CrtQueue*>(((_cl_command_queue_crt*)command_queue)->object);
    if (!queue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

        /// Check if device support Native Kernels in reported device capabilities
    CrtDeviceInfo * devInfo = OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.GetValue(queue->m_device);
    if (!(devInfo->m_deviceCapabilities & CL_EXEC_NATIVE_KERNEL))
    {
        return CL_INVALID_OPERATION;
    }

    SyncManager* synchHelper = new SyncManager;
    if (!synchHelper)
    {
        return CL_OUT_OF_HOST_MEMORY;
    }

    cl_event*   outEvents = NULL;
    cl_uint     numOutEvents = 0;
    errCode = synchHelper->PrepareToExecute(
        queue,
        num_events_in_wait_list,
        event_wait_list,
        &numOutEvents,
        &outEvents,
        0,
        NULL);

    if (CL_SUCCESS != errCode)
    {
        goto FINISH;
    }

    crtEvent = new CrtEvent(queue);
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
            errCode = CL_OUT_OF_HOST_MEMORY;
            goto FINISH;
        }
            /// Translate memory object handles
        for (cl_uint i=0; i < num_mem_objects; i++)
        {
            _cl_mem_crt * crtMemHandle = (_cl_mem_crt *)(mem_list[i]);
            CrtMemObject* crtMemObj = ((CrtMemObject*)crtMemHandle->object);
            crt_mem_list[i] = crtMemObj->getDeviceMemObj(queue->m_device);
        }
    }

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
            errCode = CL_OUT_OF_HOST_MEMORY;
            goto FINISH;
        }
        event_handle->object = (void*)crtEvent;
        *event = event_handle;
    }

FINISH:
    if (crtEvent && (!event || (CL_SUCCESS != errCode)))
    {
        crtEvent->Release();
        crtEvent->DecPendencyCnt();
    }
    if (crt_mem_list)
    {
        delete[] crt_mem_list;
    }

    if (synchHelper)
    {
        synchHelper->Release(errCode);
        delete synchHelper;
    }
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

    ctxInfo = OCLCRT::crt_ocl_module.m_contextInfoGuard.GetValue(context);
    if (!ctxInfo)
    {
        errCode = CL_INVALID_CONTEXT;
    }


    mem_handle = new _cl_mem_crt;
    if (!mem_handle)
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }

    CrtContext* ctx = (CrtContext*)(ctxInfo->m_object);
    errCode = ctx->CreateImage(
        CL_MEM_OBJECT_IMAGE2D,
        flags,
        image_format,
        image_width,
        image_height,
        1,  // Depth
        image_row_pitch,
        0,  // Slice Pitch
        host_ptr,
        (CrtMemObject**)(&mem_handle->object));

FINISH:
    if (CL_SUCCESS != errCode)
    {
        if (mem_handle)
        {
            delete mem_handle;
            mem_handle = NULL;
        }
    }
    if (errcode_ret)
    {
        *errcode_ret = errCode;
    }
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

    ctxInfo = OCLCRT::crt_ocl_module.m_contextInfoGuard.GetValue(context);
    if (!ctxInfo)
    {
        errCode = CL_INVALID_CONTEXT;
    }

    mem_handle = new _cl_mem_crt;
    if (!mem_handle)
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }

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

FINISH:
    if (CL_SUCCESS != errCode)
    {
        if (mem_handle)
        {
            delete mem_handle;
            mem_handle = NULL;
        }
    }
    if (errcode_ret)
    {
        *errcode_ret = errCode;
    }
    return mem_handle;
}


/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL EnqueueReadWriteImage(
    bool                read_command,
    cl_command_queue    command_queue,
    cl_mem              image,
    cl_bool             blocking_cmd,
    const size_t *      origin,
    const size_t *      region,
    size_t              row_pitch,
    size_t              slice_pitch,
    const void *        ptr,
    cl_uint             num_events_in_wait_list,
    const cl_event *    event_wait_list,
    cl_event *          event)
{
    cl_int errCode = CL_SUCCESS;

    if (command_queue == NULL)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    if (image == NULL)
    {
        return CL_INVALID_MEM_OBJECT;
    }

    CrtQueue* queue = reinterpret_cast<CrtQueue*>(((_cl_command_queue_crt*)command_queue)->object);
    if (!queue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    SyncManager* synchHelper = new SyncManager;
    if (!synchHelper)
    {
        return CL_OUT_OF_HOST_MEMORY;
    }

    cl_event*   outEvents = NULL;
    cl_uint     numOutEvents = 0;
    errCode = synchHelper->PrepareToExecute(
        queue,
        num_events_in_wait_list,
        event_wait_list,
        &numOutEvents,
        &outEvents,
        0,
        NULL);

    if (CL_SUCCESS != errCode)
    {
        goto FINISH;
    }

    CrtEvent* crtEvent = new CrtEvent(queue);
    if (!crtEvent)
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }
    CrtImage* crtImage = reinterpret_cast<CrtImage*>(((_cl_mem_crt*)image)->object);
    if (!crtImage)
    {   errCode = CL_INVALID_MEM_OBJECT;
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

    if (read_command)
    {
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
    }
    else
    {
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
    }

    if (errCode == CL_SUCCESS && event)
    {
        _cl_event_crt* event_handle = new _cl_event_crt;
        if (!event_handle)
        {
            errCode = CL_OUT_OF_HOST_MEMORY;
            goto FINISH;
        }
        event_handle->object = (void*)crtEvent;
        *event = event_handle;
    }


FINISH:
    if (crtEvent && (!event || (CL_SUCCESS != errCode)))
    {
        crtEvent->Release();
        crtEvent->DecPendencyCnt();
    }
    if (synchHelper)
    {
        synchHelper->Release(errCode);
        delete synchHelper;
    }
    return errCode;
}


/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
inline cl_int CL_API_CALL clEnqueueWriteImage(
    cl_command_queue    command_queue,
    cl_mem              image,
    cl_bool             blocking_read,
    const size_t *      origin,
    const size_t *      region,
    size_t              row_pitch,
    size_t              slice_pitch,
    const void *        ptr,
    cl_uint             num_events_in_wait_list,
    const cl_event *    event_wait_list,
    cl_event *          event)
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
inline cl_int CL_API_CALL clEnqueueReadImage(cl_command_queue command_queue,
    cl_mem              image,
    cl_bool             blocking_read,
    const size_t *      origin,
    const size_t *      region,
    size_t              row_pitch,
    size_t              slice_pitch,
    void *              ptr,
    cl_uint             num_events_in_wait_list,
    const cl_event *    event_wait_list,
    cl_event *          event)
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
    cl_command_queue    command_queue,
    cl_mem              src_image,
    cl_mem              dst_image,
    const size_t *      src_origin,
    const size_t *      dst_origin,
    const size_t *      region,
    cl_uint             num_events_in_wait_list,
    const cl_event *    event_wait_list,
    cl_event *          event)
{
    cl_int errCode = CL_SUCCESS;
    CrtEvent* crtEvent = NULL;

    if (command_queue == NULL)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    if (src_image == NULL || dst_image == NULL)
    {
        return CL_INVALID_MEM_OBJECT;
    }

    CrtQueue* queue = reinterpret_cast<CrtQueue*>(((_cl_command_queue_crt*)command_queue)->object);
    if (!queue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    SyncManager* synchHelper = new SyncManager;
    if (!synchHelper)
    {
        return CL_OUT_OF_HOST_MEMORY;
    }

    cl_event*   outEvents = NULL;
    cl_uint     numOutEvents = 0;
    errCode = synchHelper->PrepareToExecute(
        queue,
        num_events_in_wait_list,
        event_wait_list,
        &numOutEvents,
        &outEvents,
        0,
        NULL);

    if (CL_SUCCESS != errCode)
    {
        goto FINISH;
    }

    crtEvent = new CrtEvent(queue);
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
            errCode = CL_OUT_OF_HOST_MEMORY;
            goto FINISH;
        }
        event_handle->object = (void*)crtEvent;
        *event = event_handle;
    }

FINISH:
    if (crtEvent && (!event || (CL_SUCCESS != errCode)))
    {
        crtEvent->Release();
        crtEvent->DecPendencyCnt();
    }
    if (synchHelper)
    {
        synchHelper->Release(errCode);
        delete synchHelper;
    }
    return errCode;
}


/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clEnqueueCopyImageToBuffer(
    cl_command_queue    command_queue,
    cl_mem              src_image,
    cl_mem              dst_buffer,
    const size_t *      src_origin,
    const size_t *      region,
    size_t              dst_offset,
    cl_uint             num_events_in_wait_list,
    const cl_event *    event_wait_list,
    cl_event *          event)
{
    cl_int errCode = CL_SUCCESS;
    CrtEvent* crtEvent = NULL;

    if (command_queue == NULL)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    if (src_image == NULL || dst_buffer == NULL)
    {
        return CL_INVALID_MEM_OBJECT;
    }

    CrtQueue* queue = reinterpret_cast<CrtQueue*>(((_cl_command_queue_crt*)command_queue)->object);
    if (!queue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }
   

    SyncManager* synchHelper = new SyncManager;
    if (!synchHelper)
    {
        return CL_OUT_OF_HOST_MEMORY;
    }

    cl_event*   outEvents = NULL;
    cl_uint     numOutEvents = 0;
    errCode = synchHelper->PrepareToExecute(
        queue,
        num_events_in_wait_list,
        event_wait_list,
        &numOutEvents,
        &outEvents,
        0,
        NULL);

    if (CL_SUCCESS != errCode)
    {
        goto FINISH;
    }

    crtEvent = new CrtEvent(queue);
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
            errCode = CL_OUT_OF_HOST_MEMORY;
            goto FINISH;
        }
        event_handle->object = (void*)crtEvent;
        *event = event_handle;
    }

FINISH:
    if (crtEvent && (!event || (CL_SUCCESS != errCode)))
    {
        crtEvent->Release();
        crtEvent->DecPendencyCnt();
    }
    if (synchHelper)
    {
        synchHelper->Release(errCode);
        delete synchHelper;
    }
    return errCode;
}


/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clEnqueueCopyBufferToImage(
    cl_command_queue    command_queue,
    cl_mem              src_buffer,
    cl_mem              dst_image,
    size_t              src_offset,
    const size_t *      dst_origin,
    const size_t *      region,
    cl_uint             num_events_in_wait_list,
    const cl_event *    event_wait_list,
    cl_event *          event)
{
    cl_int errCode = CL_SUCCESS;
    CrtEvent* crtEvent = NULL;

    if (command_queue == NULL)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    if (src_buffer == NULL || dst_image == NULL)
    {
        return CL_INVALID_MEM_OBJECT;
    }

    CrtQueue* queue = reinterpret_cast<CrtQueue*>(((_cl_command_queue_crt*)command_queue)->object);
    if (!queue)
   {
        return CL_INVALID_COMMAND_QUEUE;
    }
   
    SyncManager* synchHelper = new SyncManager;
    if (!synchHelper)
    {
        return CL_OUT_OF_HOST_MEMORY;
    }

    cl_event*   outEvents = NULL;
    cl_uint     numOutEvents = 0;
    errCode = synchHelper->PrepareToExecute(
        queue,
        num_events_in_wait_list,
        event_wait_list,
        &numOutEvents,
        &outEvents,
        0,
        NULL);

    if (CL_SUCCESS != errCode)
    {
        goto FINISH;
    }

    crtEvent = new CrtEvent(queue);
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
            errCode = CL_OUT_OF_HOST_MEMORY;
            goto FINISH;
        }
        event_handle->object = (void*)crtEvent;
        *event = event_handle;
    }

FINISH:
    if (crtEvent && (!event || (CL_SUCCESS != errCode)))
    {
        crtEvent->Release();
        crtEvent->DecPendencyCnt();
    }
    if (synchHelper)
    {
        synchHelper->Release(errCode);
        delete synchHelper;
    }
    return errCode;
}


/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_sampler CL_API_CALL clCreateSampler(
    cl_context          context,
    cl_bool             normalized_coords,
    cl_addressing_mode  addressing_mode,
    cl_filter_mode      filter_mode,
    cl_int *            errcode_ret)
{
    _cl_sampler_crt* sampler_handle = NULL;
    cl_int errCode = CL_SUCCESS;
    CrtContextInfo* ctxInfo  = NULL;

    ctxInfo = OCLCRT::crt_ocl_module.m_contextInfoGuard.GetValue(context);
    if (!ctxInfo)
    {
        errCode = CL_INVALID_CONTEXT;
        goto FINISH;
    }

    sampler_handle = new _cl_sampler_crt;
    if (!sampler_handle)
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }

    CrtContext* ctx = (CrtContext*)(ctxInfo->m_object);
    errCode = ctx->CreateSampler(
        normalized_coords,
        addressing_mode,
        filter_mode,
        (CrtSampler**)(&sampler_handle->object));

FINISH:
    if (CL_SUCCESS == errCode)
    {
        if (sampler_handle)
        {
            delete sampler_handle;
            sampler_handle = NULL;
        }
    }
    if (errcode_ret)
    {
        *errcode_ret = errCode;
    }

    return sampler_handle;
};


/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clRetainSampler(cl_sampler sampler)
{
    cl_int errCode = CL_SUCCESS;
    _cl_sampler_crt* sampler_handle = (_cl_sampler_crt*)sampler;
    CrtSampler* crtSampler = (CrtSampler*)(sampler_handle->object);
    crtSampler->IncRefCnt();
    return errCode;
}


/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clReleaseSampler(cl_sampler sampler)
{
    cl_int errCode = CL_SUCCESS;
    _cl_sampler_crt* sampler_handle = (_cl_sampler_crt*)sampler;
    CrtSampler* crtSampler = (CrtSampler*)(sampler_handle->object);

    crtSampler->IncPendencyCnt();
    long refCount = crtSampler->DecRefCnt();
    if (refCount == 0)
    {
        errCode = crtSampler->Release();
        delete sampler;
    }
    crtSampler->DecPendencyCnt();
    return errCode;
}


/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clGetSamplerInfo(
    cl_sampler          sampler,
    cl_sampler_info     param_name,
    size_t              param_value_size,
    void *              param_value,
    size_t *            param_value_size_ret)
{
    cl_int retCode = CL_SUCCESS;
    CrtSampler* crtSampler = reinterpret_cast<CrtSampler*>(((_cl_sampler_crt*)sampler)->object);
        /// Pick any of the underlying created sampler objects
    cl_sampler samplerObj = crtSampler->m_ContextToSampler.begin()->second;
        /// Forward call
    retCode = samplerObj->dispatch->clGetSamplerInfo(
                        samplerObj,
                        param_name,
                        param_value_size,
                        param_value,
                        param_value_size_ret);
    return retCode;
}


/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clGetImageInfo(
    cl_mem           image,
    cl_image_info    param_name,
    size_t           param_value_size,
    void *           param_value,
    size_t *         param_value_size_ret)
{
    cl_int retCode = CL_SUCCESS;
    CrtImage* crtImage = reinterpret_cast<CrtImage*>(((_cl_mem_crt*)image)->object);
        /// Pick any of the underlying created memory objects
    cl_mem memObj = crtImage->getAnyValidDeviceMemObj();
        /// Forward call
    retCode = memObj->dispatch->clGetImageInfo( memObj,
                                                param_name,
                                                param_value_size,
                                                param_value,
                                                param_value_size_ret);
    return retCode;
}


/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clGetMemObjectInfo(
    cl_mem           memobj,
    cl_mem_info      param_name,
    size_t           param_value_size,
    void *           param_value,
    size_t *         param_value_size_ret)
{
    cl_int errCode = CL_SUCCESS;

    CrtMemObject* crtMemObj = reinterpret_cast<CrtMemObject*>(((_cl_mem_crt*)memobj)->object);

    size_t  pValueSize = 0;
    switch(param_name)
    {
        case CL_MEM_FLAGS:
            {
                pValueSize = sizeof(crtMemObj->m_flags);
                if (param_value && param_value_size >= pValueSize)
                {
                    *((cl_mem_flags*)param_value) = crtMemObj->m_flags;
                }
            }
            break;
        case CL_MEM_ASSOCIATED_MEMOBJECT:
            {
                pValueSize = sizeof(void*);
                CrtBuffer* bufObj = (CrtBuffer*)crtMemObj;
                if (param_value && param_value_size >= pValueSize)
                {
                    *((cl_mem*)param_value) = bufObj->m_parentBuffer;
                }

                if( param_value_size_ret )
                {
                    *param_value_size_ret = pValueSize;
                }
            }
            break;
        case CL_MEM_HOST_PTR:
            {
                pValueSize = sizeof(void*);
                void* host_ptr;
                if (crtMemObj->m_flags & CL_MEM_USE_HOST_PTR)
                {
                    if (crtMemObj->m_pUsrPtr)
                    {
                        // The user provided host_ptr is not aligned so
                        // so we kept it
                        host_ptr = crtMemObj->m_pUsrPtr;
                    }
                    else
                    {
                        // The user provided ptr is aligned, so we set m_pUsrPtr
                        // to null, and stored a single pointer in m_pBstPtr
                        host_ptr = crtMemObj->m_pBstPtr;
                    }
                }

                if (param_value && param_value_size >= pValueSize)
                {
                    *((void**)param_value) = host_ptr;
                }

                if( param_value_size_ret )
                {
                    *param_value_size_ret = pValueSize;
                }
            }
            break;
        case CL_MEM_REFERENCE_COUNT:
            {
                pValueSize = sizeof(crtMemObj->m_refCount);
                if (param_value && param_value_size >= pValueSize)
                {
                    *((cl_uint*)param_value) = crtMemObj->m_refCount;
                }
                if( param_value_size_ret )
                {
                    *param_value_size_ret = pValueSize;
                }
            }
            break;
        default:
            {
                cl_mem devMemObj = crtMemObj->getAnyValidDeviceMemObj();
                errCode = devMemObj->dispatch->clGetMemObjectInfo(
                    devMemObj,
                    param_name,
                    param_value_size,
                    param_value,
                    param_value_size_ret);
            }
            break;
    }


    return errCode;
}


/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clGetEventProfilingInfo(
    cl_event            event,
    cl_profiling_info   param_name,
    size_t              param_value_size,
    void *              param_value,
    size_t *            param_value_size_ret)
{
    cl_int errCode = CL_SUCCESS;
    CrtEvent* crtEvent = reinterpret_cast<CrtEvent*>(((_cl_event_crt*)event)->object);
        /// Forward call
    errCode = crtEvent->m_eventDEV->dispatch->clGetEventProfilingInfo(
                crtEvent->m_eventDEV,
                param_name,
                param_value_size,
                param_value,
                param_value_size_ret);

    return errCode;
}
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clCreateKernelsInProgram(
    cl_program  program,
    cl_uint     num_kernels,
    cl_kernel * kernels,
    cl_uint *   num_kernels_ret)
{
    cl_int errCode = CL_SUCCESS;

    if (NULL == program)
    {
        return CL_INVALID_PROGRAM;
    }
    if (!num_kernels_ret && !kernels)
    {
        return CL_INVALID_VALUE;
    }

    CrtProgram* crtProgram= reinterpret_cast<CrtProgram*>(((_cl_program_crt*)program)->object);

    // Num kernels which will be returned by each context
    cl_int numKernelsPerContext = -1;

    cl_kernel* ctxKernels = new cl_kernel[num_kernels];
    if (NULL == ctxKernels)
    {
        return CL_OUT_OF_HOST_MEMORY;
    }

    for (cl_uint i=0; i < num_kernels; i++)
    {
        kernels[i] = NULL;
    }
    CTX_PGM_MAP::iterator itr = crtProgram->m_ContextToProgram.begin();
    for(;itr != crtProgram->m_ContextToProgram.end(); itr++)
    {
        cl_uint numRetKernels = 0;
        cl_context ctxObj = itr->first;
        errCode = ctxObj->dispatch->clCreateKernelsInProgram(
            crtProgram->m_ContextToProgram[ctxObj],
            num_kernels,
            ctxKernels,
            &numRetKernels);

        if (CL_SUCCESS == errCode)
        {
            if (numKernelsPerContext < 0)
            {
                numKernelsPerContext = numRetKernels;
            }
            else
            {
                if  (numKernelsPerContext != numRetKernels)
                {
                    errCode =  CL_OUT_OF_RESOURCES;
                    goto FINISH;
                }
            }
        }
        else
        {
            goto FINISH;
        }

        for (cl_uint i=0; i < (cl_uint)numKernelsPerContext; i++)
        {
            CrtKernel* crtKernel = NULL;

            if (NULL == kernels[i])
            {
                crtKernel = new CrtKernel(crtProgram);
                if( !crtKernel )
                {
                    errCode = CL_OUT_OF_HOST_MEMORY;
                    goto FINISH;
                }

                kernels[i] = new _cl_kernel_crt;
                if (NULL == kernels[i])
                {
                    crtKernel->DecPendencyCnt();
                    errCode = CL_OUT_OF_HOST_MEMORY;
                    goto FINISH;
                }
                ((_cl_kernel_crt*)kernels[i])->object = crtKernel;
            }
            else
            {
                crtKernel = (CrtKernel*)((_cl_kernel_crt*)kernels[i])->object;
            }
            crtKernel->m_ContextToKernel[ctxObj] = ctxKernels[i];
        }
    }
FINISH:
    if (CL_SUCCESS != errCode)
    {
        for (cl_uint i=0; i < num_kernels; i++)
        {
            if (NULL != kernels[i])
            {
                CrtKernel* crtKernel = (CrtKernel*)((_cl_kernel_crt*)kernels[i])->object;
                crtKernel->Release();
                crtKernel->DecPendencyCnt();
                delete kernels[i];
                kernels[i] = NULL;
            }
        }
    }
    if (ctxKernels)
    {
        delete[] ctxKernels;
    }
    if (num_kernels_ret)
    {
        *num_kernels_ret = numKernelsPerContext;
    }
    // Delete allocated structures / data
    return errCode;
}
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clGetSupportedImageFormats(
    cl_context           context,
	cl_mem_flags         flags,
	cl_mem_object_type   image_type,
	cl_uint              num_entries,
	cl_image_format *    image_formats,
	cl_uint *            num_image_formats)
{
    cl_int errCode = CL_SUCCESS;    
    
    if (num_entries && num_entries == 0)
    {
        return CL_INVALID_VALUE;
    }

    CrtContext* ctx = reinterpret_cast<CrtContext*>(((_cl_context_crt*)context)->object);

    cl_uint numTotalAvailable = 0;
    for( SHARED_CTX_DISPATCH::iterator itr = ctx->m_contexts.begin();
        itr != ctx->m_contexts.end(); 
        itr++)		
    {
        cl_uint numAvailable = 0;
        cl_context ctxObj = itr->first;
        errCode = ctxObj->dispatch->clGetSupportedImageFormats( 
            ctxObj,
            flags,
            image_type,
            0,
            NULL,
            &numAvailable);

	    if (CL_SUCCESS != errCode)
	    {
            goto FINISH;
        }       
        numTotalAvailable += numAvailable;
    }	    

    cl_image_format* allImageFormats = new cl_image_format[numTotalAvailable];
    if (NULL == allImageFormats)
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }

    cl_uint offset = 0;
    for( SHARED_CTX_DISPATCH::iterator itr = ctx->m_contexts.begin();
        itr != ctx->m_contexts.end(); 
        itr++)		
    {        
        cl_uint numAvailable = 0;
        cl_context ctxObj = itr->first;
        errCode = ctxObj->dispatch->clGetSupportedImageFormats( 
            ctxObj,
            flags,
            image_type,
            numTotalAvailable,
            allImageFormats  + offset,
            &numAvailable);

	    if (CL_SUCCESS != errCode)
	    {
            goto FINISH;
        }       
        offset += numAvailable;
    }	       
    
    std::sort(allImageFormats, allImageFormats + numTotalAvailable);
    cl_image_format* endUnique = std::unique(allImageFormats,allImageFormats + numTotalAvailable);
    cl_uint numUnique = (cl_uint)(endUnique - allImageFormats);
    
    if (num_image_formats)
    {
        *num_image_formats = numUnique;
    }
    
    if (!num_entries)
    {
        goto FINISH;
    }
    
    if (num_entries < numUnique)
    {
        errCode = CL_INVALID_VALUE;
        goto FINISH;
    }
    for (cl_uint i=0; i < numUnique; i++)
    {
        image_formats[i] = allImageFormats[i];
    }    
    
FINISH:    

    return errCode;
}
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clGetDeviceIDsFromD3D10KHR(
    cl_platform_id              platform,
    cl_d3d10_device_source_khr  d3d_device_source,
    void                        *d3d_object,
    cl_d3d10_device_set_khr     d3d_device_set,
    cl_uint                     num_entries,
    cl_device_id                *devices,
    cl_uint                     *num_devices)
{
    cl_int errCode = CL_SUCCESS;

    // We don't support DX for shared context
    cl_uint numDevices=0;
    for (cl_uint i = 0; i < OCLCRT::crt_ocl_module.m_oclPlatforms.size(); i++)
    {
        CrtPlatform* crtPlatform = OCLCRT::crt_ocl_module.m_oclPlatforms[i];

        if (crtPlatform->m_supportedExtensions & CRT_CL_D3D10_EXT)
        {
            errCode = crtPlatform->m_platformIdDEV->dispatch->clGetDeviceIDsFromD3D10KHR(
                                        crtPlatform->m_platformIdDEV,
                                        d3d_device_source,
                                        d3d_object,
                                        d3d_device_set,
                                        num_entries,
                                        devices,
                                        num_devices);
            if (errCode == CL_SUCCESS)
            {
                goto FINISH;
            }
        }
    }

FINISH:
    return errCode;
}


/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
CL_API_ENTRY cl_int CL_API_CALL clGetDeviceIDsFromDX9INTEL(
    cl_platform_id              platform,
    cl_dx9_device_source_intel d3d_device_source,
    void *                      d3d_object,
    cl_dx9_device_set_intel    d3d_device_set,
    cl_uint                     num_entries,
    cl_device_id                *devices,
    cl_uint                     *num_devices)
{
    if (NULL == platform)
    {
        return CL_INVALID_PLATFORM;
    }

    cl_int errCode = CL_SUCCESS;

    // We don't support DX for shared context
    cl_uint numDevices=0;
    for (cl_uint i = 0; i < OCLCRT::crt_ocl_module.m_oclPlatforms.size(); i++)
    {
        CrtPlatform* crtPlatform = OCLCRT::crt_ocl_module.m_oclPlatforms[i];

        if (crtPlatform->m_supportedExtensions & CRT_CL_D3D9_EXT)
        {
            errCode = ( (CrtKHRicdVendorDispatch*)(crtPlatform->m_platformIdDEV->dispatch) )->clGetDeviceIDsFromDX9INTEL(
                                        crtPlatform->m_platformIdDEV,
                                        d3d_device_source,
                                        d3d_object,
                                        d3d_device_set,
                                        num_entries,
                                        devices,
                                        num_devices);
            if (errCode == CL_SUCCESS)
            {
                goto FINISH;
            }
        }
    }

FINISH:
    return errCode;
}

/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
CL_API_ENTRY cl_mem CL_API_CALL clCreateFromDX9MediaSurfaceINTEL(
    cl_context          context,
    cl_mem_flags        flags,
    IDirect3DSurface9 * resource,
    HANDLE              sharedHandle,
    UINT                plane,
    cl_int *            errcode_ret)
{   
    cl_int errCode = CL_SUCCESS;
    cl_mem memObj = NULL;

    if (NULL == context)
    {
        errCode = CL_INVALID_CONTEXT;
        goto FINISH;
    }

    memObj = ( (CrtKHRicdVendorDispatch*)(context->dispatch) )->clCreateFromDX9MediaSurfaceINTEL(
                                        context,
                                        flags,
                                        resource,
                                        sharedHandle,
                                        plane,
                                        &errCode);
    if (errCode == CL_SUCCESS)
    {
        goto FINISH;
    }

FINISH:
    if (errcode_ret)
    {
        *errcode_ret = errCode;
    }
    return memObj;
}


/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
CL_API_ENTRY cl_int CL_API_CALL
clEnqueueAcquireDX9ObjectsINTEL( cl_command_queue command_queue,
                                  cl_uint          num_objects,
                                  const cl_mem     *mem_objects,
                                  cl_uint          num_events_in_wait_list,
                                  const cl_event   *event_wait_list,
                                  cl_event         *ocl_event )
{
    if (NULL == command_queue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    cl_int errCode = CL_SUCCESS;

        /// We don't support DX for shared context
    errCode = ( (CrtKHRicdVendorDispatch*)(command_queue->dispatch) )->clEnqueueAcquireDX9ObjectsINTEL(
                                        command_queue,
                                        num_objects,
                                        mem_objects,
                                        num_events_in_wait_list,
                                        event_wait_list,
                                        ocl_event);
    if (errCode == CL_SUCCESS)
    {
        goto FINISH;
    }

FINISH:
    return errCode;
}


/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
CL_API_ENTRY cl_int CL_API_CALL
clEnqueueReleaseDX9ObjectsINTEL( cl_command_queue command_queue,
                                  cl_uint          num_objects,
                                  const cl_mem *   mem_objects,
                                  cl_uint          num_events_in_wait_list,
                                  const cl_event * event_wait_list,
                                  cl_event *       ocl_event )
{
    if (NULL == command_queue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    cl_int errCode = CL_SUCCESS;

    errCode = ( (CrtKHRicdVendorDispatch*)(command_queue->dispatch) )->clEnqueueReleaseDX9ObjectsINTEL(
                                        command_queue,
                                        num_objects,
                                        mem_objects,
                                        num_events_in_wait_list,
                                        event_wait_list,
                                        ocl_event);
    if (errCode == CL_SUCCESS)
    {
        goto FINISH;
    }

FINISH:
    return errCode;
}


/// Defined CRT CL API
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
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

    /// GPU specific extensions
    if ( !strcmp(funcname, "clGetDeviceIDsFromDX9INTEL" ) )
    {
        return ((void*)clGetDeviceIDsFromDX9INTEL);
    }
    if ( !strcmp(funcname, "clCreateFromDX9MediaSurfaceINTEL" ) )
    {
        return ((void*)clCreateFromDX9MediaSurfaceINTEL);
    }
    if ( !strcmp(funcname, "clEnqueueAcquireDX9ObjectsINTEL" ) )
    {
        return ((void*)clEnqueueAcquireDX9ObjectsINTEL);
    }
    if ( !strcmp(funcname, "clEnqueueReleaseDX9ObjectsINTEL" ) )
    {
        return ((void*)clEnqueueReleaseDX9ObjectsINTEL);
    }
    return NULL;
};
