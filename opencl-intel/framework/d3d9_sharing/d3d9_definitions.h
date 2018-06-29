// INTEL CONFIDENTIAL
//
// Copyright 2006-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#pragma once

#include "ocl_supported_extensions.h"
#include "MemoryObjectFactory.h"

namespace Intel { namespace OpenCL { namespace Framework {

/**
 * This class defines an interface to obtaining Direct3D Sharing definitions regardless of the
 * version of the extension (Intel or Khronos Direct3D 9 or Direct3D 11).
 */
class ID3DSharingDefinitions
{

public:

    /**
     * enumeration of the various versions of the extension
     */
    enum D3DSharingVersion {
        D3D9_KHR,
        D3D9_INTEL,
        D3D11
    };

    /**
     * @return the version of the extension in use
     */
    virtual D3DSharingVersion GetVersion() const = 0;

    /**
     * @return the name of the extension as presented in the CL_PLATFORM_EXTENSIONS or CL_DEVICE_EXTENSIONS strings
     */
    virtual std::string GetExtensionName() const = 0;

    /**
     * @return a vector of the valid device types of this extension
     */
    virtual std::vector<int> GetValidDeviceTypes() const = 0;

    /**
     * @return whether this version takes into consideration the CL_CONTEXT_INTEROP_USER_SYNC when creating the context
     */
    virtual bool IsUsingContextInteropUserSync() const = 0;

    /**
     * @return the graphics system sharing type of objects from this version of the extension
     */
    virtual int GetGfxSysSharing() const = 0;

    // macro getters:

    virtual int GetPreferredDevicesForD3D() const = 0;

    virtual int GetAllDevicesForD3D() const = 0;

    virtual int GetContextDevice() const = 0;    

    virtual int GetCommandAcquireDevice() const = 0;

    virtual int GetCommandReleaseDevice() const = 0;

    virtual int GetInvalidDevice() const = 0;

    virtual int GetInvalidResource() const = 0;

    virtual int GetResourceAlreadyAcquired() const = 0;

    virtual int GetResourceNotAcquired() const = 0;

    virtual ~ID3DSharingDefinitions() {}

};

/**
 * This class extends ID3DSharingDefinitions to add macro getters for DX9 Media Sharing
 */
class ID3D9Definitions : public ID3DSharingDefinitions
{

public:

    virtual int GetAdapterD3d9() const = 0;

    virtual int GetAdapterD3d9Ex() const = 0;

    virtual int GetAdapterDxva() const = 0;

    virtual int GetContextAdapterD3d9Ex() const = 0;

    virtual int GetContextAdapterDxva() const = 0;

    virtual int GetImageDx9MediaPlane() const = 0;

    virtual int GetGfxSysSharing() const { return CL_MEMOBJ_GFX_SHARE_DX9; }

    ~ID3D9Definitions() {};

};

/**
 * This class implements ID3DSharingDefinitions for the Khronos version of the extension
 */
class KhrD3D9Definitions : public ID3D9Definitions
{

public:

    D3DSharingVersion GetVersion() const { return D3D9_KHR; }

    std::string GetExtensionName() const { return OCL_EXT_KHR_DX9_MEDIA_SHARING; }

    std::vector<int> GetValidDeviceTypes() const
    {
        static const int validDeviceTypes[] = {CL_ADAPTER_D3D9_KHR, CL_ADAPTER_D3D9EX_KHR, CL_ADAPTER_DXVA_KHR};
        return std::vector<int>(validDeviceTypes, &validDeviceTypes[sizeof(validDeviceTypes) / sizeof(validDeviceTypes[0])]);
    }

    bool IsUsingContextInteropUserSync() const { return true; }

    int GetAdapterD3d9() const { return CL_ADAPTER_D3D9_KHR; }

    int GetAdapterD3d9Ex() const { return CL_ADAPTER_D3D9EX_KHR; }

    int GetAdapterDxva() const { return CL_ADAPTER_DXVA_KHR; }

    int GetPreferredDevicesForD3D() const { return CL_PREFERRED_DEVICES_FOR_DX9_MEDIA_ADAPTER_KHR; }

    int GetAllDevicesForD3D() const { return CL_ALL_DEVICES_FOR_DX9_MEDIA_ADAPTER_KHR; }

    int GetContextDevice() const { return CL_CONTEXT_ADAPTER_D3D9_KHR; }

    int GetContextAdapterD3d9Ex() const { return CL_CONTEXT_ADAPTER_D3D9EX_KHR; }

    int GetContextAdapterDxva() const { return CL_CONTEXT_ADAPTER_DXVA_KHR; }

    int GetImageDx9MediaPlane() const { return CL_IMAGE_DX9_MEDIA_PLANE_KHR; }

    int GetCommandAcquireDevice() const { return CL_COMMAND_ACQUIRE_DX9_MEDIA_SURFACES_KHR; }

    int GetCommandReleaseDevice() const { return CL_COMMAND_RELEASE_DX9_MEDIA_SURFACES_KHR; }

    int GetInvalidDevice() const { return CL_INVALID_DX9_MEDIA_ADAPTER_KHR; }

    int GetInvalidResource() const { return CL_INVALID_DX9_MEDIA_SURFACE_KHR; }

    int GetResourceAlreadyAcquired() const { return CL_DX9_MEDIA_SURFACE_ALREADY_ACQUIRED_KHR; }

    int GetResourceNotAcquired() const { return CL_DX9_MEDIA_SURFACE_NOT_ACQUIRED_KHR; }

    ~KhrD3D9Definitions() {};

};

/**
 * This class implements ID3DSharingDefinitions for the Intel version of the extension
 */
class IntelD3D9Definitions : public ID3D9Definitions
{

public:

    D3DSharingVersion GetVersion() const { return D3D9_INTEL; }

    std::string GetExtensionName() const { return OCL_EXT_INTEL_DX9_MEDIA_SHARING; }

    std::vector<int> GetValidDeviceTypes() const
    {
        static const int validDeviceTypes[] = {CL_D3D9_DEVICE_INTEL, CL_D3D9EX_DEVICE_INTEL, CL_DXVA_DEVICE_INTEL};
        return std::vector<int>(validDeviceTypes, &validDeviceTypes[sizeof(validDeviceTypes) / sizeof(validDeviceTypes[0])]);
    }

    bool IsUsingContextInteropUserSync() const { return false; }

    int GetAdapterD3d9() const { return CL_D3D9_DEVICE_INTEL; }

    int GetAdapterD3d9Ex() const { return CL_D3D9EX_DEVICE_INTEL; }

    int GetAdapterDxva() const { return CL_DXVA_DEVICE_INTEL; }

    int GetPreferredDevicesForD3D() const { return CL_PREFERRED_DEVICES_FOR_DX9_INTEL; }

    int GetAllDevicesForD3D() const { return CL_ALL_DEVICES_FOR_DX9_INTEL; }

    int GetContextDevice() const { return CL_CONTEXT_D3D9_DEVICE_INTEL; }

    int GetContextAdapterD3d9Ex() const { return CL_CONTEXT_D3D9EX_DEVICE_INTEL; }

    int GetContextAdapterDxva() const { return CL_CONTEXT_DXVA_DEVICE_INTEL; }

    int GetImageDx9MediaPlane() const { return CL_IMAGE_DX9_PLANE_INTEL; }

    int GetCommandAcquireDevice() const { return CL_COMMAND_ACQUIRE_DX9_OBJECTS_INTEL; }

    int GetCommandReleaseDevice() const { return CL_COMMAND_RELEASE_DX9_OBJECTS_INTEL; }

    int GetInvalidDevice() const { return CL_INVALID_DX9_DEVICE_INTEL; }

    int GetInvalidResource() const { return CL_INVALID_DX9_RESOURCE_INTEL; }

    int GetResourceAlreadyAcquired() const { return CL_DX9_RESOURCE_ALREADY_ACQUIRED_INTEL; }

    int GetResourceNotAcquired() const { return CL_DX9_RESOURCE_NOT_ACQUIRED_INTEL; }

    ~IntelD3D9Definitions() {};

};

/**
 * This class implements ID3DSharingDefinitions for Direct3D11 Sharing
 */
class D3D11Definitions : public ID3DSharingDefinitions
{

public:

    D3DSharingVersion GetVersion() const { return D3D11; }

    std::string GetExtensionName() const { return OCL_EXT_KHR_D3D11_SHARING; }

    std::vector<int> GetValidDeviceTypes() const
    {
        static const int validDeviceTypes[] = {CL_D3D11_DEVICE_KHR, CL_D3D11_DXGI_ADAPTER_KHR};
        return std::vector<int>(validDeviceTypes, &validDeviceTypes[sizeof(validDeviceTypes) / sizeof(validDeviceTypes[0])]);
    }

    bool IsUsingContextInteropUserSync() const { return true; }

    virtual int GetGfxSysSharing() const { return CL_MEMOBJ_GFX_SHARE_DX11; }

    int GetPreferredDevicesForD3D() const { return CL_PREFERRED_DEVICES_FOR_D3D11_KHR; }

    int GetAllDevicesForD3D() const { return CL_ALL_DEVICES_FOR_D3D11_KHR; }

    int GetContextDevice() const { return CL_CONTEXT_D3D11_DEVICE_KHR; }

    int GetCommandAcquireDevice() const { return CL_COMMAND_ACQUIRE_D3D11_OBJECTS_KHR; }

    int GetCommandReleaseDevice() const { return CL_COMMAND_RELEASE_D3D11_OBJECTS_KHR; }

    int GetInvalidDevice() const { return CL_INVALID_D3D11_DEVICE_KHR; }

    int GetInvalidResource() const { return CL_INVALID_D3D11_RESOURCE_KHR; }

    int GetResourceAlreadyAcquired() const { return CL_D3D11_RESOURCE_ALREADY_ACQUIRED_KHR; }

    int GetResourceNotAcquired() const { return CL_D3D11_RESOURCE_NOT_ACQUIRED_KHR; }

    ~D3D11Definitions() {};

};

}}}
