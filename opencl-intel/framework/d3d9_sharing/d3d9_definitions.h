// Copyright (c) 2006-2012 Intel Corporation
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

#include "ocl_supported_extensions.h"

namespace Intel { namespace OpenCL { namespace Framework {

/**
 * This class defines an interface to obtaining Direct3D 9 Media Sharing definitions regardless of
 * the version of the extension (Intel or Khronos). In the future this will be extended to support
 * also Direct3D 10 Sharing.
 */
class ID3D9Definitions
{

public:

    /**
     * enumeration of the various versions of the extension
     */
    enum D3D9Version {
        D3D9_KHR,
        D3D9_INTEL
    };

    /**
     * @return the version of the extension in use
     */
    virtual D3D9Version GetVersion() const = 0;

    /**
     * @return the name of the extension as presented in the CL_PLATFORM_EXTENSIONS or CL_DEVICE_EXTENSIONS strings
     */
    virtual std::string GetExtensionName() const = 0;

    virtual int GetAdapterD3d9() const = 0;

    virtual int GetAdapterD3d9Ex() const = 0;

    virtual int GetAdapterDxva() const = 0;

    virtual int GetPreferredDevicsForDx9MediaAdapter() const = 0;

    virtual int GetAllDevicesForDx9MediaAdapter() const = 0;

    virtual int GetContextAdapterD3d9() const = 0;

    virtual int GetContextAdapterD3d9Ex() const = 0;

    virtual int GetContextAdapterDxva() const = 0;

    virtual int GetImageDx9MediaPlane() const = 0;

    virtual int GetCommandAcquireDx9MediaSurface() const = 0;

    virtual int GetCommandReleaseDx9MediaSurface() const = 0;

    virtual int GetInvalidDx9MediaAdapter() const = 0;

    virtual int GetInvalidDx9MediaSurface() const = 0;

    virtual int GetDx9MediaSurfaceAlreadyAcquired() const = 0;

    virtual int GetDx9MediaSurfaceNotAcquired() const = 0;

};

/**
 * This class implements ID3D9Definitions for the Khronos version of the extension
 */
class KhrD3D9Definitions : public ID3D9Definitions
{

public:

    D3D9Version GetVersion() const { return D3D9_KHR; }

    std::string GetExtensionName() const { return OCL_KHR_DX9_MEDIA_SHARING_EXT; }

    int GetAdapterD3d9() const { return CL_ADAPTER_D3D9_KHR; }

    int GetAdapterD3d9Ex() const { return CL_ADAPTER_D3D9EX_KHR; }

    int GetAdapterDxva() const { return CL_ADAPTER_DXVA_KHR; }

    int GetPreferredDevicsForDx9MediaAdapter() const { return CL_PREFERRED_DEVICES_FOR_DX9_MEDIA_ADAPTER_KHR; }

    int GetAllDevicesForDx9MediaAdapter() const { return CL_ALL_DEVICES_FOR_DX9_MEDIA_ADAPTER_KHR; }

    int GetContextAdapterD3d9() const { return CL_CONTEXT_ADAPTER_D3D9_KHR; }

    int GetContextAdapterD3d9Ex() const { return CL_CONTEXT_ADAPTER_D3D9EX_KHR; }

    int GetContextAdapterDxva() const { return CL_CONTEXT_ADAPTER_DXVA_KHR; }

    int GetImageDx9MediaPlane() const { return CL_IMAGE_DX9_MEDIA_PLANE_KHR; }

    int GetCommandAcquireDx9MediaSurface() const { return CL_COMMAND_ACQUIRE_DX9_MEDIA_SURFACES_KHR; }

    int GetCommandReleaseDx9MediaSurface() const { return CL_COMMAND_RELEASE_DX9_MEDIA_SURFACES_KHR; }

    int GetInvalidDx9MediaAdapter() const { return CL_INVALID_DX9_MEDIA_ADAPTER_KHR; }

    int GetInvalidDx9MediaSurface() const { return CL_INVALID_DX9_MEDIA_SURFACE_KHR; }

    int GetDx9MediaSurfaceAlreadyAcquired() const { return CL_DX9_MEDIA_SURFACE_ALREADY_ACQUIRED_KHR; }

    int GetDx9MediaSurfaceNotAcquired() const { return CL_DX9_MEDIA_SURFACE_NOT_ACQUIRED_KHR; }
};

/**
 * This class implements ID3D9Definitions for the Intel version of the extension
 */
class IntelD3D9Definitions : public ID3D9Definitions
{

public:

    D3D9Version GetVersion() const { return D3D9_INTEL; }

    std::string GetExtensionName() const { return OCL_INTEL_DX9_MEDIA_SHARING_EXT; }

    int GetAdapterD3d9() const { return CL_D3D9_DEVICE_INTEL; }

    int GetAdapterD3d9Ex() const { return CL_D3D9EX_DEVICE_INTEL; }

    int GetAdapterDxva() const { return CL_DXVA_DEVICE_INTEL; }

    int GetPreferredDevicsForDx9MediaAdapter() const { return CL_PREFERRED_DEVICES_FOR_DX9_INTEL; }

    int GetAllDevicesForDx9MediaAdapter() const { return CL_ALL_DEVICES_FOR_DX9_INTEL; }

    int GetContextAdapterD3d9() const { return CL_CONTEXT_D3D9_DEVICE_INTEL; }

    int GetContextAdapterD3d9Ex() const { return CL_CONTEXT_D3D9EX_DEVICE_INTEL; }

    int GetContextAdapterDxva() const { return CL_CONTEXT_DXVA_DEVICE_INTEL; }

    int GetImageDx9MediaPlane() const { return CL_IMAGE_DX9_PLANE_INTEL; }

    int GetCommandAcquireDx9MediaSurface() const { return CL_COMMAND_ACQUIRE_DX9_OBJECTS_INTEL; }

    int GetCommandReleaseDx9MediaSurface() const { return CL_COMMAND_RELEASE_DX9_OBJECTS_INTEL; }

    int GetInvalidDx9MediaAdapter() const { return CL_INVALID_DX9_DEVICE_INTEL; }

    int GetInvalidDx9MediaSurface() const { return CL_INVALID_DX9_RESOURCE_INTEL; }

    int GetDx9MediaSurfaceAlreadyAcquired() const { return CL_DX9_RESOURCE_ALREADY_ACQUIRED_INTEL; }

    int GetDx9MediaSurfaceNotAcquired() const { return CL_DX9_RESOURCE_NOT_ACQUIRED_INTEL; }
};

}}}
