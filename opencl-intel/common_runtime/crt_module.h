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
#pragma once

#include "crt_internals.h"
#include <crt_types.h>
#include <crt_config.h>
#include <cl_synch_objects.h>
#include <string>
#include <vector>
#include <map>


using namespace CRT_ICD_DISPATCH;


namespace OCLCRT
{
    namespace SYNCH {

        enum MEM_RESIDENCY
        {
            SYNC_BUFFER_WITH_HOST   = 0x1,  // means device need synchronization when need to read/write to/from host
            SYNC_IMAGE_WITH_HOST    = 0x2   // means device need synchronization when need to read/write to/from host
        };

        enum DEVICE_SYNC_ATTRIBUTES
        {
            INTEL_GPU   =   0x2,    // For Images, means Intel GPU needs synchronization; For Buffer, it doesn't need to synchronize
            INTEL_CPU   =   0x0     // For Images and Buffers, there is not need to synchronize
        };
    }

    class IcdDispatchMgr
    {
    public:
        IcdDispatchMgr();
        KHRicdVendorDispatch    m_icdDispatchTable;
    };

    typedef std::map<cl_device_id,  CrtDeviceInfo*>     DEV_INFO_MAP;
    typedef std::map<cl_context,    CrtContextInfo*>    CTX_INFO_MAP;

    bool isSupportedContextType(const cl_context_properties* properties, cl_uint num_devices, const cl_device_id *devices);

    // Fixes the properties flag passed from the app to match the underlying platform properties
    // Like cl_platform_id need to be fixed.
    crt_err_code ReplacePlatformId( const cl_context_properties*    src_properties,
                                    cl_platform_id&                 pId,
                                    cl_context_properties**         dst_props,
                                    bool                            duplicateProps = true);



    class CrtModule
    {
    public:
        enum INIT_STATE {
            // Common Runtime has not been initialized yet
            NOT_INITIALIZED,
            // Common Runtime initialization went OK
            INITIALIZE_OK,
            // Common Runtime failed initializations.
            INITIALIZE_ERROR
        };

        CrtModule();
        crt_err_code Initialize();
        void         Shutdown();

        cl_int       isValidProperties(const cl_context_properties* properties);
        ~CrtModule();

        // Patches underlying device id allowing the CRT
        // to intercept some of the CL calls.
        crt_err_code PatchClDeviceID(cl_device_id& inDeviceId);
        crt_err_code PatchClContextID(cl_context& inContextId, KHRicdVendorDispatch* origDispatchTable);

        // Common Runtime platform id
        cl_platform_id  m_crtPlatformId;

        // Common runtime Dispatch table manager.
        IcdDispatchMgr  m_icdDispatchMgr;

        // all underlying managed platforms
        std::vector<CrtPlatform*>               m_oclPlatforms;

        // All underlying managed devices (including
        // any created sub devices)
        GuardedMap<cl_device_id,CrtDeviceInfo*> m_deviceInfoMapGuard;


        // MAPs for each context (single/shared platform contexts)
        // an info data structure
        GuardedMap<cl_context,CrtContextInfo*>  m_contextInfoGuard;


        // mutex gaurding CRT data structure modifying
        Utils::OclMutex                         m_mutex;

        // default device type
        cl_device_type                          m_defaultDeviceType;
        cl_device_type                          m_availableDeviceTypes;

        static char*                            m_common_extensions;

        // 11 - stands for OpenCL 1.1
        // 12 - stands for OpenCL 1.2
        cl_uint                                 m_CrtPlatformVersion;
    private:

        CrtConfig                               m_crtConfig;
        INIT_STATE                              m_initializeState;
        DEV_INFO_MAP                            m_deviceInfoMap;
        CTX_INFO_MAP                            m_contextInfo;
    };


}
