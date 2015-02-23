// Copyright (c) 2006-2014 Intel Corporation
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
#include <cl_secure_string.h>
#include <cl_synch_objects.h>
#include <crt_named_pipe.h>
#include <algorithm>
#include <string>
#include <numeric>
#include <vector>
#include <sstream>
#include <iterator>

namespace OCLCRT
{
    CrtModule crt_ocl_module;

    // Globally Initialized Variable
    char* CrtModule::m_common_extensions = NULL;
}

#define isValidPlatform(X) ((X) == OCLCRT::crt_ocl_module.m_crtPlatformId || NULL == (X))

// Defined CRT CL handles
_cl_platform_id_crt::_cl_platform_id_crt()
{
    dispatch = &OCLCRT::crt_ocl_module.m_icdDispatchMgr.m_icdDispatchTable;
}

_cl_context_crt::_cl_context_crt()
{
    dispatch = &OCLCRT::crt_ocl_module.m_icdDispatchMgr.m_icdDispatchTable;
}

_cl_program_crt::_cl_program_crt()
{
    dispatch = &OCLCRT::crt_ocl_module.m_icdDispatchMgr.m_icdDispatchTable;
}

_cl_kernel_crt::_cl_kernel_crt()
{
    dispatch = &OCLCRT::crt_ocl_module.m_icdDispatchMgr.m_icdDispatchTable;
}

_cl_command_queue_crt::_cl_command_queue_crt()
{
    dispatch = &OCLCRT::crt_ocl_module.m_icdDispatchMgr.m_icdDispatchTable;
}

_cl_mem_crt::_cl_mem_crt()
{
    dispatch = &OCLCRT::crt_ocl_module.m_icdDispatchMgr.m_icdDispatchTable;
}

_cl_event_crt::_cl_event_crt()
{
    dispatch = &OCLCRT::crt_ocl_module.m_icdDispatchMgr.m_icdDispatchTable;
}

_cl_sampler_crt::_cl_sampler_crt()
{
    dispatch = &OCLCRT::crt_ocl_module.m_icdDispatchMgr.m_icdDispatchTable;
}


/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
CL_API_ENTRY cl_int CL_API_CALL clGetPlatformIDs(
    cl_uint           num_entries,
    cl_platform_id *  platforms,
    cl_uint *         num_platforms )
{
    if( ( ( 0 == num_entries ) && ( NULL != platforms ) ) ||
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
}
SET_ALIAS( clGetPlatformIDs );

// Helper function for 'UpdatePlatformExtensions'
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
}

// Detects what extensions are in common for all the
// the managed platforms.
void UpdatePlatformExtensions()
{
    if (NULL == OCLCRT::crt_ocl_module.m_common_extensions)
    {
        // common extensions initialization needs to be thread-safe
        OCLCRT::Utils::OclAutoMutex CS(&OCLCRT::crt_ocl_module.m_mutex);
        // Check if any other thread has done the initialization for us
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
                    // start with all extensions from the first platform
                    common_exts.assign(platExt_it, platExt_end);
                    continue;
                }
                std::vector<std::string> platExt_vec(platExt_it, platExt_end);
                // sort the vectors, required by set_intersection
                std::sort(common_exts.begin(),common_exts.end());
                std::sort(platExt_vec.begin(),platExt_vec.end());
                // Do the intersection in-place to common_exts
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
    std::string crtVersion;

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
            switch( OCLCRT::crt_ocl_module.m_CrtPlatformVersion )
            {
            case OPENCL_1_1:
                crtVersion.assign(INTEL_OPENCL_1_1_PVER_STR, strlen( INTEL_OPENCL_1_1_PVER_STR ) );
                break;
            case OPENCL_1_2:
                crtVersion.assign(INTEL_OPENCL_1_2_PVER_STR, strlen( INTEL_OPENCL_1_2_PVER_STR ) );
                break;
            case OPENCL_2_0:
                crtVersion.assign(INTEL_OPENCL_2_0_PVER_STR, strlen( INTEL_OPENCL_2_0_PVER_STR ) );
                break;
            }
            RetSize = strnlen_s( crtVersion.c_str(), MAX_STRLEN );
            RetSize++;
            if( param_value )
            {
                if( RetSize > param_value_size )
                {
                    return CL_INVALID_VALUE;
                }
                strncpy_s( ( char * )param_value, RetSize, crtVersion.c_str(), RetSize );
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
            // there was an allocation failure in UpdatePlatformExtensions
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
SET_ALIAS( clGetPlatformInfo );
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
        !( device_type & CL_DEVICE_TYPE_ACCELERATOR ) &&
        !( device_type & CL_DEVICE_TYPE_CUSTOM ) )
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

    // Lock Devices Map, devices might be concurrently modified
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
        devType = devInfo->m_devType;
        bool devFound = ( ( 1 == OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.size() ) ||
                          ( devType == OCLCRT::crt_ocl_module.m_defaultDeviceType ) );

        if( ( ( device_type == CL_DEVICE_TYPE_DEFAULT ) && devFound ) ||
            ( devType & device_type ) )
        {
            if( devices && ( numRet < num_entries ) )
            {
                if( 0 != numRet && devType == OCLCRT::crt_ocl_module.m_defaultDeviceType )
                {
                    // place DEFAULT device at the beginning
                    devices[numRet++] = devices[0];
                    devices[0] = devIdDEV;
                }
                else
                {
                    devices[numRet++] = devIdDEV;
                }
            }
            else
            {
                if( devType == OCLCRT::crt_ocl_module.m_defaultDeviceType && num_entries > 0 )
                {
                    // replace the first entry with DEFAULT device
                    devices[0] = devIdDEV;
                }
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

    if( numRet == 0 )
    {
        errCode = CL_DEVICE_NOT_FOUND;
    }

    // Unlock Devices Map, done with it
    OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.Release();
    return errCode;
}
SET_ALIAS( clGetDeviceIDs );
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_context CL_API_CALL clCreateContext(
    const cl_context_properties *   properties,
    cl_uint                         num_devices,
    const cl_device_id *            devices,
    ctxt_logging_fn                 pfn_notify,
    void *                          user_data,
    cl_int *                        errcode_ret )
{
    cl_int errCode                  = CL_SUCCESS;
    cl_context ctx                  = NULL;
    cl_uint numPlatforms            = 0;
    cl_platform_id *pPlatformIdDEV  = NULL;
    bool isGPUPlatform              = false;

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
            pPlatformIdDEV = &OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.GetValue(devIdDEV)->m_crtPlatform->m_platformIdDEV;
        }
        else
        {
            if( *pPlatformIdDEV ==  OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.GetValue(devIdDEV)->m_crtPlatform->m_platformIdDEV)
            {
                continue;
            }
        }
        numPlatforms++;

        if( CL_DEVICE_TYPE_GPU == OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.GetValue( devIdDEV )->m_devType )
        {
            isGPUPlatform = true;
        }
    }

    if( ( errCode == CL_SUCCESS ) && ( numPlatforms == 1 ) )
    {
        // Single Platform Context (All devices belong to same underlying platform)
        const KHRicdVendorDispatch* dTable =
            (KHRicdVendorDispatch*)( &OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.GetValue( devices[0] )->m_origDispatchTable );

        if( !dTable )
        {
            errCode = CL_INVALID_DEVICE;
            goto FINISH; 
        }

        cl_context_properties* props;
        if( CRT_FAIL == OCLCRT::ReplacePlatformId(
                            properties,
                            OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.GetValue( devices[0] )->m_crtPlatform->m_platformIdDEV,
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
            ctx = dTable->clCreateContext(
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

                if( isGPUPlatform )
                {
                    pContextInfo->m_contextType = CrtContextInfo::SinglePlatformGPUContext;
                }
                else
                {
                    pContextInfo->m_contextType = CrtContextInfo::SinglePlatformCPUContext;
                }

                pContextInfo->m_crtPlatform = OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.GetValue(devices[0])->m_crtPlatform;
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
            delete[] props;
            props = NULL;
        }
    }
    else
    {
        // Shared Platform Context

        // We don't support GL shared context now!
        // We don't support DX (9/10/11) shared context now!
        // We don't support devices other than CPU or GPU in shared context now!
        if( !( OCLCRT::isSupportedContextType( properties, num_devices, devices ) ) )
        {
            errCode = CL_OUT_OF_RESOURCES;
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

        if( NULL == crtContext )
        {
            errCode = CL_OUT_OF_HOST_MEMORY;
            goto FINISH;
        }
        else if( CL_SUCCESS != errCode )
        {
            delete ctx;
            ctx = NULL;
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
            ctx = NULL;
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
SET_ALIAS( clCreateContext );
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_context CL_API_CALL clCreateContextFromType(
    const cl_context_properties *   properties,
    cl_device_type                  device_type,
    ctxt_logging_fn                 pfn_notify,
    void *                          user_data,
    cl_int *                        errcode_ret)
{
    cl_int errCode                  = CL_SUCCESS;
    cl_device_id *deviceList        = NULL;
    cl_context ctx                  = NULL;
    cl_uint numPlatforms            = 0;
    cl_uint numDevices              = 0;
    cl_platform_id pId              = NULL;
    size_t numPlatformDevices       = 0;
    bool OnlyOneAvailableDevice     = false;

    errCode = OCLCRT::crt_ocl_module.isValidProperties( properties );
    if( CL_SUCCESS != errCode )
    {
        goto FINISH;
    }

    if( !( device_type & CL_DEVICE_TYPE_DEFAULT )     &&
        !( device_type & CL_DEVICE_TYPE_CPU )         &&
        !( device_type & CL_DEVICE_TYPE_GPU )         &&
        !( device_type & CL_DEVICE_TYPE_ACCELERATOR ) )
    {
        errCode = CL_INVALID_DEVICE_TYPE;
        goto FINISH;
    }

    if( !pfn_notify && user_data )
    {
        errCode = CL_INVALID_VALUE;
        goto FINISH;
    }
    numPlatformDevices = OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.size();

    deviceList = new cl_device_id[numPlatformDevices];
    if( deviceList == NULL )
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }

    OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.Lock();

    // In case there is only one underlying device, we pick it as CL_DEFAULT_DEVICE_TYPE
    OnlyOneAvailableDevice = (numPlatformDevices == 1);

    for( OCLCRT::DEV_INFO_MAP::iterator itr = OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.get().begin();
        itr != OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.get().end();
        itr++ )
    {
        CrtDeviceInfo* devInfo = itr->second;
        if( devInfo->m_isRootDevice == false )
        {
            // Skip Sub-Devices
            continue;
        }
        cl_device_type clDevType = devInfo->m_devType;

        // In case of two-devices pick the CRT default
        if( ( device_type == CL_DEVICE_TYPE_DEFAULT ) &&
            ( ( clDevType == OCLCRT::crt_ocl_module.m_defaultDeviceType ) ||
              ( OnlyOneAvailableDevice ) ) )
        {
            numDevices = 1;
            numPlatforms = 1;
            deviceList[0] = itr->first;
            break;
        }
        if( device_type & clDevType )
        {
            if( numDevices == 0 )
            {
                numPlatforms = 1;
                pId =   devInfo->m_crtPlatform->m_platformIdDEV;
            }
            else
            {
                if( pId != devInfo->m_crtPlatform->m_platformIdDEV )
                {
                    numPlatforms++;
                }
            }
            deviceList[numDevices++] = itr->first;
        }
    }
    OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.Release();

    if( CL_SUCCESS == errCode )
    {
        if( numDevices == 0 )
        {
            errCode = CL_DEVICE_NOT_FOUND;
        }
        else
        {
            ctx = clCreateContext(properties, numDevices, deviceList, pfn_notify, user_data, &errCode);
        }
    }
FINISH:
    if( errcode_ret )
    {
        *errcode_ret = errCode;
    }
    delete[] deviceList;
    return ctx;
}
SET_ALIAS( clCreateContextFromType );
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
    if( !ctxInfo )
    {
        return CL_INVALID_CONTEXT;
    }

    if (ctxInfo->m_contextType != CrtContextInfo::SharedPlatformContext)
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
        // Shared Platform Contxt
        CrtContext* ctx = (CrtContext*)(ctxInfo->m_object);

        size_t pValueSize = 0;

        switch (param_name)
        {
        case CL_CONTEXT_REFERENCE_COUNT:
            {
                DEV_CTX_MAP::iterator itr = ctx->m_DeviceToContext.begin();
                if( OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.GetValue(itr->first) == NULL )
                {
                    errCode = CL_INVALID_VALUE;
                    break;
                }

                errCode = OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.GetValue(itr->first)->m_origDispatchTable.clGetContextInfo(
                    itr->second,
                    param_name,
                    param_value_size,
                    param_value,
                    &pValueSize);

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
                    &pValueSize);

                if( errCode == CL_SUCCESS )
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

        if( param_value && param_value_size < pValueSize )
        {
            return CL_INVALID_VALUE;
        }
    }
    return errCode;
}
SET_ALIAS( clGetContextInfo );
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clGetGLContextInfoKHR( const cl_context_properties * properties,
                                          cl_gl_context_info            param_name,
                                          size_t                        param_value_size,
                                          void *                        param_value,
                                          size_t *                      param_value_size_ret)
{
    OCLCRT::DEV_INFO_MAP::const_iterator    itr;
    cl_int errCode                          = CL_SUCCESS;
    size_t total_value_size_ret             = 0;
    void* platform_param_value              = NULL;
    size_t platform_param_value_size_ret    = 0;

    OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.Lock();

    if( param_value == NULL && param_value_size_ret == NULL )
    {
        return CL_INVALID_VALUE;
    }

    if( CL_SUCCESS != (errCode = OCLCRT::crt_ocl_module.isValidProperties( properties ) ) )
    {
        return errCode;
    }

    for( itr = OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.get().begin();
         itr != OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.get().end();
         itr++ )
    {
        CrtPlatform* devicePlatform = itr->second->m_crtPlatform;

        if( !( devicePlatform->m_supportedExtensions & CRT_CL_GL_EXT ) )
        {
            // The current platform isn't supporting DX9 interop
            continue;
        }

        cl_context_properties* props = NULL;
        if( CRT_FAIL == OCLCRT::ReplacePlatformId( properties, itr->second->m_crtPlatform->m_platformIdDEV, &props ) )
        {
            errCode = CL_OUT_OF_HOST_MEMORY;
            goto FINISH;
        }

        switch( param_name )
        {
        case CL_CURRENT_DEVICE_FOR_GL_CONTEXT_KHR:
           if( ( ( OCLCRT::crt_ocl_module.m_availableDeviceTypes & CL_DEVICE_TYPE_GPU ) &&
               ( itr->second->m_devType != CL_DEVICE_TYPE_GPU ) ) ||
               ( itr->second->m_isRootDevice == false ) )
           {
               continue;
           }
           errCode = itr->second->m_crtPlatform->m_platformIdDEV->dispatch->clGetGLContextInfoKHR( props,
                        param_name,
                        param_value_size,
                        param_value,
                        &total_value_size_ret );

           delete[] props;
           props = NULL;

           if( errCode == CL_SUCCESS )
           {
               goto FINISH;
           }
           break;

        case CL_DEVICES_FOR_GL_CONTEXT_KHR:
            if( param_value != NULL )
            {
                platform_param_value = &( ((char*)param_value)[ total_value_size_ret ] );
            }
            errCode = itr->second->m_crtPlatform->m_platformIdDEV->dispatch->clGetGLContextInfoKHR( props,
                        param_name,
                        param_value_size,
                        platform_param_value,
                        &platform_param_value_size_ret );

            delete[] props;
            props = NULL;

            if( errCode == CL_SUCCESS )
            {
                total_value_size_ret += platform_param_value_size_ret;

                if ( ( param_value != NULL ) && ( total_value_size_ret > param_value_size ) )
                {
                    errCode = CL_INVALID_VALUE;
                    goto FINISH;
                }
            }
            else
            {
                goto FINISH;
            }
            break;
        default:
            errCode = CL_INVALID_VALUE;
            goto FINISH;
            break;
        }
    }
FINISH:
    OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.Release();
    if( param_value_size_ret != NULL )
    {
        *param_value_size_ret = total_value_size_ret;
    }
    return errCode;
}
SET_ALIAS( clGetGLContextInfoKHR );
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

    if( ( retCode == CL_SUCCESS ) &&
        ( param_name == CL_DEVICE_PLATFORM ) &&
        ( param_value ) )
    {
        memcpy_s( param_value,
                  sizeof( cl_platform_id ),
                  &OCLCRT::crt_ocl_module.m_crtPlatformId,
                  sizeof( cl_platform_id ) );
    }
    return retCode;
}
SET_ALIAS( clGetDeviceInfo );
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_command_queue CL_API_CALL clCreateCommandQueue(cl_context                  context,
                                                  cl_device_id                device,
                                                  cl_command_queue_properties properties,
                                                  cl_int *                    errcode_ret)
{
    _cl_command_queue_crt*  queue_handle    = NULL;
    CrtQueue*               queue           = NULL;
    CrtContext*             ctx             = NULL;
    cl_int                  errCode         = CL_SUCCESS;
    CrtContextInfo*         ctxInfo         = NULL;

    ctxInfo = OCLCRT::crt_ocl_module.m_contextInfoGuard.GetValue( context );
    if (!ctxInfo)
    {
        errCode = CL_INVALID_CONTEXT;
        goto FINISH;
    }

    queue_handle = new _cl_command_queue_crt;
    if( !queue_handle )
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }

    ctx = (CrtContext*)(ctxInfo->m_object);
    errCode = ctx->CreateCommandQueue(queue_handle, device, properties, &queue);
    if (CL_SUCCESS != errCode)
    {
        delete queue_handle;
        queue_handle = NULL;
        goto FINISH;
    }
    queue_handle->object = (void*)queue;

FINISH:
    if (errcode_ret)
    {
        *errcode_ret = errCode;
    }

    return queue_handle;
}
SET_ALIAS( clCreateCommandQueue );
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_command_queue CL_API_CALL clCreateCommandQueueWithProperties(
    cl_context                  context,
    cl_device_id                device,
    const cl_queue_properties*  properties,
    cl_int *                    errcode_ret )
{
    _cl_command_queue_crt*  queue_handle    = NULL;
    CrtQueue*               queue           = NULL;
    CrtContext*             ctx             = NULL;
    cl_int                  errCode         = CL_SUCCESS;
    CrtContextInfo*         ctxInfo         = NULL;

    ctxInfo = OCLCRT::crt_ocl_module.m_contextInfoGuard.GetValue( context );
    if( !ctxInfo )
    {
        errCode = CL_INVALID_CONTEXT;
        goto FINISH;
    }

    queue_handle = new _cl_command_queue_crt;
    if( !queue_handle )
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }

    ctx = ( CrtContext* )( ctxInfo->m_object );
    errCode = ctx->CreateCommandQueueWithProperties( queue_handle, device, properties, &queue );
    if( CL_SUCCESS != errCode )
    {
        delete queue_handle;
        queue_handle = NULL;
        goto FINISH;
    }    
    queue_handle->object = ( void* ) queue;
FINISH:
    if( errcode_ret )
    {
        *errcode_ret = errCode;
    }

    return queue_handle;
}
SET_ALIAS( clCreateCommandQueueWithProperties );
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
    cl_int errCode      = CL_SUCCESS;
    CrtQueue* crtQueue  = reinterpret_cast<CrtQueue*>(((_cl_command_queue_crt*)command_queue)->object);

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

            if( param_value_size_ret )
            {
                *param_value_size_ret = pValueSize;
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
SET_ALIAS( clGetCommandQueueInfo );
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_mem CL_API_CALL clCreateBuffer(cl_context   context,
                                  cl_mem_flags flags,
                                  size_t       size,
                                  void *       host_ptr,
                                  cl_int *     errcode_ret)
{
    _cl_mem_crt *mem_handle     = NULL;
    cl_int errCode              = CL_SUCCESS;
    CrtContextInfo *ctxInfo     = NULL;
    CrtContext *ctx             = NULL;

    ctxInfo = OCLCRT::crt_ocl_module.m_contextInfoGuard.GetValue(context);
    if (!ctxInfo)
    {
        errCode = CL_INVALID_CONTEXT;
        goto FINISH;
    }

    mem_handle = new _cl_mem_crt;
    if( !mem_handle )
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }

    ctx = (CrtContext*)(ctxInfo->m_object);
    errCode = ctx->CreateBuffer(
        flags,
        size,
        host_ptr,
        (CrtMemObject**)(&mem_handle->object));

    if( CL_SUCCESS == errCode )
    {
        ((CrtMemObject*)(mem_handle->object))->SetMemHandle( mem_handle );
    }

FINISH:
    if( CL_SUCCESS != errCode )
    {
        delete mem_handle;
        mem_handle = NULL;
    }
    if( errcode_ret )
    {
        *errcode_ret = errCode;
    }

    return mem_handle;
}
SET_ALIAS( clCreateBuffer );
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
    _cl_mem_crt *mem_handle = NULL;
    cl_int errCode          = CL_SUCCESS;
    CrtBuffer *crtBuffer    = NULL;

    // We check this now since we rely on this internally
    // to decide if to create a buffer or a sub-buffer
    if( !buffer_create_info )
    {
        errCode = CL_INVALID_VALUE;
        goto FINISH;
    }

    crtBuffer = reinterpret_cast<CrtBuffer*>(((_cl_mem_crt*)buffer)->object);
    if( !crtBuffer )
    {
        errCode = CL_INVALID_MEM_OBJECT;
        goto FINISH;
    }

    mem_handle = new _cl_mem_crt;
    if( !mem_handle )
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
SET_ALIAS( clCreateSubBuffer );
/// ------------------------------------------------------------------------------
/// Commmon Runtime Helper function (Read/Write Buffer)
/// ------------------------------------------------------------------------------
inline cl_int CL_API_CALL EnqueueReadWriteBuffer(
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
    cl_int errCode              = CL_SUCCESS;
    CrtEvent *crtEvent          = NULL;
    SyncManager *synchHelper    = NULL;
    cl_event *outEvents         = NULL;
    cl_uint numOutEvents        = 0;

    if( command_queue == NULL )
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    if( buffer == NULL )
    {
        return CL_INVALID_MEM_OBJECT;
    }

    CrtQueue* queue = reinterpret_cast<CrtQueue*>(((_cl_command_queue_crt*)command_queue)->object);
    if( !queue )
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    CrtBuffer* crtBuffer = reinterpret_cast<CrtBuffer*>(((_cl_mem_crt*)buffer)->object);
    if( !crtBuffer )
    {
        errCode = CL_INVALID_MEM_OBJECT;
        goto FINISH;
    }

    if( crtBuffer->m_pContext != queue->m_contextCRT )
    {
        errCode = CL_INVALID_CONTEXT;
        goto FINISH;
    }

    synchHelper = new SyncManager;
    if( !synchHelper )
    {
        return CL_OUT_OF_HOST_MEMORY;
    }

    errCode = synchHelper->PrepareToExecute(
        queue,
        num_events_in_wait_list,
        event_wait_list,
        &numOutEvents,
        &outEvents);

    if( CL_SUCCESS != errCode )
    {
        goto FINISH;
    }

    crtEvent = new CrtEvent(queue);
    if( !crtEvent )
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }
    if( read_command )
    {
        if( crtBuffer->m_flags & ( CL_MEM_HOST_NO_ACCESS | CL_MEM_HOST_WRITE_ONLY ) )
        {
            errCode = CL_INVALID_OPERATION;
            goto FINISH;
        }
    }
    else
    {
        if( crtBuffer->m_flags & ( CL_MEM_HOST_NO_ACCESS | CL_MEM_HOST_READ_ONLY ) )
        {
            errCode = CL_INVALID_OPERATION;
            goto FINISH;
        }
    }

    if( blocking_cmd )
    {
        errCode = queue->m_contextCRT->FlushQueues();
        if( CL_SUCCESS != errCode )
        {
            errCode = CL_OUT_OF_RESOURCES;
            goto FINISH;
        }
    }

    if( read_command )
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

    if( errCode == CL_SUCCESS && event )
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
    if( crtEvent && ( !event || ( CL_SUCCESS != errCode ) ) )
    {
        crtEvent->Release();
        crtEvent->DecPendencyCnt();
    }
    if( synchHelper )
    {
        synchHelper->Release( errCode );
        delete synchHelper;
    }
    return errCode;
}
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clEnqueueReadBuffer(
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
SET_ALIAS( clEnqueueReadBuffer );
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clEnqueueWriteBuffer(
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
SET_ALIAS( clEnqueueWriteBuffer );
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
    cl_int errCode              = CL_SUCCESS;
    CrtEvent* crtEvent          = NULL;
    SyncManager* synchHelper    = NULL;
    cl_event *outEvents         = NULL;
    cl_uint numOutEvents        = 0;

    if( !command_queue )
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    if( !buffer )
    {
        return CL_INVALID_MEM_OBJECT;
    }

    CrtQueue* queue = reinterpret_cast<CrtQueue*>(((_cl_command_queue_crt*)command_queue)->object);
    if( !queue )
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    CrtBuffer* crtBuffer = reinterpret_cast<CrtBuffer*>(((_cl_mem_crt*)buffer)->object);
    if( !crtBuffer )
    {
        errCode = CL_INVALID_MEM_OBJECT;
        goto FINISH;
    }

    if( crtBuffer->m_pContext != queue->m_contextCRT )
    {
        errCode = CL_INVALID_CONTEXT;
        goto FINISH;
    }

    synchHelper = new SyncManager;
    if( !synchHelper )
    {
        return CL_OUT_OF_HOST_MEMORY;
    }

    errCode = synchHelper->PrepareToExecute(
        queue,
        num_events_in_wait_list,
        event_wait_list,
        &numOutEvents,
        &outEvents);

    if( CL_SUCCESS != errCode )
    {
        goto FINISH;
    }

    crtEvent = new CrtEvent(queue);
    if( !crtEvent )
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }

    if( read_command )
    {
        if( crtBuffer->m_flags & ( CL_MEM_HOST_NO_ACCESS | CL_MEM_HOST_WRITE_ONLY ) )
        {
            errCode = CL_INVALID_OPERATION;
            goto FINISH;
        }
    }
    else
    {
        if( crtBuffer->m_flags & ( CL_MEM_HOST_NO_ACCESS | CL_MEM_HOST_READ_ONLY ) )
        {
            errCode = CL_INVALID_OPERATION;
            goto FINISH;
        }
    }

    if( blocking_cmd )
    {
        errCode = queue->m_contextCRT->FlushQueues();
        if( CL_SUCCESS != errCode )
        {
            errCode = CL_OUT_OF_RESOURCES;
            goto FINISH;
        }
    }

    if( read_command )
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

    if( errCode == CL_SUCCESS && event )
    {
        _cl_event_crt* event_handle = new _cl_event_crt;
        if( !event_handle )
        {
            errCode = CL_OUT_OF_HOST_MEMORY;
            goto FINISH;
        }
        event_handle->object = (void*)crtEvent;
        *event = event_handle;
    }

FINISH:
    if( crtEvent && ( !event || ( CL_SUCCESS != errCode ) ) )
    {
        crtEvent->Release();
        crtEvent->DecPendencyCnt();
    }
    if( synchHelper )
    {
        synchHelper->Release( errCode );
        delete synchHelper;
    }
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
SET_ALIAS( clEnqueueReadBufferRect );
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
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
SET_ALIAS( clEnqueueWriteBufferRect );
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
    cl_int errCode              = CL_SUCCESS;
    CrtEvent* crtEvent          = NULL;
    SyncManager* synchHelper    = NULL;
    cl_event *outEvents         = NULL;
    cl_uint numOutEvents        = 0;

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

    CrtBuffer* crtSrcBuffer = reinterpret_cast<CrtBuffer*>(((_cl_mem_crt*)src_buffer)->object);
    CrtBuffer* crtDstBuffer = reinterpret_cast<CrtBuffer*>(((_cl_mem_crt*)dst_buffer)->object);
    if (!crtSrcBuffer || !crtDstBuffer)
    {
        errCode = CL_INVALID_MEM_OBJECT;
        goto FINISH;
    }

    if( ( crtSrcBuffer->m_pContext != queue->m_contextCRT ) ||
        ( crtDstBuffer->m_pContext != queue->m_contextCRT ) )
    {
        errCode = CL_INVALID_CONTEXT;
        goto FINISH;
    }

    synchHelper = new SyncManager;
    if (!synchHelper)
    {
        return CL_OUT_OF_HOST_MEMORY;
    }

    errCode = synchHelper->PrepareToExecute(
        queue,
        num_events_in_wait_list,
        event_wait_list,
        &numOutEvents,
        &outEvents);

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
SET_ALIAS( clEnqueueCopyBufferRect );
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
    cl_int errCode              = CL_SUCCESS;
    CrtEvent* crtEvent          = NULL;
    SyncManager* synchHelper    = NULL;
    cl_event *outEvents         = NULL;
    cl_uint numOutEvents        = 0;

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

    CrtBuffer* crtSrcBuffer = reinterpret_cast<CrtBuffer*>(((_cl_mem_crt*)src_buffer)->object);
    CrtBuffer* crtDstBuffer = reinterpret_cast<CrtBuffer*>(((_cl_mem_crt*)dst_buffer)->object);
    if( !crtSrcBuffer || !crtDstBuffer )
    {
        errCode = CL_INVALID_MEM_OBJECT;
        goto FINISH;
    }

    if( ( crtSrcBuffer->m_pContext != queue->m_contextCRT ) ||
        ( crtDstBuffer->m_pContext != queue->m_contextCRT ) )
    {
        errCode = CL_INVALID_CONTEXT;
        goto FINISH;
    }

    synchHelper = new SyncManager;
    if (!synchHelper)
    {
        return CL_OUT_OF_HOST_MEMORY;
    }

    errCode = synchHelper->PrepareToExecute(
        queue,
        num_events_in_wait_list,
        event_wait_list,
        &numOutEvents,
        &outEvents);

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
SET_ALIAS( clEnqueueCopyBuffer );
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
    cl_int errCode              = CL_SUCCESS;
    CrtEvent* crtEvent          = NULL;
    SyncManager* synchHelper    = NULL;
    void* ptr                   = NULL;
    cl_event *outEvents         = NULL;
    cl_uint numOutEvents        = 0;
    cl_mem devMemObj            = NULL;
    CrtQueue* queue             = NULL;
    CrtBuffer* crtBuffer        = NULL;

    errCode = ValidateMapFlags( map_flags );
    if( CL_SUCCESS != errCode )
    {
        goto FINISH;
    }
    if( command_queue == NULL )
    {
        errCode = CL_INVALID_COMMAND_QUEUE;
        goto FINISH;
    }
    if( buffer == NULL )
    {
        errCode = CL_INVALID_MEM_OBJECT;
        goto FINISH;
    }
    queue = reinterpret_cast<CrtQueue*>(((_cl_command_queue_crt*)command_queue)->object);
    if (!queue)
    {
        errCode = CL_INVALID_COMMAND_QUEUE;
        goto FINISH;
    }

    crtBuffer = reinterpret_cast<CrtBuffer*>(((_cl_mem_crt*)buffer)->object);
    if( !crtBuffer )
    {
        errCode = CL_INVALID_MEM_OBJECT;
        goto FINISH;
    }

    if( ( map_flags & CL_MAP_WRITE ) && ( crtBuffer->m_flags & CL_MEM_HOST_READ_ONLY ) )
    {
        errCode = CL_INVALID_MEM_OBJECT;
        goto FINISH;
    }

    if( ( map_flags & CL_MAP_WRITE ) && ( crtBuffer->m_flags & CL_MEM_HOST_NO_ACCESS ) )
    {
        errCode = CL_INVALID_MEM_OBJECT;
        goto FINISH;
    }

    if( ( map_flags & CL_MAP_READ ) && ( crtBuffer->m_flags & CL_MEM_HOST_WRITE_ONLY ) )
    {
        errCode = CL_INVALID_MEM_OBJECT;
        goto FINISH;
    }

    if( ( map_flags & CL_MAP_READ ) && ( crtBuffer->m_flags & CL_MEM_HOST_NO_ACCESS ) )
    {
        errCode = CL_INVALID_MEM_OBJECT;
        goto FINISH;
    }

    if( crtBuffer->m_pContext != queue->m_contextCRT )
    {
        errCode = CL_INVALID_CONTEXT;
        goto FINISH;
    }

    devMemObj = crtBuffer->getDeviceMemObj(queue->m_device);

    if( !crtBuffer->isInteropObject() )
    {
        errCode = crtBuffer->CheckParamsAndBounds( &offset, &cb );
        if( CL_SUCCESS != errCode )
        {
             goto FINISH;
        }

        if( crtBuffer->m_parentBuffer != NULL )
        {
            // if sub-bufer, we need to check buffer alignment

            if( crtBuffer->IsValidMemObjSize( devMemObj ) != CL_TRUE )
            {
                errCode = CL_MISALIGNED_SUB_BUFFER_OFFSET;
                goto FINISH;
            }
        }
    }

    synchHelper = new SyncManager;
    if (!synchHelper)
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        return NULL;
    }

    errCode = synchHelper->PrepareToExecute(
        queue,
        num_events_in_wait_list,
        event_wait_list,
        &numOutEvents,
        &outEvents);

    if (CL_SUCCESS != errCode)
    {
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

    crtEvent = new CrtEvent(queue);
    if( NULL == crtEvent )
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }

    if( !crtBuffer->isInteropObject() )
    {
        ptr = crtBuffer->GetMapPointer( &offset, &cb );

        crtBuffer->m_mappedPointers.push_back(ptr);
        crtBuffer->m_mapCount++;

        if( crtBuffer->HasPrivateCopy() && ( crtBuffer->m_mapCount <= 1 ) && ( ( map_flags & CL_MAP_WRITE_INVALIDATE_REGION ) == 0 ) )
        {
            // currently we copy all buffer contents and not only the mapped region.
            // copying the mapped region only will be an optimization for the future
            errCode = queue->m_cmdQueueDEV->dispatch->clEnqueueReadBuffer(
                queue->m_cmdQueueDEV,
                devMemObj,
                blocking_map,
                0,
                crtBuffer->m_size,
                crtBuffer->m_pUsrPtr,
                numOutEvents,
                outEvents,
                &crtEvent->m_eventDEV);
        }
        else
        {
            if( event )
            {
                errCode = synchHelper->EnqueueNopCommand(
                    crtBuffer,
                    queue,
                    numOutEvents,
                    outEvents,
                    &crtEvent->m_eventDEV);
            }

            if ( blocking_map )
            {
                errCode = queue->m_cmdQueueDEV->dispatch->clFinish(queue->m_cmdQueueDEV);
                if (CL_SUCCESS != errCode)
                {
                    errCode = CL_OUT_OF_RESOURCES;
                    goto FINISH;
                }
            }
        }
    }
    else
    {
        // Interop map
        queue->m_cmdQueueDEV->dispatch->clEnqueueMapBuffer(
                queue->m_cmdQueueDEV,
                devMemObj,
                blocking_map,
                map_flags,
                offset,
                cb,
                numOutEvents,
                outEvents,
                &crtEvent->m_eventDEV,
                &errCode);
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

    if( CL_SUCCESS != errCode )
    {
        if( crtEvent )
        {
            crtEvent->Release();
            crtEvent->DecPendencyCnt();
        }
        ptr = NULL;
    }
    if( synchHelper )
    {
        synchHelper->Release( errCode );
        delete synchHelper;
    }
    if( errcode_ret )
    {
        *errcode_ret = errCode;
    }

    return ptr;
}
SET_ALIAS( clEnqueueMapBuffer );
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
    cl_int errCode              = CL_SUCCESS;
    CrtEvent* crtEvent          = NULL;
    SyncManager* synchHelper    = NULL;
    void* ptr                   = NULL;
    cl_mem devMemObj            = NULL;
    cl_event *outEvents         = NULL;
    cl_uint numOutEvents        = 0;
    CrtQueue* queue             = NULL;
    CrtImage* crtImage          = NULL;

    errCode = ValidateMapFlags(map_flags);
    if( CL_SUCCESS != errCode )
    {
        goto FINISH;
    }
    if (command_queue == NULL)
    {
        errCode = CL_INVALID_COMMAND_QUEUE;
        goto FINISH;
    }
    if (image == NULL)
    {
        errCode = CL_INVALID_MEM_OBJECT;
        goto FINISH;
    }
    queue = reinterpret_cast<CrtQueue*>(((_cl_command_queue_crt*)command_queue)->object);
    if (!queue)
    {
        errCode = CL_INVALID_COMMAND_QUEUE;
        goto FINISH;
    }

    crtImage = reinterpret_cast<CrtImage*>(((_cl_mem_crt*)image)->object);
    if( !crtImage )
    {
        errCode = CL_INVALID_MEM_OBJECT;
        goto FINISH;
    }

    if( crtImage->m_pContext != queue->m_contextCRT )
    {
        errCode = CL_INVALID_CONTEXT;
        goto FINISH;
    }

    if( ( map_flags & CL_MAP_WRITE ) && ( crtImage->m_flags & CL_MEM_HOST_READ_ONLY ) )
    {
        errCode = CL_INVALID_MEM_OBJECT;
        goto FINISH;
    }

    if( ( map_flags & CL_MAP_WRITE ) && ( crtImage->m_flags & CL_MEM_HOST_NO_ACCESS ) )
    {
        errCode = CL_INVALID_MEM_OBJECT;
        goto FINISH;
    }

    if( ( map_flags & CL_MAP_READ ) && ( crtImage->m_flags & CL_MEM_HOST_WRITE_ONLY ) )
    {
        errCode = CL_INVALID_MEM_OBJECT;
        goto FINISH;
    }

    if( ( map_flags & CL_MAP_READ ) && ( crtImage->m_flags & CL_MEM_HOST_NO_ACCESS ) )
    {
        errCode = CL_INVALID_MEM_OBJECT;
        goto FINISH;
    }

    devMemObj = crtImage->getDeviceMemObj(queue->m_device);

    if( !crtImage->isInteropObject() )
    {
        if( crtImage->IsValidImageFormat( devMemObj ) != CL_TRUE )
        {
            errCode = CL_IMAGE_FORMAT_NOT_SUPPORTED;
            goto FINISH;
        }
        else if( crtImage->IsValidMemObjSize( devMemObj ) != CL_TRUE )
        {
            errCode = CL_INVALID_IMAGE_SIZE;
            goto FINISH;
        }

        errCode = crtImage->CheckParamsAndBounds(origin,region);
        if( CL_SUCCESS != errCode )
        {
            goto FINISH;
        }
    }

    synchHelper = new SyncManager;
    if (!synchHelper)
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        return NULL;
    }

    errCode = synchHelper->PrepareToExecute(
        queue,
        num_events_in_wait_list,
        event_wait_list,
        &numOutEvents,
        &outEvents);

    if (CL_SUCCESS != errCode)
    {
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

    crtEvent = new CrtEvent(queue);
    if( NULL == crtEvent )
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }

    if( !crtImage->isInteropObject() )
    {
        // The pitch for CRT Backing store may be different than that for the host_ptr
        // So we need to report the correct host_ptr pitch
        if( image_row_pitch )
        {
            *image_row_pitch = crtImage->m_hostPtrRowPitch;
        }
        if( image_slice_pitch )
        {
            if( (crtImage->m_imageDesc.desc.image_type == CL_MEM_OBJECT_IMAGE3D ) ||
                (crtImage->m_imageDesc.desc.image_type == CL_MEM_OBJECT_IMAGE1D_ARRAY ) ||
                (crtImage->m_imageDesc.desc.image_type == CL_MEM_OBJECT_IMAGE2D_ARRAY ) )
            {
                *image_slice_pitch = crtImage->m_hostPtrSlicePitch;
            }
            else
            {
                *image_slice_pitch = 0;
            }
        }
        ptr = crtImage->GetMapPointer(origin, region);

        crtImage->m_mapCount++;
        crtImage->m_mappedPointers.push_back(ptr);

        if( crtImage->HasPrivateCopy() && ( crtImage->m_mapCount <= 1 ) && ( ( map_flags & CL_MAP_WRITE_INVALIDATE_REGION ) == 0 ) )
        {
            const size_t origin[3] = {0};
            const size_t region[3] = { crtImage->m_imageDesc.desc.image_width, crtImage->m_imageDesc.desc.image_height, crtImage->m_imageDesc.desc.image_depth };
            errCode = queue->m_cmdQueueDEV->dispatch->clEnqueueReadImage(
                queue->m_cmdQueueDEV,
                devMemObj,
                blocking_map,
                origin,
                region,
                crtImage->m_hostPtrRowPitch,
                crtImage->m_hostPtrSlicePitch,
                crtImage->m_pUsrPtr,
                numOutEvents,
                outEvents,
                &crtEvent->m_eventDEV);
        }
        else
        {
            if( event )
            {
                errCode = synchHelper->EnqueueNopCommand(
                crtImage,
                queue,
                numOutEvents,
                outEvents,
                &crtEvent->m_eventDEV);
            }

            if ( blocking_map )
            {
                errCode = queue->m_cmdQueueDEV->dispatch->clFinish(queue->m_cmdQueueDEV);
                if (CL_SUCCESS != errCode)
                {
                    errCode = CL_OUT_OF_RESOURCES;
                    goto FINISH;
                }
            }
        }
    }
    else
    {
        // Interop Object
        queue->m_cmdQueueDEV->dispatch->clEnqueueMapImage(
            command_queue,
            image,
            blocking_map,
            map_flags,
            origin,
            region,
            image_row_pitch,
            image_slice_pitch,
            numOutEvents,
            outEvents,
            &crtEvent->m_eventDEV,
            &errCode);
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
    if( CL_SUCCESS != errCode )
    {
        if( crtEvent )
        {
            crtEvent->Release();
            crtEvent->DecPendencyCnt();
        }
        ptr = NULL;
    }
    if( synchHelper )
    {
        synchHelper->Release( errCode );
        delete synchHelper;
    }
    if( errcode_ret )
    {
        *errcode_ret = errCode;
    }

    return ptr;
}
SET_ALIAS( clEnqueueMapImage );
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
    MEMMAP_PTR_VEC::iterator    itr;
    cl_int errCode              = CL_SUCCESS;
    CrtEvent *crtEvent          = NULL;
    SyncManager *synchHelper    = NULL;
    CrtMemObject *crtMemObj     = NULL;
    CrtQueue* queue             = NULL;
    cl_event *outEvents         = NULL;
    cl_uint numOutEvents        = 0;

    if( command_queue == NULL )
    {
        errCode = CL_INVALID_COMMAND_QUEUE;
        goto FINISH;
    }
    if( memobj == NULL )
    {
        errCode = CL_INVALID_MEM_OBJECT;
        goto FINISH;
    }

    queue = reinterpret_cast<CrtQueue*>( ( ( _cl_command_queue_crt* )command_queue )->object );
    if (!queue)
    {
        errCode = CL_INVALID_COMMAND_QUEUE;
        goto FINISH;
    }

    crtMemObj = reinterpret_cast<CrtMemObject*>(((_cl_mem_crt*)memobj)->object);
    if( !crtMemObj )
    {
        errCode = CL_INVALID_MEM_OBJECT;
        goto FINISH;
    }

    if( crtMemObj->m_pContext != queue->m_contextCRT )
    {
        errCode = CL_INVALID_CONTEXT;
        goto FINISH;
    }

    synchHelper = new SyncManager;
    if( !synchHelper )
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        return NULL;
    }

    errCode = synchHelper->PrepareToExecute(
        queue,
        num_events_in_wait_list,
        event_wait_list,
        &numOutEvents,
        &outEvents);

    if (CL_SUCCESS != errCode)
    {
        goto FINISH;
    }

    crtEvent = new CrtEvent(queue);
    if( NULL == crtEvent )
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }

    // Book keeping only for Non-Interop memory objects
    if( !crtMemObj->isInteropObject() )
    {
        itr = std::find( crtMemObj->m_mappedPointers.begin(), crtMemObj->m_mappedPointers.end(), mapped_ptr );
        if( itr == crtMemObj->m_mappedPointers.end() )
        {
            errCode = CL_INVALID_VALUE;
            goto FINISH;
        }

        crtMemObj->m_mappedPointers.erase(itr);
        crtMemObj->m_mapCount--;

        // Theoretically we should sync the surface before the UnMap command
        // however, since we already know that UnMap on the devices will do
        // nothing besides reference counting the map/unmap operations...
        // we choose to follow the Map sequence implementation for simplicity.
        if (crtMemObj->HasPrivateCopy())
        {
            if( crtMemObj->getObjectType() == CrtObject::CL_IMAGE )
            {
                CrtImage* crtImage = (CrtImage*)crtMemObj;

                // currently we copy all buffer contents and not only the mapped region.
                // copying the mapped region only will be an optimization for the future
                const size_t origin[3] = {0};
                const size_t region[3] = { crtImage->m_imageDesc.desc.image_width, crtImage->m_imageDesc.desc.image_height, crtImage->m_imageDesc.desc.image_depth };
                errCode = queue->m_cmdQueueDEV->dispatch->clEnqueueWriteImage(
                    queue->m_cmdQueueDEV,
                    crtMemObj->getDeviceMemObj(queue->m_device),
                    false,
                    origin,
                    region,
                    crtImage->m_hostPtrRowPitch,
                    crtImage->m_hostPtrSlicePitch,
                    crtImage->m_pUsrPtr,
                    numOutEvents,
                    outEvents,
                    &crtEvent->m_eventDEV);
            }
            else
            {
                // currently we copy all buffer contents and not only the mapped region.
                // copying the mapped region only will be an optimization for the future
                CrtBuffer* crtBuffer = (CrtBuffer*)crtMemObj;
                errCode = queue->m_cmdQueueDEV->dispatch->clEnqueueWriteBuffer(
                    queue->m_cmdQueueDEV,
                    crtMemObj->getDeviceMemObj(queue->m_device),
                    false,
                    0,
                    crtBuffer->m_size,
                    crtBuffer->m_pUsrPtr,
                    numOutEvents,
                    outEvents,
                    &crtEvent->m_eventDEV);
            }
        }
        else if( event )
        {
            errCode = synchHelper->EnqueueNopCommand(
                crtMemObj,
                queue,
                numOutEvents,
                outEvents,
                &crtEvent->m_eventDEV);
        }
    }
    else
    {
        errCode = queue->m_cmdQueueDEV->dispatch->clEnqueueUnmapMemObject(
            queue->m_cmdQueueDEV,
            crtMemObj->getDeviceMemObj(queue->m_device),
            mapped_ptr,
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
    else
    {
        crtEvent->Release();
        crtEvent->DecPendencyCnt();
    }

FINISH:

    if( CL_SUCCESS != errCode )
    {
        if( crtEvent )
        {
            crtEvent->Release();
            crtEvent->DecPendencyCnt();
        }
    }
    if( synchHelper )
    {
        synchHelper->Release( errCode );
        delete synchHelper;
    }

    return errCode;
}
SET_ALIAS( clEnqueueUnmapMemObject );
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
SET_ALIAS( clFlush );
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clFinish(cl_command_queue command_queue)
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

    return queue->m_cmdQueueDEV->dispatch->clFinish(queue->m_cmdQueueDEV);
}
SET_ALIAS( clFinish );
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clRetainContext(cl_context context)
{
    CrtContextInfo* ctxInfo = OCLCRT::crt_ocl_module.m_contextInfoGuard.GetValue(context);
    if( !ctxInfo )
    {
        return CL_INVALID_CONTEXT;
    }
    if (ctxInfo->m_contextType != CrtContextInfo::SharedPlatformContext)
    {
        // Single Platform Context
        KHRicdVendorDispatch* dTable = (KHRicdVendorDispatch*)(ctxInfo->m_object);
        return dTable->clRetainContext(context);
    }
    else
    {
        // Shared Platform Context
        CrtContext* ctx = (CrtContext*)(ctxInfo->m_object);
        ctx->IncRefCnt();
    }
    return CL_SUCCESS;
}
SET_ALIAS( clRetainContext );
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clReleaseContext(cl_context context)
{
    cl_int errCode = CL_SUCCESS;

    CrtContextInfo* ctxInfo = OCLCRT::crt_ocl_module.m_contextInfoGuard.GetValue(context);
    if( !ctxInfo )
    {
        return CL_INVALID_CONTEXT;
    }

    long refCount = 0;
    // Single Platform Context
    if (ctxInfo->m_contextType != CrtContextInfo::SharedPlatformContext)
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
SET_ALIAS( clReleaseContext );
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
SET_ALIAS( clRetainCommandQueue );
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
SET_ALIAS( clReleaseCommandQueue );
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
SET_ALIAS( clReleaseMemObject );
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clRetainMemObject(cl_mem memobj)
{
    CrtMemObject* crtMemObj = reinterpret_cast<CrtMemObject*>(((_cl_mem_crt*)memobj)->object);

    crtMemObj->IncRefCnt();
    // I don't forward retains to the underlying platforms, i only send release when
    // the CRT ref counter for this mem objects reaches zero
    return CL_SUCCESS;
}
SET_ALIAS( clRetainMemObject );
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
            pgm->Release();
            pgm->DecPendencyCnt();
            errCode = CL_OUT_OF_HOST_MEMORY;
        }
        else
        {
            pgm->m_program_handle = ( cl_program ) SharedProgram;
            SharedProgram->object = (void *) pgm;
        }
    }
    if( errcode_ret )
    {
        *errcode_ret = errCode;
    }
    return SharedProgram;
}
SET_ALIAS( clCreateProgramWithSource );
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
    cl_int errCode                  = CL_SUCCESS;
    _cl_program_crt *SharedProgram  = NULL;
    CrtProgram *pgm                 = NULL;
    CrtContext *ctx                 = NULL;

    if (NULL == device_list || 0 == num_devices || NULL == lengths || NULL == binaries)
    {
        errCode = CL_INVALID_VALUE;
        goto FINISH;
    }

    pgm = NULL;
    ctx = ((CrtContext*)((_cl_context_crt*)context)->object);

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
            pgm->Release();
            pgm->DecPendencyCnt();
            errCode = CL_OUT_OF_HOST_MEMORY;
        }
        else
        {
            pgm->m_program_handle = ( cl_program ) SharedProgram;
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
SET_ALIAS( clCreateProgramWithBinary );
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
CL_API_ENTRY cl_program CL_API_CALL clCreateProgramWithBuiltInKernels(
    cl_context             context,
    cl_uint                num_devices,
    const cl_device_id *   device_list,
    const char *           kernel_names,
    cl_int *               errcode_ret )
{
    cl_int errCode = CL_SUCCESS;
    _cl_program_crt* SharedProgram = NULL;

    if( OCLCRT::crt_ocl_module.m_CrtPlatformVersion < OPENCL_1_2 )
    {
        errCode = CL_INVALID_DEVICE;
    }
    else
    {
        if( NULL == device_list || 0 == num_devices || NULL == kernel_names )
        {
            errCode = CL_INVALID_VALUE;
            goto FINISH;
        }

        CrtProgram* pgm = NULL;
        CrtContext* ctx = ( (CrtContext*)( (_cl_context_crt*)context )->object );

        errCode = ctx->CreateProgramWithBuiltInKernels(
            num_devices,
            device_list,
            kernel_names,
            &pgm);

        if( CL_SUCCESS == errCode )
        {
            SharedProgram = new _cl_program_crt;
            if( NULL == SharedProgram )
            {
                errCode = CL_OUT_OF_HOST_MEMORY;
            }
            else
            {
                SharedProgram->object = (void *) pgm;
            }
        }
    }

FINISH:
    if( CL_SUCCESS != errcode_ret )
    {
        *errcode_ret = errCode;
    }
    return SharedProgram;
}
SET_ALIAS( clCreateProgramWithBuiltInKernels );
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_bool isRequiredObjAvailable(
    cl_device_id                devId,
    std::vector<CrtProgram*>&   in_programs,
    cl_uint                     bin_types)
{
    cl_program_binary_type pgm_bin_type;

    if( bin_types == CL_PROGRAM_BINARY_TYPE_NONE )
    {
        return true;
    }

    for( cl_uint i=0; i < in_programs.size(); i++ )
    {
        cl_context ctx = in_programs[i]->m_contextCRT->m_DeviceToContext[ devId ];
        cl_program pgm = in_programs[i]->m_ContextToProgram[ ctx ];

        cl_int errCode = devId->dispatch->clGetProgramBuildInfo(
                            pgm,
                            devId,
                            CL_PROGRAM_BINARY_TYPE,
                            sizeof( cl_program_binary_type ),
                            &pgm_bin_type, NULL);

        if( !( CL_SUCCESS == errCode ) || !( bin_types & pgm_bin_type ) )
        {
            return false;
        }
    }
    return true;
}
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int getAssocDevices(
    const cl_device_id*             inDevList,
    cl_device_id**                  outDevList,
    std::vector<CrtProgram*>&       in_programs,
    CrtContext*                     crtCtx,
    cl_uint                         bin_types,
    cl_uint*                        outNumDevs)
{
    cl_int errCode = CL_SUCCESS;
    cl_uint j = 0;

    if( NULL != inDevList )
    {
        // devices have been provided by the user
        for( cl_uint i=0; i < *outNumDevs; i++ )
        {
            if( NULL == inDevList[ i ] )
            {
                errCode = CL_INVALID_DEVICE;
                goto FINISH;
            }
            // If binary exists with the required binary type
            if( isRequiredObjAvailable(
                    inDevList[ i ] ,
                    in_programs,
                    bin_types) )
            {
                ( *outDevList )[ j++ ] = inDevList[ i ];
            }
        }
        if( j == 0 )
        {
            errCode = CL_INVALID_PROGRAM;
        }
        ( *outNumDevs ) = j;
    }
    else
    {
        // devices have NOT been provided by the user
        // consider the context devices instead
        DEV_CTX_MAP::iterator itr = crtCtx->m_DeviceToContext.begin();
        for (;itr != crtCtx->m_DeviceToContext.end(); itr++)
        {
            if( isRequiredObjAvailable(
                    itr->first,
                    in_programs,
                    bin_types) )
            {
                ( *outDevList )[ j++ ] = itr->first;
            }
        }
        if( j == 0 )
        {
            errCode = CL_INVALID_PROGRAM;
        }
        ( *outNumDevs ) = j;
    }
FINISH:
    return errCode;
}
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_uint getNumRelevantContexts( cl_uint num_devices, const cl_device_id* device_list, CrtContext* crtCtx )
{
    // Perform build on specific list of devices
    cl_uint numRelevantContexts = 0;
    for( cl_uint i = 0; i < OCLCRT::crt_ocl_module.m_oclPlatforms.size(); i++ )
    {
        cl_uint matchDevices = 0;
        crtCtx->GetDevicesByPlatformId(
            num_devices,
            device_list,
            OCLCRT::crt_ocl_module.m_oclPlatforms[i]->m_platformIdDEV,
            &matchDevices,
            NULL);

        if( matchDevices > 0 )
        {
            numRelevantContexts += 1;
        }
    }
    return numRelevantContexts;
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
    std::vector<CrtProgram*>        in_programs;
    std::string                     optReflect;
    cl_int errCode                  = CL_SUCCESS;
    cl_device_id* deviceList        = NULL;
    cl_uint deviceListSize          = 0;
    CrtBuildCallBackData* crtData   = NULL;
    cl_device_id* outDevices        = NULL;

    CrtProgram* crtProg = reinterpret_cast<CrtProgram*>(((_cl_program_crt*)program)->object);

    // Add kernel reflection to build options
    if( options )
    {
        crtProg->m_options.assign( options );
    }
    else
    {
        crtProg->m_options.assign( "" );
    }
    optReflect.assign( crtProg ->m_options );
    if( optReflect.find("-cl-kernel-arg-info") == std::string::npos )
    {
        optReflect.append(" -cl-kernel-arg-info");
    }

    if( ( ( num_devices > 0 ) && ( device_list == NULL ) ) ||
        ( ( num_devices == 0 ) && ( device_list != NULL ) ) )
    {
        errCode = CL_INVALID_VALUE;
        goto FINISH;
    }

    crtData = new CrtBuildCallBackData( program, pfn_notify, user_data );
    if (NULL == crtData)
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }

    in_programs.push_back( crtProg );

    if( NULL == device_list )
    {
        deviceListSize = crtProg->m_contextCRT->m_DeviceToContext.size();
    }
    else
    {
        deviceListSize = num_devices;
    }
    deviceList = new cl_device_id[ deviceListSize ];
    if( NULL == deviceList )
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }

    errCode = getAssocDevices( device_list, &deviceList, in_programs, crtProg->m_contextCRT,
        CL_PROGRAM_BINARY_TYPE_NONE,
        &num_devices);

    if( CL_SUCCESS != errCode )
    {
        goto FINISH;
    }

    crtData->m_numBuild = getNumRelevantContexts( num_devices, deviceList, crtProg->m_contextCRT );

    outDevices = new cl_device_id[ num_devices ];
    if( NULL == outDevices )
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }

    for( cl_uint i = 0; i < OCLCRT::crt_ocl_module.m_oclPlatforms.size(); i++ )
    {
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
        // mark the underlying context as context a build has been requested for
        crtProg->m_buildContexts.push_back(ctx);
        cl_program prog = crtProg->m_ContextToProgram[ctx];

        errCode = prog->dispatch->clBuildProgram(
            prog,
            matchDevices,
            outDevices,
            optReflect.c_str(),
            buildCompleteFn,
            crtData);

        // if ( CL_BUILD_PROGRAM_FAILURE != errCode ) is True; it means that
        // This is the first call to the underlying platforms, since the previous
        // platform should have caught this error
        if( ( CL_SUCCESS != errCode ) && ( CL_BUILD_PROGRAM_FAILURE != errCode ) )
        {
            goto FINISH;
        }
    }

    if (!pfn_notify)
    {
        crtData->m_lock.Wait();
        delete crtData;
        crtData = NULL;

        // According to the spec we must return the build status on returning
        // from a blocking build request
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
    if( CL_SUCCESS != errCode )
    {
        if( crtData )
        {
            delete crtData;
            crtData = NULL;
        }
    }
    if( deviceList )
    {
        delete[] deviceList;
        deviceList = NULL;
    }
    if( outDevices )
    {
        delete[] outDevices;
        outDevices = NULL;
    }
    return errCode;
}
SET_ALIAS( clBuildProgram );
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
CL_API_ENTRY cl_program CL_API_CALL clLinkProgram(
    cl_context            context,
    cl_uint               num_devices,
    const cl_device_id *  device_list,
    const char *          options,
    cl_uint               num_input_programs,
    const cl_program *    input_programs,
    prog_logging_fn       pfn_notify,
    void *                user_data,
    cl_int *              errcode_ret)
{
    std::vector<CrtProgram*>            in_programs;
    SHARED_CTX_DISPATCH::iterator       itr;
    cl_int errCode                      = CL_SUCCESS;
    cl_device_id* deviceList            = NULL;
    cl_uint deviceListSize              = 0;
    CrtBuildCallBackData* crtData       = NULL;
    _cl_program_crt* program            = NULL;
    CrtProgram* crtProg                 = NULL;
    cl_device_id* outDevices            = NULL;
    cl_program* devPrograms             = NULL;
    CrtContext* crtCtx                  = NULL;

    if( OCLCRT::crt_ocl_module.m_CrtPlatformVersion < OPENCL_1_2 )
    {
        errCode = CL_INVALID_DEVICE;
        goto FINISH;
    }

    crtCtx = ((CrtContext*)((_cl_context_crt*)context)->object);

    if( ( ( num_devices > 0 ) && ( device_list == NULL ) ) ||
        ( ( num_devices == 0 ) && ( device_list != NULL ) ) )
    {
        errCode = CL_INVALID_VALUE;
        goto FINISH;
    }

    if( ( 0 == num_input_programs ) || ( NULL == input_programs ) )
    {
        errCode = CL_INVALID_VALUE;
        goto FINISH;

    }
    // Create program object to be returned
    crtProg = new CrtProgram(crtCtx);
    if( !crtProg )
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }

    program = new _cl_program_crt;
    if( program == NULL )
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }
    crtProg->m_program_handle = ( cl_program ) program;
    program->object = (void *) crtProg;

    crtData = new CrtBuildCallBackData( program, pfn_notify, user_data );
    if (NULL == crtData)
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }
    // Validate in_programs parameter
    for(cl_uint i=0; i < num_input_programs; i++ )
    {
        if( NULL == input_programs[i] )
        {
            errCode = CL_INVALID_PROGRAM;
            goto FINISH;
        }
        in_programs.push_back( ((CrtProgram*)((_cl_program_crt*)input_programs[i])->object) );
    }

    if( NULL == device_list )
    {
        deviceListSize = crtCtx->m_DeviceToContext.size();
    }
    else
    {
        deviceListSize = num_devices;
    }
    deviceList = new cl_device_id[ deviceListSize ];
    if( NULL == deviceList )
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }

    // Retrieves the list of devices with a compiled object
    errCode = getAssocDevices( device_list, &deviceList, in_programs, crtCtx,
        CL_PROGRAM_BINARY_TYPE_COMPILED_OBJECT | CL_PROGRAM_BINARY_TYPE_LIBRARY,
        &num_devices);

    if( CL_SUCCESS != errCode )
    {
        goto FINISH;
    }

    for( cl_uint i = 0; i < num_devices; i++ )
    {
        crtProg->m_assocDevices.push_back( deviceList[i] );
    }

    crtData->m_numBuild = getNumRelevantContexts( num_devices, deviceList, crtCtx );

    outDevices  = new cl_device_id[ num_devices ];
    if( NULL == outDevices )
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }
    devPrograms = new cl_program[ num_input_programs ];
    if( NULL == devPrograms )
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }

    // Iterate over the contexts
    itr = crtCtx->m_contexts.begin();
    for( ;itr != crtCtx->m_contexts.end(); itr++ )
    {
        cl_platform_id pId;
        itr->first->dispatch->clGetPlatformIDs(1, &pId, NULL );

        cl_uint matchDevices = 0;
        crtCtx->GetDevicesByPlatformId(
            num_devices,
            deviceList,
            pId,
            &matchDevices,
            outDevices);

        if (matchDevices == 0)
        {
            continue;
        }

        int p = 0;
        for( std::vector<CrtProgram*>::iterator pItr = in_programs.begin(); pItr != in_programs.end(); pItr++ )
        {
            devPrograms[ p++ ] = (*pItr)->m_ContextToProgram[ itr->first ];
        }

        cl_program devPro = itr->second.clLinkProgram(
            itr->first,
            matchDevices,
            outDevices,
            options,
            num_input_programs,
            devPrograms,
            buildCompleteFn,
            crtData,
            &errCode);

        if( devPro )
        {
            crtProg->m_ContextToProgram[ itr->first ] = devPro;
            crtProg->m_buildContexts.push_back( itr->first );
        }

        // if ( CL_COMPILE_PROGRAM_FAILURE != errCode ) is True; it means that
        // This is the first call to the underlying platforms, since the previous
        // platform should have caught this error
        if( (CL_SUCCESS != errCode) && ( CL_LINK_PROGRAM_FAILURE != errCode ) )
        {
            goto FINISH;
        }
    }

    if (!pfn_notify)
    {
        crtData->m_lock.Wait();
        delete crtData;
        crtData = NULL;

        // According to the spec we must return the build status on returning
        // from a blocking build request
        for (cl_uint i=0; i < num_devices; i++)
        {
            cl_build_status build_status;
            cl_context ctx = crtCtx->m_DeviceToContext[deviceList[i]];

            deviceList[i]->dispatch->clGetProgramBuildInfo(
                crtProg->m_ContextToProgram[ctx],
                deviceList[i],
                CL_PROGRAM_BUILD_STATUS,
                sizeof(build_status),
                &build_status,
                NULL);

            if (build_status != CL_BUILD_SUCCESS)
            {
                errCode = CL_LINK_PROGRAM_FAILURE;
                goto FINISH;
            }
        }
    }
FINISH:
    if( CL_SUCCESS != errCode )
    {
        if( NULL != crtData )
        {
            delete crtData;
            crtData = NULL;
        }
        if( NULL != crtProg )
        {
            crtProg->Release();
            crtProg->DecPendencyCnt();
        }
        if( NULL != program )
        {
            delete program;
            program = NULL;
        }
    }

    if( deviceList )
    {
        delete[] deviceList;
        deviceList = NULL;
    }
    if( outDevices )
    {
        delete[] outDevices;
        outDevices = NULL;
    }
    if( devPrograms )
    {
        delete[] devPrograms;
        devPrograms = NULL;
    }

    if( errcode_ret )
    {
        *errcode_ret = errCode;
    }
    return program;
}
SET_ALIAS( clLinkProgram );
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
CL_API_ENTRY cl_int CL_API_CALL clCompileProgram(
    cl_program            program,
    cl_uint               num_devices,
    const cl_device_id *  device_list,
    const char *          options,
    cl_uint               num_input_headers,
    const cl_program *    input_headers,
    const char **         header_include_names,
    prog_logging_fn       pfn_notify,
    void *                user_data)
{
    std::vector<CrtProgram*>        in_programs;
    std::string                     optReflect;
    cl_int errCode                  = CL_SUCCESS;
    CrtBuildCallBackData* crtData   = NULL;
    cl_device_id* deviceList        = NULL;
    cl_uint deviceListSize          = 0;
    cl_device_id* outDevices        = NULL;
    CrtProgram* crtProg             = NULL;

    if( OCLCRT::crt_ocl_module.m_CrtPlatformVersion < OPENCL_1_2 )
    {
        errCode = CL_INVALID_DEVICE;
        goto FINISH;
    }

    crtProg = reinterpret_cast<CrtProgram*>(((_cl_program_crt*)program)->object);

    // Add kernel reflection to build options
    if( options )
    {
        crtProg->m_options.assign( options );
    }
    optReflect.assign( crtProg ->m_options );
    if( optReflect.find("-cl-kernel-arg-info") == std::string::npos )
    {
        optReflect.append(" -cl-kernel-arg-info");
    }

    if( ( ( num_devices > 0 ) && ( device_list == NULL ) ) ||
        ( ( num_devices == 0 ) && ( device_list != NULL ) ) )
    {
        errCode = CL_INVALID_VALUE;
        goto FINISH;
    }

    crtData = new CrtBuildCallBackData( program, pfn_notify, user_data );
    if (NULL == crtData)
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }
    in_programs.push_back( crtProg );


    if( NULL == device_list )
    {
        deviceListSize = crtProg->m_contextCRT->m_DeviceToContext.size();
    }
    else
    {
        deviceListSize = num_devices;
    }
    deviceList = new cl_device_id[ deviceListSize ];
    if( NULL == deviceList )
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }

    errCode = getAssocDevices( device_list, &deviceList, in_programs, crtProg->m_contextCRT,
        CL_PROGRAM_BINARY_TYPE_NONE,
        &num_devices);

    if( CL_SUCCESS != errCode )
    {
        goto FINISH;
    }

    crtData->m_numBuild = getNumRelevantContexts( num_devices, deviceList, crtProg->m_contextCRT );

    outDevices = new cl_device_id[ num_devices ];
    if( NULL == outDevices )
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }

    for( cl_uint i = 0; i < OCLCRT::crt_ocl_module.m_oclPlatforms.size(); i++ )
    {
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
        // mark the underlying context as context a build has been requested for
        crtProg->m_buildContexts.push_back(ctx);
        cl_program prog = crtProg->m_ContextToProgram[ctx];

        errCode = prog->dispatch->clCompileProgram(
            prog,
            matchDevices,
            outDevices,
            optReflect.c_str(),
            num_input_headers,
            input_headers,
            header_include_names,
            *buildCompleteFn,
            crtData);

        // if ( CL_COMPILE_PROGRAM_FAILURE != errCode ) is True; it means that
        // This is the first call to the underlying platforms, since the previous
        // platform should have caught this error
        if( ( CL_SUCCESS != errCode ) && ( CL_COMPILE_PROGRAM_FAILURE != errCode ) )
        {
            goto FINISH;
        }
    }

    if (!pfn_notify)
    {
        crtData->m_lock.Wait();
        delete crtData;
        crtData = NULL;

        // According to the spec we must return the build status on returning
        // from a blocking build request
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
                errCode = CL_COMPILE_PROGRAM_FAILURE;
                goto FINISH;
            }
        }
    }
FINISH:
    if( CL_SUCCESS != errCode )
    {
        if( crtData )
        {
            delete crtData;
            crtData = NULL;
        }
    }

    if( deviceList )
    {
        delete[] deviceList;
        deviceList = NULL;
    }
    if( outDevices )
    {
        delete[] outDevices;
        outDevices = NULL;
    }
    return errCode;
}
SET_ALIAS( clCompileProgram );
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
CL_API_ENTRY cl_event CL_API_CALL clCreateEventFromGLsyncKHR(
    cl_context      context,
    cl_GLsync        sync,
    cl_int *         errcode_ret )
{
    cl_int errCode = CL_SUCCESS;

    if( errcode_ret )
    {
        *errcode_ret = errCode;
    }
    return NULL;
}
SET_ALIAS( clCreateEventFromGLsyncKHR );
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
CL_API_ENTRY cl_int CL_API_CALL clUnloadPlatformCompiler(
    cl_platform_id platform )
{
    if( OCLCRT::crt_ocl_module.m_CrtPlatformVersion < OPENCL_1_2 )
    {
        return CL_INVALID_DEVICE;
    }
    if( ( platform != NULL) && !( isValidPlatform( platform ) ) )
    {
        return CL_INVALID_PLATFORM;
    }
    return CL_SUCCESS;
}
SET_ALIAS( clUnloadPlatformCompiler );
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

    if( param_name == CL_PROGRAM_BUILD_OPTIONS )
    {
        size_t pValueSize = ( pgm->m_options.size() == 0 ) ? 0 : ( pgm->m_options.size() + 1 );
        if( param_value != NULL )
        {
            if( param_value_size < pValueSize )
            {
                errCode = CL_INVALID_VALUE;
                goto FINISH;
            }
            if( pValueSize > 0 )
            {
                strcpy_s( ( char* )param_value, pgm->m_options.size()+1, pgm->m_options.c_str() );
            }
        }
        if( param_value_size_ret )
        {
            *param_value_size_ret = pValueSize;
        }
    }
    else
    {
        CrtContext* ctx = pgm->m_contextCRT;

        CTX_PGM_MAP::iterator it = pgm->m_ContextToProgram.find( ctx->GetContextByDeviceID( device ) );

        if( it != pgm->m_ContextToProgram.end() )
        {
            cl_program currPgm = it->second;
            errCode = currPgm->dispatch->clGetProgramBuildInfo( currPgm,
                                                            device,
                                                            param_name,
                                                            param_value_size,
                                                            param_value,
                                                            param_value_size_ret );
            goto FINISH;
        }

        // else, device is not associated with program
        errCode = CL_INVALID_DEVICE;
    }

FINISH:
    return errCode;
}
SET_ALIAS( clGetProgramBuildInfo );
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
SET_ALIAS( clRetainProgram );
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
SET_ALIAS( clReleaseProgram );
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_kernel CL_API_CALL clCreateKernel( cl_program       program ,
                                      const char      *kernel_name ,
                                      cl_int          *errcode_ret )
{
    cl_int errCode                  = CL_SUCCESS;
    _cl_kernel_crt *SharedKernel    = NULL;
    cl_uint refNumKernelArgs        = 0;
    CrtProgram *pgm                 = reinterpret_cast<CrtProgram*>(((_cl_program_crt*)program)->object);
    CrtContext* ctx                 = NULL;

    CrtKernel *crtKernel = new CrtKernel(pgm);
    if(NULL == crtKernel )
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }

    ctx = pgm->m_contextCRT;

    for( cl_uint i=0; i < pgm->m_buildContexts.size(); i++)
    {
        cl_context ctxObj = pgm->m_buildContexts[i];
        cl_kernel knlObj = ctxObj->dispatch->clCreateKernel( pgm->m_ContextToProgram[ctxObj],
                                                  kernel_name,
                                                  &errCode );
        if (CL_SUCCESS == errCode)
        {
            crtKernel->m_ContextToKernel[ctxObj] = knlObj;

            // As for now, we only support #difference number of arguments detection for the sake of
            // supporting CL_INVALID_KERNEL_DEFINITION
            if( i == 0 )
            {
                knlObj->dispatch->clGetKernelInfo(knlObj, CL_KERNEL_NUM_ARGS, sizeof( cl_uint ), &refNumKernelArgs, NULL );
            }
            else
            {
                cl_uint numKernelArgs = 0;
                knlObj->dispatch->clGetKernelInfo(knlObj, CL_KERNEL_NUM_ARGS, sizeof( cl_uint ), &numKernelArgs, NULL );
                if( refNumKernelArgs != numKernelArgs )
                {
                    errCode = CL_INVALID_KERNEL_DEFINITION;
                    goto FINISH;
                }
            }
        }
    }

    if( crtKernel->m_ContextToKernel.size() == 0 )
    {
        errCode = CL_INVALID_PROGRAM_EXECUTABLE;
        goto FINISH;
    }
    else
    {
        // At least there is one successful executable on any of the devices
        errCode = CL_SUCCESS;
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
SET_ALIAS( clCreateKernel );
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

    size_t  pValueSize = 0;
    switch(param_name)
    {
        case CL_KERNEL_REFERENCE_COUNT:
            {
                pValueSize = sizeof(crtKernel->m_refCount);
                if (param_value && param_value_size >= pValueSize)
                {
                    *((cl_uint*)param_value) = crtKernel->m_refCount;
                }

                if( param_value_size_ret )
                {
                    *param_value_size_ret = pValueSize;
                }
            }
            break;
        case CL_KERNEL_CONTEXT:
            {
                pValueSize = sizeof(cl_context);
                if (param_value && param_value_size >= pValueSize)
                {
                    *((cl_context*)param_value) = crtKernel->m_programCRT->m_contextCRT->m_context_handle;
                }

                if( param_value_size_ret )
                {
                    *param_value_size_ret = pValueSize;
                }
            }
            break;
        case CL_KERNEL_PROGRAM:
            {
                pValueSize = sizeof(cl_program);
                if (param_value && param_value_size >= pValueSize)
                {
                    *((cl_program*)param_value) = crtKernel->m_programCRT->m_program_handle;
                }

                if( param_value_size_ret )
                {
                    *param_value_size_ret = pValueSize;
                }
            }
            break;
        default:
            {
                // Pick any of the underlying created kernel objects
                cl_kernel kernelObj = crtKernel->m_ContextToKernel.begin()->second;
                // Forward call
                retCode = kernelObj->dispatch->clGetKernelInfo(
                                    kernelObj,
                                    param_name,
                                    param_value_size,
                                    param_value,
                                    param_value_size_ret);
            }
            break;
    }
    return retCode;
}
SET_ALIAS( clGetKernelInfo );
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clSetKernelArg(
    cl_kernel    kernel,
    cl_uint      arg_index,
    size_t       arg_size,
    const void * arg_value)
{
    cl_int errCode              = CL_SUCCESS;
    bool isImage                = false;
    bool isBuffer               = false;
    bool isSampler              = false;
    CrtSampler* crtSamplerObj   = NULL;
    CrtMemObject* crtMemObj     = NULL;
    bool succeed                = false;
    CTX_KRN_MAP::iterator       itr;
    std::string paramT;

    CrtKernel* crtKernel = reinterpret_cast<CrtKernel*>(((_cl_kernel_crt*)kernel)->object);
    cl_kernel kernelDevObj = crtKernel->m_ContextToKernel.begin()->second;

    paramT.resize( MAX_STRLEN );

    char* paramType = new char[MAX_STRLEN];
    if( paramType == NULL )
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }
    errCode = ( (SOCLEntryPointsTable*)kernelDevObj )->crtDispatch->clGetKernelArgInfo(
        kernelDevObj,
        arg_index,
        CL_KERNEL_ARG_TYPE_NAME,
        MAX_STRLEN,
        paramType,
        NULL);

    if( CL_SUCCESS != errCode )
    {
        if( ( CL_KERNEL_ARG_INFO_NOT_AVAILABLE == errCode ) ||
            ( CL_INVALID_VALUE == errCode ) )
        {
            errCode = CL_OUT_OF_RESOURCES;
        }
        goto FINISH;
    }

    paramT.assign(paramType);

    // conformance 1.2 require the image type be without prefix; as "image"; but we still support 1.1 too; so we have both forms here
    isImage    = ( !(paramT.compare(0, strlen("image"),"image")) || !(paramT.compare(0, strlen("__image"),"__image"))  );
    isBuffer   = ( paramT[paramT.size()-1] == '*' );
    isSampler  = !(paramT.compare(0, strlen("sampler"),"sampler"));

    if( isImage )
    {
        if( NULL == arg_value )
        {
            errCode = CL_INVALID_ARG_VALUE;
            goto FINISH;
        }
        _cl_mem_crt* ClImage = *((_cl_mem_crt**)(arg_value));
        if (NULL == ClImage)
        {
            errCode = CL_INVALID_ARG_VALUE;
            goto FINISH;
        }
        crtMemObj = reinterpret_cast<CrtMemObject*>(ClImage->object);
        if (NULL == crtMemObj)
        {
            errCode = CL_INVALID_MEM_OBJECT;
            goto FINISH;
        }
    }
    else if( ( isBuffer ) &&
             ( arg_value != NULL ) )
    {
        _cl_mem_crt* ClBuffer = *((_cl_mem_crt**)(arg_value));
        if( NULL != ClBuffer )
        {
            crtMemObj = reinterpret_cast<CrtMemObject*>(ClBuffer->object);
            if( NULL == crtMemObj )
            {
                errCode = CL_INVALID_MEM_OBJECT;
                goto FINISH;
            }
        }
    }
    else if( isSampler )
    {
        if( NULL == arg_value )
        {
            errCode = CL_INVALID_ARG_VALUE;
            goto FINISH;
        }
        _cl_sampler_crt* ClSampler = *((_cl_sampler_crt**)(arg_value));
        if( NULL == ClSampler )
        {
            errCode = CL_INVALID_ARG_VALUE;
            goto FINISH;
        }
        crtSamplerObj = reinterpret_cast<CrtSampler*>(ClSampler->object);
        if( NULL == crtSamplerObj )
        {
            errCode = CL_INVALID_SAMPLER;
            goto FINISH;
        }
    }

    itr = crtKernel->m_ContextToKernel.begin();
    for (;itr != crtKernel->m_ContextToKernel.end(); itr++)
    {
        void* devObject = NULL;
        if( crtMemObj != NULL )
        {
            cl_mem memObj = crtMemObj->m_ContextToMemObj[itr->first];
            if( ( crtMemObj->IsValidImageFormat( memObj ) != CL_TRUE ) ||
                ( crtMemObj->IsValidMemObjSize( memObj ) != CL_TRUE ) )
            {
                // This is an image format which is not supported on the underlying
                // context devices, we shall continue to other contexts
                // There is no case that the image format isn't supported by any
                // device since clCreateImage already succeeded previously.
                continue;
            }
            devObject = (void*)(&memObj);
        }
        else if( crtSamplerObj != NULL )
        {
            devObject = (void*)(&(crtSamplerObj->m_ContextToSampler[itr->first]));
        }

        if( devObject == NULL )
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
        if( CL_SUCCESS == errCode )
        {
            succeed = true;
        }
    }
FINISH:
    if( paramType )
    {
        delete[] paramType;
        paramType = NULL;
    }
    if( succeed )
    {
        errCode = CL_SUCCESS;
    }
    return errCode;
}
SET_ALIAS( clSetKernelArg );
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
    cl_int errCode    = CL_SUCCESS;
    cl_context devCtx = NULL;

    CrtKernel* crtKernel = reinterpret_cast<CrtKernel*>(((_cl_kernel_crt*)kernel)->object);

    if( NULL == device )
    {
        if( crtKernel->m_programCRT->m_buildContexts.size() > 1 )
        {
            return CL_INVALID_DEVICE;
        }
        devCtx = crtKernel->m_programCRT->m_buildContexts[0];
    }
    else
    {
        devCtx = crtKernel->m_programCRT->m_contextCRT->m_DeviceToContext[device];
    }

    if (NULL == devCtx)
    {
        return CL_INVALID_DEVICE;
    }

    if( crtKernel->m_ContextToKernel.find(devCtx) ==
        crtKernel->m_ContextToKernel.end() )
    {
        // Not sure this the correct error value o
        return CL_INVALID_DEVICE;
    }

    errCode = devCtx->dispatch->clGetKernelWorkGroupInfo(
        crtKernel->m_ContextToKernel[devCtx],
        device,
        param_name,
        param_value_size,
        param_value,
        param_value_size_ret);

    return errCode;
}
SET_ALIAS( clGetKernelWorkGroupInfo );
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clGetKernelSubGroupInfoKHR(
    cl_kernel                kernel,
    cl_device_id             device,
    cl_kernel_sub_group_info param_name,
    size_t                   input_value_size,
    const void *             input_value,
    size_t                   param_value_size,
    void *                   param_value,
    size_t *                 param_value_size_ret)
{
    cl_int errCode    = CL_SUCCESS;
    cl_context devCtx = NULL;

    CrtKernel* crtKernel = reinterpret_cast<CrtKernel*>( ( ( _cl_kernel_crt* )kernel )->object );

    if( NULL == device )
    {
        if( crtKernel->m_programCRT->m_buildContexts.size() > 1 )
        {
            return CL_INVALID_DEVICE;
        }
        devCtx = crtKernel->m_programCRT->m_buildContexts[0];
    }
    else
    {
        devCtx = crtKernel->m_programCRT->m_contextCRT->m_DeviceToContext[device];
    }
    if( NULL == devCtx )
    {
        return CL_INVALID_DEVICE;
    }

    if( crtKernel->m_ContextToKernel.find( devCtx ) ==
        crtKernel->m_ContextToKernel.end() )
    {
        return CL_INVALID_DEVICE;
    }

    if( devCtx->dispatch->clGetKernelSubGroupInfoKHR == NULL )
    {
        return CL_INVALID_DEVICE;
    }

    errCode = devCtx->dispatch->clGetKernelSubGroupInfoKHR(
        crtKernel->m_ContextToKernel[devCtx],
        device,
        param_name,
        input_value_size,
        input_value,
        param_value_size,
        param_value,
        param_value_size_ret);

    return errCode;
}
SET_ALIAS( clGetKernelSubGroupInfoKHR );

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
SET_ALIAS( clRetainKernel );
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
        // deletes CRT handle _cl_kernel_crt
        delete kernel;
    }
    crtKernel->DecPendencyCnt();
    return errCode;
}
SET_ALIAS( clReleaseKernel );
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL updateAddedDevicesInfo(
    CrtDeviceInfo*                              parentDevInfo,
    const cl_device_id*                         out_devices,
    cl_uint                                     num_devices)
{
    cl_int errCode = CL_SUCCESS;
     // Tracks number of additions to m_deviceInfoMapGuard
    cl_uint numProcessed = 0;

    for( cl_uint i=0; i< num_devices; i++ )
    {
        cl_device_id dev = out_devices[i];
        CrtDeviceInfo* devInfo = new CrtDeviceInfo;
        if( NULL == devInfo )
        {
            errCode = CL_OUT_OF_HOST_MEMORY;
            break;
        }
        *( devInfo )        = *parentDevInfo;
        devInfo->m_refCount = 1;
        devInfo->m_devType  = parentDevInfo->m_devType;
        // This is a sub-device
        devInfo->m_isRootDevice = false;
        devInfo->m_origDispatchTable = parentDevInfo->m_origDispatchTable;
        // Patch new created device IDs. some platforms don't use the same table
        // for all handles (gpu), so we need to call Patch for each new created handle
        OCLCRT::crt_ocl_module.PatchClDeviceID(dev);
        OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.Add(dev,devInfo);
        numProcessed++;
    }

    if( CL_SUCCESS != errCode )
    {
        for( cl_uint i=0; i < numProcessed; i++ )
        {
            CrtDeviceInfo* pDevInfo = OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.GetValue(out_devices[i]);
            if( pDevInfo )
            {
                delete pDevInfo;
            }
            OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.Remove(out_devices[i]);
        }
    }
    return errCode;
}
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
CL_API_ENTRY cl_int CL_API_CALL clCreateSubDevices(
    cl_device_id                                device,
    const cl_device_partition_property*         properties,
    cl_uint                                     num_devices,
    cl_device_id*                               out_devices,
    cl_uint*                                    num_devices_ret)
{
    cl_int errCode = CL_SUCCESS;

    CrtDeviceInfo* parentDevInfo = OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.GetValue(device);
    if (!parentDevInfo)
    {
        return CL_INVALID_DEVICE;
    }

    cl_uint tmp_num_devices_ret;
    errCode = parentDevInfo->m_origDispatchTable.clCreateSubDevices(
                device,
                properties,
                num_devices,
                out_devices,
                &tmp_num_devices_ret);

    if ((CL_SUCCESS == errCode) && (NULL != out_devices))
    {
        errCode = updateAddedDevicesInfo( parentDevInfo, out_devices, tmp_num_devices_ret );
    }

    if( num_devices_ret )
    {
        *num_devices_ret = tmp_num_devices_ret;
    }
    return errCode;
}
SET_ALIAS( clCreateSubDevices );

/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clReleaseDevice(cl_device_id device)
{
    cl_int errCode = CL_SUCCESS;

    CrtDeviceInfo* devInfo = OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.GetValue(device);

    if( devInfo == NULL )
    {
        return CL_INVALID_DEVICE;
    }

    // No need to do anything for Root devices
    if( devInfo->m_isRootDevice )
    {
        return CL_SUCCESS;
    }

    // If we reached here, then this is a sub-device
    long refCount = atomic_decrement(&(devInfo->m_refCount));
    if( refCount == 0 )
    {
        errCode = devInfo->m_origDispatchTable.clReleaseDevice(device);
        OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.Remove(device);
        delete devInfo;
    }
    return errCode;
}
SET_ALIAS( clReleaseDevice );

/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clRetainDevice(cl_device_id device)
{
    cl_int errCode = CL_SUCCESS;

    CrtDeviceInfo* devInfo = OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.GetValue(device);

    if( devInfo == NULL )
    {
        return CL_INVALID_DEVICE;
    }

    // No need to do anything for Root devices
    if (devInfo->m_isRootDevice)
    {
        return CL_SUCCESS;
    }
    // If we reached here, then this is a sub-device
    // No need to forward the retain request, just incremement
    // the reference counter
    atomic_increment(&(devInfo->m_refCount));

    return errCode;
}
SET_ALIAS( clRetainDevice );

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
SET_ALIAS( clSetMemObjectDestructorCallback );
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clRetainEvent(cl_event event)
{
    CrtEvent* crtEvent = reinterpret_cast<CrtEvent*>(((_cl_event_crt*)event)->object);

    crtEvent->IncRefCnt();

    return CL_SUCCESS;
}
SET_ALIAS( clRetainEvent );
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
SET_ALIAS( clReleaseEvent );
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clWaitForEvents(cl_uint num_events, const cl_event * event_list)
{
    cl_int errCode                  = CL_SUCCESS;
    CrtContext *crtContext          = NULL;
    cl_event *pEvents               = NULL;
    cl_uint current                 = 0;
    SHARED_CTX_DISPATCH::iterator itr;

    // Implements Option 1 from the Design document

    if (0 == num_events)
    {
        errCode = CL_INVALID_VALUE;
        goto FINISH;
    }
    else
    {
        CrtEvent* crtEvent = reinterpret_cast<CrtEvent*>(((_cl_event_crt*)(event_list[0]))->object);
        crtContext = crtEvent->getContext();

        for( cl_uint i=1; i < num_events; i++ )
        {
            crtEvent = reinterpret_cast<CrtEvent*>(((_cl_event_crt*)(event_list[i]))->object);
            if( crtEvent->getContext() != crtContext )
            {
                errCode = CL_INVALID_CONTEXT;
                goto FINISH;
            }
        }
        crtContext->FlushQueues();
    }

    // accumulate events for the same underlying platform
    pEvents = new cl_event[num_events];
    if( NULL == pEvents )
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }

    itr = crtContext->m_contexts.begin();
    for( ;itr != crtContext->m_contexts.end(); itr++ )
    {
        for( cl_uint i=0; i < num_events; i++ )
        {
            CrtEvent* crtEvent = reinterpret_cast<CrtEvent*>(((_cl_event_crt*)(event_list[i]))->object);
            if( crtEvent->m_isUserEvent )
            {
                // Pick the corrsponding queue event on that context
                pEvents[current++] = ((CrtUserEvent*)crtEvent)->m_ContextToEvent[itr->first];
            }
            else if( itr->first == crtEvent->getContext()->m_DeviceToContext[crtEvent->m_queueCRT->m_device] )
            {
                pEvents[current++] = crtEvent->m_eventDEV;
            }
        }
        if( current > 0 )
        {
            errCode = pEvents[0]->dispatch->clWaitForEvents(current, pEvents);
        }
        if( CL_SUCCESS != errCode )
        {
            goto FINISH;
        }
        current = 0;
    }
FINISH:
    if( pEvents )
    {
        delete[] pEvents;
        pEvents = NULL;
    }
    return errCode;
}
SET_ALIAS( clWaitForEvents );
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

    if( errCode == CL_SUCCESS && param_value )
    {
        if( param_name == CL_EVENT_CONTEXT )
        {
            // since the app receives a context handle from the CRT, we need to assure
            // we are returning the CRT context and not the underlying context.
            memcpy_s(param_value,
                sizeof(cl_context),
                &(crtContext->m_context_handle),
                sizeof(cl_context));
        }
        else if( !crtEvent->m_isUserEvent && ( param_name == CL_EVENT_COMMAND_QUEUE ) )
        {            
            memcpy_s(param_value,
                sizeof(cl_command_queue),
                &(crtEvent->m_queueCRT->m_queue_handle),
                sizeof(cl_command_queue));
        }
        else if( param_name == CL_EVENT_REFERENCE_COUNT )
        {
            // The CRT maintains the correct reference count, so we need to return
            // it from the CRT; recall CRT doesn't forward retain calls to the underlying
            // platforms.
            *((cl_uint*)param_value) = crtEvent->m_refCount;
        }
    }
    return errCode;
}
SET_ALIAS( clGetEventInfo );
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
    CrtProgram* crtProgram= reinterpret_cast<CrtProgram*>(((_cl_program_crt*)program)->object);

    size_t  pValueSize = 0;
    switch(param_name)
    {
        case CL_PROGRAM_REFERENCE_COUNT:
            {
                pValueSize = sizeof(crtProgram->m_refCount);
                if( param_value != NULL )
                {
                    if( param_value_size < pValueSize )
                    {
                        errCode = CL_INVALID_VALUE;
                        goto FINISH;
                    }
                    *((cl_uint*)param_value) = crtProgram->m_refCount;
                }

                if( param_value_size_ret )
                {
                    *param_value_size_ret = pValueSize;
                }
            }
            break;
        case CL_PROGRAM_CONTEXT:
            {
                pValueSize = sizeof(cl_context);
                if( param_value != NULL )
                {
                    if( param_value_size < pValueSize )
                    {
                        errCode = CL_INVALID_VALUE;
                        goto FINISH;
                    }
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
                cl_uint numDevices = ( cl_uint )crtProgram->m_assocDevices.size();
                if( param_value != NULL )
                {
                    if( param_value_size < pValueSize )
                    {
                        errCode = CL_INVALID_VALUE;
                        goto FINISH;
                    }
                    *((cl_uint*)param_value) = numDevices;
                }

                if( param_value_size_ret )
                {
                    *param_value_size_ret = pValueSize;
                }
            }
            break;
        case CL_PROGRAM_DEVICES:
            {
                cl_device_id* pParamValue = (cl_device_id*)param_value;
                cl_uint numAssocDevices = (cl_uint)crtProgram->m_assocDevices.size();

                pValueSize = numAssocDevices * sizeof(cl_device_id);

                if( ( param_value != NULL ) &&
                    ( param_value_size < pValueSize ) )
                {
                    errCode = CL_INVALID_VALUE;
                    goto FINISH;
                }

                cl_uint totalCount = 0;
                CTX_PGM_MAP::iterator itr = crtProgram->m_ContextToProgram.begin();
                for (;itr != crtProgram->m_ContextToProgram.end(); itr++)
                {
                    size_t retSize = 0;
                    errCode = itr->second->dispatch->clGetProgramInfo(
                        itr->second,
                        CL_PROGRAM_DEVICES,
                        pValueSize,
                        ( param_value == NULL ) ? NULL : ( pParamValue + totalCount ),
                        &retSize);

                    if( errCode != CL_SUCCESS )
                    {
                        errCode = CL_OUT_OF_HOST_MEMORY;
                        goto FINISH;
                    }
                    totalCount += ( cl_uint )( retSize / ( sizeof( cl_device_id ) ) );
                }
                if( param_value_size_ret )
                {
                    *param_value_size_ret = pValueSize;
                }
            }
            break;
        case CL_PROGRAM_BINARY_SIZES:
            {
                size_t* pParamValue = (size_t*)param_value;
                cl_uint numAssocDevices = (cl_uint)crtProgram->m_assocDevices.size();

                pValueSize = numAssocDevices * sizeof(size_t);

                if( ( param_value != NULL ) &&
                    ( param_value_size < pValueSize ) )
                {
                    errCode = CL_INVALID_VALUE;
                    goto FINISH;
                }

                cl_uint totalCount = 0;
                CTX_PGM_MAP::iterator itr = crtProgram->m_ContextToProgram.begin();
                for (;itr != crtProgram->m_ContextToProgram.end(); itr++)
                {
                    size_t retSize = 0;
                    errCode = itr->second->dispatch->clGetProgramInfo(
                        itr->second,
                        CL_PROGRAM_BINARY_SIZES,
                        pValueSize,
                        ( param_value == NULL ) ? NULL : ( pParamValue + totalCount ),
                        &retSize);

                    if( errCode != CL_SUCCESS )
                    {
                        errCode = CL_OUT_OF_HOST_MEMORY;
                        goto FINISH;
                    }
                    totalCount += ( cl_uint )( retSize / ( sizeof( size_t ) ) );
                }
                if( param_value_size_ret )
                {
                    *param_value_size_ret = pValueSize;
                }
            }
            break;
        case CL_PROGRAM_BINARIES:
            {
                unsigned char** pParamValue = (unsigned char**)param_value;
                cl_uint numAssocDevices = (cl_uint)crtProgram->m_assocDevices.size();

                pValueSize = numAssocDevices * sizeof( size_t );

                if( ( param_value != NULL ) &&
                    ( param_value_size < pValueSize ) )
                {
                    errCode = CL_INVALID_VALUE;
                    goto FINISH;
                }

                cl_uint totalCount = 0;
                CTX_PGM_MAP::iterator itr = crtProgram->m_ContextToProgram.begin();
                for (;itr != crtProgram->m_ContextToProgram.end(); itr++)
                {
                    size_t retSize = 0;
                    errCode = itr->second->dispatch->clGetProgramInfo(
                        itr->second,
                        CL_PROGRAM_BINARIES,
                        pValueSize,
                        ( param_value == NULL ) ? NULL : ( pParamValue + totalCount ),
                        &retSize);

                    if( errCode != CL_SUCCESS )
                    {
                        errCode = CL_OUT_OF_HOST_MEMORY;
                        goto FINISH;
                    }
                    totalCount += ( cl_uint )( retSize / ( sizeof( unsigned char** ) ) );
                }
                if( param_value_size_ret )
                {
                    *param_value_size_ret = pValueSize;
                }
            }
            break;
        default:
            {
                cl_program pgmObj = NULL;
                if( crtProgram->m_buildContexts.empty() )
                {
                    pgmObj = crtProgram->m_ContextToProgram.begin()->second;
                }
                else
                {
                    pgmObj = crtProgram->m_ContextToProgram[ crtProgram->m_buildContexts[0] ];
                }
                errCode = pgmObj->dispatch->clGetProgramInfo(
                    pgmObj,
                    param_name,
                    param_value_size,
                    param_value,
                    param_value_size_ret);
            }
            break;
    }
FINISH:
    return errCode;
}
SET_ALIAS( clGetProgramInfo );
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
        cl_event eventDEV = itr->second.clCreateUserEvent(itr->first, &errCode);
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
SET_ALIAS( clCreateUserEvent );
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
SET_ALIAS( clSetUserEventStatus );
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
    CrtSetEventCallBackData* crtEvCBackData = NULL;

    CrtEvent* crtEvent = reinterpret_cast<CrtEvent*>(((_cl_event_crt*)event)->object);
    if (!crtEvent)
    {
        errCode = CL_INVALID_EVENT;
        goto FINISH;
    }
    else
    {
        cl_event tgtEvent = NULL;
        if (false == crtEvent->m_isUserEvent)
        {
            tgtEvent = crtEvent->m_eventDEV;
        }
        else
        {
            CrtUserEvent* crtUserEvent = (CrtUserEvent*)crtEvent;
            // No point on registering the callback notification on all
            // underlying platforms, so we pick randomly the first one.
            tgtEvent = crtUserEvent->m_ContextToEvent.begin()->second;
        }

        crtEvCBackData = new CrtSetEventCallBackData;
        if( NULL == crtEvCBackData )
        {
            errCode = CL_OUT_OF_HOST_MEMORY;
            goto FINISH;
        }

        crtEvCBackData->m_userData = user_data;
        crtEvCBackData->m_crtEvent = event;
        crtEvCBackData->m_userPfnNotify = notify_callback;

        errCode = tgtEvent->dispatch->clSetEventCallback(
                tgtEvent,
                command_exec_callback_type,
                CrtSetEventCallBack,
                crtEvCBackData);
    }
FINISH:
    if( CL_SUCCESS != errCode )
    {
        if( crtEvCBackData )
        {
            delete crtEvCBackData;
            crtEvCBackData = NULL;
        }
    }
    return errCode;
}
SET_ALIAS( clSetEventCallback );
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
    cl_int errCode              = CL_SUCCESS;
    SyncManager *synchHelper    = NULL;
    CrtEvent *crtEvent          = NULL;
    CrtKernel *crtKernel        = NULL;
    cl_event *outEvents         = NULL;
    cl_uint numOutEvents        = 0;
    cl_context targetContext    = NULL;

    if( command_queue == NULL )
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    if( kernel == NULL )
    {
        return CL_INVALID_KERNEL;
    }

    CrtQueue* queue = reinterpret_cast<CrtQueue*>(((_cl_command_queue_crt*)command_queue)->object);
    if (!queue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    synchHelper = new SyncManager;
    if( !synchHelper )
    {
        return CL_OUT_OF_HOST_MEMORY;
    }

    errCode = synchHelper->PrepareToExecute(
        queue,
        num_events_in_wait_list,
        event_wait_list,
        &numOutEvents,
        &outEvents);

    if( CL_SUCCESS != errCode )
    {
        goto FINISH;
    }

    crtEvent = new CrtEvent( queue );
    if( !crtEvent )
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }

    targetContext = queue->m_contextCRT->m_DeviceToContext[queue->m_device];
    crtKernel = reinterpret_cast<CrtKernel*>(((_cl_kernel_crt*)kernel)->object);
    if (!crtKernel)
    {
        errCode = CL_INVALID_KERNEL;
        goto FINISH;
    }

    if( crtKernel->m_programCRT->m_buildContexts.end() == ( std::find( crtKernel->m_programCRT->m_buildContexts.begin(),
                                                                       crtKernel->m_programCRT->m_buildContexts.end(),
                                                                       targetContext ) ) )
    {
        errCode = CL_INVALID_PROGRAM_EXECUTABLE;
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

    if( ( errCode == CL_SUCCESS ) && event )
    {
        _cl_event_crt* event_handle = new _cl_event_crt;
        if( !event_handle )
        {
            delete crtEvent;
            crtEvent = NULL;
            errCode = CL_OUT_OF_HOST_MEMORY;
            goto FINISH;
        }
        event_handle->object = (void*)crtEvent;
        *event = event_handle;
    }
    else
    {
        // We need to check for erros
        // conseptually it might fail because of INVALID_MEMOBJ_SIZE or
        // INVALID_IMG_FORMAT. however, i have no way right now to
        // distinguish so i am returning one of them
    }
FINISH:
    if( crtEvent && ( !event || ( CL_SUCCESS != errCode ) ) )
    {
        crtEvent->Release();
        crtEvent->DecPendencyCnt();
    }
    if( synchHelper )
    {
        synchHelper->Release(errCode);
        delete synchHelper;
    }
    return errCode;
}
SET_ALIAS( clEnqueueNDRangeKernel );
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
    cl_int errCode              = CL_SUCCESS;
    SyncManager *synchHelper    = NULL;
    CrtEvent *crtEvent          = NULL;
    CrtKernel *crtKernel        = NULL;
    cl_context targetContext    = NULL;

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

    synchHelper = new SyncManager;
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
        &outEvents);

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
    crtKernel = reinterpret_cast<CrtKernel*>(((_cl_kernel_crt*)kernel)->object);
    if (!crtKernel)
    {
        errCode = CL_INVALID_KERNEL;
        goto FINISH;
    }

    targetContext = queue->m_contextCRT->m_DeviceToContext[ queue->m_device ];
    if( crtKernel->m_ContextToKernel.find( targetContext ) == crtKernel->m_ContextToKernel.end() )
    {
        errCode = CL_INVALID_PROGRAM_EXECUTABLE;
        goto FINISH;
    }

    errCode = queue->m_cmdQueueDEV->dispatch->clEnqueueTask(
        queue->m_cmdQueueDEV,
        crtKernel->m_ContextToKernel[ targetContext ],
        numOutEvents,
        outEvents,
        &crtEvent->m_eventDEV );

    if( errCode == CL_SUCCESS && event )
    {
        _cl_event_crt* event_handle = new _cl_event_crt;
        if( !event_handle )
        {
            delete crtEvent;
            crtEvent = NULL;
            errCode = CL_OUT_OF_HOST_MEMORY;
            goto FINISH;
        }
        event_handle->object = (void*)crtEvent;
        *event = event_handle;
    }
    else
    {
        // We need to check for erros
        // conseptually it might fail because of INVALID_MEMOBJ_SIZE or
        // INVALID_IMG_FORMAT. however, i have no way right now to
        // distinguish so i am returning one of them
    }

FINISH:
    if( crtEvent && ( !event || ( CL_SUCCESS != errCode ) ) )
    {
        crtEvent->Release();
        crtEvent->DecPendencyCnt();
    }
    if( synchHelper )
    {
        synchHelper->Release( errCode );
        delete synchHelper;
    }
    return errCode;
}
SET_ALIAS( clEnqueueTask );
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
SET_ALIAS( clEnqueueMarker );
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
SET_ALIAS( clEnqueueBarrier );
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clEnqueueWaitForEvents(
    cl_command_queue    command_queue,
    cl_uint             num_events_in_wait_list,
    const cl_event *    event_wait_list)
{
    cl_int errCode           = CL_SUCCESS;
    SyncManager* synchHelper = NULL;

    if (command_queue == NULL)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    CrtQueue* queue = reinterpret_cast<CrtQueue*>(((_cl_command_queue_crt*)command_queue)->object);
    if (!queue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    synchHelper = new SyncManager;
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
        &outEvents);

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
SET_ALIAS( clEnqueueWaitForEvents );
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL EnqueueMarkerOrBarrierWithWaitList(
    cl_command_type         cmdType,
    cl_command_queue        command_queue,
    cl_uint                 num_events_in_wait_list,
    const cl_event *        event_wait_list,
    cl_event *              event )
{
    cl_int errCode              = CL_SUCCESS;
    CrtEvent *crtEvent          = NULL;
    SyncManager *synchHelper    = NULL;
    cl_event *outEvents         = NULL;
    cl_uint numOutEvents        = 0;
    CrtQueue* queue             = NULL;

    if( OCLCRT::crt_ocl_module.m_CrtPlatformVersion < OPENCL_1_2 )
    {
        errCode = CL_INVALID_DEVICE;
        goto FINISH;
    }

    if (command_queue == NULL)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    queue = reinterpret_cast<CrtQueue*>(((_cl_command_queue_crt*)command_queue)->object);
    if (!queue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    synchHelper = new SyncManager;
    if (!synchHelper)
    {
        return CL_OUT_OF_HOST_MEMORY;
    }

    errCode = synchHelper->PrepareToExecute(
        queue,
        num_events_in_wait_list,
        event_wait_list,
        &numOutEvents,
        &outEvents);

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

    if( cmdType == CL_COMMAND_MARKER )
    {
        errCode = queue->m_cmdQueueDEV->dispatch->clEnqueueMarkerWithWaitList(
            queue->m_cmdQueueDEV,
            numOutEvents,
            outEvents,
            &crtEvent->m_eventDEV);
    }
    else
    {
        errCode = queue->m_cmdQueueDEV->dispatch->clEnqueueBarrierWithWaitList(
            queue->m_cmdQueueDEV,
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
    if( crtEvent && ( !event || ( CL_SUCCESS != errCode ) ) )
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
CL_API_ENTRY cl_int CL_API_CALL clEnqueueMarkerWithWaitList(
    cl_command_queue  command_queue,
    cl_uint           num_events_in_wait_list,
    const cl_event *  event_wait_list,
    cl_event *        event )
{
    return EnqueueMarkerOrBarrierWithWaitList(
        CL_COMMAND_MARKER,
        command_queue,
        num_events_in_wait_list,
        event_wait_list,
        event);
}
SET_ALIAS( clEnqueueMarkerWithWaitList );
// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
CL_API_ENTRY cl_int CL_API_CALL clEnqueueBarrierWithWaitList(
    cl_command_queue  command_queue,
    cl_uint           num_events_in_wait_list,
    const cl_event *  event_wait_list,
    cl_event *        event )
{
    return EnqueueMarkerOrBarrierWithWaitList(
        CL_COMMAND_BARRIER,
        command_queue,
        num_events_in_wait_list,
        event_wait_list,
        event);
}
SET_ALIAS( clEnqueueBarrierWithWaitList );
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
    cl_int errCode           = CL_SUCCESS;
    CrtEvent* crtEvent       = NULL;
    SyncManager* synchHelper = NULL;
    cl_mem* crt_mem_list     = NULL;

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

    // Check if device support Native Kernels in reported device capabilities
    CrtDeviceInfo * devInfo = OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.GetValue(queue->m_device);

    if( !devInfo )
    {
        return CL_INVALID_DEVICE;
    }

    if (!(devInfo->m_deviceCapabilities & CL_EXEC_NATIVE_KERNEL))
    {
        return CL_INVALID_OPERATION;
    }

    synchHelper = new SyncManager;
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
        &outEvents);

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

    if (num_mem_objects > 0)
    {
        crt_mem_list = new cl_mem[num_mem_objects];
        if (!crt_mem_list)
        {
            errCode = CL_OUT_OF_HOST_MEMORY;
            goto FINISH;
        }
        // Translate memory object handles
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
SET_ALIAS( clEnqueueNativeKernel );
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
    _cl_mem_crt *mem_handle     = NULL;
    cl_int errCode              = CL_SUCCESS;
    CrtContextInfo *ctxInfo     = NULL;
    CrtContext *ctx             = NULL;

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

    cl_image_desc  image_desc;
    image_desc.image_type = CL_MEM_OBJECT_IMAGE2D;
    image_desc.image_width = image_width;
    image_desc.image_height = image_height;
    image_desc.image_depth = 1;
    image_desc.image_row_pitch = image_row_pitch;
    image_desc.image_slice_pitch = 0;
    image_desc.image_array_size = 1;
    image_desc.mem_object = NULL;
    image_desc.num_mip_levels = 0;
    image_desc.num_samples = 0;

    ctx = (CrtContext*)(ctxInfo->m_object);
    errCode = ctx->CreateImage(
        flags,
        image_format,
        &image_desc,
        host_ptr,
        (CrtMemObject**)(&mem_handle->object));

FINISH:
    if (CL_SUCCESS != errCode)
    {
        if( mem_handle )
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
SET_ALIAS( clCreateImage2D );
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
    _cl_mem_crt *mem_handle     = NULL;
    cl_int errCode              = CL_SUCCESS;
    CrtContextInfo *ctxInfo     = NULL;
    CrtContext *ctx             = NULL;

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

    cl_image_desc image_desc;
    image_desc.image_type = CL_MEM_OBJECT_IMAGE3D;
    image_desc.image_width = image_width;
    image_desc.image_height = image_height;
    image_desc.image_depth = image_depth;
    image_desc.image_row_pitch = image_row_pitch;
    image_desc.image_slice_pitch = image_slice_pitch;
    image_desc.image_array_size = 1;
    image_desc.mem_object = NULL;
    image_desc.num_mip_levels = 0;
    image_desc.num_samples = 0;

    ctx = (CrtContext*)(ctxInfo->m_object);
    errCode = ctx->CreateImage(
        flags,
        image_format,
        &image_desc,
        host_ptr,
        (CrtMemObject**)(&mem_handle->object));

FINISH:
    if( CL_SUCCESS != errCode )
    {
        if( mem_handle )
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
SET_ALIAS( clCreateImage3D );
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
inline cl_int CL_API_CALL EnqueueReadWriteImage(
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
    cl_int errCode              = CL_SUCCESS;
    CrtEvent *crtEvent          = NULL;
    SyncManager *synchHelper    = NULL;
    cl_mem devMemObj            = NULL;
    cl_event *outEvents         = NULL;
    cl_uint numOutEvents        = 0;

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

    CrtImage* crtImage = reinterpret_cast<CrtImage*>(((_cl_mem_crt*)image)->object);
    if( !crtImage )
    {   errCode = CL_INVALID_MEM_OBJECT;
        goto FINISH;
    }

    if( crtImage->m_pContext != queue->m_contextCRT )
    {
        errCode = CL_INVALID_CONTEXT;
        goto FINISH;
    }

    devMemObj = crtImage->getDeviceMemObj(queue->m_device);
    if( crtImage->IsValidImageFormat( devMemObj ) != CL_TRUE )
    {
        errCode = CL_IMAGE_FORMAT_NOT_SUPPORTED;
        goto FINISH;
    }
    else if( crtImage->IsValidMemObjSize( devMemObj ) != CL_TRUE )
    {
        errCode = CL_INVALID_IMAGE_SIZE;
        goto FINISH;
    }

    synchHelper = new SyncManager;
    if (!synchHelper)
    {
        return CL_OUT_OF_HOST_MEMORY;
    }

    errCode = synchHelper->PrepareToExecute(
        queue,
        num_events_in_wait_list,
        event_wait_list,
        &numOutEvents,
        &outEvents);

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

    if( read_command )
    {
        if( crtImage->m_flags & ( CL_MEM_HOST_NO_ACCESS | CL_MEM_HOST_WRITE_ONLY ) )
        {
            errCode = CL_INVALID_OPERATION;
            goto FINISH;
        }
    }
    else
    {
        if( crtImage->m_flags & ( CL_MEM_HOST_NO_ACCESS | CL_MEM_HOST_READ_ONLY ) )
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
        errCode = queue->m_cmdQueueDEV->dispatch->clEnqueueReadImage(
            queue->m_cmdQueueDEV,
            devMemObj,
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
cl_int CL_API_CALL clEnqueueWriteImage(
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
}
SET_ALIAS( clEnqueueWriteImage );
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clEnqueueReadImage(
    cl_command_queue    command_queue,
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
}
SET_ALIAS( clEnqueueReadImage );
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
    cl_int errCode              = CL_SUCCESS;
    CrtEvent *crtEvent          = NULL;
    SyncManager *synchHelper    = NULL;
    cl_mem devSrcMemObj         = NULL;
    cl_mem devDstMemObj         = NULL;
    cl_event *outEvents         = NULL;
    cl_uint numOutEvents        = 0;

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

    CrtImage* crtSrcImage = reinterpret_cast<CrtImage*>(((_cl_mem_crt*)src_image)->object);
    CrtImage* crtDstImage = reinterpret_cast<CrtImage*>(((_cl_mem_crt*)dst_image)->object);
    if( ( !crtSrcImage ) || ( !crtDstImage ) )
    {
        errCode = CL_INVALID_MEM_OBJECT;
        goto FINISH;
    }

    if( ( crtSrcImage->m_pContext != queue->m_contextCRT ) || ( crtDstImage->m_pContext != queue->m_contextCRT ) )
    {
        errCode = CL_INVALID_CONTEXT;
        goto FINISH;
    }

    devSrcMemObj = crtSrcImage->getDeviceMemObj(queue->m_device);
    devDstMemObj = crtDstImage->getDeviceMemObj(queue->m_device);

    if( ( crtSrcImage->IsValidImageFormat( devSrcMemObj ) != CL_TRUE ) ||
         ( crtDstImage->IsValidImageFormat( devDstMemObj ) != CL_TRUE ) )
    {
        errCode = CL_IMAGE_FORMAT_NOT_SUPPORTED;
        goto FINISH;
    }
    else if( ( crtSrcImage->IsValidMemObjSize( devSrcMemObj ) != CL_TRUE ) ||
             ( crtDstImage->IsValidMemObjSize( devDstMemObj ) != CL_TRUE ) )
    {
        errCode = CL_INVALID_IMAGE_SIZE;
        goto FINISH;
    }

    synchHelper = new SyncManager;
    if (!synchHelper)
    {
        return CL_OUT_OF_HOST_MEMORY;
    }

    errCode = synchHelper->PrepareToExecute(
        queue,
        num_events_in_wait_list,
        event_wait_list,
        &numOutEvents,
        &outEvents);

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

    errCode = queue->m_cmdQueueDEV->dispatch->clEnqueueCopyImage(
        queue->m_cmdQueueDEV,
        devSrcMemObj,
        devDstMemObj,
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
SET_ALIAS( clEnqueueCopyImage );
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
    cl_int errCode              = CL_SUCCESS;
    CrtEvent *crtEvent          = NULL;
    SyncManager *synchHelper    = NULL;
    cl_mem devSrcMemObj         = NULL;
    cl_event *outEvents         = NULL;
    cl_uint numOutEvents        = 0;

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

    CrtImage* crtSrcImage = reinterpret_cast<CrtImage*>(((_cl_mem_crt*)src_image)->object);
    CrtBuffer* crtDstBuffer = reinterpret_cast<CrtBuffer*>(((_cl_mem_crt*)dst_buffer)->object);
    if( !crtSrcImage || !crtDstBuffer )
    {
        errCode = CL_INVALID_MEM_OBJECT;
        goto FINISH;
    }

    if( ( crtSrcImage->m_pContext != queue->m_contextCRT ) || ( crtDstBuffer->m_pContext != queue->m_contextCRT ) )
    {
        errCode = CL_INVALID_CONTEXT;
        goto FINISH;
    }

    devSrcMemObj = crtSrcImage->getDeviceMemObj(queue->m_device);
    if( crtSrcImage->IsValidImageFormat( devSrcMemObj ) != CL_TRUE )
    {
        errCode = CL_IMAGE_FORMAT_NOT_SUPPORTED;
        goto FINISH;
    }
    else if( crtSrcImage->IsValidMemObjSize( devSrcMemObj ) != CL_TRUE )
    {
        errCode = CL_INVALID_IMAGE_SIZE;
        goto FINISH;
    }

    synchHelper = new SyncManager;
    if (!synchHelper)
    {
        return CL_OUT_OF_HOST_MEMORY;
    }

    errCode = synchHelper->PrepareToExecute(
        queue,
        num_events_in_wait_list,
        event_wait_list,
        &numOutEvents,
        &outEvents);

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

    errCode = queue->m_cmdQueueDEV->dispatch->clEnqueueCopyImageToBuffer(
        queue->m_cmdQueueDEV,
        devSrcMemObj,
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
    if( crtEvent && ( !event || ( CL_SUCCESS != errCode ) ) )
    {
        crtEvent->Release();
        crtEvent->DecPendencyCnt();
    }
    if( synchHelper )
    {
        synchHelper->Release( errCode );
        delete synchHelper;
    }
    return errCode;
}
SET_ALIAS( clEnqueueCopyImageToBuffer );
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
    cl_int errCode              = CL_SUCCESS;
    CrtEvent* crtEvent          = NULL;
    SyncManager* synchHelper    = NULL;
    cl_mem devDstMemObj         = NULL;
    cl_event *outEvents         = NULL;
    cl_uint numOutEvents        = 0;

    if( command_queue == NULL )
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    if( ( src_buffer == NULL ) || ( dst_image == NULL ) )
    {
        return CL_INVALID_MEM_OBJECT;
    }

    CrtQueue* queue = reinterpret_cast<CrtQueue*>(((_cl_command_queue_crt*)command_queue)->object);
    if (!queue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    CrtBuffer* crtSrcBuffer = reinterpret_cast<CrtBuffer*>(((_cl_mem_crt*)src_buffer)->object);
    CrtImage*  crtDstImage = reinterpret_cast<CrtImage*>(((_cl_mem_crt*)dst_image)->object);
    if( !crtSrcBuffer || !crtDstImage )
    {
        errCode = CL_INVALID_MEM_OBJECT;
        goto FINISH;
    }

    if( ( crtSrcBuffer->m_pContext != queue->m_contextCRT ) || ( crtDstImage->m_pContext != queue->m_contextCRT ) )
    {
        errCode = CL_INVALID_CONTEXT;
        goto FINISH;
    }

    devDstMemObj = crtDstImage->getDeviceMemObj(queue->m_device);
    if( crtDstImage->IsValidImageFormat( devDstMemObj ) != CL_TRUE )
    {
        errCode = CL_IMAGE_FORMAT_NOT_SUPPORTED;
        goto FINISH;
    }
    else if( crtDstImage->IsValidMemObjSize( devDstMemObj ) != CL_TRUE )
    {
        errCode = CL_INVALID_IMAGE_SIZE;
        goto FINISH;
    }

    synchHelper = new SyncManager;
    if (!synchHelper)
    {
        return CL_OUT_OF_HOST_MEMORY;
    }

    errCode = synchHelper->PrepareToExecute(
        queue,
        num_events_in_wait_list,
        event_wait_list,
        &numOutEvents,
        &outEvents);

    if( CL_SUCCESS != errCode )
    {
        goto FINISH;
    }

    crtEvent = new CrtEvent(queue);
    if( !crtEvent )
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }

    errCode = queue->m_cmdQueueDEV->dispatch->clEnqueueCopyBufferToImage(
        queue->m_cmdQueueDEV,
        crtSrcBuffer->getDeviceMemObj(queue->m_device),
        devDstMemObj,
        src_offset,
        dst_origin,
        region,
        numOutEvents,
        outEvents,
        &crtEvent->m_eventDEV);

    if( ( errCode == CL_SUCCESS ) && event )
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
    if( crtEvent && ( !event || ( CL_SUCCESS != errCode ) ) )
    {
        crtEvent->Release();
        crtEvent->DecPendencyCnt();
    }
    if( synchHelper )
    {
        synchHelper->Release(errCode);
        delete synchHelper;
    }
    return errCode;
}
SET_ALIAS( clEnqueueCopyBufferToImage );
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
CL_API_ENTRY cl_int CL_API_CALL clEnqueueFillBuffer(
    cl_command_queue  command_queue ,
    cl_mem            buffer ,
    const void*       pattern ,
    size_t            pattern_size ,
    size_t            offset ,
    size_t            cb ,
    cl_uint           num_events_in_wait_list ,
    const cl_event *  event_wait_list ,
    cl_event *        event)
{
    cl_int errCode              = CL_SUCCESS;
    CrtEvent *crtEvent          = NULL;
    SyncManager *synchHelper    = NULL;
    cl_mem devMemObj            = NULL;
    cl_event *outEvents         = NULL;
    cl_uint numOutEvents        = 0;

    if( OCLCRT::crt_ocl_module.m_CrtPlatformVersion < OPENCL_1_2 )
    {
        return CL_INVALID_DEVICE;
    }

    if( command_queue == NULL )
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    if( buffer == NULL )
    {
        return CL_INVALID_MEM_OBJECT;
    }

    CrtQueue* queue = reinterpret_cast<CrtQueue*>(((_cl_command_queue_crt*)command_queue)->object);
    if( !queue )
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    CrtBuffer* crtBuffer = reinterpret_cast<CrtBuffer*>(((_cl_mem_crt*)buffer)->object);
    if( !crtBuffer )
    {
        errCode = CL_INVALID_MEM_OBJECT;
        goto FINISH;
    }

    if( crtBuffer->m_pContext != queue->m_contextCRT )
    {
        errCode = CL_INVALID_CONTEXT;
        goto FINISH;
    }

    devMemObj = crtBuffer->getDeviceMemObj(queue->m_device);
    synchHelper = new SyncManager;
    if( !synchHelper )
    {
        return CL_OUT_OF_HOST_MEMORY;
    }

    errCode = synchHelper->PrepareToExecute(
        queue,
        num_events_in_wait_list,
        event_wait_list,
        &numOutEvents,
        &outEvents);

    if( CL_SUCCESS != errCode )
    {
        goto FINISH;
    }

    crtEvent = new CrtEvent(queue);
    if( !crtEvent )
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }

    errCode = queue->m_cmdQueueDEV->dispatch->clEnqueueFillBuffer(
        queue->m_cmdQueueDEV,
        devMemObj,
        pattern,
        pattern_size,
        offset,
        cb,
        numOutEvents,
        outEvents,
        &crtEvent->m_eventDEV);

    if( ( errCode == CL_SUCCESS ) && event )
    {
        _cl_event_crt* event_handle = new _cl_event_crt;
        if( !event_handle )
        {
            errCode = CL_OUT_OF_HOST_MEMORY;
            goto FINISH;
        }
        event_handle->object = (void*)crtEvent;
        *event = event_handle;
    }
FINISH:
    if( crtEvent && (!event || (CL_SUCCESS != errCode)) )
    {
        crtEvent->Release();
        crtEvent->DecPendencyCnt();
    }
    if( synchHelper )
    {
        synchHelper->Release(errCode);
        delete synchHelper;
    }
    return errCode;
}
SET_ALIAS( clEnqueueFillBuffer );
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
CL_API_ENTRY cl_int CL_API_CALL clEnqueueFillImage( cl_command_queue  command_queue ,
    cl_mem            image ,
    const void*       fill_color,
    const size_t*     origin,
    const size_t*     region,
    cl_uint           num_events_in_wait_list ,
    const cl_event *  event_wait_list ,
    cl_event *        event)
{
    cl_int errCode              = CL_SUCCESS;
    CrtEvent *crtEvent          = NULL;
    SyncManager *synchHelper    = NULL;
    cl_mem devMemObj            = NULL;
    cl_event *outEvents         = NULL;
    cl_uint numOutEvents        = 0;

    if( OCLCRT::crt_ocl_module.m_CrtPlatformVersion < OPENCL_1_2 )
    {
        return CL_INVALID_DEVICE;
    }

    if( command_queue == NULL )
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    if( image == NULL )
    {
        return CL_INVALID_MEM_OBJECT;
    }

    CrtQueue* queue = reinterpret_cast<CrtQueue*>(((_cl_command_queue_crt*)command_queue)->object);
    if( !queue )
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    CrtImage* crtImage = reinterpret_cast<CrtImage*>(((_cl_mem_crt*)image)->object);
    if( !crtImage )
    {
        errCode = CL_INVALID_MEM_OBJECT;
        goto FINISH;
    }

    if( crtImage->m_pContext != queue->m_contextCRT )
    {
        errCode = CL_INVALID_CONTEXT;
        goto FINISH;
    }

    devMemObj = crtImage->getDeviceMemObj(queue->m_device);
    if( crtImage->IsValidImageFormat( devMemObj ) != CL_TRUE )
    {
        errCode = CL_IMAGE_FORMAT_NOT_SUPPORTED;
        goto FINISH;
    }
    else if( crtImage->IsValidMemObjSize( devMemObj ) != CL_TRUE )
    {
        errCode = CL_INVALID_IMAGE_SIZE;
        goto FINISH;
    }

    synchHelper = new SyncManager;
    if( !synchHelper )
    {
        return CL_OUT_OF_HOST_MEMORY;
    }

    errCode = synchHelper->PrepareToExecute(
        queue,
        num_events_in_wait_list,
        event_wait_list,
        &numOutEvents,
        &outEvents);

    if( CL_SUCCESS != errCode )
    {
        goto FINISH;
    }

    crtEvent = new CrtEvent(queue);
    if( !crtEvent )
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }

    errCode = queue->m_cmdQueueDEV->dispatch->clEnqueueFillImage(
        queue->m_cmdQueueDEV,
        devMemObj,
        fill_color,
        origin,
        region,
        numOutEvents,
        outEvents,
        &crtEvent->m_eventDEV);

    if( ( errCode == CL_SUCCESS ) && event )
    {
        _cl_event_crt* event_handle = new _cl_event_crt;
        if( !event_handle )
        {
            errCode = CL_OUT_OF_HOST_MEMORY;
            goto FINISH;
        }
        event_handle->object = (void*)crtEvent;
        *event = event_handle;
    }
FINISH:
    if( crtEvent && ( !event || ( CL_SUCCESS != errCode ) ) )
    {
        crtEvent->Release();
        crtEvent->DecPendencyCnt();
    }
    if( synchHelper )
    {
        synchHelper->Release(errCode);
        delete synchHelper;
    }
    return errCode;
}
SET_ALIAS( clEnqueueFillImage );
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
CL_API_ENTRY cl_int CL_API_CALL clEnqueueMigrateMemObjects(
    cl_command_queue        command_queue,
    cl_uint                 num_mem_objects,
    const cl_mem *          mem_objects,
    cl_mem_migration_flags  flags,
    cl_uint                 num_events_in_wait_list,
    const cl_event *        event_wait_list,
    cl_event *              event )
{
    cl_int errCode              = CL_SUCCESS;
    CrtEvent *crtEvent          = NULL;
    SyncManager *synchHelper    = NULL;
    cl_mem *crt_mem_list        = NULL;
    CrtQueue* queue             = NULL;
    cl_event *outEvents         = NULL;
    cl_uint numOutEvents        = 0;

    if( OCLCRT::crt_ocl_module.m_CrtPlatformVersion < OPENCL_1_2 )
    {
        return CL_INVALID_DEVICE;
    }

    if( command_queue == NULL )
    {
        errCode = CL_INVALID_COMMAND_QUEUE;
        goto FINISH;
    }

    if( ( num_mem_objects == 0 ) || ( mem_objects == NULL ) )
    {
        errCode = CL_INVALID_VALUE;
        goto FINISH;
    }

    queue = reinterpret_cast<CrtQueue*>(((_cl_command_queue_crt*)command_queue)->object);
    if( !queue )
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    if( num_mem_objects > 0 )
    {
        crt_mem_list = new cl_mem[num_mem_objects];
        if( !crt_mem_list )
        {
            errCode = CL_OUT_OF_HOST_MEMORY;
            goto FINISH;
        }
        // Translate memory object handles
        for( cl_uint i=0; i < num_mem_objects; i++ )
        {
            _cl_mem_crt * crtMemHandle = (_cl_mem_crt *)(mem_objects[i]);
            CrtMemObject* crtMemObj = ((CrtMemObject*)crtMemHandle->object);
            if( crtMemObj->m_pContext != queue->m_contextCRT )
            {
                errCode = CL_INVALID_MEM_OBJECT;
                goto FINISH;
            }
            crt_mem_list[i] = crtMemObj->getDeviceMemObj(queue->m_device);
        }
    }
    synchHelper = new SyncManager;
    if( !synchHelper )
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }
    errCode = synchHelper->PrepareToExecute(
        queue,
        num_events_in_wait_list,
        event_wait_list,
        &numOutEvents,
        &outEvents);

    if( CL_SUCCESS != errCode )
    {
        goto FINISH;
    }

    crtEvent = new CrtEvent(queue);
    if( !crtEvent )
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }

    errCode = queue->m_cmdQueueDEV->dispatch->clEnqueueMigrateMemObjects(
        queue->m_cmdQueueDEV,
        num_mem_objects,
        crt_mem_list,
        flags,
        numOutEvents,
        outEvents,
        &crtEvent->m_eventDEV);

    if( ( errCode == CL_SUCCESS ) && event )
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
    if( crtEvent && ( !event || ( CL_SUCCESS != errCode ) ) )
    {
        crtEvent->Release();
        crtEvent->DecPendencyCnt();
    }
    if( crt_mem_list )
    {
        delete[] crt_mem_list;
    }
    if( synchHelper )
    {
        synchHelper->Release(errCode);
        delete synchHelper;
    }
    return errCode;
}
SET_ALIAS( clEnqueueMigrateMemObjects );
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_mem CL_API_CALL clCreateImage(
    cl_context              context,
    cl_mem_flags            flags,
    const cl_image_format * image_format,
    const cl_image_desc *   image_desc,
    void *                  host_ptr,
    cl_int *                errcode_ret)
{
    _cl_mem_crt *mem_handle = NULL;
    CrtContextInfo *ctxInfo = NULL;
    cl_int errCode          = CL_SUCCESS;
    CrtContext *ctx         = NULL;

    if( OCLCRT::crt_ocl_module.m_CrtPlatformVersion < OPENCL_1_2 )
    {
        errCode = CL_INVALID_DEVICE;
        goto FINISH;
    }

    ctxInfo = OCLCRT::crt_ocl_module.m_contextInfoGuard.GetValue(context);
    if( !ctxInfo )
    {
        errCode = CL_INVALID_CONTEXT;
        goto FINISH;
    }

    mem_handle = new _cl_mem_crt;
    if( !mem_handle )
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }

    ctx = (CrtContext*)(ctxInfo->m_object);
    errCode = ctx->CreateImage(
        flags,
        image_format,
        image_desc,
        host_ptr,
        (CrtMemObject**)(&mem_handle->object));
FINISH:
    if( CL_SUCCESS != errCode )
    {
        if( mem_handle )
        {
            delete mem_handle;
            mem_handle = NULL;
        }
    }
    if( errcode_ret )
    {
        *errcode_ret = errCode;
    }
    return mem_handle;
}
SET_ALIAS( clCreateImage );
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
    _cl_sampler_crt *sampler_handle = NULL;
    cl_int errCode                  = CL_SUCCESS;
    CrtContextInfo *ctxInfo         = NULL;
    CrtContext* ctx                 = NULL;

    ctxInfo = OCLCRT::crt_ocl_module.m_contextInfoGuard.GetValue(context);
    if( !ctxInfo )
    {
        errCode = CL_INVALID_CONTEXT;
        goto FINISH;
    }

    sampler_handle = new _cl_sampler_crt;
    if( !sampler_handle )
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }

    ctx = (CrtContext*)(ctxInfo->m_object);
    errCode = ctx->CreateSampler(
        normalized_coords,
        addressing_mode,
        filter_mode,
        (CrtSampler**)(&sampler_handle->object));

FINISH:
    if( CL_SUCCESS != errCode )
    {
        if( sampler_handle )
        {
            delete sampler_handle;
            sampler_handle = NULL;
        }
    }
    if( errcode_ret )
    {
        *errcode_ret = errCode;
    }
    return sampler_handle;
}
SET_ALIAS( clCreateSampler );

/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_sampler CL_API_CALL clCreateSamplerWithProperties(
    cl_context                  context,
    const cl_sampler_properties *sampler_properties,
    cl_int                      *errcode_ret)
{
    _cl_sampler_crt *sampler_handle = NULL;
    cl_int          errCode         = CL_SUCCESS;
    CrtContextInfo  *ctxInfo        = NULL;
    CrtContext      *ctx            = NULL;

    if( OCLCRT::crt_ocl_module.m_CrtPlatformVersion < OPENCL_2_0 )
    {
        errCode = CL_INVALID_DEVICE;
        goto FINISH;
    }

    ctxInfo = OCLCRT::crt_ocl_module.m_contextInfoGuard.GetValue( context );
    if( !ctxInfo )
    {
        errCode = CL_INVALID_CONTEXT;
        goto FINISH;
    }

    sampler_handle = new _cl_sampler_crt;
    if( !sampler_handle )
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }

    ctx = ( CrtContext * )( ctxInfo->m_object );
    errCode = ctx->clCreateSamplerWithProperties(
        sampler_properties,
        ( CrtSampler ** )( &sampler_handle->object ) );

FINISH:
    if( CL_SUCCESS != errCode )
    {
        if( sampler_handle )
        {
            delete sampler_handle;
            sampler_handle = NULL;
        }
    }
    if( errcode_ret )
    {
        *errcode_ret = errCode;
    }
    return sampler_handle;
}
SET_ALIAS( clCreateSamplerWithProperties );
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clRetainSampler(
    cl_sampler sampler)
{
    cl_int errCode = CL_SUCCESS;
    _cl_sampler_crt *sampler_handle = (_cl_sampler_crt*)sampler;
    CrtSampler *crtSampler = (CrtSampler*)(sampler_handle->object);
    crtSampler->IncRefCnt();
    return errCode;
}
SET_ALIAS( clRetainSampler );
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
    if( refCount == 0 )
    {
        errCode = crtSampler->Release();
        delete sampler;
    }
    crtSampler->DecPendencyCnt();
    return errCode;
}
SET_ALIAS( clReleaseSampler );
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clGetSamplerInfo(
    cl_sampler          sampler,
    cl_sampler_info     param_name,
    size_t              param_value_size,
    void                *param_value,
    size_t              *param_value_size_ret)
{
    cl_int retCode          = CL_SUCCESS;
    CrtSampler* crtSampler  = reinterpret_cast<CrtSampler*>(((_cl_sampler_crt*)sampler)->object);

    size_t  pValueSize = 0;
    switch( param_name )
    {
        case CL_SAMPLER_REFERENCE_COUNT:
            {
                pValueSize = sizeof( crtSampler->m_refCount );
                if( ( param_value ) && ( param_value_size >= pValueSize ) )
                {
                    *((cl_uint*)param_value) = crtSampler->m_refCount;
                }

                if( param_value_size_ret )
                {
                    *param_value_size_ret = pValueSize;
                }
            }
            break;
        case CL_SAMPLER_CONTEXT:
            {
                pValueSize = sizeof( cl_context );
                if( ( param_value ) && ( param_value_size >= pValueSize ) )
                {
                    *((cl_context*)param_value) = crtSampler->m_contextCRT->m_context_handle;
                }

                if( param_value_size_ret )
                {
                    *param_value_size_ret = pValueSize;
                }
            }
            break;
        default:
            {
                // Pick any of the underlying created sampler objects
                cl_sampler samplerObj = crtSampler->m_ContextToSampler.begin()->second;
                // Forward call
                retCode = samplerObj->dispatch->clGetSamplerInfo(
                                    samplerObj,
                                    param_name,
                                    param_value_size,
                                    param_value,
                                    param_value_size_ret);
            }
            break;
    }
    return retCode;
}
SET_ALIAS( clGetSamplerInfo );
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
    // Pick any of the underlying created memory objects
    cl_mem memObj = crtImage->getAnyValidDeviceMemObj();
    // Forward call
    retCode = memObj->dispatch->clGetImageInfo( memObj,
                                                param_name,
                                                param_value_size,
                                                param_value,
                                                param_value_size_ret);
    return retCode;
}
SET_ALIAS( clGetImageInfo );
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

    if( NULL == memobj )
    {
        return CL_INVALID_MEM_OBJECT;
    }
    CrtMemObject* crtMemObj = reinterpret_cast<CrtMemObject*>(((_cl_mem_crt*)memobj)->object);

    size_t  pValueSize = 0;
    switch( param_name )
    {
        case CL_MEM_FLAGS:
            {
                pValueSize = sizeof(crtMemObj->m_flags);
                if( param_value && ( param_value_size >= pValueSize ) )
                {
                    *((cl_mem_flags*)param_value) = crtMemObj->m_flags;
                }

                if( param_value_size_ret )
                {
                    *param_value_size_ret = pValueSize;
                }
            }
            break;
        case CL_MEM_ASSOCIATED_MEMOBJECT:
            {
                pValueSize = sizeof(void*);
                CrtBuffer* bufObj = (CrtBuffer*)crtMemObj;
                if( param_value && ( param_value_size >= pValueSize ) )
                {
                    if( bufObj->m_parentBuffer )
                    {
                        *((cl_mem*)param_value) = bufObj->m_parentBuffer->GetMemHandle();
                    }
                    else
                    {
                        *((cl_mem*)param_value) = NULL;
                    }
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
                void* host_ptr = NULL;
                if( crtMemObj->m_flags & CL_MEM_USE_HOST_PTR )
                {
                    if( crtMemObj->m_pUsrPtr)
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
                if( param_value && ( param_value_size >= pValueSize ) )
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
                if( param_value && ( param_value_size >= pValueSize ) )
                {
                    *((cl_uint*)param_value) = crtMemObj->m_refCount;
                }
                if( param_value_size_ret )
                {
                    *param_value_size_ret = pValueSize;
                }
            }
            break;
        case CL_MEM_CONTEXT:
            {
                pValueSize = sizeof(cl_context);
                if( param_value && ( param_value_size >= pValueSize ) )
                {
                    *((cl_context*)param_value) = crtMemObj->m_pContext->m_context_handle;
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
SET_ALIAS( clGetMemObjectInfo );
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

    if( crtEvent->m_isUserEvent )
    {
        // Forward call to any of the underlying platforms
        cl_event ev = ( ( CrtUserEvent* )crtEvent )->m_ContextToEvent.begin()->second;
        errCode = ev->dispatch->clGetEventProfilingInfo(
                ev,
                param_name,
                param_value_size,
                param_value,
                param_value_size_ret);
    }
    else
    {
        // Forward call
        errCode = crtEvent->m_eventDEV->dispatch->clGetEventProfilingInfo(
                crtEvent->m_eventDEV,
                param_name,
                param_value_size,
                param_value,
                param_value_size_ret);
    }
    return errCode;
}
SET_ALIAS( clGetEventProfilingInfo );
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

    if( NULL == program )
    {
        return CL_INVALID_PROGRAM;
    }
    if( !num_kernels_ret && !kernels )
    {
        return CL_INVALID_VALUE;
    }

    CrtProgram* crtProgram= reinterpret_cast<CrtProgram*>(((_cl_program_crt*)program)->object);

    // Num kernels which will be returned by each context
    cl_int numKernelsPerContext = -1;
    cl_kernel* ctxKernels       = NULL;

    if( num_kernels !=0 )
    {
        ctxKernels = new cl_kernel[num_kernels];
        if( NULL == ctxKernels )
        {
            return CL_OUT_OF_HOST_MEMORY;
        }
        for( cl_uint i=0; i < num_kernels; i++ )
        {
            kernels[i] = NULL;
        }
    }
    for( cl_uint i=0 ; i < crtProgram->m_buildContexts.size(); i++ )
    {
        cl_int numRetKernels = 0;
        cl_context ctxObj = crtProgram->m_buildContexts[i];
        errCode = ctxObj->dispatch->clCreateKernelsInProgram(
            crtProgram->m_ContextToProgram[ctxObj],
            num_kernels,
            ctxKernels,
            (cl_uint*)&numRetKernels);

        if( CL_SUCCESS == errCode )
        {
            if( numKernelsPerContext < 0 )
            {
                numKernelsPerContext = numRetKernels;
            }
            else
            {
                if( numKernelsPerContext != numRetKernels )
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

        if( num_kernels !=0 )
        {
            for( cl_uint i=0; i < (cl_uint)numKernelsPerContext; i++ )
            {
                CrtKernel* crtKernel = NULL;

                if( NULL == kernels[i] )
                {
                    crtKernel = new CrtKernel( crtProgram );
                    if( !crtKernel )
                    {
                        errCode = CL_OUT_OF_HOST_MEMORY;
                        goto FINISH;
                    }

                    kernels[ i ] = new _cl_kernel_crt;
                    if( NULL == kernels[i] )
                    {
                        crtKernel->DecPendencyCnt();
                        errCode = CL_OUT_OF_HOST_MEMORY;
                        goto FINISH;
                    }
                    ( (_cl_kernel_crt*)kernels[i] )->object = crtKernel;
                }
                else
                {
                    crtKernel = (CrtKernel*)( (_cl_kernel_crt*)kernels[i] )->object;
                }
                crtKernel->m_ContextToKernel[ ctxObj ] = ctxKernels[i];
            }
        }
    }
FINISH:
    if( CL_SUCCESS != errCode )
    {
        for( cl_uint i=0; i < num_kernels; i++ )
        {
            if( NULL != kernels[i] )
            {
                CrtKernel* crtKernel = (CrtKernel*)((_cl_kernel_crt*)kernels[i])->object;
                crtKernel->Release();
                crtKernel->DecPendencyCnt();
                delete kernels[i];
                kernels[i] = NULL;
            }
        }
    }
    if( ctxKernels )
    {
        delete[] ctxKernels;
    }
    if( num_kernels_ret )
    {
        *num_kernels_ret = numKernelsPerContext;
    }
    // Delete allocated structures / data
    return errCode;
}
SET_ALIAS( clCreateKernelsInProgram );
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
    cl_int errCode                      = CL_SUCCESS;
    cl_image_format *allImageFormats    = NULL;
    cl_uint numUnique                   = 0;
    cl_image_format* endUnique          = NULL;
    cl_uint offset                      = 0;

    if( num_entries && num_entries == 0 )
    {
        return CL_INVALID_VALUE;
    }

    CrtContext* ctx = reinterpret_cast<CrtContext*>(((_cl_context_crt*)context)->object);

    cl_uint numTotalAvailable = 0;
    for( SHARED_CTX_DISPATCH::iterator itr = ctx->m_contexts.begin();
        itr != ctx->m_contexts.end();
        itr++ )
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

        if( CL_SUCCESS != errCode )
        {
            goto FINISH;
        }
        numTotalAvailable += numAvailable;
    }

    allImageFormats = new cl_image_format[numTotalAvailable];
    if( NULL == allImageFormats )
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }

    for( SHARED_CTX_DISPATCH::iterator itr = ctx->m_contexts.begin();
        itr != ctx->m_contexts.end();
        itr++ )
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

        if( CL_SUCCESS != errCode )
        {
            goto FINISH;
        }
        offset += numAvailable;
    }

    std::sort( allImageFormats, allImageFormats + numTotalAvailable );

    // Are we returning Union of supported image formats OR intersection?? 'default is intersection'
#ifndef IMAGE_FORMATS_UNION

    cl_uint j = 0;
    cl_uint match = 1;
    for( cl_uint i=1; i < numTotalAvailable; i++ )
    {
        if( ( allImageFormats[i].image_channel_data_type == allImageFormats[i-1].image_channel_data_type )  &&
             ( allImageFormats[i].image_channel_order == allImageFormats[i-1].image_channel_order ) )
        {
            if( ++match == ctx->m_contexts.size() )
            {
                allImageFormats[j++] = allImageFormats[i];
            }
            else
            {
                continue;
            }
        }
        match = 1;

    }
    numUnique = j;

#else   //  IMAGE_FORMATS_UNION
    endUnique = std::unique( allImageFormats,allImageFormats + numTotalAvailable );
    numUnique = (cl_uint)( endUnique - allImageFormats );
#endif

    if( num_image_formats )
    {
        *num_image_formats = numUnique;
    }

    if( !num_entries )
    {
        goto FINISH;
    }
    if( num_entries < numUnique )
    {
        errCode = CL_INVALID_VALUE;
        goto FINISH;
    }
    if( image_formats != NULL )
    {
        for( cl_uint i=0; i < numUnique; i++ )
        {
            image_formats[i] = allImageFormats[i];
        }
    }
FINISH:
    if( allImageFormats )
    {
        delete[] allImageFormats;
    }
    return errCode;
}
SET_ALIAS( clGetSupportedImageFormats );
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clUnloadCompiler()
{
    for( cl_uint i = 0; i < OCLCRT::crt_ocl_module.m_oclPlatforms.size(); i++ )
    {
        CrtPlatform* crtPlatform = OCLCRT::crt_ocl_module.m_oclPlatforms[i];
        crtPlatform->m_platformIdDEV->dispatch->clUnloadCompiler();
    }
    return CL_SUCCESS;
}
SET_ALIAS( clUnloadCompiler );
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL EnqueueAcquireReleaseInteropObjects(
    cl_command_type  interop_sync_cmd_type,
    cl_command_queue command_queue,
    cl_uint          num_objects,
    const cl_mem     *mem_objects,
    cl_uint          num_events_in_wait_list,
    const cl_event   *event_wait_list,
    cl_event *       event )
{
    cl_int errCode              = CL_SUCCESS;
    CrtEvent* crtEvent          = NULL;
    SyncManager* synchHelper    = NULL;
    cl_mem* crt_mem_list        = NULL;

    if( command_queue == NULL )
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    if( ( NULL == mem_objects ) || (0  == num_objects ) )
    {
        return CL_INVALID_VALUE;;
    }

    if( ( ( mem_objects == NULL ) && (num_objects > 0 ) ) &&
        ( ( mem_objects != NULL ) && (num_objects == 0 ) ) )
    {
        return CL_INVALID_VALUE;
    }

    CrtQueue* queue = reinterpret_cast<CrtQueue*>(((_cl_command_queue_crt*)command_queue)->object);
    if( !queue )
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    synchHelper = new SyncManager;
    if( !synchHelper )
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
        &outEvents);

    if( CL_SUCCESS != errCode )
    {
        goto FINISH;
    }

    crtEvent = new CrtEvent(queue);
    if( !crtEvent )
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }

    if (num_objects > 0)
    {
        crt_mem_list = new cl_mem[num_objects];
        if (!crt_mem_list)
        {
            errCode = CL_OUT_OF_HOST_MEMORY;
            goto FINISH;
        }
        // Translate memory object handles
        for (cl_uint i=0; i < num_objects; i++)
        {
            _cl_mem_crt * crtMemHandle = (_cl_mem_crt *)(mem_objects[i]);
            CrtMemObject* crtMemObj = ((CrtMemObject*)crtMemHandle->object);
            crt_mem_list[i] = crtMemObj->getDeviceMemObj(queue->m_device);
        }
    }

    switch( interop_sync_cmd_type )
    {
    case CL_COMMAND_ACQUIRE_GL_OBJECTS:
        errCode = queue->m_cmdQueueDEV->dispatch->clEnqueueAcquireGLObjects(
            queue->m_cmdQueueDEV,
            num_objects,
            crt_mem_list,
            numOutEvents,
            outEvents,
            &crtEvent->m_eventDEV);
        break;
    case CL_COMMAND_RELEASE_GL_OBJECTS:
        errCode = queue->m_cmdQueueDEV->dispatch->clEnqueueReleaseGLObjects(
            queue->m_cmdQueueDEV,
            num_objects,
            crt_mem_list,
            numOutEvents,
            outEvents,
            &crtEvent->m_eventDEV);
        break;
#ifdef _WIN32
    case CL_COMMAND_ACQUIRE_DX9_OBJECTS_INTEL:
        errCode = ( (SOCLEntryPointsTable*)queue->m_cmdQueueDEV )->crtDispatch->clEnqueueAcquireDX9ObjectsINTEL(
            queue->m_cmdQueueDEV,
            num_objects,
            crt_mem_list,
            numOutEvents,
            outEvents,
            &crtEvent->m_eventDEV);
        break;
    case CL_COMMAND_RELEASE_DX9_OBJECTS_INTEL:
        errCode = ( (SOCLEntryPointsTable*)queue->m_cmdQueueDEV )->crtDispatch->clEnqueueReleaseDX9ObjectsINTEL(
            queue->m_cmdQueueDEV,
            num_objects,
            crt_mem_list,
            numOutEvents,
            outEvents,
            &crtEvent->m_eventDEV);
        break;
#endif
    default:
        break;
    };

    if( errCode == CL_SUCCESS && event )
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
    if( crtEvent && ( !event || ( CL_SUCCESS != errCode ) ) )
    {
        crtEvent->Release();
        crtEvent->DecPendencyCnt();
    }
    if (crt_mem_list)
    {
        delete[] crt_mem_list;
    }
    if( synchHelper )
    {
        synchHelper->Release(errCode);
        delete synchHelper;
    }
    return errCode;
}
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
CL_API_ENTRY cl_int CL_API_CALL clGetKernelArgInfo(
    cl_kernel           kernel,
    cl_uint             arg_indx,
    cl_kernel_arg_info  param_name,
    size_t              param_value_size,
    void *              param_value,
    size_t *            param_value_size_ret )
{
    cl_int errCode = CL_SUCCESS;

    if( NULL == kernel )
    {
        return CL_INVALID_KERNEL;
    }

    CrtKernel* crtKernel = reinterpret_cast<CrtKernel*>(((_cl_kernel_crt*)kernel)->object);
    cl_kernel kernelDevObj = crtKernel->m_ContextToKernel.begin()->second;

    errCode = kernelDevObj->dispatch->clGetKernelArgInfo(
        kernelDevObj,
        arg_indx,
        param_name,
        param_value_size,
        param_value,
        param_value_size_ret);

    return errCode;
}
SET_ALIAS( clGetKernelArgInfo );
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
#ifdef _WIN32
CL_API_ENTRY cl_int CL_API_CALL clGetDeviceIDsFromDX9INTEL(
    cl_platform_id              platform,
    cl_dx9_device_source_intel  d3d_device_source,
    void *                      d3d_object,
    cl_dx9_device_set_intel     d3d_device_set,
    cl_uint                     num_entries,
    cl_device_id                *devices,
    cl_uint                     *num_devices)
{
    cl_int errCode                  = CL_SUCCESS;
    OCLCRT::DEV_INFO_MAP::const_iterator itr;
    cl_uint num_devices_ret         = 0;
    cl_uint platform_num_devices    = 0;
    cl_device_id* platform_devices  = NULL;


    OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.Lock();

    if( ( platform == NULL ) || !isValidPlatform( platform ) )
    {
        errCode = CL_INVALID_PLATFORM;
        goto FINISH;
    }

    if( ( ( devices != NULL ) && ( num_entries == 0 ) ) ||
        ( ( devices == NULL ) && ( num_devices == NULL ) ) )
    {
        errCode = CL_INVALID_VALUE;
        goto FINISH;
    }

    for( itr = OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.get().begin();
         itr != OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.get().end();
         itr++ )
    {
        CrtPlatform* devicePlatform = itr->second->m_crtPlatform;

        if( !( devicePlatform->m_supportedExtensions & CRT_CL_INTEL_D3D9_EXT ) )
        {
            // The current platform isn't supporting DX9 interop
            continue;
        }
        switch( d3d_device_set )
        {
        case CL_PREFERRED_DEVICES_FOR_DX9_INTEL:
           if( ( ( OCLCRT::crt_ocl_module.m_availableDeviceTypes & CL_DEVICE_TYPE_GPU ) &&
               ( itr->second->m_devType != CL_DEVICE_TYPE_GPU ) ) ||
               ( itr->second->m_isRootDevice == false ) )
           {
               continue;
           }

           errCode = ( (SOCLEntryPointsTable*) devicePlatform->m_platformIdDEV )->crtDispatch->clGetDeviceIDsFromDX9INTEL(
                devicePlatform->m_platformIdDEV,
                d3d_device_source,
                d3d_object,
                d3d_device_set,
                num_entries,
                devices,
                &num_devices_ret);

           goto FINISH;

           break;

        case CL_ALL_DEVICES_FOR_DX9_INTEL:
            if( devices != NULL )
            {
                platform_devices = &devices[ num_devices_ret ];
            }
            errCode = ( (SOCLEntryPointsTable*)devicePlatform->m_platformIdDEV )->crtDispatch->clGetDeviceIDsFromDX9INTEL(
                devicePlatform->m_platformIdDEV,
                d3d_device_source,
                d3d_object,
                d3d_device_set,
                num_entries,
                platform_devices,
                &platform_num_devices);

            if( errCode == CL_SUCCESS )
            {
                num_devices_ret += platform_num_devices;

                if ( ( devices != NULL ) && ( num_devices_ret > num_entries ) )
                {
                    errCode = CL_INVALID_VALUE;
                    goto FINISH;
                }
            }
            else
            {
                goto FINISH;
            }
            break;
        default:
            errCode = CL_INVALID_VALUE;
            goto FINISH;
            break;
        }
    }

FINISH:
    OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.Release();
    if( num_devices != NULL )
    {
        *num_devices = num_devices_ret;
    }
    return errCode;
}
SET_ALIAS( clGetDeviceIDsFromDX9INTEL );
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
CL_API_ENTRY cl_int CL_API_CALL clGetDeviceIDsFromDX9MediaAdapterKHR(
    cl_platform_id                 platform,
    cl_uint                        num_media_adapters,
    cl_dx9_media_adapter_type_khr* media_adapter_type,
    void *                         media_adapters[],
    cl_dx9_media_adapter_set_khr   media_adapter_set,
    cl_uint                        num_entries,
    cl_device_id *                 devices,
    cl_uint *                      num_devices)
{
    cl_int errCode = CL_SUCCESS;
    OCLCRT::DEV_INFO_MAP::const_iterator itr;
    cl_uint num_devices_ret = 0;
    cl_uint platform_num_devices = 0;
    cl_device_id* platform_devices = NULL;

    OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.Lock();

    if( OCLCRT::crt_ocl_module.m_CrtPlatformVersion < OPENCL_1_2 )
    {
        return CL_INVALID_DEVICE;
    }

    if( ( platform == NULL ) || !isValidPlatform( platform ) )
    {
        errCode = CL_INVALID_PLATFORM;
        goto FINISH;
    }

    if( ( ( devices != NULL ) && ( num_entries == 0 ) ) ||
        ( ( devices == NULL ) && ( num_devices == NULL ) ) )
    {
        errCode = CL_INVALID_VALUE;
        goto FINISH;
    }

    for( itr = OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.get().begin();
         itr != OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.get().end();
         itr++ )
    {
        CrtPlatform* devicePlatform = itr->second->m_crtPlatform;

        if( !( devicePlatform->m_supportedExtensions & CRT_CL_D3D9_EXT ) )
        {
            // The current platform isn't supporting DX9 interop
            continue;
        }
        switch( media_adapter_set )
        {
        case CL_PREFERRED_DEVICES_FOR_DX9_MEDIA_ADAPTER_KHR:
           if( ( ( OCLCRT::crt_ocl_module.m_availableDeviceTypes & CL_DEVICE_TYPE_GPU ) &&
               ( itr->second->m_devType != CL_DEVICE_TYPE_GPU ) ) ||
               ( itr->second->m_isRootDevice == false ) )
           {
               continue;
           }

           errCode = ( devicePlatform->m_platformIdDEV )->dispatch->clGetDeviceIDsFromDX9MediaAdapterKHR(
                devicePlatform->m_platformIdDEV,
                num_media_adapters,
                media_adapter_type,
                media_adapters,
                media_adapter_set,
                num_entries,
                devices,
                &num_devices_ret);

           goto FINISH;
           break;

        case CL_ALL_DEVICES_FOR_DX9_MEDIA_ADAPTER_KHR:
            if( devices != NULL )
            {
                platform_devices = &devices[ num_devices_ret ];
            }
            errCode = ( devicePlatform->m_platformIdDEV )->dispatch->clGetDeviceIDsFromDX9MediaAdapterKHR(
                devicePlatform->m_platformIdDEV,
                num_media_adapters,
                media_adapter_type,
                media_adapters,
                media_adapter_set,
                num_entries,
                platform_devices,
                &platform_num_devices);

            if( errCode == CL_SUCCESS )
            {
                num_devices_ret += platform_num_devices;

                if ( ( devices != NULL ) && ( num_devices_ret > num_entries ) )
                {
                    errCode = CL_INVALID_VALUE;
                    goto FINISH;
                }
            }
            else
            {
                goto FINISH;
            }
            break;
        default:
            errCode = CL_INVALID_VALUE;
            goto FINISH;
            break;
        }
    }

FINISH:
    OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.Release();
    if( num_devices != NULL )
    {
        *num_devices = num_devices_ret;
    }
    return errCode;
}
SET_ALIAS( clGetDeviceIDsFromDX9MediaAdapterKHR );
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
CL_API_ENTRY cl_int CL_API_CALL clGetDeviceIDsFromD3D10KHR(
    cl_platform_id              platform,
    cl_d3d10_device_source_khr  d3d_device_source,
    void                        *d3d_object,
    cl_d3d10_device_set_khr     d3d_device_set,
    cl_uint                     num_entries,
    cl_device_id                *devices,
    cl_uint                     *num_devices)
{
    cl_int errCode                  = CL_SUCCESS;
    OCLCRT::DEV_INFO_MAP::const_iterator itr;
    cl_uint num_devices_ret         = 0;
    cl_uint platform_num_devices    = 0;
    cl_device_id* platform_devices  = NULL;

    OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.Lock();

    if( ( platform == NULL ) || !isValidPlatform( platform ) )
    {
        errCode = CL_INVALID_PLATFORM;
        goto FINISH;
    }

    if( ( ( devices != NULL ) && ( num_entries == 0 ) ) ||
        ( ( devices == NULL ) && ( num_devices == NULL ) ) )
    {
        errCode = CL_INVALID_VALUE;
        goto FINISH;
    }

    for( itr = OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.get().begin();
         itr != OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.get().end();
         itr++ )
    {
        CrtPlatform* devicePlatform = itr->second->m_crtPlatform;

        if( !( devicePlatform->m_supportedExtensions & CRT_CL_D3D10_EXT ) )
        {
            // The current platform isn't supporting DX9 interop
            continue;
        }
        switch( d3d_device_set )
        {
        case CL_PREFERRED_DEVICES_FOR_D3D10_KHR:
           if( ( ( OCLCRT::crt_ocl_module.m_availableDeviceTypes & CL_DEVICE_TYPE_GPU ) &&
               ( itr->second->m_devType != CL_DEVICE_TYPE_GPU ) ) ||
               ( itr->second->m_isRootDevice == false ) )
           {
               continue;
           }

           errCode = ( devicePlatform->m_platformIdDEV )->dispatch->clGetDeviceIDsFromD3D10KHR(
                devicePlatform->m_platformIdDEV,
                d3d_device_source,
                d3d_object,
                d3d_device_set,
                num_entries,
                devices,
                &num_devices_ret);

           goto FINISH;
           break;

        case CL_ALL_DEVICES_FOR_D3D10_KHR:
            if( devices != NULL )
            {
                platform_devices = &devices[ num_devices_ret ];
            }
            errCode = ( devicePlatform->m_platformIdDEV )->dispatch->clGetDeviceIDsFromD3D10KHR(
                devicePlatform->m_platformIdDEV,
                d3d_device_source,
                d3d_object,
                d3d_device_set,
                num_entries,
                platform_devices,
                &platform_num_devices);

            if( errCode == CL_SUCCESS )
            {
                num_devices_ret += platform_num_devices;

                if( ( devices != NULL ) && ( num_devices_ret > num_entries ) )
                {
                    errCode = CL_INVALID_VALUE;
                    goto FINISH;
                }
            }
            else
            {
                goto FINISH;
            }
            break;
        default:
            errCode = CL_INVALID_VALUE;
            goto FINISH;
            break;
        }
    }

FINISH:
    OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.Release();
    if( num_devices != NULL )
    {
        *num_devices = num_devices_ret;
    }
    return errCode;
}
SET_ALIAS( clGetDeviceIDsFromD3D10KHR );
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
CL_API_ENTRY cl_int CL_API_CALL clGetDeviceIDsFromD3D11KHR(
    cl_platform_id              platform,
    cl_d3d10_device_source_khr  d3d_device_source,
    void                        *d3d_object,
    cl_d3d10_device_set_khr     d3d_device_set,
    cl_uint                     num_entries,
    cl_device_id                *devices,
    cl_uint                     *num_devices)
{
    cl_int errCode                  = CL_SUCCESS;
    OCLCRT::DEV_INFO_MAP::const_iterator itr;
    cl_uint num_devices_ret         = 0;
    cl_uint platform_num_devices    = 0;
    cl_device_id* platform_devices  = NULL;

    OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.Lock();

    if( OCLCRT::crt_ocl_module.m_CrtPlatformVersion < OPENCL_1_2 )
    {
        return CL_INVALID_DEVICE;
    }

    if( ( platform == NULL ) || !isValidPlatform( platform ) )
    {
        errCode = CL_INVALID_PLATFORM;
        goto FINISH;
    }

    if( ( ( devices != NULL ) && ( num_entries == 0 ) ) ||
        ( ( devices == NULL ) && ( num_devices == NULL ) ) )
    {
        errCode = CL_INVALID_VALUE;
        goto FINISH;
    }

    for( itr = OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.get().begin();
         itr != OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.get().end();
         itr++ )
    {
        CrtPlatform* devicePlatform = itr->second->m_crtPlatform;

        if( !( devicePlatform->m_supportedExtensions & CRT_CL_D3D11_EXT ) )
        {
            // The current platform isn't supporting DX9 interop
            continue;
        }
        switch( d3d_device_set )
        {
        case CL_PREFERRED_DEVICES_FOR_D3D11_KHR:
           if( ( ( OCLCRT::crt_ocl_module.m_availableDeviceTypes & CL_DEVICE_TYPE_GPU ) &&
               ( itr->second->m_devType != CL_DEVICE_TYPE_GPU ) ) ||
               ( itr->second->m_isRootDevice == false ) )
           {
               continue;
           }

           errCode = ( devicePlatform->m_platformIdDEV )->dispatch->clGetDeviceIDsFromD3D11KHR(
                devicePlatform->m_platformIdDEV,
                d3d_device_source,
                d3d_object,
                d3d_device_set,
                num_entries,
                devices,
                &num_devices_ret);

           goto FINISH;
           break;

        case CL_ALL_DEVICES_FOR_D3D11_KHR:
            if( devices != NULL )
            {
                platform_devices = &devices[ num_devices_ret ];
            }
            errCode = ( devicePlatform->m_platformIdDEV )->dispatch->clGetDeviceIDsFromD3D11KHR(
                devicePlatform->m_platformIdDEV,
                d3d_device_source,
                d3d_object,
                d3d_device_set,
                num_entries,
                platform_devices,
                &platform_num_devices);

            if( errCode == CL_SUCCESS )
            {
                num_devices_ret += platform_num_devices;

                if( ( devices != NULL ) && ( num_devices_ret > num_entries ) )
                {
                    errCode = CL_INVALID_VALUE;
                    goto FINISH;
                }
            }
            else
            {
                goto FINISH;
            }
            break;
        default:
            errCode = CL_INVALID_VALUE;
            goto FINISH;
            break;
        }
    }

FINISH:
    OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.Release();
    if( num_devices != NULL )
    {
        *num_devices = num_devices_ret;
    }
    return errCode;
}
SET_ALIAS( clGetDeviceIDsFromD3D11KHR );
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_mem CL_API_CALL CreateFromDX9MediaSurface(
    cl_context              context,
    cl_mem_flags            flags,
    IDirect3DSurface9*      resource,
    HANDLE                  sharedHandle,
    UINT                    plane,
    cl_int *                errcode_ret)
{
    _cl_mem_crt *mem_handle = NULL;
    cl_int errCode          = CL_SUCCESS;
    CrtContextInfo *ctxInfo = NULL;
    CrtContext *ctx         = NULL;

    ctxInfo = OCLCRT::crt_ocl_module.m_contextInfoGuard.GetValue(context);
    if (!ctxInfo)
    {
        errCode = CL_INVALID_CONTEXT;
        goto FINISH;
    }

    mem_handle = new _cl_mem_crt;
    if( !mem_handle )
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }

    ctx = (CrtContext*)(ctxInfo->m_object);
    errCode = ctx->CreateFromDX9MediaSurface(
        flags,
        resource,
        sharedHandle,
        plane,
        (CrtMemObject**)(&mem_handle->object));

FINISH:
    if( CL_SUCCESS != errCode )
    {
        delete mem_handle;
        mem_handle = NULL;
    }
    if( errcode_ret )
    {
        *errcode_ret = errCode;
    }

    return mem_handle;
}
SET_ALIAS( CreateFromDX9MediaSurface );
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
    cl_mem memObj  = NULL;

    CrtContextInfo* ctxInfo = OCLCRT::crt_ocl_module.m_contextInfoGuard.GetValue( context );
    if( ( !ctxInfo ) ||
        ( ctxInfo->m_contextType == CrtContextInfo::SharedPlatformContext ) )
    {
        errCode = CL_INVALID_CONTEXT;
        goto FINISH;
    }

    memObj = ( (SOCLEntryPointsTable*)context )->crtDispatch->clCreateFromDX9MediaSurfaceINTEL(
                                        context,
                                        flags,
                                        resource,
                                        sharedHandle,
                                        plane,
                                        &errCode);

    if( errCode == CL_SUCCESS )
    {
        goto FINISH;
    }

FINISH:
    if( errcode_ret )
    {
        *errcode_ret = errCode;
    }
    return memObj;
}
SET_ALIAS( clCreateFromDX9MediaSurfaceINTEL );
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
CL_API_ENTRY cl_int CL_API_CALL clEnqueueAcquireDX9ObjectsINTEL(
    cl_command_queue command_queue,
    cl_uint          num_objects,
    const cl_mem     *mem_objects,
    cl_uint          num_events_in_wait_list,
    const cl_event   *event_wait_list,
    cl_event         *ocl_event )
{
    cl_int errCode = CL_SUCCESS;

    if( command_queue == NULL )
    {
        errCode = CL_INVALID_COMMAND_QUEUE;
        goto FINISH;
    }

    // We don't support DX for shared context
    errCode = ( (SOCLEntryPointsTable*)command_queue )->crtDispatch->clEnqueueAcquireDX9ObjectsINTEL(
                                        command_queue,
                                        num_objects,
                                        mem_objects,
                                        num_events_in_wait_list,
                                        event_wait_list,
                                        ocl_event);
    if( errCode == CL_SUCCESS )
    {
        goto FINISH;
    }
FINISH:
    return errCode;
}
SET_ALIAS( clEnqueueAcquireDX9ObjectsINTEL );
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
CL_API_ENTRY cl_int CL_API_CALL clEnqueueReleaseDX9ObjectsINTEL(
    cl_command_queue command_queue,
    cl_uint          num_objects,
    const cl_mem *   mem_objects,
    cl_uint          num_events_in_wait_list,
    const cl_event * event_wait_list,
    cl_event *       ocl_event )
{
    cl_int errCode = CL_SUCCESS;

    if( command_queue == NULL )
    {
        errCode = CL_INVALID_COMMAND_QUEUE;
        goto FINISH;
    }

    errCode = ( (SOCLEntryPointsTable*)command_queue )->crtDispatch->clEnqueueReleaseDX9ObjectsINTEL(
                                        command_queue,
                                        num_objects,
                                        mem_objects,
                                        num_events_in_wait_list,
                                        event_wait_list,
                                        ocl_event);
    if( errCode == CL_SUCCESS )
    {
        goto FINISH;
    }

FINISH:
    return errCode;
}
SET_ALIAS( clEnqueueReleaseDX9ObjectsINTEL );

#endif //_WIN32
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
CL_API_ENTRY cl_command_queue CL_API_CALL clCreatePerfCountersCommandQueueINTEL(
    cl_context                   context,
    cl_device_id                 device,
    cl_command_queue_properties  properties,
    cl_uint                      configuration,
    cl_int *                     errcode_ret )
{
    cl_int errCode                  = CL_SUCCESS;
    cl_command_queue commandQueue   = NULL;

    if( ( context == NULL ) ||
        ( device == NULL ) )
    {
        errCode = CL_INVALID_CONTEXT;
        goto FINISH;
    }
    else
    {
        CrtDeviceInfo* devInfo = OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.GetValue( device );
        if( devInfo == NULL )
        {
            errCode = CL_INVALID_DEVICE;
            goto FINISH;
        }
        if( ( errCode == CL_SUCCESS )&&
            ( devInfo->m_devType == CL_DEVICE_TYPE_GPU ) )
        {
            CrtContextInfo* ctxInfo = OCLCRT::crt_ocl_module.m_contextInfoGuard.GetValue( context );
            if( ctxInfo == NULL)
            {
                errCode = CL_INVALID_CONTEXT;
                goto FINISH;
            }

            if( ctxInfo->m_contextType == CrtContextInfo::SinglePlatformGPUContext )
            {
                commandQueue = ( (SOCLEntryPointsTable*)context )->crtDispatch->clCreatePerfCountersCommandQueueINTEL(
                    context,
                    device,
                    properties,
                    configuration,
                    &errCode );
            }
            else
            {
                errCode = CL_INVALID_CONTEXT;
            }
        }
        else
        {
            errCode = CL_INVALID_DEVICE;
        }
    }
FINISH:
    if( errcode_ret )
    {
        *errcode_ret = errCode;
    }
    return commandQueue;
}
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_mem CL_API_CALL CreateFromGLBuffer(
    bool            is_render_buffer,
    cl_context      context,
    cl_mem_flags    flags,
    GLuint          bufobj,
    cl_int *        errcode_ret)
{
    _cl_mem_crt *mem_handle = NULL;
    cl_int errCode          = CL_SUCCESS;
    CrtContextInfo *ctxInfo = NULL;
    CrtContext* ctx         = NULL;

    ctxInfo = OCLCRT::crt_ocl_module.m_contextInfoGuard.GetValue(context);
    if (!ctxInfo)
    {
        errCode = CL_INVALID_CONTEXT;
        goto FINISH;
    }

    mem_handle = new _cl_mem_crt;
    if( !mem_handle )
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }

    ctx = (CrtContext*)(ctxInfo->m_object);
    errCode = ctx->CreateGLBuffer(
        is_render_buffer,
        flags,
        bufobj,
        (CrtMemObject**)(&mem_handle->object));

    if( CL_SUCCESS == errCode )
    {
        ((CrtMemObject*)(mem_handle->object))->SetMemHandle( mem_handle );
    }

FINISH:
    if( CL_SUCCESS != errCode )
    {
        delete mem_handle;
        mem_handle = NULL;
    }
    if( errcode_ret )
    {
        *errcode_ret = errCode;
    }

    return mem_handle;
}
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
CL_API_ENTRY cl_mem CL_API_CALL clCreateFromGLBuffer(
    cl_context      context,
    cl_mem_flags    flags,
    GLuint          bufobj,
    cl_int *        errcode_ret)
{
    return CreateFromGLBuffer(
        false,  // False = Not RenderBuffer
        context,
        flags,
        bufobj,
        errcode_ret);
}
SET_ALIAS( clCreateFromGLBuffer );
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
CL_API_ENTRY cl_mem CL_API_CALL clCreateFromGLRenderbuffer(
    cl_context      context,
    cl_mem_flags    flags,
    GLuint          renderbuffer,
    cl_int *        errcode_ret)
{
    return CreateFromGLBuffer(
        true,   // True = RenderBuffer
        context,
        flags,
        renderbuffer,
        errcode_ret);
}
SET_ALIAS( clCreateFromGLRenderbuffer );
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_mem CL_API_CALL CreateFromGLTexture(
    cl_context      context,
    cl_uint         dim_count,
    cl_mem_flags    flags,
    cl_GLenum       target,
    cl_GLint        miplevel,
    cl_GLuint       texture,
    cl_int *        errcode_ret)
{
    _cl_mem_crt* mem_handle     = NULL;
    cl_int errCode              = CL_SUCCESS;
    CrtContextInfo* ctxInfo     = NULL;
    CrtContext* ctx             = NULL;

    ctxInfo = OCLCRT::crt_ocl_module.m_contextInfoGuard.GetValue(context);
    if( !ctxInfo )
    {
        errCode = CL_INVALID_CONTEXT;
        goto FINISH;
    }

    mem_handle = new _cl_mem_crt;
    if( !mem_handle )
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }

    ctx = (CrtContext*)(ctxInfo->m_object);
    errCode = ctx->CreateGLImage(
        dim_count,
        flags,
        target,
        miplevel,
        texture,
        (CrtMemObject**)(&mem_handle->object));

FINISH:
    if( CL_SUCCESS != errCode )
    {
        if( mem_handle )
        {
            delete mem_handle;
            mem_handle = NULL;
        }
    }
    if( errcode_ret )
    {
        *errcode_ret = errCode;
    }
    return mem_handle;
}
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
CL_API_ENTRY cl_mem CL_API_CALL clCreateFromGLTexture(
    cl_context      context,
    cl_mem_flags    flags,
    GLenum          target,
    GLint           miplevel,
    GLuint          texture,
    cl_int *        errcode_ret)
{
    if( OCLCRT::crt_ocl_module.m_CrtPlatformVersion < OPENCL_1_2 )
    {
        if( errcode_ret )
        {
            *errcode_ret = CL_INVALID_DEVICE;
        }
        return NULL;
    }

    return CreateFromGLTexture(
        context,
        0,
        flags,
        target,
        miplevel,
        texture,
        errcode_ret);
}
SET_ALIAS( clCreateFromGLTexture );
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
CL_API_ENTRY cl_mem CL_API_CALL clCreateFromGLTexture2D(
    cl_context      context,
    cl_mem_flags    flags,
    GLenum          target,
    GLint           miplevel,
    GLuint          texture,
    cl_int *        errcode_ret)
{
    return CreateFromGLTexture(
        context,
        2,
        flags,
        target,
        miplevel,
        texture,
        errcode_ret);
}
SET_ALIAS( clCreateFromGLTexture2D );
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
CL_API_ENTRY cl_mem CL_API_CALL clCreateFromGLTexture3D(
    cl_context      context,
    cl_mem_flags    flags,
    GLenum          target,
    GLint           miplevel,
    GLuint          texture,
    cl_int *        errcode_ret)
{
    return CreateFromGLTexture(
        context,
        3,
        flags,
        target,
        miplevel,
        texture,
        errcode_ret);
}
SET_ALIAS( clCreateFromGLTexture3D );
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
CL_API_ENTRY cl_int CL_API_CALL clGetGLObjectInfo(
    cl_mem              memobj,
    cl_gl_object_type * gl_object_type,
    GLuint *            gl_object_name)
{
    if( NULL == memobj )
    {
        return CL_INVALID_MEM_OBJECT;
    }
    CrtMemObject* crtMemObj = reinterpret_cast<CrtMemObject*>(((_cl_mem_crt*)memobj)->object);
    cl_mem devMemObj = crtMemObj->getAnyValidDeviceMemObj();

    return devMemObj->dispatch->clGetGLObjectInfo(
        devMemObj,
        gl_object_type,
        gl_object_name);
}
SET_ALIAS( clGetGLObjectInfo );
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
CL_API_ENTRY cl_int CL_API_CALL clGetGLTextureInfo(
    cl_mem             memobj,
    cl_gl_texture_info param_name,
    size_t             param_value_size,
    void *             param_value,
    size_t *           param_value_size_ret)
{
    if( NULL == memobj )
    {
        return CL_INVALID_MEM_OBJECT;
    }
    CrtMemObject* crtMemObj = reinterpret_cast<CrtMemObject*>(((_cl_mem_crt*)memobj)->object);
    cl_mem devMemObj = crtMemObj->getAnyValidDeviceMemObj();

    return devMemObj->dispatch->clGetGLTextureInfo(
        devMemObj,
        param_name,
        param_value_size,
        param_value,
        param_value_size_ret);
}
SET_ALIAS( clGetGLTextureInfo );

// Defined CRT CL API
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
CL_API_ENTRY cl_int CL_API_CALL clEnqueueAcquireGLObjects(
    cl_command_queue command_queue,
    cl_uint          num_objects,
    const cl_mem     *mem_objects,
    cl_uint          num_events_in_wait_list,
    const cl_event   *event_wait_list,
    cl_event *       event )
{
    return EnqueueAcquireReleaseInteropObjects(
        CL_COMMAND_ACQUIRE_GL_OBJECTS,
        command_queue,
        num_objects,
        mem_objects,
        num_events_in_wait_list,
        event_wait_list,
        event);
}
SET_ALIAS( clEnqueueAcquireGLObjects );
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
CL_API_ENTRY cl_int CL_API_CALL clEnqueueReleaseGLObjects(
    cl_command_queue command_queue,
    cl_uint          num_objects,
    const cl_mem *   mem_objects,
    cl_uint          num_events_in_wait_list,
    const cl_event * event_wait_list,
    cl_event *       event )
{
    return EnqueueAcquireReleaseInteropObjects(
        CL_COMMAND_RELEASE_GL_OBJECTS,
        command_queue,
        num_objects,
        mem_objects,
        num_events_in_wait_list,
        event_wait_list,
        event);
}
SET_ALIAS( clEnqueueReleaseGLObjects );
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
CL_API_ENTRY cl_accelerator_intel CL_API_CALL clCreateAcceleratorINTEL(
                                       cl_context                   context,
                                       cl_accelerator_type_intel    type,
                                       size_t                       desc_size,
                                       const void *                 desc,
                                       cl_int *                     errcode_ret)
{
    cl_accelerator_intel acceleratorObj = NULL;
    cl_int errCode = CL_SUCCESS;

    CrtContextInfo* ctxInfo = OCLCRT::crt_ocl_module.m_contextInfoGuard.GetValue( context );
    if( ( !ctxInfo ) ||
        ( ctxInfo->m_contextType != CrtContextInfo::SinglePlatformGPUContext ) )
    {
        errCode = CL_INVALID_CONTEXT;
        goto FINISH;
    }

    acceleratorObj = ( (SOCLEntryPointsTable*)context )->crtDispatch->clCreateAcceleratorINTEL(
        context,
        type,
        desc_size,
        desc,
        &errCode);

FINISH:
    if( errcode_ret )
    {
        *errcode_ret = errCode;
    }

    return acceleratorObj;
}
SET_ALIAS( clCreateAcceleratorINTEL );

/******************************************************************************\

Function:
    clCreateProfiledProgramWithSourceINTEL

\******************************************************************************/
CL_API_ENTRY cl_program CL_API_CALL clCreateProfiledProgramWithSourceINTEL(
    cl_context          context,
    cl_uint             count,
    const char**        sources,
    const size_t*       lengths,
    const void*         configurations,
    cl_uint             configurations_count,
    cl_int*             error_code )
{
    cl_int errorCode = CL_SUCCESS;
    cl_program program = NULL;
    CrtContextInfo* pCtxInfo = OCLCRT::crt_ocl_module.m_contextInfoGuard.GetValue( context );

    if( NULL == pCtxInfo ||
        pCtxInfo->m_contextType != CrtContextInfo::SinglePlatformGPUContext )
    {
        errorCode = CL_INVALID_CONTEXT;
        goto FINISH;
    }

    program = ((SOCLEntryPointsTable*) context)->crtDispatch->clCreateProfiledProgramWithSourceINTEL(
        context,
        count,
        sources,
        lengths,
        configurations,
        configurations_count,
        &errorCode );

FINISH:
    if( error_code )
    {
        *error_code = errorCode;
    }
    return program;
}

/******************************************************************************\

Function:
    clCreateKernelProfilingJournalINTEL

\******************************************************************************/
CL_API_ENTRY cl_int CL_API_CALL clCreateKernelProfilingJournalINTEL(
    cl_context    context,
    const void*   configuration )
{
    cl_int errorCode = CL_SUCCESS;
    CrtContextInfo* pCtxInfo = OCLCRT::crt_ocl_module.m_contextInfoGuard.GetValue( context );

    if( NULL == pCtxInfo ||
        pCtxInfo->m_contextType != CrtContextInfo::SinglePlatformGPUContext )
    {
        errorCode = CL_INVALID_CONTEXT;
        goto FINISH;
    }

    errorCode = ((SOCLEntryPointsTable*) context)->crtDispatch->clCreateKernelProfilingJournalINTEL(
        context,
        configuration );

FINISH:
    return errorCode;
}

/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
CL_API_ENTRY cl_int CL_API_CALL clGetAcceleratorInfoINTEL(
                           cl_accelerator_intel         accelerator,
                           cl_accelerator_info_intel    param_name,
                           size_t                       param_value_size,
                           void *                       param_value,
                           size_t *                     param_value_size_ret)
{
    cl_int errCode = CL_SUCCESS;

    if( accelerator == NULL )
    {
        errCode = CL_INVALID_ACCELERATOR_INTEL;
        goto FINISH;
    }

    errCode = ( (SOCLEntryPointsTable*)accelerator )->crtDispatch->clGetAcceleratorInfoINTEL(
                           accelerator,
                           param_name,
                           param_value_size,
                           param_value,
                           param_value_size_ret );

FINISH:
    return errCode;
}
SET_ALIAS( clGetAcceleratorInfoINTEL );

/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clRetainAcceleratorINTEL( cl_accelerator_intel accelerator )
{
    cl_int errCode = CL_SUCCESS;

    if( accelerator == NULL )
    {
        errCode = CL_INVALID_ACCELERATOR_INTEL;
        goto FINISH;
    }

    errCode = ( (SOCLEntryPointsTable*)accelerator )->crtDispatch->clRetainAcceleratorINTEL( accelerator );

FINISH:
    return errCode;
}
SET_ALIAS( clRetainAcceleratorINTEL );

/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clReleaseAcceleratorINTEL( cl_accelerator_intel accelerator )
{
    cl_int errCode = CL_SUCCESS;

    if( accelerator == NULL )
    {
        errCode = CL_INVALID_ACCELERATOR_INTEL;
        goto FINISH;
    }

    errCode = ( (SOCLEntryPointsTable*)accelerator )->crtDispatch->clReleaseAcceleratorINTEL( accelerator );

FINISH:
    return errCode;
}
SET_ALIAS( clReleaseAcceleratorINTEL );

#ifdef LIBVA_SHARING

/******************************************************************************\

Function:
    clGetDeviceIDsFromVAMediaAdapterINTEL

\******************************************************************************/
CL_API_ENTRY cl_int CL_API_CALL clGetDeviceIDsFromVA_APIMediaAdapterINTEL(
        cl_platform_id                platform,
        cl_va_api_device_source_intel media_adapter_type,
        void                          *media_adapter,
        cl_va_api_device_set_intel    media_adapter_set,
        cl_uint                       num_entries,
        cl_device_id                  *devices,
        cl_uint                       *num_devices )
{
    cl_int errorCode = CL_SUCCESS;
    cl_int  ErrorCode   = CL_SUCCESS;
    VADisplay dpy       = NULL;

    if( !isValidPlatform( platform ) )
    {
        return CL_INVALID_PLATFORM;
    }

    if( media_adapter_type != CL_VA_API_DISPLAY_INTEL )
    {
            ErrorCode = CL_INVALID_VALUE;
            goto ERROR_HANDLER;
    }

    // Lock Devices Map, devices might be concurrently modified
    OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.Lock();

    for( OCLCRT::DEV_INFO_MAP::const_iterator itr = OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.get().begin();
         itr != OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.get().end();
         itr++ )
    {
        const cl_device_id& devIdDEV = itr->first;

        CrtDeviceInfo* devInfo = itr->second;

        if( num_devices != NULL )
        {
            *num_devices = 0;
        }

        if ( devInfo->m_devType != CL_DEVICE_TYPE_GPU )
        {
            continue;
        }

        switch( media_adapter_set )
        {
        case CL_PREFERRED_DEVICES_FOR_VA_API_INTEL:
        case CL_ALL_DEVICES_FOR_VA_API_INTEL:
            if( devices != NULL )
            {
                *devices = devIdDEV;
            }
            if( num_devices != NULL )
            {
                *num_devices = 1;
            }
            break;
        default:
            ErrorCode = CL_INVALID_VALUE;
            break;
        }
    }
    
ERROR_HANDLER:
    OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.Release();
    return ErrorCode;
}

/******************************************************************************\

Function:
     clEnqueueReleaseVA_APIMediaSurfacesINTEL

\******************************************************************************/
CL_API_ENTRY cl_int CL_API_CALL  clEnqueueReleaseVA_APIMediaSurfacesINTEL(
                                      cl_command_queue command_queue,
                                      cl_uint          num_objects,
                                      const cl_mem     *mem_objects,
                                      cl_uint          num_events_in_wait_list,
                                      const cl_event   *event_wait_list,
                                      cl_event         *event )
{
    cl_int errorCode = CL_SUCCESS;
    if( command_queue == NULL )
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    CrtQueue* queue = reinterpret_cast<CrtQueue*>(((_cl_command_queue_crt*)command_queue)->object);
    if( !queue )
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    errorCode = ( (SOCLEntryPointsTable*)queue->m_cmdQueueDEV )->crtDispatch->clEnqueueReleaseVA_APIMediaSurfacesINTEL(
                                                                command_queue,
                                                                num_objects,
                                                                mem_objects,
                                                                num_events_in_wait_list,
                                                                event_wait_list,
                                                                event );

    return errorCode;
}

/******************************************************************************\

Function:
    clEnqueueAcquireVA_APIMediaSurfacesINTEL

\******************************************************************************/
CL_API_ENTRY cl_int CL_API_CALL clEnqueueAcquireVA_APIMediaSurfacesINTEL(
                                    cl_command_queue command_queue,
                                    cl_uint          num_objects,
                                    const cl_mem     *mem_objects,
                                    cl_uint          num_events_in_wait_list,
                                    const cl_event   *event_wait_list,
                                    cl_event         *event )
{
    cl_int errorCode   = CL_SUCCESS;

    if( command_queue == NULL )
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    CrtQueue* queue = reinterpret_cast<CrtQueue*>(((_cl_command_queue_crt*)command_queue)->object);
    if( !queue )
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    errorCode = ( (SOCLEntryPointsTable*)queue->m_cmdQueueDEV )->crtDispatch->clEnqueueAcquireVA_APIMediaSurfacesINTEL(
                                                                    command_queue,
                                                                    num_objects,
                                                                    mem_objects,
                                                                    num_events_in_wait_list,
                                                                    event_wait_list,
                                                                    event );

    return errorCode;
}

/******************************************************************************\

Function:
    clCreateFromVA_APIMediaSurfaceINTEL

\******************************************************************************/
CL_API_ENTRY cl_mem CL_API_CALL clCreateFromVA_APIMediaSurfaceINTEL(
                                                    cl_context   context,
                                                    cl_mem_flags flags,
                                                    VASurfaceID  *surface,
                                                    cl_uint      plane,
                                                    cl_int       *errcode_ret )
{
    cl_int errorCode = CL_SUCCESS;
    cl_mem CLMem     = NULL;
    CrtContextInfo* pCtxInfo = OCLCRT::crt_ocl_module.m_contextInfoGuard.GetValue( context );

    if( NULL == pCtxInfo ||
        pCtxInfo->m_contextType != CrtContextInfo::SinglePlatformGPUContext )
    {
        errorCode = CL_INVALID_CONTEXT;
        goto FINISH;
    }

    CLMem = ( (SOCLEntryPointsTable* ) context)->crtDispatch->clCreateFromVA_APIMediaSurfaceINTEL(
                                                                        context,
                                                                        flags,
                                                                        surface,
                                                                        plane,
                                                                        &errorCode );

FINISH:
    if( errcode_ret )
    {
        *errcode_ret = errorCode;
    }
    
    return CLMem;
}

#endif

/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
#ifdef _WIN32
// This function enables us to determine if common-runtime
// supports named pipe.
// Returns CRT_FAIL on failure and CRT_SUCCESS on success.
int CL_API_CALL GetCRTInfo(
    crt_info    param_name,
    size_t      param_value_size,
    void *      param_value,
    size_t *    param_value_size_ret )
{
    if( ( NULL != param_value && 0 == param_value_size ) ||
        ( NULL == param_value && 0 != param_value_size ) )
    {
        return CRT_FAIL;
    }

    switch( param_name )
    {
    case CRT_NAMED_PIPE:
        if( NULL != param_value_size_ret )
        {
            *param_value_size_ret = sizeof(unsigned char);
        }
        if( NULL != param_value )
        {
            if( param_value_size < sizeof(unsigned char) )
            {
                return CRT_FAIL;
            }
            *(unsigned char*)param_value = true;
        }
        break;

    default:
        return CRT_FAIL;
    }

    return CRT_SUCCESS;
}
#endif

/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
CLAPI_EXPORT void * CL_API_CALL clGetExtensionFunctionAddress(
    const char *funcname)
{
    if( funcname && !strcmp( funcname, "clIcdGetPlatformIDsKHR" ) )
    {
        if( OCLCRT::Utils::isAPIDebuggingEnabled() )
        {
            // API debugging is enabled, refuse to load CRT library
            return NULL;
        }
        return ( ( void* )( ptrdiff_t )GET_ALIAS( clGetPlatformIDs ) );
    }
#ifdef _WIN32
    if( funcname && !strcmp( funcname, "clGetDeviceIDsFromDX9INTEL" ) )
    {
        return ( ( void* )( ptrdiff_t )GET_ALIAS( clGetDeviceIDsFromDX9INTEL ) );
    }
    if( funcname && !strcmp( funcname, "clCreateFromDX9MediaSurfaceINTEL" ) )
    {
        return ( ( void* )( ptrdiff_t )GET_ALIAS( clCreateFromDX9MediaSurfaceINTEL ) );
    }
    if( funcname && !strcmp( funcname, "clEnqueueAcquireDX9ObjectsINTEL" ) )
    {
        return ( ( void* )( ptrdiff_t )GET_ALIAS( clEnqueueAcquireDX9ObjectsINTEL ) );
    }
    if( funcname && !strcmp( funcname, "clEnqueueReleaseDX9ObjectsINTEL" ) )
    {
        return ( ( void* )( ptrdiff_t )GET_ALIAS( clEnqueueReleaseDX9ObjectsINTEL ) );
    }
    if( funcname && !strcmp( funcname, "clCreatePerfCountersCommandQueueINTEL" ) )
    {
        return ( ( void* )( ptrdiff_t )GET_ALIAS( clCreatePerfCountersCommandQueueINTEL ) );
    }
    if( funcname && !strcmp( funcname, "GetCRTInfo" ) )
    {
        return ( ( void* )( ptrdiff_t )GET_ALIAS( GetCRTInfo ) );
    }
#endif
// no accelerators for Android
#ifndef __ANDROID__
    if( funcname && !strcmp( funcname, "clCreateAcceleratorINTEL" ) )
    {
        return ( ( void* )( ptrdiff_t )GET_ALIAS( clCreateAcceleratorINTEL ) );
    }
    if( funcname && !strcmp( funcname, "clGetAcceleratorInfoINTEL" ) )
    {
        return ( ( void* )( ptrdiff_t )GET_ALIAS( clGetAcceleratorInfoINTEL ) );
    }
    if( funcname && !strcmp( funcname, "clRetainAcceleratorINTEL" ) )
    {
        return ( ( void* )( ptrdiff_t )GET_ALIAS( clRetainAcceleratorINTEL ) );
    }
    if( funcname && !strcmp( funcname, "clReleaseAcceleratorINTEL" ) )
    {
        return ( ( void* )( ptrdiff_t )GET_ALIAS( clReleaseAcceleratorINTEL ) );
    }
#endif //__ANDROID__
#ifdef _WIN32
    if( funcname && !strcmp( funcname, "clCreateProfiledProgramWithSourceINTEL" ) )
    {
        return ( ( void* )( ptrdiff_t )GET_ALIAS( clCreateProfiledProgramWithSourceINTEL ) );
    }
    if( funcname && !strcmp( funcname, "clCreateKernelProfilingJournalINTEL" ) )
    {
        return ( ( void* )( ptrdiff_t )GET_ALIAS( clCreateKernelProfilingJournalINTEL ) );
    }
#else
#ifdef LIBVA_SHARING
    if( funcname && !strcmp( funcname, "clGetDeviceIDsFromVA_APIMediaAdapterINTEL" ) )
    {
        return ( ( void* )( ptrdiff_t )( clGetDeviceIDsFromVA_APIMediaAdapterINTEL ) );
    }
    if( funcname && !strcmp( funcname, "clCreateFromVA_APIMediaSurfaceINTEL" ) )
    {
        return ( ( void* )( ptrdiff_t )( clCreateFromVA_APIMediaSurfaceINTEL ) );
    }
    if( funcname && !strcmp( funcname, "clEnqueueAcquireVA_APIMediaSurfacesINTEL" ) )
    {
        return ( ( void* )( ptrdiff_t )( clEnqueueAcquireVA_APIMediaSurfacesINTEL ) );
    }
    if( funcname && !strcmp( funcname, "clEnqueueReleaseVA_APIMediaSurfacesINTEL" ) )
    {
        return ( ( void* )( ptrdiff_t )( clEnqueueReleaseVA_APIMediaSurfacesINTEL ) );
    }
#endif
#endif
    return NULL;
}
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
CL_API_ENTRY void * CL_API_CALL clGetExtensionFunctionAddressForPlatform(
    cl_platform_id platform,
    const char* funcname)
{
    if( OCLCRT::crt_ocl_module.m_CrtPlatformVersion < OPENCL_1_2 )
    {
        return NULL;
    }
    if( ( platform != NULL) && !( isValidPlatform( platform ) ) )
    {
        return NULL;
    }
    return clGetExtensionFunctionAddress( funcname );
}

// TODO: move those up
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
void * CL_API_CALL clSVMAlloc(
    cl_context          context,
    cl_svm_mem_flags    flags,
    size_t              size,
    unsigned int        alignment )
{
    void*           pSvmPtr     = NULL;
    CrtContextInfo* pCtxInfo    = NULL;
    CrtContext*     pCrtCtx     = NULL;
    cl_context      gpuContext  = NULL;

    pCtxInfo = OCLCRT::crt_ocl_module.m_contextInfoGuard.GetValue( context );
    if( NULL == pCtxInfo )
    {
        // Invalid context
        goto FINISH;
    }

    if( 0 == size )
    {
        goto FINISH;
    }

    pCrtCtx = (CrtContext*)(pCtxInfo->m_object);

    // GPU handles SVM allocation
    gpuContext = pCrtCtx->GetContextByDeviceID( pCrtCtx->GetDeviceByType( CL_DEVICE_TYPE_GPU ) );
    if( NULL == gpuContext )
    {
        goto FINISH;
    }

    pSvmPtr = gpuContext->dispatch->clSVMAlloc(
                                        gpuContext,
                                        flags,
                                        size,
                                        alignment );

    // cache the SVM pointer
    if( NULL != pSvmPtr )
    {
        pCrtCtx->m_svmPointers.push_back( pSvmPtr );
    }

FINISH:
    return pSvmPtr;
}
SET_ALIAS( clSVMAlloc );

/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
void CL_API_CALL clSVMFree(
    cl_context context,
    void *     svm_pointer )
{
    CrtContextInfo* pCtxInfo    = NULL;
    CrtContext*     pCrtCtx     = NULL;
    cl_context      gpuContext  = NULL;

    pCtxInfo = OCLCRT::crt_ocl_module.m_contextInfoGuard.GetValue( context );
    if( NULL == pCtxInfo )
    {
        // Invalid context
        return;
    }

    if( NULL == svm_pointer )
    {
        return;
    }

    pCrtCtx = ( CrtContext* )( pCtxInfo->m_object );

    // GPU handles SVM allocation
    gpuContext = pCrtCtx->GetContextByDeviceID( pCrtCtx->GetDeviceByType( CL_DEVICE_TYPE_GPU ) );
    if( NULL == gpuContext )
    {
        return;
    }

    gpuContext->dispatch->clSVMFree(
                            gpuContext,
                            svm_pointer );

    // remove the SVM pointer from cache
    pCrtCtx->m_svmPointers.remove( svm_pointer );
}
SET_ALIAS( clSVMFree );

/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clEnqueueSVMFree(
    cl_command_queue        command_queue,
    cl_uint                 num_svm_pointers,
    void *                  svm_pointers[],
    pfn_free                pfn_free_func,
    void *                  user_data,
    cl_uint                 num_events_in_wait_list,
    const cl_event *        event_wait_list,
    cl_event *              event )
{
    cl_int                  errCode         = CL_SUCCESS;
    CrtEvent*               crtEvent        = NULL;
    SyncManager*            synchHelper     = NULL;
    cl_event*               outEvents       = NULL;
    cl_uint                 numOutEvents    = 0;
    CrtDeviceInfo*          devInfo         = NULL;
    SVMFreeCallbackData*    clbkData        = NULL;
    CrtQueue*               crtQueue        = NULL;
    _cl_event_crt*          event_handle    = NULL;
    cl_event                marker;

    if( NULL == command_queue )
    {
        errCode = CL_INVALID_COMMAND_QUEUE;
        goto FINISH;
    }

    if( 0 == num_svm_pointers || NULL == svm_pointers )
    {
        errCode = CL_INVALID_VALUE;
        goto FINISH;
    }
    for( cl_uint i = 0; i < num_svm_pointers; i++ )
    {
        if( NULL == svm_pointers[i] )
        {
            errCode = CL_INVALID_VALUE;
        }
    }

    crtQueue = reinterpret_cast<CrtQueue*>( ( ( _cl_command_queue_crt* )command_queue )->object );
    if( NULL == crtQueue )
    {
        errCode = CL_INVALID_COMMAND_QUEUE;
        goto FINISH;
    }

    synchHelper = new SyncManager;
    if( NULL == synchHelper )
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }

    errCode = synchHelper->PrepareToExecute(
        crtQueue,
        num_events_in_wait_list,
        event_wait_list,
        &numOutEvents,
        &outEvents );
    if( CL_SUCCESS != errCode )
    {
        goto FINISH;
    }

    // This event will be freed in the SVMFreeCallbackFunction if it's not returned to user
    crtEvent = new CrtEvent( crtQueue );
    if( NULL == crtEvent )
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }
    if( event )
    {
        event_handle = new _cl_event_crt;
        if( NULL == event_handle )
        {
            errCode = CL_OUT_OF_HOST_MEMORY;
            goto FINISH;
        }
        event_handle->object = ( void* )crtEvent;
    }

    // This will be freed in the SVMFreeCallbackFunction
    clbkData = new SVMFreeCallbackData();
    if( NULL == clbkData )
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }
    if( !clbkData->CopySVMPointers( svm_pointers, num_svm_pointers ) )
    {
        errCode = CL_OUT_OF_RESOURCES;
        goto FINISH;
    }
    clbkData->m_svmFreeUserEvent = crtEvent;
    clbkData->m_shouldReleaseEvent = ( NULL == event );
    clbkData->m_originalCallback = pfn_free_func;
    clbkData->m_originalUserData = user_data;

    devInfo = OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.GetValue( crtQueue->m_device );
    if( NULL == devInfo )
    {
        errCode = CL_INVALID_COMMAND_QUEUE;
        goto FINISH;
    }

    if( devInfo->m_devType == CL_DEVICE_TYPE_GPU )
    {
        // GPU's queue; forward call
        errCode = crtQueue->m_cmdQueueDEV->dispatch->clEnqueueSVMFree(
                                                        crtQueue->m_cmdQueueDEV,
                                                        num_svm_pointers,
                                                        svm_pointers,
                                                        pfn_free_func,
                                                        user_data,
                                                        numOutEvents,
                                                        outEvents,
                                                        &crtEvent->m_eventDEV );
        if( CL_SUCCESS != errCode )
        {
            goto FINISH;
        }

        // Need callback so we can remove the SVM pointers from cache
        clbkData->m_isGpuQueue = true;

        errCode = clSetEventCallback( crtEvent->m_eventDEV, CL_COMPLETE, SVMFreeCallbackFunction, clbkData );
        if( CL_SUCCESS != errCode )
        {
            errCode = CL_OUT_OF_RESOURCES;
            goto FINISH;
        }
    }
    else
    {
        // CPU's queue;
        // CPU cannot free SVM, we need the GPU to do it

        if( NULL == pfn_free_func )
        {
            std::list<void *> * svmPointers = &( crtQueue->m_contextCRT->m_svmPointers );

            // user didn't provide free function;
            // so all SVM pointers must have been created using clSVMAlloc
            for( cl_uint i = 0; i < num_svm_pointers; i++ )
            {
                if( svmPointers->end() == std::find( svmPointers->begin(), svmPointers->end(), svm_pointers[i] ) )
                {
                    // pointer was not found in cache
                    errCode = CL_INVALID_VALUE;
                    goto FINISH;
                }
            }
        }

        clbkData->m_isGpuQueue = false;

        crtEvent->m_eventDEV = clCreateUserEvent( crtQueue->m_contextCRT->m_context_handle, &errCode );
        if( CL_SUCCESS != errCode )
        {
            errCode = CL_OUT_OF_RESOURCES;
            goto FINISH;
        }
        errCode = clSetUserEventStatus( crtEvent->m_eventDEV, CL_QUEUED );
        if( CL_SUCCESS != errCode )
        {
            errCode = CL_OUT_OF_RESOURCES;
            goto FINISH;
        }

        errCode = crtQueue->m_cmdQueueDEV->dispatch->clEnqueueMarkerWithWaitList(
                                                            crtQueue->m_queue_handle,
                                                            numOutEvents,
                                                            outEvents,
                                                            &marker);
        if( CL_SUCCESS != errCode )
        {
            errCode = CL_OUT_OF_RESOURCES;
            goto FINISH;
        }

        errCode = clSetEventCallback( marker, CL_COMPLETE, SVMFreeCallbackFunction, clbkData );
        if( CL_SUCCESS != errCode )
        {
            errCode = CL_OUT_OF_RESOURCES;
            goto FINISH;
        }
    }

    // clSetEventCallback MUST be the last call that could fail
    // after successful clSetEventCallback no failures are accepted
    // because any other failure AFTER clSetEventCallback could result with double delete of clbkData
    // first delete is below in FINISH:
    // second delete is in SVMFreeCallbackFunction

    if( CL_SUCCESS == errCode && event )
    {
        *event = event_handle;
    }

FINISH:
    if( CL_SUCCESS != errCode && NULL != crtEvent )
    {
        crtEvent->Release();
        crtEvent->DecPendencyCnt();
    }
    if( CL_SUCCESS != errCode && NULL != event_handle )
    {
        delete event_handle;
    }
    if( synchHelper )
    {
        synchHelper->Release( errCode );
        delete synchHelper;
    }
    if( CL_SUCCESS != errCode && NULL != clbkData)
    {
        delete clbkData;
    }

    return errCode;
}
SET_ALIAS( clEnqueueSVMFree );

/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clEnqueueSVMMemcpy(
    cl_command_queue        command_queue,
    cl_bool                 blocking_copy,
    void *                  dst_ptr,
    const void *            src_ptr,
    size_t                  size,
    cl_uint                 num_events_in_wait_list,
    const cl_event *        event_wait_list,
    cl_event *              event )
{
    cl_int          errCode         = CL_SUCCESS;
    CrtEvent*       crtEvent        = NULL;
    SyncManager*    synchHelper     = NULL;
    cl_event*       outEvents       = NULL;
    cl_uint         numOutEvents    = 0;
    CrtQueue*       crtQueue        = NULL;

    if( NULL == command_queue )
    {
        errCode = CL_INVALID_COMMAND_QUEUE;
        goto FINISH;
    }

    if( NULL == dst_ptr || NULL == src_ptr )
    {
        errCode = CL_INVALID_VALUE;
        goto FINISH;
    }

    crtQueue = reinterpret_cast<CrtQueue*>( ( ( _cl_command_queue_crt* )command_queue )->object );
    if( NULL == crtQueue )
    {
        errCode = CL_INVALID_COMMAND_QUEUE;
        goto FINISH;
    }

    synchHelper = new SyncManager;
    if( NULL == synchHelper )
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }

    errCode = synchHelper->PrepareToExecute(
        crtQueue,
        num_events_in_wait_list,
        event_wait_list,
        &numOutEvents,
        &outEvents );
    if( CL_SUCCESS != errCode )
    {
        goto FINISH;
    }

    crtEvent = new CrtEvent(crtQueue);
    if( NULL == crtEvent )
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }

    if( blocking_copy )
    {
        errCode = crtQueue->m_contextCRT->FlushQueues();
        if( CL_SUCCESS != errCode )
        {
            errCode = CL_OUT_OF_RESOURCES;
            goto FINISH;
        }
    }

    errCode = crtQueue->m_cmdQueueDEV->dispatch->clEnqueueSVMMemcpy(
                                                    crtQueue->m_cmdQueueDEV,
                                                    blocking_copy,
                                                    dst_ptr,
                                                    src_ptr,
                                                    size,
                                                    numOutEvents,
                                                    outEvents,
                                                    &crtEvent->m_eventDEV );

    if( CL_SUCCESS == errCode && event )
    {
        _cl_event_crt* event_handle = new _cl_event_crt;
        if( NULL == event_handle )
        {
            errCode = CL_OUT_OF_HOST_MEMORY;
            goto FINISH;
        }
        event_handle->object = ( void* )crtEvent;
        *event = event_handle;
    }

FINISH:
    if( crtEvent && ( !event || ( CL_SUCCESS != errCode ) ) )
    {
        crtEvent->Release();
        crtEvent->DecPendencyCnt();
    }
    if( synchHelper )
    {
        synchHelper->Release( errCode );
        delete synchHelper;
    }
    return errCode;
}
SET_ALIAS( clEnqueueSVMMemcpy );

/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clEnqueueSVMMemFill(
    cl_command_queue        command_queue,
    void *                  svm_ptr,
    const void *            pattern,
    size_t                  pattern_size,
    size_t                  size,
    cl_uint                 num_events_in_wait_list,
    const cl_event *        event_wait_list,
    cl_event *              event )
{
    cl_int          errCode         = CL_SUCCESS;
    CrtEvent*       crtEvent        = NULL;
    SyncManager*    synchHelper     = NULL;
    cl_event*       outEvents       = NULL;
    cl_uint         numOutEvents    = 0;
    CrtQueue*       crtQueue        = NULL;

    if( NULL == command_queue )
    {
        errCode = CL_INVALID_COMMAND_QUEUE;
        goto FINISH;
    }

    if( NULL == svm_ptr )
    {
        errCode = CL_INVALID_VALUE;
        goto FINISH;
    }

    if( NULL == pattern || !IsPowerOf2( pattern_size ) || pattern_size > 128 )
    {
        errCode = CL_INVALID_VALUE;
        goto FINISH;
    }

    crtQueue = reinterpret_cast<CrtQueue*>( ( ( _cl_command_queue_crt* )command_queue )->object );
    if( NULL == crtQueue )
    {
        errCode = CL_INVALID_COMMAND_QUEUE;
        goto FINISH;
    }

    synchHelper = new SyncManager;
    if( !synchHelper )
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }

    errCode = synchHelper->PrepareToExecute(
        crtQueue,
        num_events_in_wait_list,
        event_wait_list,
        &numOutEvents,
        &outEvents );
    if( CL_SUCCESS != errCode )
    {
        goto FINISH;
    }

    crtEvent = new CrtEvent( crtQueue );
    if( NULL == crtEvent )
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }

    errCode = crtQueue->m_cmdQueueDEV->dispatch->clEnqueueSVMMemFill(
                                                    crtQueue->m_cmdQueueDEV,
                                                    svm_ptr,
                                                    pattern,
                                                    pattern_size,
                                                    size,
                                                    numOutEvents,
                                                    outEvents,
                                                    &crtEvent->m_eventDEV );

    if( CL_SUCCESS == errCode && event )
    {
        _cl_event_crt* event_handle = new _cl_event_crt;
        if( NULL == event_handle )
        {
            errCode = CL_OUT_OF_HOST_MEMORY;
            goto FINISH;
        }
        event_handle->object = ( void* )crtEvent;
        *event = event_handle;
    }

FINISH:
    if( crtEvent && ( !event || ( CL_SUCCESS != errCode ) ) )
    {
        crtEvent->Release();
        crtEvent->DecPendencyCnt();
    }
    if( synchHelper )
    {
        synchHelper->Release( errCode );
        delete synchHelper;
    }
    return errCode;
}
SET_ALIAS( clEnqueueSVMMemFill );

/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clEnqueueSVMMap(
    cl_command_queue        command_queue,
    cl_bool                 blocking_map,
    cl_map_flags            map_flags,
    void *                  svm_ptr,
    size_t                  size,
    cl_uint                 num_events_in_wait_list,
    const cl_event *        event_wait_list,
    cl_event *              event )
{
    cl_int          errCode         = CL_SUCCESS;
    CrtEvent*       crtEvent        = NULL;
    SyncManager*    synchHelper     = NULL;
    cl_event*       outEvents       = NULL;
    cl_uint         numOutEvents    = 0;
    CrtQueue*       crtQueue        = NULL;

    if( NULL == command_queue )
    {
        errCode = CL_INVALID_COMMAND_QUEUE;
        goto FINISH;
    }

    if( NULL == svm_ptr || 0 == size )
    {
        errCode = CL_INVALID_VALUE;
        goto FINISH;
    }

    crtQueue = reinterpret_cast<CrtQueue*>( ( ( _cl_command_queue_crt* )command_queue )->object );
    if( NULL == crtQueue )
    {
        errCode = CL_INVALID_COMMAND_QUEUE;
        goto FINISH;
    }

    synchHelper = new SyncManager;
    if( NULL == synchHelper )
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }

    errCode = synchHelper->PrepareToExecute(
        crtQueue,
        num_events_in_wait_list,
        event_wait_list,
        &numOutEvents,
        &outEvents );
    if( CL_SUCCESS != errCode )
    {
        goto FINISH;
    }

    crtEvent = new CrtEvent( crtQueue );
    if( NULL == crtEvent )
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }

    if( blocking_map )
    {
        errCode = crtQueue->m_contextCRT->FlushQueues();
        if( CL_SUCCESS != errCode )
        {
            errCode = CL_OUT_OF_RESOURCES;
            goto FINISH;
        }
    }

    errCode = crtQueue->m_cmdQueueDEV->dispatch->clEnqueueSVMMap(
                                                    crtQueue->m_cmdQueueDEV,
                                                    blocking_map,
                                                    map_flags,
                                                    svm_ptr,
                                                    size,
                                                    numOutEvents,
                                                    outEvents,
                                                    &crtEvent->m_eventDEV );

    if( CL_SUCCESS == errCode && event )
    {
        _cl_event_crt* event_handle = new _cl_event_crt;
        if( NULL == event_handle )
        {
            errCode = CL_OUT_OF_HOST_MEMORY;
            goto FINISH;
        }
        event_handle->object = ( void* )crtEvent;
        *event = event_handle;
    }

FINISH:
    if( crtEvent && ( !event || ( CL_SUCCESS != errCode ) ) )
    {
        crtEvent->Release();
        crtEvent->DecPendencyCnt();
    }
    if( synchHelper )
    {
        synchHelper->Release( errCode );
        delete synchHelper;
    }
    return errCode;
}
SET_ALIAS( clEnqueueSVMMap );

/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clEnqueueSVMUnmap(
    cl_command_queue        command_queue,
    void *                  svm_ptr,
    cl_uint                 num_events_in_wait_list,
    const cl_event *        event_wait_list,
    cl_event *              event )
{
    cl_int          errCode         = CL_SUCCESS;
    CrtEvent*       crtEvent        = NULL;
    SyncManager*    synchHelper     = NULL;
    cl_event*       outEvents       = NULL;
    cl_uint         numOutEvents    = 0;
    CrtQueue*       crtQueue        = NULL;

    if( NULL == command_queue )
    {
        errCode = CL_INVALID_COMMAND_QUEUE;
        goto FINISH;
    }

    if( NULL == svm_ptr )
    {
        errCode = CL_INVALID_VALUE;
        goto FINISH;
    }

    crtQueue = reinterpret_cast<CrtQueue*>( ( ( _cl_command_queue_crt* )command_queue )->object );
    if( NULL == crtQueue )
    {
        errCode = CL_INVALID_COMMAND_QUEUE;
        goto FINISH;
    }

    synchHelper = new SyncManager;
    if( NULL == synchHelper )
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }

    errCode = synchHelper->PrepareToExecute(
        crtQueue,
        num_events_in_wait_list,
        event_wait_list,
        &numOutEvents,
        &outEvents );
    if( CL_SUCCESS != errCode )
    {
        goto FINISH;
    }

    crtEvent = new CrtEvent( crtQueue );
    if( NULL == crtEvent )
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }

    errCode = crtQueue->m_cmdQueueDEV->dispatch->clEnqueueSVMUnmap(
                                                    crtQueue->m_cmdQueueDEV,
                                                    svm_ptr,
                                                    numOutEvents,
                                                    outEvents,
                                                    &crtEvent->m_eventDEV );

    if( CL_SUCCESS == errCode && event )
    {
        _cl_event_crt* event_handle = new _cl_event_crt;
        if( NULL == event_handle )
        {
            errCode = CL_OUT_OF_HOST_MEMORY;
            goto FINISH;
        }
        event_handle->object = ( void* )crtEvent;
        *event = event_handle;
    }

FINISH:
    if( crtEvent && ( !event || ( CL_SUCCESS != errCode ) ) )
    {
        crtEvent->Release();
        crtEvent->DecPendencyCnt();
    }
    if( synchHelper )
    {
        synchHelper->Release( errCode );
        delete synchHelper;
    }
    return errCode;
}
SET_ALIAS( clEnqueueSVMUnmap );

/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clSetKernelArgSVMPointer(
    cl_kernel               kernel,
    cl_uint                 arg_index,
    const void *            arg_value )
{
    cl_int      errCode     = CL_SUCCESS;
    bool        succeed     = false;
    CrtKernel*  crtKernel   = NULL;
    CTX_KRN_MAP::iterator   itr;

    if( NULL == kernel )
    {
        errCode = CL_INVALID_KERNEL;
        goto FINISH;
    }

    crtKernel = reinterpret_cast<CrtKernel*>( ( ( _cl_kernel_crt* )kernel )->object );
    if( NULL == crtKernel )
    {
        errCode = CL_INVALID_KERNEL;
        goto FINISH;
    }

    itr = crtKernel->m_ContextToKernel.begin();
    for( ;itr != crtKernel->m_ContextToKernel.end(); itr++ )
    {
        errCode = itr->second->dispatch->clSetKernelArgSVMPointer(
                itr->second,
                arg_index,
                arg_value );

        if( CL_SUCCESS == errCode )
        {
            succeed = true;
        }
    }

FINISH:
    if( succeed )
    {
        errCode = CL_SUCCESS;
    }

    return errCode;
}
SET_ALIAS( clSetKernelArgSVMPointer );

/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clSetKernelExecInfo(
    cl_kernel               kernel,
    cl_kernel_exec_info     param_name,
    size_t                  param_value_size,
    const void *            param_value )
{
    cl_int      errCode     = CL_SUCCESS;
    CrtKernel*  crtKernel   = NULL;
    CrtContext* crtCtx      = NULL;
    cl_context  gpuCtx      = NULL;
    cl_kernel   gpuKernel   = NULL;

    if( NULL == kernel )
    {
        errCode = CL_INVALID_KERNEL;
        goto FINISH;
    }

    if( NULL == param_value )
    {
        errCode = CL_INVALID_VALUE;
        goto FINISH;
    }

    crtKernel = reinterpret_cast<CrtKernel*>( ( ( _cl_kernel_crt* )kernel )->object );
    if( NULL == crtKernel )
    {
        errCode = CL_INVALID_KERNEL;
        goto FINISH;
    }

    crtCtx      = crtKernel->m_programCRT->m_contextCRT;
    gpuCtx      = crtCtx->GetContextByDeviceID( crtCtx->GetDeviceByType( CL_DEVICE_TYPE_GPU ) );
    gpuKernel   = crtKernel->m_ContextToKernel[ gpuCtx ];

    // call GPU's API only
    errCode = gpuKernel->dispatch->clSetKernelExecInfo(
                                        gpuKernel,
                                        param_name,
                                        param_value_size,
                                        param_value );

FINISH:
    return errCode;
}
SET_ALIAS( clSetKernelExecInfo );
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_mem CL_API_CALL clCreatePipe(
    cl_context                  context,
    cl_mem_flags                flags,
    cl_uint                     pipe_packet_size,
    cl_uint                     pipe_max_packets,
    const cl_pipe_properties *  properties,
    cl_int *                    errcode_ret )
{
    cl_int          errCode     = CL_SUCCESS;
    _cl_mem_crt*    mem_handle  = NULL;
    CrtContextInfo* ctxInfo     = NULL;
    CrtContext*     ctx         = NULL;

    ctxInfo = OCLCRT::crt_ocl_module.m_contextInfoGuard.GetValue( context );
    if( NULL == ctxInfo )
    {
        errCode = CL_INVALID_CONTEXT;
        goto FINISH;
    }

    mem_handle = new _cl_mem_crt;
    if( NULL == mem_handle )
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }

    ctx = ( CrtContext* )( ctxInfo->m_object );
    errCode = ctx->CreatePipe(
                        flags,
                        pipe_packet_size,
                        pipe_max_packets,
                        properties,
                        ( CrtMemObject** )( &mem_handle->object ) );

    if( CL_SUCCESS == errCode )
    {
        ( ( CrtMemObject* )( mem_handle->object ) )->SetMemHandle( mem_handle );
    }

FINISH:
    if( CL_SUCCESS != errCode )
    {
        delete mem_handle;
        mem_handle = NULL;
    }
    if( NULL != errcode_ret )
    {
        *errcode_ret = errCode;
    }

    return mem_handle;
}
SET_ALIAS( clCreatePipe );
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CL_API_CALL clGetPipeInfo(
    cl_mem          pipe,
    cl_pipe_info    param_name,
    size_t          param_value_size,
    void *          param_value,
    size_t *        param_value_size_ret )
{
    cl_int      errCode = CL_SUCCESS;
    CrtPipe*    crtPipe = NULL;

    if( NULL == pipe )
    {
        return CL_INVALID_MEM_OBJECT;
    }

    crtPipe = reinterpret_cast<CrtPipe*>( ( ( _cl_mem_crt* )pipe )->object );
    if( NULL == crtPipe )
    {
        return CL_INVALID_MEM_OBJECT;
    }

    cl_mem devMemObj = crtPipe->getAnyValidDeviceMemObj();
    errCode = devMemObj->dispatch->clGetMemObjectInfo(
                                        devMemObj,
                                        param_name,
                                        param_value_size,
                                        param_value,
                                        param_value_size_ret );

    return errCode;
}
SET_ALIAS( clGetPipeInfo );