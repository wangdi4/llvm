#include "CL.h"
#include <string>

class Extensions
{
    std::string m_ext;

    bool IsExtSupported(const std::string& ext)
    {
        return m_ext.find(ext) != std::string::npos;
    }
public:
    Extensions(const std::string& ext):
        m_ext(ext)
    {}

    bool IsSpirSupported()              { return IsExtSupported("cl_khr_spir"); }

    bool IsICDSupported()               { return IsExtSupported("cl_khr_icd"); }

    bool IsDX9Supported()               { return IsExtSupported("cl_khr_dx9_media_sharing"); }

    bool IsDX9Supported_INTEL()         { return IsExtSupported("cl_intel_dx9_media_sharing"); }

    bool IsDX11Supported()              { return IsExtSupported("cl_khr_d3d11_sharing"); }

    bool IsGLSupported()                { return IsExtSupported("cl_khr_gl_sharing"); }

    bool IsLocalThreadSupported_INTEL() { return IsExtSupported("cl_intel_exec_by_local_thread"); }

    bool IsVecLenHintSupported()        { return IsExtSupported("cl_intel_vec_len_hint"); }
};

void CL::CheckExtensions()
{
#if defined (__ANDROID__)
    std::string os("android.");
#elif defined (_WIN32)
    std::string os("windows.");
#else
    std::string os("linux.");
#endif

    cl_int iRet = CL_SUCCESS;

    size_t ret_size = 0;
    iRet = clGetPlatformInfo(m_platform,
                             CL_PLATFORM_EXTENSIONS,
                             /*param_value_size*/0,
                             /*param_value*/nullptr,
                             &ret_size);
    ASSERT_EQ(CL_SUCCESS, iRet) << " clGetPlatformInfo failed. ";

    std::string ext_string(ret_size, '\0');

    iRet = clGetPlatformInfo(m_platform,
                             CL_PLATFORM_EXTENSIONS,
                             ext_string.size(),
                             &ext_string[0],
                             &ret_size);
    ASSERT_EQ(CL_SUCCESS, iRet) << " clGetPlatformInfo failed. ";

    iRet = clGetDeviceInfo(m_device,
                           CL_DEVICE_EXTENSIONS,
                           /*param_value_size*/0,
                           /*param_value*/nullptr,
                           &ret_size);
    ASSERT_EQ(CL_SUCCESS, iRet) << " clGetPlatformInfo failed. ";

    std::string device_ext_string(ret_size, '\0');

    iRet = clGetDeviceInfo(m_device,
                           CL_DEVICE_EXTENSIONS,
                           device_ext_string.size(),
                           &device_ext_string[0],
                           &ret_size);
    ASSERT_EQ(CL_SUCCESS, iRet) << " clGetPlatformInfo failed. ";

    ASSERT_TRUE(ext_string == device_ext_string)
        << " Expected that platform and device extensions are equal!";

    Extensions extensions(ext_string);

    ASSERT_TRUE(extensions.IsSpirSupported())
        << " Expected that cl_khr_spir is not supported on " << os;

    ASSERT_TRUE(extensions.IsICDSupported())
        << " Expected that cl_khr_icd  is not supported on " << os;

    ASSERT_TRUE(extensions.IsVecLenHintSupported())
        << " Expected that cl_intel_vec_len_hint  is supported on " << os;

#if defined (__ANDROID__)
    ASSERT_TRUE(extensions.IsLocalThreadSupported_INTEL())
        << " Expected that cl_intel_exec_by_local_thread is supported on " << os;

    ASSERT_FALSE(extensions.IsDX9Supported_INTEL())
        << " Expected that cl_intel_dx9_media_sharing is not supported on " << os;

    ASSERT_FALSE(extensions.IsDX9Supported())
        << " Expected that cl_khr_dx9_media_sharing is not supported on " << os;

    ASSERT_FALSE(extensions.IsDX11Supported())
        << " Expected that cl_khr_d3d11_sharing is not supported on " << os;

    ASSERT_FALSE(extensions.IsGLSupported())
        << " Expected that cl_khr_gl_sharing is not supported on " << os;

#elif defined (_WIN32)
    ASSERT_TRUE(extensions.IsLocalThreadSupported_INTEL())
        << " Expected that cl_intel_exec_by_local_thread is supported on " << os;

    ASSERT_TRUE(extensions.IsDX9Supported_INTEL())
        << " Expected that cl_intel_dx9_media_sharing is supported on " << os;

    ASSERT_TRUE(extensions.IsDX9Supported())
        << " Expected that cl_khr_dx9_media_sharing is supported on " << os;

    ASSERT_TRUE(extensions.IsDX11Supported())
        << " Expected that cl_khr_d3d11_sharing is supported on " << os;

    ASSERT_TRUE(extensions.IsGLSupported())
        << " Expected that cl_khr_gl_sharing is supported on " << os;

#else //LINUX
    ASSERT_TRUE(extensions.IsLocalThreadSupported_INTEL())
        << " Expected that cl_intel_exec_by_local_thread is supported on " << os;

    ASSERT_FALSE(extensions.IsDX9Supported_INTEL())
        << " Expected that cl_intel_dx9_media_sharing is not supported on " << os;

    ASSERT_FALSE(extensions.IsDX9Supported())
        << " Expected that cl_khr_dx9_media_sharing is not supported on " << os;

    ASSERT_FALSE((bool)extensions.IsDX11Supported())
        << " Expected that cl_khr_d3d11_sharing is not supported on " << os;

    ASSERT_FALSE(extensions.IsGLSupported())
        << " Expected that cl_khr_gl_sharing is not supported on " << os;

#endif
}
