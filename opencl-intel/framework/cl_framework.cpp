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

#include <iomanip>


#include <cl_cpu_detect.h>
#include <ocl_itt.h>
#include <cl_objects_map.h>
#include <cl_shared_ptr.hpp>

#include "cl_framework.h"
#include "framework_proxy.h"
#include "cl_user_logger.h"
#include "UserLoggerOutputParams.h"
#if defined (_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <cl_linux_utils.h>
#include "cl_framework_alias_linux.h"
#endif

using namespace Intel::OpenCL::Framework;
using namespace Intel::OpenCL::Utils;

// Error value to return from API calls when process is in the shutdown state
#define API_DISABLED_USER_RETURN_VALUE  CL_SUCCESS

#if defined(USE_ITT)

#ifdef WIN32
#define thread_local __declspec(thread)
#else
#define thread_local __thread
#endif

#define __startITTTask(pGPAData, ittID, fnctName)	\
    ittID = __itt_id_make(&ittID, (unsigned long long)0);\
	__itt_id_create(pGPAData->pAPIDomain, ittID);\
	static thread_local __itt_string_handle* pAPINameHandle = nullptr;\
	if ( nullptr == pAPINameHandle )\
	{\
		pAPINameHandle = __itt_string_handle_create(fnctName);\
	}\
	__itt_task_begin(pGPAData->pAPIDomain, ittID, __itt_null, pAPINameHandle);

#define __endITTTask(pGPAData, ittID)	\
	__itt_task_end(pGPAData->pAPIDomain); \
    __itt_id_destroy(pGPAData->pAPIDomain, ittID);

#define CALL_INSTRUMENTED_API_LOGGER(module, return_type, function_call) \
	ocl_gpa_data *pGPAData = module->GetGPAData(); \
	if ((nullptr != pGPAData) && (pGPAData->bUseGPA) && (pGPAData->bEnableAPITracing)) \
	{ \
		__itt_id ittID; \
		__startITTTask(pGPAData, ittID, __FUNCTION__); \
		return_type ret_val = (API_IS_DISABLED) ? API_DISABLED_USER_RETURN_VALUE : module->function_call; \
		__endITTTask(pGPAData, ittID); \
        apiLogger.EndApiFunc(ret_val); \
		return ret_val; \
	} else { \
    if (API_IS_DISABLED) { \
        apiLogger.EndApiFunc(API_DISABLED_USER_RETURN_VALUE); \
        return API_DISABLED_USER_RETURN_VALUE; \
    } else { \
		    return_type ret_val = module->function_call; \
        apiLogger.EndApiFunc(ret_val); \
        return ret_val; \
    } \
	}

#define CALL_INSTRUMENTED_API(module, return_type, function_call) \
	ocl_gpa_data *pGPAData = module->GetGPAData(); \
	if ((nullptr != pGPAData) && (pGPAData->bUseGPA) && (pGPAData->bEnableAPITracing)) \
	{ \
		__itt_id ittID; \
		__startITTTask(pGPAData, ittID, __FUNCTION__); \
		return_type ret_val = (API_IS_DISABLED) ? API_DISABLED_USER_RETURN_VALUE : module->function_call; \
		__endITTTask(pGPAData, ittID); \
		return ret_val; \
	} else { \
	    if (API_IS_DISABLED) { \
	        return API_DISABLED_USER_RETURN_VALUE; \
	    } else { \
			    return module->function_call; \
	    } \
	}

#define CALL_INSTRUMENTED_API_LOGGER_NO_RET(module, function_call) \
ocl_gpa_data *pGPAData = module->GetGPAData(); \
if ((nullptr != pGPAData) && (pGPAData->bUseGPA) && (pGPAData->bEnableAPITracing)) \
{ \
    __itt_id ittID; \
    __startITTTask(pGPAData, ittID, __FUNCTION__); \
    module->function_call; \
    apiLogger.EndApiFunc(); \
    __endITTTask(pGPAData, ittID); } else { \
        module->function_call; \
        apiLogger.EndApiFunc(); \
    }

#define CALL_INSTRUMENTED_API_NO_RET(module, function_call) \
ocl_gpa_data *pGPAData = module->GetGPAData(); \
if ((nullptr != pGPAData) && (pGPAData->bUseGPA) && (pGPAData->bEnableAPITracing)) \
{ \
    __itt_id ittID; \
    __startITTTask(pGPAData, ittID, __FUNCTION__); \
    module->function_call; \
    __endITTTask(pGPAData, ittID); } else { \
        module->function_call; \
}

#else

#define CALL_INSTRUMENTED_API_LOGGER(module, return_type, function_call) \
    if (API_IS_DISABLED) \
    { \
        return API_DISABLED_USER_RETURN_VALUE; \
    } \
    else \
    { \
        return_type ret = module->function_call; \
        apiLogger.EndApiFunc(ret); \
        return ret; \
    }

#define CALL_INSTRUMENTED_API(module, return_type, function_call) \
return (API_IS_DISABLED) ? API_DISABLED_USER_RETURN_VALUE : module->function_call;

#define CALL_INSTRUMENTED_API_LOGGER_NO_RET(module, function_call) \
    { \
	    module->function_call; \
        apiLogger.EndApiFunc(); \
    }

#define CALL_INSTRUMENTED_API_NO_RET(module, function_call) \
	module->function_call;

#endif

ExtensionFunctionAddressResolveMap g_extFuncResolveMap;
void* RegisterExtensionFunctionAddress(const char* pFuncName, void* pFuncPtr)
{
	g_extFuncResolveMap.insert(std::pair<std::string, void*>(pFuncName,pFuncPtr));
	return pFuncPtr;
}

#define START_LOG_API(API_NAME) ApiLogger apiLogger(#API_NAME);

static void* GetExtensionFunctionAddress(const char *funcname)
{
  if ( nullptr == funcname )
	{
		return nullptr;
	}
	ExtensionFunctionAddressResolveMap::const_iterator ptr = g_extFuncResolveMap.find(funcname);
	if ( g_extFuncResolveMap.end() == ptr)
	{
		return nullptr;
	}
	return ptr->second;
}

#if defined( __GNUC__ ) &&  __GNUC__  > 7
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wattribute-alias"
#endif

void * CL_API_CALL clGetExtensionFunctionAddress(const char *funcname)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clGetExtensionFunctionAddress);
        apiLogger << "const char *funcname";
        apiLogger.PrintCStringVal(funcname);
        void* addr = GetExtensionFunctionAddress(funcname);
        apiLogger.EndApiFunc(addr);
        return addr;
    }
    else
    {
        return GetExtensionFunctionAddress(funcname);
    }
}
SET_ALIAS(clGetExtensionFunctionAddress);

void* CL_API_CALL clGetExtensionFunctionAddressForPlatform(cl_platform_id platform, const char* funcname)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clGetExtensionFunctionAddressForPlatform);
        apiLogger << "cl_platform_id platform" << platform << "const char* funcname";
        apiLogger.PrintCStringVal(funcname);
        void* addr = !PLATFORM_MODULE->CheckPlatformId(platform) ? nullptr : GetExtensionFunctionAddress(funcname);
        apiLogger.EndApiFunc(addr);
        return addr;
    }
    else
    {
        return !PLATFORM_MODULE->CheckPlatformId(platform) ? nullptr : GetExtensionFunctionAddress(funcname);
    }
}
SET_ALIAS(clGetExtensionFunctionAddressForPlatform);

///////////////////////////////////////////////////////////////////////////////////////////////////
// Platform APIs
///////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * This class is responsible for printing simple values (i.e. not structures etc.)
 * @param T the value's type
 */
template<typename T>
class SimpleValuePrinter
{
public:

    /**
     * @param val the value to print
     * @return the printed string
     */
    std::string Print(const T& val) const
    {
        std::stringstream stream;
        stream << val;
        return stream.str();
    }

};

/**
 * This class is responsible for printing list output parameters
 * @param ELEM_TYPE type of elements in the list
 * @param SIZE_TYPE type of the list's size
 */
template<typename ELEM_TYPE, typename SIZE_TYPE, class VALUE_PRINTER = SimpleValuePrinter<ELEM_TYPE> >
class OutputListPrinter : public OutputParamsValueProvider::SpecialOutputParamPrinter
{
public:

    /**
     * Constructor
     * @param listName          the list's name
     * @param pList             a pointer to the 1st element in the list
     * @param pListSize         a pointer to the size of the list
     * @param defaultListSize   optional default sze of the list in case pListSize is NULL
     */
    OutputListPrinter(const std::string& listName, const ELEM_TYPE* pList, const SIZE_TYPE* pListSize, SIZE_TYPE defaultListSize = 0) :
        m_listName(listName), m_pList(pList), m_pListSize(pListSize), m_defaultListSize(defaultListSize) { }

    // overriden methods:

    virtual std::string GetStringToPrint() const
    {
        if (nullptr == m_pList)
        {
            return "";
        }
        std::ostringstream stream;
        stream << m_listName << ":";
        const SIZE_TYPE actualListSize = nullptr != m_pListSize ? *m_pListSize : m_defaultListSize;
        VALUE_PRINTER valPrinter;
        for (cl_uint i = 0; i < actualListSize; ++i)
        {
            stream << " " << valPrinter.Print(m_pList[i]);
            if (i < actualListSize - 1)
            {
                stream << ",";
            }
        }
        return stream.str();
    }

private:

    const std::string m_listName;
    const ELEM_TYPE* m_pList;
    const SIZE_TYPE* m_pListSize;
    const SIZE_TYPE m_defaultListSize;

};

cl_int CL_API_CALL clGetPlatformIDs(cl_uint num_entries, cl_platform_id * platforms, cl_uint * num_platforms)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        ApiLogger apiLogger("clGetPlatformIDs");
        apiLogger << "cl_uint num_entries" << num_entries << "cl_platform_id * platforms" << platforms << "cl_uint * num_platforms" << num_platforms;
        OutputListPrinter<cl_platform_id, cl_uint> listPrinter("platforms", platforms, num_platforms, num_entries);
        OutputParamsValueProvider provider(apiLogger, &listPrinter);
        CALL_INSTRUMENTED_API_LOGGER(PLATFORM_MODULE,  cl_int, GetPlatformIDs(num_entries, platforms, num_platforms));
    }
    else
    {
        CALL_INSTRUMENTED_API(PLATFORM_MODULE,  cl_int, GetPlatformIDs(num_entries, platforms, num_platforms));
    }
}
SET_ALIAS(clGetPlatformIDs);
REGISTER_EXTENSION_FUNCTION(clIcdGetPlatformIDsKHR, clGetPlatformIDs);

cl_int CL_API_CALL clGetPlatformInfo(cl_platform_id platform,
						 cl_platform_info param_name,
						 size_t param_value_size,
						 void* param_value,
						 size_t* param_value_size_ret)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clGetPlatformInfo);
        apiLogger << "cl_platform_id platform" << platform << "cl_platform_info param_name" << param_name << "size_t param_value_size" << param_value_size << "void* param_value" << param_value << "size_t* param_value_size_ret" << param_value_size_ret;
        OutputParamsValueProvider provider(apiLogger);
        provider.AddParam("param_value_size_ret", param_value_size_ret, false, true);
        CALL_INSTRUMENTED_API_LOGGER(PLATFORM_MODULE,  cl_int, GetPlatformInfo(platform, param_name, param_value_size, param_value, param_value_size_ret));
    }
    else
    {
        CALL_INSTRUMENTED_API(PLATFORM_MODULE,  cl_int, GetPlatformInfo(platform, param_name, param_value_size, param_value, param_value_size_ret));
    }
}
SET_ALIAS(clGetPlatformInfo);

///////////////////////////////////////////////////////////////////////////////////////////////////
// clGetHostTimer
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_int CL_API_CALL clGetHostTimer(cl_device_id device,
                                  cl_ulong* host_timestamp)
{
    if (FrameworkProxy::Instance()->GetOCLConfig()->GetOpenCLVersion() < OPENCL_VERSION_2_1)
    {
        return CL_INVALID_OPERATION;
    }

    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        ApiLogger apiLogger("clGetHostTimer");
        apiLogger << "cl_device_id device" << device << "cl_ulong* host_timestamp" << host_timestamp;
        OutputParamsValueProvider provider(apiLogger);
        provider.AddParam("host_timestamp", host_timestamp, false, true);
        CALL_INSTRUMENTED_API_LOGGER(PLATFORM_MODULE, cl_int, GetHostTimer(device, host_timestamp));
    }
    else
    {
        CALL_INSTRUMENTED_API(PLATFORM_MODULE, cl_int, GetHostTimer(device, host_timestamp));
    }
}
SET_ALIAS(clGetHostTimer);

///////////////////////////////////////////////////////////////////////////////////////////////////
// clGetDeviceAndHostTimer
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_int CL_API_CALL clGetDeviceAndHostTimer(cl_device_id device,
                                           cl_ulong* device_timestamp,
                                           cl_ulong* host_timestamp)
{
    if (FrameworkProxy::Instance()->GetOCLConfig()->GetOpenCLVersion() < OPENCL_VERSION_2_1)
    {
        return CL_INVALID_OPERATION;
    }

    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        ApiLogger apiLogger("clGetDeviceAndHostTimer");
        apiLogger << "cl_device_id device" << device << "cl_ulong* device_timestamp" << device_timestamp <<
            "cl_ulong* host_timestamp" << host_timestamp;
        OutputParamsValueProvider provider(apiLogger);
        provider.AddParam("host_timestamp",   host_timestamp,   false, true);
        provider.AddParam("device_timestamp", device_timestamp, false, true);
        CALL_INSTRUMENTED_API_LOGGER(PLATFORM_MODULE, cl_int, GetDeviceAndHostTimer(device, device_timestamp, host_timestamp));
    }
    else
    {
        CALL_INSTRUMENTED_API(PLATFORM_MODULE, cl_int, GetDeviceAndHostTimer(device, device_timestamp, host_timestamp));
    }
}
SET_ALIAS(clGetDeviceAndHostTimer);

///////////////////////////////////////////////////////////////////////////////////////////////////
// Device APIs
///////////////////////////////////////////////////////////////////////////////////////////////////

cl_int CL_API_CALL clGetDeviceIDs(cl_platform_id platform,
					  cl_device_type device_type,
					  cl_uint num_entries,
					  cl_device_id* devices,
			          cl_uint* num_devices)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clGetDeviceIDs);
        apiLogger << "cl_platform_id platform" << platform << "cl_device_type device_type" << device_type << "cl_uint num_entries" << num_entries << "cl_device_id* devices" << devices << "cl_uint* num_devices" << num_devices;
        OutputListPrinter<cl_device_id, cl_uint> printer("devices", devices, num_devices, num_entries);
        OutputParamsValueProvider provider(apiLogger, &printer);
        CALL_INSTRUMENTED_API_LOGGER(PLATFORM_MODULE, cl_int, GetDeviceIDs(platform, device_type, num_entries, devices, num_devices));
    }
    else
    {
        CALL_INSTRUMENTED_API(PLATFORM_MODULE, cl_int, GetDeviceIDs(platform, device_type, num_entries, devices, num_devices));
    }
}
SET_ALIAS(clGetDeviceIDs);

cl_int CL_API_CALL clGetDeviceInfo(cl_device_id device,
					   cl_device_info param_name,
					   size_t param_value_size,
					   void* param_value,
					   size_t* param_value_size_ret)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clGetDeviceInfo);
        apiLogger << "cl_device_id device" << device << "cl_device_info param_name" << param_name << "size_t param_value_size" << param_value_size << "void* param_value" << param_value << "size_t* param_value_size_ret" << param_value_size_ret;
        OutputParamsValueProvider provider(apiLogger);
        provider.AddParam("param_value_size_ret", param_value_size_ret, false, true);
        CALL_INSTRUMENTED_API_LOGGER(PLATFORM_MODULE, cl_int, GetDeviceInfo(device, param_name, param_value_size, param_value, param_value_size_ret));
    }
    else
    {
        CALL_INSTRUMENTED_API(PLATFORM_MODULE, cl_int, GetDeviceInfo(device, param_name, param_value_size, param_value, param_value_size_ret));
    }
}
SET_ALIAS(clGetDeviceInfo);

///////////////////////////////////////////////////////////////////////////////////////////////////
// Context APIs
///////////////////////////////////////////////////////////////////////////////////////////////////

cl_context CL_API_CALL clCreateContext(const cl_context_properties * properties,
						   cl_uint num_devices,
						   const cl_device_id * devices,
						   logging_fn pfn_notify,
						   void * user_data,
						   cl_int * errcode_ret)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clCreateContext);
        apiLogger << "const cl_context_properties * properties" << properties << "cl_uint num_devices" << num_devices << "const cl_device_id * devices" << devices << "logging_fn pfn_notify" << pfn_notify << "void * user_data" << user_data << "cl_int * errcode_ret" << errcode_ret;
        OutputParamsValueProvider provider(apiLogger);
        provider.AddParam("errcode_ret", errcode_ret, false, false);
	      CALL_INSTRUMENTED_API_LOGGER(CONTEXT_MODULE, cl_context, CreateContext(properties, num_devices, devices, pfn_notify, user_data, errcode_ret));
    }
    else
    {
        CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_context, CreateContext(properties, num_devices, devices, pfn_notify, user_data, errcode_ret));
    }
}
SET_ALIAS(clCreateContext);

cl_context CL_API_CALL clCreateContextFromType(const cl_context_properties * properties,
								   cl_device_type          device_type,
								   logging_fn              pfn_notify,
								   void *                  user_data,
								   cl_int *                errcode_ret)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clCreateContextFromType);
        apiLogger << "const cl_context_properties * properties" << properties << "cl_device_type device_type" << device_type << "logging_fn pfn_notify" << pfn_notify << "void * user_data" << user_data << "cl_int * errcode_ret" << errcode_ret;
        OutputParamsValueProvider provider(apiLogger);
        provider.AddParam("errcode_ret", errcode_ret, false, false);
	      CALL_INSTRUMENTED_API_LOGGER(CONTEXT_MODULE, cl_context, CreateContextFromType(properties, device_type, pfn_notify, user_data, errcode_ret));
    }
    else
    {
        CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_context, CreateContextFromType(properties, device_type, pfn_notify, user_data, errcode_ret));
    }
}
SET_ALIAS(clCreateContextFromType);

cl_int CL_API_CALL clRetainContext(cl_context context)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clRetainContext);
        apiLogger << "cl_context context" << context;
	      CALL_INSTRUMENTED_API_LOGGER(CONTEXT_MODULE, cl_int, RetainContext(context));
    }
    else
    {
        CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_int, RetainContext(context));
    }
}
SET_ALIAS(clRetainContext);

cl_int CL_API_CALL clReleaseContext(cl_context context)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clReleaseContext);
        apiLogger << "cl_context context" << context;
	      CALL_INSTRUMENTED_API_LOGGER(CONTEXT_MODULE, cl_int, ReleaseContext(context));
    }
    else
    {
        CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_int, ReleaseContext(context));
    }
}
SET_ALIAS(clReleaseContext);

cl_int CL_API_CALL clGetContextInfo(cl_context      context,
						cl_context_info param_name,
						size_t          param_value_size,
						void *          param_value,
						size_t *        param_value_size_ret)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clGetContextInfo);
        apiLogger << "cl_context context" << context << "cl_context_info param_name" << param_name << "size_t param_value_size" << param_value_size << "void * param_value" << param_value << "size_t * param_value_size_ret" << param_value_size_ret;
        OutputParamsValueProvider provider(apiLogger);
        provider.AddParam("param_value_size_ret", param_value_size_ret, false, true);
	      CALL_INSTRUMENTED_API_LOGGER(CONTEXT_MODULE, cl_int, GetContextInfo(context, param_name, param_value_size, param_value, param_value_size_ret));
    }
    else
    {
        CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_int, GetContextInfo(context, param_name, param_value_size, param_value, param_value_size_ret));
    }
}
SET_ALIAS(clGetContextInfo);
///////////////////////////////////////////////////////////////////////////////////////////////////
// Command Queue APIs
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_command_queue CL_API_CALL clCreateCommandQueue(cl_context                  context,
									  cl_device_id                device,
									  cl_command_queue_properties properties,
									  cl_int *                    errcode_ret)
{
    const cl_command_queue_properties propertiesArr[] = { CL_QUEUE_PROPERTIES, properties, 0 };
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clCreateCommandQueue);
        apiLogger << "cl_context context" << context << "cl_device_id device" << device << "cl_command_queue_properties properties" << properties << "cl_int * errcode_ret" << errcode_ret;
        OutputParamsValueProvider provider(apiLogger);
        provider.AddParam("errcode_ret", errcode_ret, false, false);
	      CALL_INSTRUMENTED_API_LOGGER(EXECUTION_MODULE, cl_command_queue, CreateCommandQueue(context, device, propertiesArr, errcode_ret));
    }
    else
    {
        CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_command_queue, CreateCommandQueue(context, device, propertiesArr, errcode_ret));
    }
}
SET_ALIAS(clCreateCommandQueue);
cl_int CL_API_CALL clRetainCommandQueue(cl_command_queue command_queue)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clRetainCommandQueue);
        apiLogger << "cl_command_queue command_queue" << command_queue;
	      CALL_INSTRUMENTED_API_LOGGER(EXECUTION_MODULE, cl_int, RetainCommandQueue(command_queue));
    }
    else
    {
        CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_int, RetainCommandQueue(command_queue));
    }
}
SET_ALIAS(clRetainCommandQueue);
cl_int CL_API_CALL clReleaseCommandQueue(cl_command_queue command_queue)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clReleaseCommandQueue);
        apiLogger << "cl_command_queue command_queue" << command_queue;
	      CALL_INSTRUMENTED_API_LOGGER(EXECUTION_MODULE, cl_int, ReleaseCommandQueue(command_queue));
    }
    else
    {
        CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_int, ReleaseCommandQueue(command_queue));
    }
}
SET_ALIAS(clReleaseCommandQueue);
cl_int CL_API_CALL clGetCommandQueueInfo(cl_command_queue      command_queue,
							 cl_command_queue_info param_name,
							 size_t                param_value_size,
							 void *                param_value,
							 size_t *              param_value_size_ret)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clGetCommandQueueInfo);
        apiLogger << "cl_command_queue command_queue" << command_queue << "cl_command_queue_info param_name" << param_name << "size_t param_value_size" << param_value_size << "void * param_value" << param_value << "size_t * param_value_size_ret" << param_value_size_ret;
        OutputParamsValueProvider provider(apiLogger);
        provider.AddParam("param_value_size_ret", param_value_size_ret, false, true);
	      CALL_INSTRUMENTED_API_LOGGER(EXECUTION_MODULE, cl_int, GetCommandQueueInfo(command_queue, param_name, param_value_size, param_value, param_value_size_ret));
    }
    else
    {
        CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_int, GetCommandQueueInfo(command_queue, param_name, param_value_size, param_value, param_value_size_ret));
    }
}
SET_ALIAS(clGetCommandQueueInfo);
cl_int CL_API_CALL clSetCommandQueueProperty(cl_command_queue              command_queue,
								 cl_command_queue_properties   properties,
								 cl_bool                       enable,
								 cl_command_queue_properties * old_properties)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clSetCommandQueueProperty);
        apiLogger << "cl_command_queue command_queue" << command_queue << "cl_command_queue_properties properties" << properties << "cl_bool enable" << enable << "cl_command_queue_properties * old_properties" << old_properties;
	      CALL_INSTRUMENTED_API_LOGGER(EXECUTION_MODULE, cl_int, SetCommandQueueProperty(command_queue, properties, enable, old_properties));
    }
    else
    {
        CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_int, SetCommandQueueProperty(command_queue, properties, enable, old_properties));
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Memory Object APIs
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_mem CL_API_CALL clCreateBuffer(cl_context   context,
					  cl_mem_flags flags,
					  size_t       size,
					  void *       host_ptr,
					  cl_int *     errcode_ret)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clCreateBuffer);
        apiLogger << "cl_context context" << context << "cl_mem_flags flags" << flags << "size_t size" << size << "void * host_ptr" << host_ptr << "cl_int * errcode_ret" << errcode_ret;
        OutputParamsValueProvider provider(apiLogger);
        provider.AddParam("errcode_ret", errcode_ret, false, false);
	      CALL_INSTRUMENTED_API_LOGGER(CONTEXT_MODULE, cl_mem, CreateBuffer(context, flags, size, host_ptr, errcode_ret));
    }
    else
    {
        CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_mem, CreateBuffer(context, flags, size, host_ptr, errcode_ret));
    }
}
SET_ALIAS(clCreateBuffer);

cl_mem CL_API_CALL clCreateSubBuffer(cl_mem buffer,
				  cl_mem_flags				flags,
				  cl_buffer_create_type     buffer_create_type,
				  const void *              buffer_create_info,
				  cl_int *                  errcode_ret)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clCreateSubBuffer);
        apiLogger << "cl_mem buffer" << buffer << "cl_mem_flags flags" << flags << "cl_buffer_create_type buffer_create_type" << buffer_create_type << "const void * buffer_create_info" << buffer_create_info << "cl_int * errcode_ret" << errcode_ret;
        OutputParamsValueProvider provider(apiLogger);
        provider.AddParam("errcode_ret", errcode_ret, false, false);
	      CALL_INSTRUMENTED_API_LOGGER(CONTEXT_MODULE, cl_mem, CreateSubBuffer(buffer, flags, buffer_create_type, buffer_create_info, errcode_ret));
    }
    else
    {
        CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_mem, CreateSubBuffer(buffer, flags, buffer_create_type, buffer_create_info, errcode_ret));
    }
}
SET_ALIAS(clCreateSubBuffer);

cl_int CL_API_CALL
clSetMemObjectDestructorCallback(cl_mem			memObj,
								 mem_dtor_fn	pfn_notify,
								void *			pUserData )
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(CL_API_CALLclSetMemObjectDestructorCallback);
        apiLogger << "cl_mem memObj" << memObj << "mem_dtor_fn pfn_notify" << pfn_notify << "void * pUserData" << pUserData;
	      CALL_INSTRUMENTED_API_LOGGER(CONTEXT_MODULE, cl_int, SetMemObjectDestructorCallback(memObj, pfn_notify, pUserData));
    }
    else
    {
        CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_int, SetMemObjectDestructorCallback(memObj, pfn_notify, pUserData));
    }
}
SET_ALIAS(clSetMemObjectDestructorCallback);

cl_mem CL_API_CALL clCreateImage(
               cl_context context,
               cl_mem_flags flags,
               const cl_image_format *image_format,
               const cl_image_desc *image_desc,
               void *host_ptr,
               cl_int *errcode_ret)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clCreateImage);
        apiLogger << "cl_context context" << context << "cl_mem_flags flags" << flags << "const cl_image_format *image_format" << image_format << "const cl_image_desc *image_desc" << image_desc << "void *host_ptr" << host_ptr << "cl_int *errcode_ret" << errcode_ret;
        OutputParamsValueProvider provider(apiLogger);
        provider.AddParam("errcode_ret", errcode_ret, false, false);
        CALL_INSTRUMENTED_API_LOGGER(CONTEXT_MODULE, cl_mem, CreateImage(context, flags, image_format, image_desc, host_ptr, errcode_ret));
    }
    else
    {
        CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_mem, CreateImage(context, flags, image_format, image_desc, host_ptr, errcode_ret));
    }
}
SET_ALIAS(clCreateImage);

cl_mem CL_API_CALL clCreateImage2D(cl_context              context,
					   cl_mem_flags            flags,
					   const cl_image_format * image_format,
					   size_t                  image_width,
					   size_t                  image_height,
					   size_t                  image_row_pitch,
					   void *                  host_ptr,
					   cl_int *                errcode_ret)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clCreateImage2D);
        apiLogger << "cl_context context" << context << "cl_mem_flags flags" << flags << "const cl_image_format * image_format" << image_format << "size_t image_width" << image_width << "size_t image_height" << image_height << "size_t image_row_pitch" << image_row_pitch << "void * host_ptr" << host_ptr << "cl_int * errcode_ret" << errcode_ret;
        OutputParamsValueProvider provider(apiLogger);
        provider.AddParam("errcode_ret", errcode_ret, false, false);
	      CALL_INSTRUMENTED_API_LOGGER(CONTEXT_MODULE, cl_mem, CreateImage2D(context, flags, image_format, image_width, image_height, image_row_pitch, host_ptr, errcode_ret));
    }
    else
    {
        CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_mem, CreateImage2D(context, flags, image_format, image_width, image_height, image_row_pitch, host_ptr, errcode_ret));
    }
}
SET_ALIAS(clCreateImage2D);

cl_mem CL_API_CALL clCreateImage3D(cl_context              context,
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
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clCreateImage3D);
        apiLogger << "cl_context context" << context << "cl_mem_flags flags" << flags << "const cl_image_format * image_format" << image_format << "size_t image_width" << image_width << "size_t image_height" << image_height << "size_t image_depth" << image_depth << "size_t image_row_pitch" << image_row_pitch << "size_t image_slice_pitch" << image_slice_pitch << "void * host_ptr" << host_ptr << "cl_int * errcode_ret" << errcode_ret;
        OutputParamsValueProvider provider(apiLogger);
        provider.AddParam("errcode_ret", errcode_ret, false, false);
	      CALL_INSTRUMENTED_API_LOGGER(CONTEXT_MODULE, cl_mem, CreateImage3D(context, flags, image_format, image_width, image_height, image_depth, image_row_pitch, image_slice_pitch, host_ptr, errcode_ret));
    }
    else
    {
        CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_mem, CreateImage3D(context, flags, image_format, image_width, image_height, image_depth, image_row_pitch, image_slice_pitch, host_ptr, errcode_ret));
    }
}
SET_ALIAS(clCreateImage3D);

cl_int CL_API_CALL clRetainMemObject(cl_mem memobj)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clRetainMemObject);
        apiLogger << "cl_mem memobj" << memobj;
	      CALL_INSTRUMENTED_API_LOGGER(CONTEXT_MODULE, cl_int, RetainMemObject(memobj));
    }
    else
    {
        CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_int, RetainMemObject(memobj));
    }
}
SET_ALIAS(clRetainMemObject);

cl_int CL_API_CALL clReleaseMemObject(cl_mem memobj)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clReleaseMemObject);
        apiLogger << "cl_mem memobj" << memobj;
	      CALL_INSTRUMENTED_API_LOGGER(CONTEXT_MODULE, cl_int, ReleaseMemObject(memobj));
    }
    else
    {
        CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_int, ReleaseMemObject(memobj));
    }
}
SET_ALIAS(clReleaseMemObject);

class ImageFormatValuePrinter
{
public:

    std::string Print(const cl_image_format& imageFormat) const
    {
        std::stringstream stream;
        stream << "(0x" << std::hex << imageFormat.image_channel_order << ",0x" << imageFormat.image_channel_data_type << ")";
        return stream.str();
    }
};

cl_int CL_API_CALL clGetSupportedImageFormats(cl_context           context,
								  cl_mem_flags         flags,
								  cl_mem_object_type   image_type,
								  cl_uint              num_entries,
								  cl_image_format *    image_formats,
								  cl_uint *            num_image_formats)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clGetSupportedImageFormats);
        apiLogger << "cl_context context" << context << "cl_mem_flags flags" << flags << "cl_mem_object_type image_type" << image_type << "cl_uint num_entries" << num_entries << "cl_image_format * image_formats" << image_formats << "cl_uint * num_image_formats" << num_image_formats;
        OutputListPrinter<cl_image_format, cl_uint, ImageFormatValuePrinter> printer("image_formats (order,type)", image_formats, num_image_formats, num_entries);
        OutputParamsValueProvider provider(apiLogger, &printer);
	      CALL_INSTRUMENTED_API_LOGGER(CONTEXT_MODULE, cl_int, GetSupportedImageFormats(context, flags, image_type, num_entries, image_formats, num_image_formats));
    }
    else
    {
        CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_int, GetSupportedImageFormats(context, flags, image_type, num_entries, image_formats, num_image_formats));
    }
}
SET_ALIAS(clGetSupportedImageFormats);

cl_int CL_API_CALL clGetMemObjectInfo(cl_mem           memobj,
						  cl_mem_info      param_name,
						  size_t           param_value_size,
						  void *           param_value,
						  size_t *         param_value_size_ret)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clGetMemObjectInfo);
        apiLogger << "cl_mem memobj" << memobj << "cl_mem_info param_name" << param_name << "size_t param_value_size" << param_value_size << "void * param_value" << param_value << "size_t * param_value_size_ret" << param_value_size_ret;
        OutputParamsValueProvider provider(apiLogger);
        provider.AddParam("param_value_size_ret", param_value_size_ret, false, true);
	      CALL_INSTRUMENTED_API_LOGGER(CONTEXT_MODULE, cl_int, GetMemObjectInfo(memobj, param_name, param_value_size, param_value, param_value_size_ret));
    }
    else
    {
        CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_int, GetMemObjectInfo(memobj, param_name, param_value_size, param_value, param_value_size_ret));
    }
}
SET_ALIAS(clGetMemObjectInfo);

cl_int CL_API_CALL clGetImageInfo(cl_mem           image,
					  cl_image_info    param_name,
					  size_t           param_value_size,
					  void *           param_value,
					  size_t *         param_value_size_ret)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clGetImageInfo);
        apiLogger << "cl_mem image" << image << "cl_image_info param_name" << param_name << "size_t param_value_size" << param_value_size << "void * param_value" << param_value << "size_t * param_value_size_ret" << param_value_size_ret;
        OutputParamsValueProvider provider(apiLogger);
        provider.AddParam("param_value_size_ret", param_value_size_ret, false, true);
	      CALL_INSTRUMENTED_API_LOGGER(CONTEXT_MODULE, cl_int, GetImageInfo(image, param_name, param_value_size, param_value, param_value_size_ret));
    }
    else
    {
        CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_int, GetImageInfo(image, param_name, param_value_size, param_value, param_value_size_ret));
    }
}
SET_ALIAS(clGetImageInfo);
///////////////////////////////////////////////////////////////////////////////////////////////////
// Sampler APIs
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_sampler CL_API_CALL clCreateSampler(cl_context			context,
						   cl_bool				normalized_coords,
						   cl_addressing_mode	addressing_mode,
						   cl_filter_mode		filter_mode,
						   cl_int *				errcode_ret)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clCreateSampler);
        apiLogger << "cl_context context" << context << "cl_bool normalized_coords" << normalized_coords << "cl_addressing_mode addressing_mode" << addressing_mode << "cl_filter_mode filter_mode" << filter_mode << "cl_int * errcode_ret" << errcode_ret;
        OutputParamsValueProvider provider(apiLogger);
        provider.AddParam("errcode_ret", errcode_ret, false, false);
	      CALL_INSTRUMENTED_API_LOGGER(CONTEXT_MODULE, cl_sampler, CreateSampler(context, normalized_coords, addressing_mode, filter_mode, errcode_ret));
    }
    else
    {
        CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_sampler, CreateSampler(context, normalized_coords, addressing_mode, filter_mode, errcode_ret));
    }
}
SET_ALIAS(clCreateSampler);

cl_sampler CL_API_CALL clCreateSamplerWithProperties(cl_context context,
	const cl_sampler_properties *sampler_properties,
	cl_int *errcode_ret)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clCreateSamplerWithProperties);
        apiLogger << "cl_context context" << context << "const cl_sampler_properties *sampler_properties" << sampler_properties << "cl_int *errcode_ret" << errcode_ret;
        OutputParamsValueProvider provider(apiLogger);
        provider.AddParam("errcode_ret", errcode_ret, false, false);
	      CALL_INSTRUMENTED_API_LOGGER(CONTEXT_MODULE, cl_sampler, CreateSamplerWithProperties(context, sampler_properties, errcode_ret));
    }
    else
    {
        CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_sampler, CreateSamplerWithProperties(context, sampler_properties, errcode_ret));
    }
}
SET_ALIAS(clCreateSamplerWithProperties);

cl_int CL_API_CALL clRetainSampler(cl_sampler sampler)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clRetainSampler);
        apiLogger << "cl_sampler sampler" << sampler;
	      CALL_INSTRUMENTED_API_LOGGER(CONTEXT_MODULE, cl_int, RetainSampler(sampler));
    }
    else
    {
        CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_int, RetainSampler(sampler));
    }
}
SET_ALIAS(clRetainSampler);

cl_int CL_API_CALL clReleaseSampler(cl_sampler sampler)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clReleaseSampler);
        apiLogger << "cl_sampler sampler" << sampler;
	      CALL_INSTRUMENTED_API_LOGGER(CONTEXT_MODULE, cl_int, ReleaseSampler(sampler));
    }
    else
    {
        CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_int, ReleaseSampler(sampler));
    }
}
SET_ALIAS(clReleaseSampler);

cl_int CL_API_CALL clGetSamplerInfo(cl_sampler		sampler,
						cl_sampler_info	param_name,
						size_t			param_value_size,
						void *			param_value,
						size_t *		param_value_size_ret)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clGetSamplerInfo);
        apiLogger << "cl_sampler sampler" << sampler << "cl_sampler_info param_name" << param_name << "size_t param_value_size" << param_value_size << "void * param_value" << param_value << "size_t * param_value_size_ret" << param_value_size_ret;
        OutputParamsValueProvider provider(apiLogger);
        provider.AddParam("param_value_size_ret", param_value_size_ret, false, true);
	      CALL_INSTRUMENTED_API_LOGGER(CONTEXT_MODULE, cl_int, GetSamplerInfo(sampler, param_name, param_value_size, param_value, param_value_size_ret));
    }
    else
    {
        CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_int, GetSamplerInfo(sampler, param_name, param_value_size, param_value, param_value_size_ret));
    }
}
SET_ALIAS(clGetSamplerInfo);

///////////////////////////////////////////////////////////////////////////////////////////////////
// Program Object APIs
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_program CL_API_CALL clCreateProgramWithSource(cl_context     context,
									 cl_uint        count,
									 const char **  strings,
									 const size_t * lengths,
									 cl_int *       errcode_ret)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clCreateProgramWithSource);
        apiLogger << "cl_context context" << context << "cl_uint count" << count << "const char ** strings" << strings << "const size_t * lengths" << lengths << "cl_int * errcode_ret" << errcode_ret;
        OutputParamsValueProvider provider(apiLogger);
        provider.AddParam("errcode_ret", errcode_ret, false, false);
	      CALL_INSTRUMENTED_API_LOGGER(CONTEXT_MODULE, cl_program, CreateProgramWithSource(context, count, strings, lengths, errcode_ret));
    }
    else
    {
        CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_program, CreateProgramWithSource(context, count, strings, lengths, errcode_ret));
    }
}
SET_ALIAS(clCreateProgramWithSource);

cl_program CL_API_CALL clCreateProgramWithBinary(cl_context           context,
									 cl_uint              num_devices,
									 const cl_device_id *	device_list,
									 const size_t *			lengths,
									 const unsigned char **	binaries,
									 cl_int *				binary_status,
									 cl_int *				errcode_ret)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clCreateProgramWithBinary);
        apiLogger << "cl_context context" << context << "cl_uint num_devices" << num_devices << "const cl_device_id * device_list" << device_list << "const size_t * lengths" << lengths << "const unsigned char ** binaries" << binaries << "cl_int * binary_status" << binary_status << "cl_int * errcode_ret" << errcode_ret;
        OutputListPrinter<cl_int, cl_uint> printer("binary_statuses", binary_status, nullptr, num_devices);
        OutputParamsValueProvider provider(apiLogger, &printer);
        provider.AddParam("errcode_ret", errcode_ret, false, false);
	      CALL_INSTRUMENTED_API_LOGGER(CONTEXT_MODULE, cl_program, CreateProgramWithBinary(context, num_devices, device_list, lengths, binaries, binary_status, errcode_ret));
    }
    else
    {
        CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_program, CreateProgramWithBinary(context, num_devices, device_list, lengths, binaries, binary_status, errcode_ret));
    }
}
SET_ALIAS(clCreateProgramWithBinary);

cl_program CL_API_CALL clCreateProgramWithBuiltInKernels(cl_context            context,
    cl_uint               num_devices,
    const cl_device_id *  device_list,
    const char *          kernel_names,
    cl_int *              errcode_ret)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clCreateProgramWithBuiltInKernels);
        apiLogger << "cl_context context" << context << "cl_uint num_devices" << num_devices << "const cl_device_id * device_list" << device_list << "const char * kernel_names";
        apiLogger.PrintCStringVal(kernel_names) << "cl_int * errcode_ret" << errcode_ret;
        OutputParamsValueProvider provider(apiLogger);
        provider.AddParam("errcode_ret", errcode_ret, false, false);
	      CALL_INSTRUMENTED_API_LOGGER(CONTEXT_MODULE, cl_program, CreateProgramWithBuiltInKernels(context, num_devices, device_list, kernel_names, errcode_ret));
    }
    else
    {
        CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_program, CreateProgramWithBuiltInKernels(context, num_devices, device_list, kernel_names, errcode_ret));
    }
}
SET_ALIAS(clCreateProgramWithBuiltInKernels);

cl_int CL_API_CALL clRetainProgram(cl_program program)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clRetainProgram);
        apiLogger << "cl_program program" << program;
	      CALL_INSTRUMENTED_API_LOGGER(CONTEXT_MODULE, cl_int, RetainProgram(program));
    }
    else
    {
        CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_int, RetainProgram(program));
    }
}
SET_ALIAS(clRetainProgram);

cl_int CL_API_CALL clReleaseProgram(cl_program program)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clReleaseProgram);
        apiLogger << "cl_program program" << program;
	      CALL_INSTRUMENTED_API_LOGGER(CONTEXT_MODULE, cl_int, ReleaseProgram(program));
    }
    else
    {
        CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_int, ReleaseProgram(program));
    }
}
SET_ALIAS(clReleaseProgram);

cl_int CL_API_CALL clBuildProgram(cl_program           program,
					  cl_uint              num_devices,
					  const cl_device_id * device_list,
					  const char *         options,
					  void (CL_CALLBACK *pfn_notify)(cl_program program, void * user_data),
					  void *               user_data)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clBuildProgram);
        apiLogger << "cl_program program" << program << "cl_uint num_devices" << num_devices << "const cl_device_id * device_list" << device_list << "const char * options";
        apiLogger.PrintCStringVal(options) << "void (CL_CALLBACK *pfn_notify)(cl_program program, void * user_data)" << pfn_notify << "void * user_data" << user_data;
	      CALL_INSTRUMENTED_API_LOGGER(CONTEXT_MODULE, cl_int, BuildProgram(program, num_devices, device_list, options, pfn_notify, user_data));
    }
    else
    {
        CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_int, BuildProgram(program, num_devices, device_list, options, pfn_notify, user_data));
    }
}
SET_ALIAS(clBuildProgram);

cl_int CL_API_CALL clUnloadPlatformCompiler(cl_platform_id platform)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clUnloadPlatformCompiler);
        CALL_INSTRUMENTED_API_LOGGER(PLATFORM_MODULE, cl_int, UnloadPlatformCompiler(platform));
    }
    else
    {
        CALL_INSTRUMENTED_API(PLATFORM_MODULE, cl_int, UnloadPlatformCompiler(platform));
    }
}
SET_ALIAS(clUnloadPlatformCompiler);

cl_int CL_API_CALL clUnloadCompiler(void)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clUnloadCompiler);
	      CALL_INSTRUMENTED_API_LOGGER(PLATFORM_MODULE, cl_int, UnloadCompiler());
    }
    else
    {
        CALL_INSTRUMENTED_API(PLATFORM_MODULE, cl_int, UnloadCompiler());
    }
}
SET_ALIAS(clUnloadCompiler);

cl_int CL_API_CALL clGetProgramInfo(cl_program      program,
						cl_program_info param_name,
						size_t          param_value_size,
						void *          param_value,
						size_t *        param_value_size_ret)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clGetProgramInfo);
        apiLogger << "cl_program program" << program << "cl_program_info param_name" << param_name << "size_t param_value_size" << param_value_size << "void * param_value" << param_value << "size_t * param_value_size_ret" << param_value_size_ret;
        OutputParamsValueProvider provider(apiLogger);
        provider.AddParam("param_value_size_ret", param_value_size_ret, false, true);
	      CALL_INSTRUMENTED_API_LOGGER(CONTEXT_MODULE, cl_int, GetProgramInfo(program, param_name, param_value_size, param_value, param_value_size_ret));
    }
    else
    {
        CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_int, GetProgramInfo(program, param_name, param_value_size, param_value, param_value_size_ret));
    }
}
SET_ALIAS(clGetProgramInfo);

cl_int CL_API_CALL clGetProgramBuildInfo(cl_program            program,
							 cl_device_id          device,
							 cl_program_build_info param_name,
							 size_t                param_value_size,
							 void *                param_value,
							 size_t *              param_value_size_ret)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clGetProgramBuildInfo);
        apiLogger << "cl_program program" << program << "cl_device_id device" << device << "cl_program_build_info param_name" << param_name << "size_t param_value_size" << param_value_size << "void * param_value" << param_value << "size_t * param_value_size_ret" << param_value_size_ret;
        OutputParamsValueProvider provider(apiLogger);
        provider.AddParam("param_value_size_ret", param_value_size_ret, false, true);
	      CALL_INSTRUMENTED_API_LOGGER(CONTEXT_MODULE, cl_int, GetProgramBuildInfo(program, device, param_name, param_value_size, param_value, param_value_size_ret));
    }
    else
    {
        CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_int, GetProgramBuildInfo(program, device, param_name, param_value_size, param_value, param_value_size_ret));
    }
}
SET_ALIAS(clGetProgramBuildInfo);

///////////////////////////////////////////////////////////////////////////////////////////////////
// Kernel Object APIs
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_kernel CL_API_CALL clCreateKernel(cl_program   program,
						 const char * kernel_name,
						 cl_int *     errcode_ret)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clCreateKernel);
        apiLogger << "cl_program program" << program << "const char * kernel_name";
        apiLogger.PrintCStringVal(kernel_name) << "cl_int * errcode_ret" << errcode_ret;
        OutputParamsValueProvider provider(apiLogger);
        provider.AddParam("errcode_ret", errcode_ret, false, false);
	      CALL_INSTRUMENTED_API_LOGGER(CONTEXT_MODULE, cl_kernel, CreateKernel(program, kernel_name, errcode_ret));
    }
    else
    {
        CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_kernel, CreateKernel(program, kernel_name, errcode_ret));
    }
}
SET_ALIAS(clCreateKernel);

cl_int CL_API_CALL clCreateKernelsInProgram(cl_program  program,
								cl_uint     num_kernels,
								cl_kernel * kernels,
								cl_uint *   num_kernels_ret)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clCreateKernelsInProgram);
        apiLogger << "cl_program program" << program << "cl_uint num_kernels" << num_kernels << "cl_kernel * kernels" << kernels << "cl_uint * num_kernels_ret" << num_kernels_ret;
        OutputListPrinter<cl_kernel, cl_uint> printer("kernels", kernels, num_kernels_ret, num_kernels);
        OutputParamsValueProvider provider(apiLogger, &printer);
	      CALL_INSTRUMENTED_API_LOGGER(CONTEXT_MODULE, cl_int, CreateKernelsInProgram(program, num_kernels, kernels, num_kernels_ret));
    }
    else
    {
        CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_int, CreateKernelsInProgram(program, num_kernels, kernels, num_kernels_ret));
    }
}
SET_ALIAS(clCreateKernelsInProgram);

cl_int CL_API_CALL clRetainKernel(cl_kernel kernel)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clRetainKernel);
        apiLogger << "cl_kernel kernel" << kernel;
	      CALL_INSTRUMENTED_API_LOGGER(CONTEXT_MODULE, cl_int, RetainKernel(kernel));
    }
    else
    {
        CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_int, RetainKernel(kernel));
    }
}
SET_ALIAS(clRetainKernel);

cl_int CL_API_CALL clReleaseKernel(cl_kernel kernel)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clReleaseKernel);
        apiLogger << "cl_kernel kernel" << kernel;
	      CALL_INSTRUMENTED_API_LOGGER(CONTEXT_MODULE, cl_int, ReleaseKernel(kernel));
    }
    else
    {
        CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_int, ReleaseKernel(kernel));
    }
}
SET_ALIAS(clReleaseKernel);

cl_int CL_API_CALL clSetKernelArg(cl_kernel    kernel,
					  cl_uint      arg_indx,
					  size_t       arg_size,
					  const void * arg_value)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clSetKernelArg);
        apiLogger << "cl_kernel kernel" << kernel << "cl_uint arg_indx" << arg_indx << "size_t arg_size" << arg_size << "const void * arg_value" << arg_value;
	      CALL_INSTRUMENTED_API_LOGGER(CONTEXT_MODULE, cl_int, SetKernelArg(kernel, arg_indx, arg_size, arg_value));
    }
    else
    {
        CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_int, SetKernelArg(kernel, arg_indx, arg_size, arg_value));
    }
}
SET_ALIAS(clSetKernelArg);

cl_int CL_API_CALL clGetKernelInfo(cl_kernel      kernel,
					   cl_kernel_info param_name,
					   size_t         param_value_size,
					   void *         param_value,
					   size_t *       param_value_size_ret)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clGetKernelInfo);
        apiLogger << "cl_kernel kernel" << kernel << "cl_kernel_info param_name" << param_name << "size_t param_value_size" << param_value_size << "void * param_value" << param_value << "size_t * param_value_size_ret" << param_value_size_ret;
        OutputParamsValueProvider provider(apiLogger);
        provider.AddParam("param_value_size_ret", param_value_size_ret, false, true);
	      CALL_INSTRUMENTED_API_LOGGER(CONTEXT_MODULE, cl_int, GetKernelInfo(kernel, param_name, param_value_size, param_value, param_value_size_ret));
    }
    else
    {
        CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_int, GetKernelInfo(kernel, param_name, param_value_size, param_value, param_value_size_ret));
    }
}
SET_ALIAS(clGetKernelInfo);

cl_int CL_API_CALL clGetKernelWorkGroupInfo(cl_kernel                 kernel,
								cl_device_id              device,
								cl_kernel_work_group_info param_name,
								size_t                    param_value_size,
								void *                    param_value,
								size_t *                  param_value_size_ret)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clGetKernelWorkGroupInfo);
        apiLogger << "cl_kernel kernel" << kernel << "cl_device_id device" << device << "cl_kernel_work_group_info param_name" << param_name << "size_t param_value_size" << param_value_size << "void * param_value" << param_value << "size_t * param_value_size_ret" << param_value_size_ret;
        OutputParamsValueProvider provider(apiLogger);
        provider.AddParam("param_value_size_ret", param_value_size_ret, false, true);
	      CALL_INSTRUMENTED_API_LOGGER(CONTEXT_MODULE, cl_int, GetKernelWorkGroupInfo(kernel, device, param_name, param_value_size, param_value, param_value_size_ret));
    }
    else
    {
        CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_int, GetKernelWorkGroupInfo(kernel, device, param_name, param_value_size, param_value, param_value_size_ret));
    }
}
SET_ALIAS(clGetKernelWorkGroupInfo);
///////////////////////////////////////////////////////////////////////////////////////////////////
// Event Object APIs
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_int CL_API_CALL clWaitForEvents(cl_uint num_events, const cl_event * event_list)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clWaitForEvents);
        apiLogger << "cl_uint num_events" << num_events << "const cl_event * event_list" << event_list;
	      CALL_INSTRUMENTED_API_LOGGER(EXECUTION_MODULE, cl_int, WaitForEvents(num_events, event_list));
    }
    else
    {
        CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_int, WaitForEvents(num_events, event_list));
    }
}
SET_ALIAS(clWaitForEvents);
cl_int CL_API_CALL clGetEventInfo(cl_event		event,
					  cl_event_info	param_name,
					  size_t		param_value_size,
					  void *		param_value,
					  size_t *		param_value_size_ret)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clGetEventInfo);
        apiLogger << "cl_event event" << event << "cl_event_info param_name" << param_name << "size_t param_value_size" << param_value_size << "void * param_value" << param_value << "size_t * param_value_size_ret" << param_value_size_ret;
        OutputParamsValueProvider provider(apiLogger);
        provider.AddParam("param_value_size_ret", param_value_size_ret, false, true);
	      CALL_INSTRUMENTED_API_LOGGER(EXECUTION_MODULE, cl_int, GetEventInfo(event, param_name, param_value_size, param_value, param_value_size_ret));
    }
    else
    {
        CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_int, GetEventInfo(event, param_name, param_value_size, param_value, param_value_size_ret));
    }
}
SET_ALIAS(clGetEventInfo);

cl_int CL_API_CALL clRetainEvent(cl_event event)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clRetainEvent);
        apiLogger << "cl_event event" << event;
	      CALL_INSTRUMENTED_API_LOGGER(EXECUTION_MODULE, cl_int, RetainEvent(event));
    }
    else
    {
        CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_int, RetainEvent(event));
    }
}
SET_ALIAS(clRetainEvent);

cl_int CL_API_CALL clReleaseEvent(cl_event event)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clReleaseEvent);
        apiLogger << "cl_event event" << event;
	      CALL_INSTRUMENTED_API_LOGGER(EXECUTION_MODULE, cl_int, ReleaseEvent(event));
    }
    else
    {
        CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_int, ReleaseEvent(event));
    }
}
SET_ALIAS(clReleaseEvent);
///////////////////////////////////////////////////////////////////////////////////////////////////
// Profiling APIs
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_int CL_API_CALL clGetEventProfilingInfo(cl_event				event,
							   cl_profiling_info	param_name,
							   size_t				param_value_size,
							   void *				param_value,
							   size_t *				param_value_size_ret)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clGetEventProfilingInfo);
        apiLogger << "cl_event event" << event << "cl_profiling_info param_name" << param_name << "size_t param_value_size" << param_value_size << "void * param_value" << param_value << "size_t * param_value_size_ret" << param_value_size_ret;
        OutputParamsValueProvider provider(apiLogger);
        provider.AddParam("param_value_size_ret", param_value_size_ret, false, true);
	      CALL_INSTRUMENTED_API_LOGGER(EXECUTION_MODULE, cl_int, GetEventProfilingInfo(event, param_name, param_value_size, param_value, param_value_size_ret));
    }
    else
    {
        CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_int, GetEventProfilingInfo(event, param_name, param_value_size, param_value, param_value_size_ret));
    }
}
SET_ALIAS(clGetEventProfilingInfo);
///////////////////////////////////////////////////////////////////////////////////////////////////
// Flush and Finish APIs
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_int CL_API_CALL clFlush(cl_command_queue command_queue)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clFlush);
        apiLogger << "cl_command_queue command_queue" << command_queue;
	      CALL_INSTRUMENTED_API_LOGGER(EXECUTION_MODULE, cl_int, Flush(command_queue));
    }
    else
    {
        CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_int, Flush(command_queue));
    }
}
SET_ALIAS(clFlush);
cl_int CL_API_CALL clFinish(cl_command_queue command_queue)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clFinish);
        apiLogger << "cl_command_queue command_queue" << command_queue;
	      CALL_INSTRUMENTED_API_LOGGER(EXECUTION_MODULE, cl_int, Finish(command_queue));
    }
    else
    {
        CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_int, Finish(command_queue));
    }
}
SET_ALIAS(clFinish);

///////////////////////////////////////////////////////////////////////////////////////////////////
// Enqueued Commands APIs
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_int CL_API_CALL clEnqueueReadBuffer(cl_command_queue	command_queue,
						   cl_mem			buffer,
						   cl_bool			blocking_read,
						   size_t			offset,
						   size_t			cb,
						   void *			ptr,
						   cl_uint			num_events_in_wait_list,
						   const cl_event * event_wait_list,
						   cl_event *		event)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clEnqueueReadBuffer);
        apiLogger << "cl_command_queue command_queue" << command_queue << "cl_mem buffer" << buffer << "cl_bool blocking_read" << blocking_read << "size_t offset" << offset << "size_t cb" << cb << "void * ptr" << ptr << "cl_uint num_events_in_wait_list" << num_events_in_wait_list << "const cl_event * event_wait_list" << event_wait_list << "cl_event * event" << event;
        OutputParamsValueProvider provider(apiLogger);
        provider.AddParam("event", event, true);
        CALL_INSTRUMENTED_API_LOGGER(EXECUTION_MODULE, cl_int, EnqueueReadBuffer(command_queue, buffer, blocking_read, offset, cb, ptr, num_events_in_wait_list, event_wait_list, event, &apiLogger));
    }
    else
    {
        CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_int, EnqueueReadBuffer(command_queue, buffer, blocking_read, offset, cb, ptr, num_events_in_wait_list, event_wait_list, event, nullptr));
    }
}
SET_ALIAS(clEnqueueReadBuffer);

cl_int CL_API_CALL clEnqueueReadBufferRect(
					cl_command_queue    command_queue,
                        cl_mem              buffer,
                        cl_bool             blocking_read,
                        const size_t        buffer_origin[MAX_WORK_DIM],
                        const size_t        host_origin[MAX_WORK_DIM],
                        const size_t        region[MAX_WORK_DIM],
                        size_t              buffer_row_pitch,
                        size_t              buffer_slice_pitch,
                        size_t              host_row_pitch,
                        size_t              host_slice_pitch,
                        void *              ptr,
                        cl_uint             num_events_in_wait_list,
                        const cl_event *    event_wait_list,
                        cl_event *          event )
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clEnqueueReadBufferRect);
        apiLogger << "cl_command_queue command_queue" << command_queue << "cl_mem buffer" << buffer << "cl_bool blocking_read" << blocking_read
            << "const size_t buffer_origin[MAX_WORK_DIM]" << buffer_origin << "const size_t host_origin[MAX_WORK_DIM]" << host_origin << "const size_t region[MAX_WORK_DIM]"
            << "size_t buffer_row_pitch" << buffer_row_pitch << "size_t buffer_slice_pitch" << buffer_slice_pitch << "size_t host_row_pitch" << host_row_pitch
            << "size_t host_slice_pitch" << host_slice_pitch << "void * ptr" << ptr << "cl_uint num_events_in_wait_list" << num_events_in_wait_list
            << "const cl_event * event_wait_list" << event_wait_list << "cl_event * event" << event;
        OutputParamsValueProvider provider(apiLogger);
        provider.AddParam("event", event, true);
	      CALL_INSTRUMENTED_API_LOGGER(EXECUTION_MODULE, cl_int, EnqueueReadBufferRect(command_queue, buffer, blocking_read, buffer_origin, host_origin, region, buffer_row_pitch, buffer_slice_pitch, host_row_pitch, host_slice_pitch, ptr, num_events_in_wait_list, event_wait_list, event, &apiLogger));
    }
    else
    {
        CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_int, EnqueueReadBufferRect(command_queue, buffer, blocking_read, buffer_origin, host_origin, region, buffer_row_pitch, buffer_slice_pitch, host_row_pitch, host_slice_pitch, ptr, num_events_in_wait_list, event_wait_list, event, nullptr));
    }
}
SET_ALIAS(clEnqueueReadBufferRect);

cl_int CL_API_CALL clEnqueueWriteBuffer(cl_command_queue	command_queue,
							cl_mem				buffer,
							cl_bool				blocking_write,
							size_t				offset,
							size_t				cb,
							const void *		ptr,
							cl_uint				num_events_in_wait_list,
							const cl_event *	event_wait_list,
							cl_event *			event)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clEnqueueWriteBuffer);
        apiLogger << "cl_command_queue command_queue" << command_queue << "cl_mem buffer" << buffer << "cl_bool blocking_write" << blocking_write << "size_t offset" << offset << "size_t cb" << cb << "const void * ptr" << ptr << "cl_uint num_events_in_wait_list" << num_events_in_wait_list << "const cl_event * event_wait_list" << event_wait_list << "cl_event * event" << event;
        OutputParamsValueProvider provider(apiLogger);
        provider.AddParam("event", event, true);
	      CALL_INSTRUMENTED_API_LOGGER(EXECUTION_MODULE, cl_int, EnqueueWriteBuffer(command_queue, buffer, blocking_write, offset, cb, ptr, num_events_in_wait_list, event_wait_list, event, &apiLogger));
    }
    else
    {
        CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_int, EnqueueWriteBuffer(command_queue, buffer, blocking_write, offset, cb, ptr, num_events_in_wait_list, event_wait_list, event, nullptr));
    }
}
SET_ALIAS(clEnqueueWriteBuffer);

cl_int CL_API_CALL  clEnqueueWriteBufferRect(
                         cl_command_queue    command_queue,
                         cl_mem              buffer,
                         cl_bool             blocking_read,
                         const size_t        buffer_origin[MAX_WORK_DIM],
                         const size_t        host_origin[MAX_WORK_DIM],
                         const size_t        region[MAX_WORK_DIM],
                         size_t              buffer_row_pitch,
                         size_t              buffer_slice_pitch,
                         size_t              host_row_pitch,
                         size_t              host_slice_pitch,
                         const void *        ptr,
                         cl_uint             num_events_in_wait_list,
                         const cl_event *    event_wait_list,
                         cl_event *          event)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clEnqueueWriteBufferRect);
        apiLogger << "cl_command_queue command_queue" << command_queue << "cl_mem buffer" << buffer << "cl_bool blocking_read" << blocking_read
            << "buffer_origin[0]" << buffer_origin[0] << "buffer_origin[1]" << buffer_origin[1] << "buffer_origin[2]" << buffer_origin[2]
            << "host_origin[0]" << host_origin[0] << "host_origin[1]" << host_origin[1] << "host_origin[2]" << host_origin[2]
            << "region[0]" << region[0] << "region[1]" << region[1] << "region[2]" << region[2] <<  "size_t buffer_row_pitch" << buffer_row_pitch
            << "size_t buffer_slice_pitch" << buffer_slice_pitch << "size_t host_row_pitch" << host_row_pitch << "size_t host_slice_pitch" << host_slice_pitch
            << "const void * ptr" << ptr << "cl_uint num_events_in_wait_list" << num_events_in_wait_list << "const cl_event * event_wait_list" << event_wait_list
            << "cl_event * event" << event;
        OutputParamsValueProvider provider(apiLogger);
        provider.AddParam("event", event, true);
	      CALL_INSTRUMENTED_API_LOGGER(EXECUTION_MODULE, cl_int, EnqueueWriteBufferRect(command_queue, buffer, blocking_read, buffer_origin, host_origin, region, buffer_row_pitch, buffer_slice_pitch, host_row_pitch, host_slice_pitch, ptr, num_events_in_wait_list, event_wait_list, event, &apiLogger));
    }
    else
    {
        CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_int, EnqueueWriteBufferRect(command_queue, buffer, blocking_read, buffer_origin, host_origin, region, buffer_row_pitch, buffer_slice_pitch, host_row_pitch, host_slice_pitch, ptr, num_events_in_wait_list, event_wait_list, event, nullptr));
    }
}
SET_ALIAS(clEnqueueWriteBufferRect);

cl_int CL_API_CALL clEnqueueCopyBuffer(cl_command_queue	command_queue,
						   cl_mem			src_buffer,
						   cl_mem			dst_buffer,
						   size_t			src_offset,
						   size_t			dst_offset,
						   size_t			cb,
						   cl_uint			num_events_in_wait_list,
						   const cl_event * event_wait_list,
						   cl_event *		event)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clEnqueueCopyBuffer);
        apiLogger << "cl_command_queue command_queue" << command_queue << "cl_mem src_buffer" << src_buffer << "cl_mem dst_buffer" << dst_buffer << "size_t src_offset" << src_offset << "size_t dst_offset" << dst_offset << "size_t cb" << cb << "cl_uint num_events_in_wait_list" << num_events_in_wait_list << "const cl_event * event_wait_list" << event_wait_list << "cl_event * event" << event;
        OutputParamsValueProvider provider(apiLogger);
        provider.AddParam("event", event, true);
	      CALL_INSTRUMENTED_API_LOGGER(EXECUTION_MODULE, cl_int, EnqueueCopyBuffer(command_queue, src_buffer, dst_buffer, src_offset, dst_offset, cb, num_events_in_wait_list, event_wait_list, event, &apiLogger));
    }
    else
    {
        CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_int, EnqueueCopyBuffer(command_queue, src_buffer, dst_buffer, src_offset, dst_offset, cb, num_events_in_wait_list, event_wait_list, event, nullptr));
    }
}
SET_ALIAS(clEnqueueCopyBuffer);

cl_int CL_API_CALL clEnqueueCopyBufferRect(cl_command_queue    command_queue,
							cl_mem              src_buffer,
							cl_mem              dst_buffer,
							const size_t        src_origin[MAX_WORK_DIM],
							const size_t        dst_origin[MAX_WORK_DIM],
							const size_t        region[MAX_WORK_DIM],
							size_t              src_row_pitch,
							size_t              src_slice_pitch,
							size_t              dst_row_pitch,
							size_t              dst_slice_pitch,
							cl_uint             num_events_in_wait_list,
							const cl_event *    event_wait_list,
							cl_event *          event)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clEnqueueCopyBufferRect);
        apiLogger << "cl_command_queue command_queue" << command_queue << "cl_mem src_buffer" << src_buffer << "cl_mem dst_buffer" << dst_buffer
            << "src_origin[0]" << src_origin[0] << "src_origin[1]" << src_origin[1] << "src_origin[2]" << src_origin[2]
            << "dst_origin[0]" << dst_origin[0] << "dst_origin[1]" << dst_origin[1] << "dst_origin[2]" << dst_origin[2]
            << "region[0]" << region[0] << "region[1]" << region[1] << "region[2]" << region[2] << "size_t src_row_pitch" << src_row_pitch
            << "size_t src_slice_pitch" << src_slice_pitch << "size_t dst_row_pitch" << dst_row_pitch << "size_t dst_slice_pitch" << dst_slice_pitch
            << "cl_uint num_events_in_wait_list" << num_events_in_wait_list << "const cl_event * event_wait_list" << event_wait_list << "cl_event * event" << event;
        OutputParamsValueProvider provider(apiLogger);
        provider.AddParam("event", event, true);
	      CALL_INSTRUMENTED_API_LOGGER(EXECUTION_MODULE, cl_int, EnqueueCopyBufferRect(command_queue, src_buffer, dst_buffer, src_origin, dst_origin, region, src_row_pitch, src_slice_pitch, dst_row_pitch, dst_slice_pitch, num_events_in_wait_list, event_wait_list, event, &apiLogger));
    }
    else
    {
        CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_int, EnqueueCopyBufferRect(command_queue, src_buffer, dst_buffer, src_origin, dst_origin, region, src_row_pitch, src_slice_pitch, dst_row_pitch, dst_slice_pitch, num_events_in_wait_list, event_wait_list, event, nullptr));
    }
}
SET_ALIAS(clEnqueueCopyBufferRect);

cl_int CL_API_CALL clEnqueueFillBuffer(cl_command_queue command_queue,
		cl_mem buffer,
		const void *pattern,
		size_t pattern_size,
		size_t offset,
		size_t size,
		cl_uint num_events_in_wait_list,
		const cl_event *event_wait_list,
		cl_event *event) CL_API_SUFFIX__VERSION_1_2
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clEnqueueFillBuffer);
        apiLogger << "cl_command_queue command_queue" << command_queue << "cl_mem buffer" << buffer << "const void *pattern" << pattern << "size_t pattern_size" << pattern_size << "size_t offset" << offset << "size_t size" << size << "cl_uint num_events_in_wait_list" << num_events_in_wait_list << "const cl_event *event_wait_list" << event_wait_list << "cl_event *event" << event;
        OutputParamsValueProvider provider(apiLogger);
        provider.AddParam("event", event, true);
	      CALL_INSTRUMENTED_API_LOGGER(EXECUTION_MODULE, cl_int, EnqueueFillBuffer (command_queue, buffer, pattern, pattern_size, offset, size, num_events_in_wait_list, event_wait_list, event, &apiLogger));
    }
    else
    {
        CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_int, EnqueueFillBuffer (command_queue, buffer, pattern, pattern_size, offset, size, num_events_in_wait_list, event_wait_list, event, nullptr));
    }
}
SET_ALIAS(clEnqueueFillBuffer);

cl_int CL_API_CALL clEnqueueReadImage(cl_command_queue command_queue,
						  cl_mem			image,
						  cl_bool			blocking_read,
						  const size_t	    origin[MAX_WORK_DIM],
						  const size_t	    region[MAX_WORK_DIM],
						  size_t			row_pitch,
						  size_t			slice_pitch,
						  void *			ptr,
						  cl_uint			num_events_in_wait_list,
						  const cl_event *	event_wait_list,
						  cl_event *		event)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clEnqueueReadImage);
        apiLogger << "cl_command_queue command_queue" << command_queue << "cl_mem image" << image << "cl_bool blocking_read" << blocking_read
            << "origin[0]" << origin[0] << "origin[1]" << origin[1] << "origin[2]" << origin[2]
            << "region[0]" << region[0] << "region[1]" << region[1] << "region[2]" << region[2] << "size_t row_pitch" << row_pitch << "size_t slice_pitch" << slice_pitch
            << "void * ptr" << ptr << "cl_uint num_events_in_wait_list" << num_events_in_wait_list << "const cl_event * event_wait_list" << event_wait_list
            << "cl_event * event" << event;
        OutputParamsValueProvider provider(apiLogger);
        provider.AddParam("event", event, true);
	      CALL_INSTRUMENTED_API_LOGGER(EXECUTION_MODULE, cl_int, EnqueueReadImage(command_queue, image, blocking_read, origin, region, row_pitch, slice_pitch, ptr, num_events_in_wait_list, event_wait_list, event, &apiLogger));
    }
    else
    {
        CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_int, EnqueueReadImage(command_queue, image, blocking_read, origin, region, row_pitch, slice_pitch, ptr, num_events_in_wait_list, event_wait_list, event, nullptr));
    }
}
SET_ALIAS(clEnqueueReadImage);

cl_int CL_API_CALL clEnqueueWriteImage(cl_command_queue command_queue,
						   cl_mem			image,
						   cl_bool			blocking_write,
						   const size_t	    origin[MAX_WORK_DIM],
						   const size_t	    region[MAX_WORK_DIM],
						   size_t			input_row_pitch,
						   size_t			input_slice_pitch,
						   const void *		ptr,
						   cl_uint			num_events_in_wait_list,
						   const cl_event *	event_wait_list,
						   cl_event *		event)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clEnqueueWriteImage);
        apiLogger << "cl_command_queue command_queue" << command_queue << "cl_mem image" << image << "cl_bool blocking_write" << blocking_write
            << "origin[0]" << origin[0] << "origin[1]" << origin[1] << "origin[2]" << origin[2]
            << "region[0]" << region[0] << "region[1]" << region[1] << "region[2]" << region[2] << "size_t input_row_pitch" << input_row_pitch
            << "size_t input_slice_pitch" << input_slice_pitch << "const void * ptr" << ptr << "cl_uint num_events_in_wait_list" << num_events_in_wait_list
            << "const cl_event * event_wait_list" << event_wait_list << "cl_event * event" << event;
        OutputParamsValueProvider provider(apiLogger);
        provider.AddParam("event", event, true);
	      CALL_INSTRUMENTED_API_LOGGER(EXECUTION_MODULE, cl_int, EnqueueWriteImage(command_queue, image, blocking_write, origin, region, input_row_pitch, input_slice_pitch, ptr, num_events_in_wait_list, event_wait_list, event, &apiLogger));
    }
    else
    {
        CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_int, EnqueueWriteImage(command_queue, image, blocking_write, origin, region, input_row_pitch, input_slice_pitch, ptr, num_events_in_wait_list, event_wait_list, event, nullptr));
    }
}
SET_ALIAS(clEnqueueWriteImage);

cl_int CL_API_CALL clEnqueueCopyImage(cl_command_queue	command_queue,
						  cl_mem			src_image,
						  cl_mem			dst_image,
						  const size_t  	src_origin[MAX_WORK_DIM],
						  const size_t  	dst_origin[MAX_WORK_DIM],
						  const size_t  	region[MAX_WORK_DIM],
						  cl_uint			num_events_in_wait_list,
						  const cl_event *	event_wait_list,
						  cl_event *		event)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clEnqueueCopyImage);
        apiLogger << "cl_command_queue command_queue" << command_queue << "cl_mem src_image" << src_image << "cl_mem dst_image" << dst_image
            << "src_origin[0]" << src_origin[0] << "src_origin[1]" << src_origin[1] << "src_origin[2]" << src_origin[2]
            << "dst_origin[0]" << dst_origin[0] << "dst_origin[1]" << dst_origin[1] << "dst_origin[2]" << dst_origin[2]
            << "region[0]" << region[0] << "region[1]" << region[1] << "region[2]" << region[2]
            << "cl_uint num_events_in_wait_list" << num_events_in_wait_list << "const cl_event * event_wait_list" << event_wait_list << "cl_event * event" << event;
        OutputParamsValueProvider provider(apiLogger);
        provider.AddParam("event", event, true);
	      CALL_INSTRUMENTED_API_LOGGER(EXECUTION_MODULE, cl_int, EnqueueCopyImage(command_queue, src_image, dst_image, src_origin, dst_origin, region, num_events_in_wait_list, event_wait_list, event, &apiLogger));
    }
    else
    {
        CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_int, EnqueueCopyImage(command_queue, src_image, dst_image, src_origin, dst_origin, region, num_events_in_wait_list, event_wait_list, event, nullptr));
    }
}
SET_ALIAS(clEnqueueCopyImage);


cl_int CL_API_CALL clEnqueueFillImage (cl_command_queue command_queue,
						cl_mem image,
						const void *fill_color,
						const size_t *origin,
						const size_t *region,
						cl_uint num_events_in_wait_list,
						const cl_event *event_wait_list,
						cl_event *event)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clEnqueueFillImage);
        apiLogger << "cl_command_queue command_queue" << command_queue << "cl_mem image" << image << "const void *fill_color" << fill_color << "const size_t *origin" << origin << "const size_t *region" << region << "cl_uint num_events_in_wait_list" << num_events_in_wait_list << "const cl_event *event_wait_list" << event_wait_list << "cl_event *event" << event;
        OutputParamsValueProvider provider(apiLogger);
        provider.AddParam("event", event, true);
	      CALL_INSTRUMENTED_API_LOGGER(EXECUTION_MODULE, cl_int, EnqueueFillImage(command_queue, image, fill_color, origin, region, num_events_in_wait_list, event_wait_list, event, &apiLogger));
    }
    else
    {
        CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_int, EnqueueFillImage(command_queue, image, fill_color, origin, region, num_events_in_wait_list, event_wait_list, event, nullptr));
    }
}
SET_ALIAS(clEnqueueFillImage);

cl_int CL_API_CALL clEnqueueCopyImageToBuffer(cl_command_queue	command_queue,
								  cl_mem			src_image,
								  cl_mem			dst_buffer,
								  const size_t  	src_origin[MAX_WORK_DIM],
								  const size_t  	region[MAX_WORK_DIM],
								  size_t			dst_offset,
								  cl_uint			num_events_in_wait_list,
								  const cl_event *	event_wait_list,
								  cl_event *		event)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clEnqueueCopyImageToBuffer);
        apiLogger << "cl_command_queue command_queue" << command_queue << "cl_mem src_image" << src_image << "cl_mem dst_buffer" << dst_buffer
            << "src_origin[0]" << src_origin[0] << "src_origin[1]" << src_origin[1] << "src_origin[2]" << src_origin[2]
            << "region[0]" << region[0] << "region[1]" << region[1] << "region[2]" << region[2] << "size_t dst_offset" << dst_offset << "cl_uint num_events_in_wait_list" << num_events_in_wait_list << "const cl_event * event_wait_list" << event_wait_list << "cl_event * event" << event;
        OutputParamsValueProvider provider(apiLogger);
        provider.AddParam("event", event, true);
	      CALL_INSTRUMENTED_API_LOGGER(EXECUTION_MODULE, cl_int, EnqueueCopyImageToBuffer(command_queue, src_image, dst_buffer, src_origin, region, dst_offset, num_events_in_wait_list, event_wait_list, event, &apiLogger));
    }
    else
    {
        CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_int, EnqueueCopyImageToBuffer(command_queue, src_image, dst_buffer, src_origin, region, dst_offset, num_events_in_wait_list, event_wait_list, event, nullptr));
    }
}
SET_ALIAS(clEnqueueCopyImageToBuffer);

cl_int CL_API_CALL clEnqueueCopyBufferToImage(cl_command_queue	command_queue,
								  cl_mem			src_buffer,
								  cl_mem			dst_image,
								  size_t			src_offset,
								  const size_t  	dst_origin[MAX_WORK_DIM],
								  const size_t  	region[MAX_WORK_DIM],
								  cl_uint			num_events_in_wait_list,
								  const cl_event *	event_wait_list,
								  cl_event *		event)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clEnqueueCopyBufferToImage);
        apiLogger << "cl_command_queue command_queue" << command_queue << "cl_mem src_buffer" << src_buffer << "cl_mem dst_image" << dst_image
            << "size_t src_offset" << src_offset << "dst_origin[0]" << dst_origin[0] << "dst_origin[1]" << dst_origin[1] << "dst_origin[2]" << dst_origin[2]
            << "region[0]" << region[0] << "region[1]" << region[1] << "region[2]" << region[2] << "cl_uint num_events_in_wait_list" << num_events_in_wait_list << "const cl_event * event_wait_list" << event_wait_list << "cl_event * event" << event;
        OutputParamsValueProvider provider(apiLogger);
        provider.AddParam("event", event, true);
	      CALL_INSTRUMENTED_API_LOGGER(EXECUTION_MODULE, cl_int, EnqueueCopyBufferToImage(command_queue, src_buffer, dst_image, src_offset, dst_origin, region, num_events_in_wait_list, event_wait_list, event, &apiLogger));
    }
    else
    {
        CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_int, EnqueueCopyBufferToImage(command_queue, src_buffer, dst_image, src_offset, dst_origin, region, num_events_in_wait_list, event_wait_list, event, nullptr));
    }
}
SET_ALIAS(clEnqueueCopyBufferToImage);

void * CL_API_CALL clEnqueueMapBuffer(cl_command_queue	command_queue,
						  cl_mem			buffer,
						  cl_bool			blocking_map,
						  cl_map_flags		map_flags,
						  size_t			offset,
						  size_t			cb,
						  cl_uint			num_events_in_wait_list,
						  const cl_event *	event_wait_list,
						  cl_event *		event,
						  cl_int *			errcode_ret)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clEnqueueMapBuffer);
        apiLogger << "cl_command_queue command_queue" << command_queue << "cl_mem buffer" << buffer << "cl_bool blocking_map" << blocking_map << "cl_map_flags map_flags" << map_flags << "size_t offset" << offset << "size_t cb" << cb << "cl_uint num_events_in_wait_list" << num_events_in_wait_list << "const cl_event * event_wait_list" << event_wait_list << "cl_event * event" << event << "cl_int * errcode_ret" << errcode_ret;
        OutputParamsValueProvider provider(apiLogger);
        provider.AddParam("event", event, true);
        provider.AddParam("errcode_ret", errcode_ret, false, false);
	      CALL_INSTRUMENTED_API_LOGGER(EXECUTION_MODULE, void *, EnqueueMapBuffer(command_queue, buffer, blocking_map, map_flags, offset, cb, num_events_in_wait_list, event_wait_list, event, errcode_ret, &apiLogger));
    }
    else
    {
        CALL_INSTRUMENTED_API(EXECUTION_MODULE, void *, EnqueueMapBuffer(command_queue, buffer, blocking_map, map_flags, offset, cb, num_events_in_wait_list, event_wait_list, event, errcode_ret, nullptr));
    }
}
SET_ALIAS(clEnqueueMapBuffer);

void * CL_API_CALL clEnqueueMapImage(cl_command_queue	command_queue,
						 cl_mem				image,
						 cl_bool			blocking_map,
						 cl_map_flags		map_flags,
						 const size_t 		origin[MAX_WORK_DIM],
						 const size_t 		region[MAX_WORK_DIM],
						 size_t *			image_row_pitch,
						 size_t *			image_slice_pitch,
						 cl_uint			num_events_in_wait_list,
						 const cl_event *	event_wait_list,
						 cl_event *			event,
						 cl_int *			errcode_ret)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clEnqueueMapImage);
        apiLogger << "cl_command_queue command_queue" << command_queue << "cl_mem image" << image << "cl_bool blocking_map" << blocking_map << "cl_map_flags map_flags" << map_flags
            << "origin[0]" << origin[0] << "origin[1]" << origin[1] << "origin[2]" << origin[2]
            << "region[0]" << region[0] << "region[1]" << region[1] << "region[2]" << region[2] << "size_t * image_row_pitch" << image_row_pitch << "size_t * image_slice_pitch" << image_slice_pitch << "cl_uint num_events_in_wait_list" << num_events_in_wait_list << "const cl_event * event_wait_list" << event_wait_list << "cl_event * event" << event << "cl_int * errcode_ret" << errcode_ret;
        OutputParamsValueProvider provider(apiLogger);
        provider.AddParam("event", event, true);
        provider.AddParam("errcode_ret", errcode_ret, false, false);
	      CALL_INSTRUMENTED_API_LOGGER(EXECUTION_MODULE, void *, EnqueueMapImage(command_queue, image, blocking_map, map_flags, origin, region, image_row_pitch, image_slice_pitch, num_events_in_wait_list, event_wait_list, event, errcode_ret, &apiLogger));
    }
    else
    {
        CALL_INSTRUMENTED_API(EXECUTION_MODULE, void *, EnqueueMapImage(command_queue, image, blocking_map, map_flags, origin, region, image_row_pitch, image_slice_pitch, num_events_in_wait_list, event_wait_list, event, errcode_ret, nullptr));
    }
}
SET_ALIAS(clEnqueueMapImage);

cl_int CL_API_CALL clEnqueueUnmapMemObject(cl_command_queue	command_queue,
							   cl_mem			memobj,
							   void *			mapped_ptr,
							   cl_uint			num_events_in_wait_list,
							   const cl_event * event_wait_list,
							   cl_event *		event)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clEnqueueUnmapMemObject);
        apiLogger << "cl_command_queue command_queue" << command_queue << "cl_mem memobj" << memobj << "void * mapped_ptr" << mapped_ptr << "cl_uint num_events_in_wait_list" << num_events_in_wait_list << "const cl_event * event_wait_list" << event_wait_list << "cl_event * event" << event;
        OutputParamsValueProvider provider(apiLogger);
        provider.AddParam("event", event, true);
	      CALL_INSTRUMENTED_API_LOGGER(EXECUTION_MODULE, cl_int, EnqueueUnmapMemObject(command_queue, memobj, mapped_ptr, num_events_in_wait_list, event_wait_list, event, &apiLogger));
    }
    else
    {
        CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_int, EnqueueUnmapMemObject(command_queue, memobj, mapped_ptr, num_events_in_wait_list, event_wait_list, event, nullptr));
    }
}
SET_ALIAS(clEnqueueUnmapMemObject);

cl_int CL_API_CALL clEnqueueNDRangeKernel(cl_command_queue	command_queue,
							  cl_kernel			kernel,
							  cl_uint			work_dim,
							  const size_t *	global_work_offset,
							  const size_t *	global_work_size,
							  const size_t *	local_work_size,
							  cl_uint			num_events_in_wait_list,
							  const cl_event *	event_wait_list,
							  cl_event *		event)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clEnqueueNDRangeKernel);
        apiLogger << "cl_command_queue command_queue" << command_queue << "cl_kernel kernel" << kernel << "cl_uint work_dim" << work_dim << "const size_t * global_work_offset" << global_work_offset << "const size_t * global_work_size" << global_work_size << "const size_t * local_work_size" << local_work_size << "cl_uint num_events_in_wait_list" << num_events_in_wait_list << "const cl_event * event_wait_list" << event_wait_list << "cl_event * event" << event;
        OutputParamsValueProvider provider(apiLogger);
        provider.AddParam("event", event, true);
	      CALL_INSTRUMENTED_API_LOGGER(EXECUTION_MODULE, cl_int, EnqueueNDRangeKernel(command_queue, kernel, work_dim, global_work_offset, global_work_size, local_work_size, num_events_in_wait_list, event_wait_list, event, &apiLogger));
    }
    else
    {
        CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_int, EnqueueNDRangeKernel(command_queue, kernel, work_dim, global_work_offset, global_work_size, local_work_size, num_events_in_wait_list, event_wait_list, event, nullptr));
    }
}
SET_ALIAS(clEnqueueNDRangeKernel);

cl_int CL_API_CALL clEnqueueTask(cl_command_queue	command_queue,
					 cl_kernel			kernel,
					 cl_uint			num_events_in_wait_list,
					 const cl_event *	event_wait_list,
					 cl_event *			event)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clEnqueueTask);
        apiLogger << "cl_command_queue command_queue" << command_queue << "cl_kernel kernel" << kernel << "cl_uint num_events_in_wait_list" << num_events_in_wait_list << "const cl_event * event_wait_list" << event_wait_list << "cl_event * event" << event;
        OutputParamsValueProvider provider(apiLogger);
        provider.AddParam("event", event, true);
	      CALL_INSTRUMENTED_API_LOGGER(EXECUTION_MODULE, cl_int, EnqueueTask(command_queue, kernel, num_events_in_wait_list, event_wait_list, event, &apiLogger));
    }
    else
    {
        CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_int, EnqueueTask(command_queue, kernel, num_events_in_wait_list, event_wait_list, event, nullptr));
    }
}
SET_ALIAS(clEnqueueTask);

cl_int CL_API_CALL clEnqueueNativeKernel(cl_command_queue	command_queue,
							 void (CL_CALLBACK *user_func)(void *),
							 void *				args,
							 size_t				cb_args,
							 cl_uint			num_mem_objects,
							 const cl_mem *		mem_list,
							 const void **		args_mem_loc,
							 cl_uint			num_events_in_wait_list,
							 const cl_event *	event_wait_list,
							 cl_event *			event)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clEnqueueNativeKernel);
        apiLogger << "cl_command_queue command_queue" << command_queue << "void (CL_CALLBACK *user_func)(void *)" << user_func << "void * args" << args << "size_t cb_args" << cb_args << "cl_uint num_mem_objects" << num_mem_objects << "const cl_mem * mem_list" << mem_list << "const void ** args_mem_loc" << args_mem_loc << "cl_uint num_events_in_wait_list" << num_events_in_wait_list << "const cl_event * event_wait_list" << event_wait_list << "cl_event * event" << event;
        OutputParamsValueProvider provider(apiLogger);
        provider.AddParam("event", event, true);
	      CALL_INSTRUMENTED_API_LOGGER(EXECUTION_MODULE, cl_int, EnqueueNativeKernel(command_queue, user_func, args, cb_args, num_mem_objects, mem_list, args_mem_loc, num_events_in_wait_list, event_wait_list, event, &apiLogger));
    }
    else
    {
        CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_int, EnqueueNativeKernel(command_queue, user_func, args, cb_args, num_mem_objects, mem_list, args_mem_loc, num_events_in_wait_list, event_wait_list, event, nullptr));
    }
}
SET_ALIAS(clEnqueueNativeKernel);

cl_int CL_API_CALL clEnqueueMarker(cl_command_queue command_queue, cl_event * event)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clEnqueueMarker);
        apiLogger << "cl_command_queue command_queue" << command_queue << "cl_event * event" << event;
        OutputParamsValueProvider provider(apiLogger);
        provider.AddParam("event", event, true);
	      CALL_INSTRUMENTED_API_LOGGER(EXECUTION_MODULE, cl_int, EnqueueMarker(command_queue, event, &apiLogger));
    }
    else
    {
        CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_int, EnqueueMarker(command_queue, event, nullptr));
    }
}
SET_ALIAS(clEnqueueMarker);

cl_int CL_API_CALL clEnqueueWaitForEvents(cl_command_queue	command_queue,
							  cl_uint			num_events,
							  const cl_event *	event_list)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clEnqueueWaitForEvents);
        apiLogger << "cl_command_queue command_queue" << command_queue << "cl_uint num_events" << num_events << "const cl_event * event_list" << event_list;
	      CALL_INSTRUMENTED_API_LOGGER(EXECUTION_MODULE, cl_int, EnqueueWaitForEvents(command_queue, num_events, event_list, &apiLogger));
    }
    else
    {
        CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_int, EnqueueWaitForEvents(command_queue, num_events, event_list, nullptr));
    }
}
SET_ALIAS(clEnqueueWaitForEvents);

cl_int CL_API_CALL clEnqueueBarrier(cl_command_queue command_queue)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clEnqueueBarrier);
        apiLogger << "cl_command_queue command_queue" << command_queue;
	      CALL_INSTRUMENTED_API_LOGGER(EXECUTION_MODULE, cl_int, EnqueueBarrier(command_queue, &apiLogger));
    }
    else
    {
        CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_int, EnqueueBarrier(command_queue, nullptr));
    }
}
SET_ALIAS(clEnqueueBarrier);

///////////////////////////////////////////////////////////////////////////////////////////////////
// New OpenCL 1.1 functions
///////////////////////////////////////////////////////////////////////////////////////////////////

cl_event CL_API_CALL
clCreateUserEvent(cl_context    context,
				  cl_int *      errcode_ret)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(CL_API_CALLclCreateUserEvent);
        apiLogger << "cl_context context" << context << "cl_int * errcode_ret" << errcode_ret;
        OutputParamsValueProvider provider(apiLogger);
        provider.AddParam("errcode_ret", errcode_ret, false, false);
	      CALL_INSTRUMENTED_API_LOGGER(EXECUTION_MODULE, cl_event, CreateUserEvent(context, errcode_ret));
    }
    else
    {
        CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_event, CreateUserEvent(context, errcode_ret));
    }
}
SET_ALIAS(clCreateUserEvent);

cl_int CL_API_CALL
clSetEventCallback( cl_event    evt,
				   cl_int      command_exec_callback_type,
				   void (CL_CALLBACK *pfn_notify)(cl_event, cl_int, void *),
				   void *      user_data)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(CL_API_CALLclSetEventCallback);
        apiLogger << "cl_event evt" << evt << "cl_int command_exec_callback_type" << command_exec_callback_type
            << "void (CL_CALLBACK *pfn_notify)(cl_event, cl_int, void *)" << pfn_notify << "void * user_data" << user_data;
	      CALL_INSTRUMENTED_API_LOGGER(EXECUTION_MODULE, cl_int, SetEventCallback(evt, command_exec_callback_type, pfn_notify, user_data));
    }
    else
    {
        CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_int, SetEventCallback(evt, command_exec_callback_type, pfn_notify, user_data));
    }
}
SET_ALIAS(clSetEventCallback);

cl_int CL_API_CALL
clSetUserEventStatus(cl_event   evt,
					 cl_int     execution_status)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(CL_API_CALLclSetUserEventStatus);
        apiLogger << "cl_event evt" << evt << "cl_int execution_status" << execution_status;
	      CALL_INSTRUMENTED_API_LOGGER(EXECUTION_MODULE, cl_int, SetUserEventStatus(evt, execution_status));
    }
    else
    {
        CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_int, SetUserEventStatus(evt, execution_status));
    }
}
SET_ALIAS(clSetUserEventStatus);

// Check if the current CPU is supported. returns 0 if it does and 1 othrewise
// Criteria: supports SSSE3 and SSE4.1 and SSE4.2

int IsCPUSupported(void)
{
	if( CPUDetect::GetInstance()->IsFeatureSupported(CFS_SSE41) )
	{
		return 0;
	}
	return 1;
}

// check if the cpu feature is supported
// returns 0 is it does and 1 otherwise
int IsFeatureSupported(int iCPUFeature)
{
	if (CPUDetect::GetInstance()->IsFeatureSupported((Intel::OpenCL::Utils::ECPUFeatureSupport)iCPUFeature))
	{
		return 0;
	}
	return 1;
}

cl_int CL_API_CALL clRetainDevice(cl_device_id devId)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clRetainDevice);
        apiLogger << "cl_device_id devId" << devId;
	      CALL_INSTRUMENTED_API_LOGGER(PLATFORM_MODULE, cl_int, clRetainDevice(devId));
    }
    else
    {
        CALL_INSTRUMENTED_API(PLATFORM_MODULE, cl_int, clRetainDevice(devId));
    }
}
SET_ALIAS(clRetainDevice);

cl_int CL_API_CALL clReleaseDevice(cl_device_id device)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clReleaseDevice);
        apiLogger << "cl_device_id device" << device;
	      CALL_INSTRUMENTED_API_LOGGER(PLATFORM_MODULE, cl_int, clReleaseDevice(device));
    }
    else
    {
        CALL_INSTRUMENTED_API(PLATFORM_MODULE, cl_int, clReleaseDevice(device));
    }
}
SET_ALIAS(clReleaseDevice);

cl_int CL_API_CALL clCreateSubDevices(cl_device_id device,
									  const cl_device_partition_property* properties,
									  cl_uint num_entries,
									  cl_device_id* out_devices,
									  cl_uint* num_devices)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clCreateSubDevices);
        apiLogger << "cl_device_id device" << device << "const cl_device_partition_property* properties" << properties << "cl_uint num_entries" << num_entries << "cl_device_id* out_devices" << out_devices << "cl_uint* num_devices" << num_devices;
        OutputListPrinter<cl_device_id, cl_uint> printer("out_devices", out_devices, num_devices, num_entries);
        OutputParamsValueProvider provider(apiLogger, &printer);
	      CALL_INSTRUMENTED_API_LOGGER(PLATFORM_MODULE, cl_int, clCreateSubDevices(device, properties, num_entries, out_devices, num_devices));
    }
    else
    {
        CALL_INSTRUMENTED_API(PLATFORM_MODULE, cl_int, clCreateSubDevices(device, properties, num_entries, out_devices, num_devices));
    }
}
SET_ALIAS(clCreateSubDevices);

///////////////////////////////////////////////////////////////////////////////////////////
// OpenCL 1.2 functions
///////////////////////////////////////////////////////////////////////////////////////////
cl_int CL_API_CALL clGetKernelArgInfo(cl_kernel		kernel,
								cl_uint				arg_indx,
								cl_kernel_arg_info	param_name,
								size_t				param_value_size,
								void *				param_value,
								size_t *			param_value_size_ret)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clGetKernelArgInfo);
        apiLogger << "cl_kernel kernel" << kernel << "cl_uint arg_indx" << arg_indx << "cl_kernel_arg_info param_name" << param_name << "size_t param_value_size" << param_value_size << "void * param_value" << param_value << "size_t * param_value_size_ret" << param_value_size_ret;
        OutputParamsValueProvider provider(apiLogger);
        provider.AddParam("param_value_size_ret", param_value_size_ret, false, true);
	      CALL_INSTRUMENTED_API_LOGGER(CONTEXT_MODULE, cl_int, GetKernelArgInfo(kernel, arg_indx, param_name, param_value_size, param_value, param_value_size_ret));
    }
    else
    {
        CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_int, GetKernelArgInfo(kernel, arg_indx, param_name, param_value_size, param_value, param_value_size_ret));
    }
}
SET_ALIAS(clGetKernelArgInfo);

cl_int CL_API_CALL clEnqueueMarkerWithWaitList(cl_command_queue command_queue,
                                               cl_uint num_events_in_wait_list,
                                               const cl_event *event_wait_list,
                                               cl_event *event)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clEnqueueMarkerWithWaitList);
        apiLogger << "cl_command_queue command_queue" << command_queue << "cl_uint num_events_in_wait_list" << num_events_in_wait_list << "const cl_event *event_wait_list" << event_wait_list << "cl_event *event" << event;
        OutputParamsValueProvider provider(apiLogger);
        provider.AddParam("event", event, true);
        CALL_INSTRUMENTED_API_LOGGER(EXECUTION_MODULE, cl_int, EnqueueMarkerWithWaitList(command_queue, num_events_in_wait_list, event_wait_list, event, &apiLogger));
    }
    else
    {
        CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_int, EnqueueMarkerWithWaitList(command_queue, num_events_in_wait_list, event_wait_list, event, nullptr));
    }
}
SET_ALIAS(clEnqueueMarkerWithWaitList);

cl_int CL_API_CALL clEnqueueBarrierWithWaitList(cl_command_queue command_queue,
                                                cl_uint num_events_in_wait_list,
                                                const cl_event *event_wait_list,
                                                cl_event *event)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clEnqueueBarrierWithWaitList);
        apiLogger << "cl_command_queue command_queue" << command_queue << "cl_uint num_events_in_wait_list" << num_events_in_wait_list << "const cl_event *event_wait_list" << event_wait_list << "cl_event *event" << event;
        OutputParamsValueProvider provider(apiLogger);
        provider.AddParam("event", event, true);
        CALL_INSTRUMENTED_API_LOGGER(EXECUTION_MODULE, cl_int, EnqueueBarrierWithWaitList(command_queue, num_events_in_wait_list, event_wait_list, event, &apiLogger));
    }
    else
    {
        CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_int, EnqueueBarrierWithWaitList(command_queue, num_events_in_wait_list, event_wait_list, event, nullptr));
    }
}
SET_ALIAS(clEnqueueBarrierWithWaitList);

cl_int CL_API_CALL clEnqueueMigrateMemObjects(cl_command_queue command_queue,
                                              cl_uint num_mem_objects,
                                              const cl_mem *mem_objects,
                                              cl_mem_migration_flags flags,
                                              cl_uint num_events_in_wait_list,
                                              const cl_event *event_wait_list,
                                              cl_event *event)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clEnqueueMigrateMemObjects);
        apiLogger << "cl_command_queue command_queue" << command_queue << "cl_uint num_mem_objects" << num_mem_objects << "const cl_mem *mem_objects" << mem_objects << "cl_mem_migration_flags flags" << flags << "cl_uint num_events_in_wait_list" << num_events_in_wait_list << "const cl_event *event_wait_list" << event_wait_list << "cl_event *event" << event;
        OutputParamsValueProvider provider(apiLogger);
        provider.AddParam("event", event, true);
        CALL_INSTRUMENTED_API_LOGGER(EXECUTION_MODULE, cl_int, EnqueueMigrateMemObjects(command_queue,
            num_mem_objects, mem_objects, flags, num_events_in_wait_list, event_wait_list, event, &apiLogger));
    }
    else
    {
        CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_int, EnqueueMigrateMemObjects(command_queue,
            num_mem_objects, mem_objects, flags, num_events_in_wait_list, event_wait_list, event, nullptr));
    }
}
SET_ALIAS(clEnqueueMigrateMemObjects);

cl_int CL_API_CALL clEnqueueSVMMigrateMem(cl_command_queue command_queue,
                                          cl_uint          num_svm_pointers,
                                          const void**     svm_pointers,
                                          const size_t*    sizes,
                                          cl_mem_migration_flags flags,
                                          cl_uint num_events_in_wait_list,
                                          const cl_event*  event_wait_list,
                                          cl_event*        event)
{
    if (FrameworkProxy::Instance()->GetOCLConfig()->GetOpenCLVersion() < OPENCL_VERSION_2_1)
    {
        return CL_INVALID_OPERATION;
    }

    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clEnqueueSVMMigrateMem);
        apiLogger << "cl_command_queue command_queue" << command_queue
                  << "cl_uint num_svm_pointers" << num_svm_pointers
                  << "const void** svm_pointers" << svm_pointers
                  << "const size_t* sizes" << sizes
                  << "cl_mem_migration_flags flags" << flags
                  << "cl_uint num_events_in_wait_list" << num_events_in_wait_list
                  << "const cl_event* event_wait_list" << event_wait_list
                  << "cl_event* event" << event;
        OutputParamsValueProvider provider(apiLogger);
        provider.AddParam("event", event, true);
        CALL_INSTRUMENTED_API_LOGGER(EXECUTION_MODULE, cl_int, EnqueueSVMMigrateMem(command_queue, num_svm_pointers, svm_pointers, sizes, flags, num_events_in_wait_list, event_wait_list, event, &apiLogger));
    }
    else
    {
        CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_int, EnqueueSVMMigrateMem(command_queue, num_svm_pointers, svm_pointers, sizes, flags, num_events_in_wait_list, event_wait_list, event, nullptr));
    }
}
SET_ALIAS(clEnqueueSVMMigrateMem);

cl_int CL_API_CALL clCompileProgram(cl_program program,
                                    cl_uint num_devices,
                                    const cl_device_id *device_list,
                                    const char *options,
                                    cl_uint num_input_headers,
                                    const cl_program *input_headers,
                                    const char **header_include_names,
                                    void (CL_CALLBACK *pfn_notify)(cl_program program, void *user_data),
                                    void *user_data)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clCompileProgram);
        apiLogger << "cl_program program" << program << "cl_uint num_devices" << num_devices << "const cl_device_id *device_list" << device_list << "const char *options";
        apiLogger.PrintCStringVal(options) << "cl_uint num_input_headers" << num_input_headers << "const cl_program *input_headers" << input_headers << "const char **header_include_names" << header_include_names << "void (CL_CALLBACK *pfn_notify)(cl_program program, void *user_data)" << pfn_notify << "void *user_data" << user_data;
        CALL_INSTRUMENTED_API_LOGGER(CONTEXT_MODULE, cl_int, CompileProgram(program, num_devices, device_list, options,
            num_input_headers, input_headers, header_include_names, pfn_notify, user_data));
    }
    else
    {
        CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_int, CompileProgram(program, num_devices, device_list, options,
            num_input_headers, input_headers, header_include_names, pfn_notify, user_data));
    }
}
SET_ALIAS(clCompileProgram);

cl_program CL_API_CALL clLinkProgram(cl_context context,
                                     cl_uint num_devices,
                                     const cl_device_id *device_list,
                                     const char *options,
                                     cl_uint num_input_programs,
                                     const cl_program *input_programs,
                                     void (CL_CALLBACK *pfn_notify)(cl_program program, void *user_data),
                                     void *user_data,
                                     cl_int *errcode_ret)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clLinkProgram);
        apiLogger << "cl_context context" << context << "cl_uint num_devices" << num_devices << "const cl_device_id *device_list" << device_list << "const char *options";
        apiLogger.PrintCStringVal(options) << "cl_uint num_input_programs" << num_input_programs << "const cl_program *input_programs" << input_programs << "void (CL_CALLBACK *pfn_notify)(cl_program program, void *user_data)" << pfn_notify << "void *user_data" << user_data << "cl_int *errcode_ret" << errcode_ret;
        OutputParamsValueProvider provider(apiLogger);
        provider.AddParam("errcode_ret", errcode_ret, false, false);
        CALL_INSTRUMENTED_API_LOGGER(CONTEXT_MODULE, cl_program, LinkProgram(context, num_devices, device_list, options,
            num_input_programs, input_programs, pfn_notify, user_data, errcode_ret));
    }
    else
    {
        CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_program, LinkProgram(context, num_devices, device_list, options,
            num_input_programs, input_programs, pfn_notify, user_data, errcode_ret));
    }
}
SET_ALIAS(clLinkProgram);

void* CL_API_CALL clSVMAlloc(cl_context context,
							 cl_svm_mem_flags flags,
							 size_t size,
							 unsigned int alignment)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clSVMAlloc);
        apiLogger << "cl_context context" << context << "cl_svm_mem_flags flags" << flags << "size_t size" << size << "unsigned int alignment" << alignment;
	      CALL_INSTRUMENTED_API_LOGGER(CONTEXT_MODULE, void*, SVMAlloc(context, flags, size, alignment));
    }
    else
    {
        CALL_INSTRUMENTED_API(CONTEXT_MODULE, void*, SVMAlloc(context, flags, size, alignment));
    }
}
SET_ALIAS(clSVMAlloc);

void CL_API_CALL clSVMFree(cl_context context,
						   void* svm_pointer)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clSVMFree);
        apiLogger << "cl_context context" << context << "void* svm_pointer" << svm_pointer;
	      CALL_INSTRUMENTED_API_LOGGER_NO_RET(CONTEXT_MODULE, SVMFree(context, svm_pointer));
    }
    else
    {
        CALL_INSTRUMENTED_API_NO_RET(CONTEXT_MODULE, SVMFree(context, svm_pointer));
    }
}
SET_ALIAS(clSVMFree);

cl_int CL_API_CALL clEnqueueSVMFree(cl_command_queue command_queue,
									cl_uint num_svm_pointers,
									void* svm_pointers[],
									void (CL_CALLBACK* pfn_free_func)(
										cl_command_queue queue,
										cl_uint num_svm_pointers,
										void* svm_pointers[],
										void* user_data),
									void* user_data,
									cl_uint num_events_in_wait_list,
									const cl_event* event_wait_list,
									cl_event* event)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clEnqueueSVMFree);
        apiLogger << "cl_command_queue command_queue" << command_queue << "cl_uint num_svm_pointers" << num_svm_pointers << "void* svm_pointers[]" << svm_pointers
            << "void (CL_CALLBACK* pfn_free_func)(cl_command_queue queue queue, cl_uint num_svm_pointers, void* svm_pointers[], void* user_data)" << pfn_free_func << "void* user_data" << user_data << "cl_uint num_events_in_wait_list" << num_events_in_wait_list << "const cl_event* event_wait_list" << event_wait_list << "cl_event* event" << event;
        OutputParamsValueProvider provider(apiLogger);
        provider.AddParam("event", event, true);
	      CALL_INSTRUMENTED_API_LOGGER(EXECUTION_MODULE, cl_int, EnqueueSVMFree(command_queue, num_svm_pointers, svm_pointers, pfn_free_func, user_data, num_events_in_wait_list, event_wait_list,
		        event, &apiLogger));
    }
    else
    {
        CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_int, EnqueueSVMFree(command_queue, num_svm_pointers, svm_pointers, pfn_free_func, user_data, num_events_in_wait_list, event_wait_list,
		        event, nullptr));
    }
}
SET_ALIAS(clEnqueueSVMFree);

cl_int CL_API_CALL clEnqueueSVMMemcpy(cl_command_queue command_queue,
									  cl_bool blocking_copy,
									  void *dst_ptr,
									  const void *src_ptr,
									  size_t size,
									  cl_uint num_events_in_wait_list,
									  const cl_event *event_wait_list,
									 cl_event *event)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clEnqueueSVMMemcpy);
        apiLogger << "cl_command_queue command_queue" << command_queue << "cl_bool blocking_copy" << blocking_copy << "void *dst_ptr" << dst_ptr << "const void *src_ptr" << src_ptr << "size_t size" << size << "cl_uint num_events_in_wait_list" << num_events_in_wait_list << "const cl_event *event_wait_list" << event_wait_list << "cl_event *event" << event;
        OutputParamsValueProvider provider(apiLogger);
        provider.AddParam("event", event, true);
	      CALL_INSTRUMENTED_API_LOGGER(EXECUTION_MODULE, cl_int, EnqueueSVMMemcpy(command_queue, blocking_copy, dst_ptr, src_ptr, size, num_events_in_wait_list, event_wait_list, event, &apiLogger));
    }
    else
    {
        CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_int, EnqueueSVMMemcpy(command_queue, blocking_copy, dst_ptr, src_ptr, size, num_events_in_wait_list, event_wait_list, event, nullptr));
    }
}
SET_ALIAS(clEnqueueSVMMemcpy);

cl_int CL_API_CALL clEnqueueSVMMemFill(cl_command_queue command_queue,
									   void* svm_ptr,
									   const void* pattern,
									   size_t pattern_size,
									   size_t size,
									   cl_uint num_events_in_wait_list,
									   const cl_event* event_wait_list,
									   cl_event* event)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clEnqueueSVMMemFill);
        apiLogger << "cl_command_queue command_queue" << command_queue << "void* svm_ptr" << svm_ptr << "const void* pattern" << pattern << "size_t pattern_size" << pattern_size << "size_t size" << size << "cl_uint num_events_in_wait_list" << num_events_in_wait_list << "const cl_event* event_wait_list" << event_wait_list << "cl_event* event" << event;
        OutputParamsValueProvider provider(apiLogger);
        provider.AddParam("event", event, true);
	      CALL_INSTRUMENTED_API_LOGGER(EXECUTION_MODULE, cl_int, EnqueueSVMMemFill(command_queue, svm_ptr, pattern, pattern_size, size, num_events_in_wait_list, event_wait_list, event, &apiLogger));
    }
    else
    {
        CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_int, EnqueueSVMMemFill(command_queue, svm_ptr, pattern, pattern_size, size, num_events_in_wait_list, event_wait_list, event, nullptr));
    }
}
SET_ALIAS(clEnqueueSVMMemFill);

cl_int CL_API_CALL clEnqueueSVMMap(cl_command_queue command_queue,
								   cl_bool blocking_map,
								   cl_map_flags map_flags,
								   void* svm_ptr,
								   size_t size,
								   cl_uint num_events_in_wait_list,
								   const cl_event* event_wait_list,
								   cl_event* event)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clEnqueueSVMMap);
        apiLogger << "cl_command_queue command_queue" << command_queue << "cl_bool blocking_map" << blocking_map << "cl_map_flags map_flags" << map_flags << "void* svm_ptr" << svm_ptr << "size_t size" << size << "cl_uint num_events_in_wait_list" << num_events_in_wait_list << "const cl_event* event_wait_list" << event_wait_list << "cl_event* event" << event;
        OutputParamsValueProvider provider(apiLogger);
        provider.AddParam("event", event, true);
	      CALL_INSTRUMENTED_API_LOGGER(EXECUTION_MODULE, cl_int, EnqueueSVMMap(command_queue, blocking_map, map_flags, svm_ptr, size, num_events_in_wait_list, event_wait_list, event, &apiLogger));
    }
    else
    {
        CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_int, EnqueueSVMMap(command_queue, blocking_map, map_flags, svm_ptr, size, num_events_in_wait_list, event_wait_list, event, nullptr));
    }
}
SET_ALIAS(clEnqueueSVMMap);

cl_int CL_API_CALL clEnqueueSVMUnmap(cl_command_queue command_queue,
									 void* svm_ptr,
									 cl_uint num_events_in_wait_list,
									 const cl_event* event_wait_list,
									 cl_event* event)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clEnqueueSVMUnmap);
        apiLogger << "cl_command_queue command_queue" << command_queue << "void* svm_ptr" << svm_ptr << "cl_uint num_events_in_wait_list" << num_events_in_wait_list << "const cl_event* event_wait_list" << event_wait_list << "cl_event* event" << event;
        OutputParamsValueProvider provider(apiLogger);
        provider.AddParam("event", event, true);
	      CALL_INSTRUMENTED_API_LOGGER(EXECUTION_MODULE, cl_int, EnqueueSVMUnmap(command_queue, svm_ptr, num_events_in_wait_list, event_wait_list, event, &apiLogger));
    }
    else
    {
        CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_int, EnqueueSVMUnmap(command_queue, svm_ptr, num_events_in_wait_list, event_wait_list, event, nullptr));
    }
}
SET_ALIAS(clEnqueueSVMUnmap);

cl_int CL_API_CALL clSetKernelArgSVMPointer(cl_kernel kernel,
											cl_uint arg_index,
											const void* arg_value)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clSetKernelArgSVMPointer);
        apiLogger << "cl_kernel kernel" << kernel << "cl_uint arg_index" << arg_index << "const void* arg_value" << arg_value;
	      CALL_INSTRUMENTED_API_LOGGER(CONTEXT_MODULE, cl_int, SetKernelArgSVMPointer(kernel, arg_index, arg_value));
    }
    else
    {
        CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_int, SetKernelArgSVMPointer(kernel, arg_index, arg_value));
    }
}
SET_ALIAS(clSetKernelArgSVMPointer);

cl_int CL_API_CALL clSetKernelExecInfo(cl_kernel kernel,
									   cl_kernel_exec_info param_name,
									   size_t param_value_size,
									   const void* param_value)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clSetKernelExecInfo);
        apiLogger << "cl_kernel kernel" << kernel << "cl_kernel_exec_info param_name" << param_name << "size_t param_value_size" << param_value_size << "const void* param_value" << param_value;
	      CALL_INSTRUMENTED_API_LOGGER(CONTEXT_MODULE, cl_int, SetKernelExecInfo(kernel, param_name, param_value_size, param_value));
    }
    else
    {
        CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_int, SetKernelExecInfo(kernel, param_name, param_value_size, param_value));
    }
}
SET_ALIAS(clSetKernelExecInfo);

cl_mem CL_API_CALL clCreatePipe(cl_context context,
								cl_mem_flags flags,
								cl_uint pipe_packet_size,
								cl_uint pipe_max_packets,
								const cl_pipe_properties *properties,
								cl_int *errcode_ret)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clCreatePipe);
        apiLogger << "cl_context context" << context << "cl_mem_flags flags" << flags << "cl_uint pipe_packet_size" << pipe_packet_size << "cl_uint pipe_max_packets" << pipe_max_packets << "const cl_pipe_properties *properties" << properties << "cl_int *errcode_ret" << errcode_ret;
        OutputParamsValueProvider provider(apiLogger);
        provider.AddParam("errcode_ret", errcode_ret, false, false);
	      CALL_INSTRUMENTED_API_LOGGER(CONTEXT_MODULE, cl_mem, CreatePipe(context, flags, pipe_packet_size, pipe_max_packets, properties, nullptr, nullptr, errcode_ret));
    }
    else
    {
        CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_mem, CreatePipe(context, flags, pipe_packet_size, pipe_max_packets, properties, nullptr, nullptr, errcode_ret));
    }
}
SET_ALIAS(clCreatePipe);

cl_mem CL_API_CALL clCreatePipeINTEL(
        cl_context                  context,
        cl_mem_flags                flags,
        cl_uint                     pipe_packet_size,
        cl_uint                     pipe_max_packets,
        const cl_pipe_properties*   properties,
        void *                      host_ptr,
        size_t *                    size_ret,
        cl_int *                    errcode_ret )
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clCreatePipeINTEL);
        apiLogger << "cl_context context" << context << "cl_mem_flags flags" << flags << "cl_uint pipe_packet_size" << pipe_packet_size << "cl_uint pipe_max_packets" << pipe_max_packets << "const cl_pipe_properties* properties" << properties << "void * host_ptr" << host_ptr << "size_t * size_ret" << size_ret << "cl_int * errcode_ret" << errcode_ret;
        OutputParamsValueProvider provider(apiLogger);
        provider.AddParam("size_ret", size_ret, false, true);
        provider.AddParam("errcode_ret", errcode_ret, false, false);
        CALL_INSTRUMENTED_API_LOGGER(CONTEXT_MODULE, cl_mem, CreatePipe(context, flags, pipe_packet_size, pipe_max_packets, properties, host_ptr, size_ret, errcode_ret));
    }
    else
    {
        CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_mem, CreatePipe(context, flags, pipe_packet_size, pipe_max_packets, properties, host_ptr, size_ret, errcode_ret));
    }
}
SET_ALIAS(clCreatePipeINTEL);
REGISTER_EXTENSION_FUNCTION(clCreatePipeINTEL, clCreatePipeINTEL);

cl_int CL_API_CALL clGetPipeInfo(cl_mem pipe,
								 cl_pipe_info param_name,
								 size_t param_value_size,
								 void *param_value,
								 size_t *param_value_size_ret)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clGetPipeInfo);
        apiLogger << "cl_mem pipe" << pipe << "cl_pipe_info param_name" << param_name << "size_t param_value_size" << param_value_size << "void *param_value" << param_value << "size_t *param_value_size_ret" << param_value_size_ret;
        OutputParamsValueProvider provider(apiLogger);
        provider.AddParam("param_value_size_ret", param_value_size_ret, false, true);
	      CALL_INSTRUMENTED_API_LOGGER(CONTEXT_MODULE, cl_int, GetPipeInfo(pipe, param_name, param_value_size, param_value, param_value_size_ret));
    }
    else
    {
        CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_int, GetPipeInfo(pipe, param_name, param_value_size, param_value, param_value_size_ret));
    }
}
SET_ALIAS(clGetPipeInfo);
// OpenCL 2.0 functions:

static vector<cl_command_queue_properties> GetCommandQueueProps(cl_context context, cl_device_id device_id, const cl_queue_properties* properties)
{
	  vector<cl_command_queue_properties> propVec;
	  if (nullptr != properties)
	  {
		    const cl_queue_properties* pCurrProp = properties;
		    while (*pCurrProp != 0)
		    {
			      propVec.push_back((cl_command_queue_properties)*pCurrProp);
			      ++pCurrProp;
			      propVec.push_back((cl_command_queue_properties)*pCurrProp);
			      ++pCurrProp;
		    }
		    propVec.push_back(0);
	  }
    return propVec;
}

cl_command_queue CL_API_CALL clCreateCommandQueueWithProperties(cl_context context,
																cl_device_id device_id,
																const cl_queue_properties* properties,
																cl_int* errcode_ret)
{
    std::vector<cl_command_queue_properties> propVec = GetCommandQueueProps(context, device_id, properties);
    const cl_command_queue_properties* pCmdQueueProps = propVec.empty() ? nullptr : &propVec[0];
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clCreateCommandQueueWithProperties);
        apiLogger << "cl_context context" << context << "cl_device_id device_id" << device_id << "const cl_queue_properties* properties" << properties << "cl_int* errcode_ret" << errcode_ret;
        OutputParamsValueProvider provider(apiLogger);
        provider.AddParam("errcode_ret", errcode_ret, false, false);
	      CALL_INSTRUMENTED_API_LOGGER(EXECUTION_MODULE, cl_command_queue, CreateCommandQueue(context, device_id, pCmdQueueProps, errcode_ret));
    }
    else
    {
        CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_command_queue, CreateCommandQueue(context, device_id, pCmdQueueProps, errcode_ret));
    }
}
SET_ALIAS(clCreateCommandQueueWithProperties);

cl_program CL_API_CALL clCreateProgramWithIL(cl_context context,
                                             const void* il,
                                             size_t lengths,
                                             cl_int* errcode_ret)
{
    if (FrameworkProxy::Instance()->GetOCLConfig()->GetOpenCLVersion() < OPENCL_VERSION_2_1)
    {
        if (errcode_ret != nullptr)
        {
            *errcode_ret = CL_INVALID_OPERATION;
        }
        return CL_INVALID_HANDLE;
    }

    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clCreateProgramWithIL);
        apiLogger << "cl_context context" << context << "const void* il" << il << "size_t lengths" << lengths << "cl_int * errcode_ret" << errcode_ret;
        OutputParamsValueProvider provider(apiLogger);
        provider.AddParam("errcode_ret", errcode_ret, false, false);
        CALL_INSTRUMENTED_API_LOGGER(CONTEXT_MODULE, cl_program, CreateProgramWithIL(context, (const unsigned char*)il, lengths, errcode_ret));
    }
    else
    {
        CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_program, CreateProgramWithIL(context, (const unsigned char*)il, lengths, errcode_ret));
    }
}
SET_ALIAS(clCreateProgramWithIL);

cl_program CL_API_CALL clCreateProgramWithILKHR(cl_context context,
                                                const void *il, size_t lengths,
                                                cl_int *errcode_ret) {
  if (g_pUserLogger->IsApiLoggingEnabled()) {
    START_LOG_API(clCreateProgramWithILKHR);
    apiLogger << "cl_context context" << context << "const void* il" << il
              << "size_t lengths" << lengths << "cl_int * errcode_ret"
              << errcode_ret;
    OutputParamsValueProvider provider(apiLogger);
    provider.AddParam("errcode_ret", errcode_ret, false, false);
    CALL_INSTRUMENTED_API_LOGGER(CONTEXT_MODULE, cl_program,
                                 CreateProgramWithIL(context,
                                                     (const unsigned char *)il,
                                                     lengths, errcode_ret));
  } else {
    CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_program,
                          CreateProgramWithIL(context,
                                              (const unsigned char *)il,
                                              lengths, errcode_ret));
  }
}

SET_ALIAS(clCreateProgramWithILKHR);
REGISTER_EXTENSION_FUNCTION(clCreateProgramWithILKHR, clCreateProgramWithILKHR);

cl_kernel CL_API_CALL clCloneKernel(cl_kernel source_kernel,
                                    cl_int* errcode_ret)
{
    if (FrameworkProxy::Instance()->GetOCLConfig()->GetOpenCLVersion() < OPENCL_VERSION_2_1)
    {
        if (errcode_ret != nullptr)
        {
            *errcode_ret = CL_INVALID_OPERATION;
        }
        return CL_INVALID_HANDLE;
    }
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clCloneKernel);
        apiLogger << "const cl_kernel source_kernel" << source_kernel << "cl_int* errcode_ret" << errcode_ret;
        OutputParamsValueProvider provider(apiLogger);
        provider.AddParam("errcode_ret", errcode_ret, false, false);
        CALL_INSTRUMENTED_API_LOGGER(CONTEXT_MODULE, cl_kernel, CloneKernel(source_kernel, errcode_ret));
    }
    else
    {
        CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_kernel, CloneKernel(source_kernel, errcode_ret));
    }
}
SET_ALIAS(clCloneKernel);

cl_int CL_API_CALL clSetDefaultDeviceCommandQueue(cl_context context,
                                                  cl_device_id device,
                                                  cl_command_queue command_queue)
{
    if (FrameworkProxy::Instance()->GetOCLConfig()->GetOpenCLVersion() < OPENCL_VERSION_2_1)
    {
        return CL_INVALID_OPERATION;
    }

    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clSetDefaultCommandQueue);
        apiLogger << "cl_context context" << context << "cl_device_id device" << device << "cl_command_queue" << command_queue;
        OutputParamsValueProvider provider(apiLogger);
        CALL_INSTRUMENTED_API_LOGGER(EXECUTION_MODULE, cl_int, SetDefaultDeviceCommandQueue(context, device, command_queue));
    }
    else
    {
        CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_int, SetDefaultDeviceCommandQueue(context, device, command_queue));
    }
}
SET_ALIAS(clSetDefaultDeviceCommandQueue);

cl_int CL_API_CALL clGetKernelSubGroupInfo(cl_kernel kernel,
                                           cl_device_id device,
                                           cl_kernel_sub_group_info param_name,
                                           size_t input_value_size,
                                           const void* input_value,
                                           size_t param_value_size,
                                           void* param_value,
                                           size_t* param_value_size_ret)
{
    if (FrameworkProxy::Instance()->GetOCLConfig()->GetOpenCLVersion() < OPENCL_VERSION_2_1)
    {
        return CL_INVALID_OPERATION;
    }

    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        ApiLogger apiLogger("clGetKernelSubGroupInfo");
        apiLogger << "cl_kernel kernel" << kernel
                  << "cl_device_id device" << device
                  << "cl_kernel_sub_group_info param_name" << param_name
                  << "size_t input_value_size" << input_value_size
                  << "const void* input_value" << input_value
                  << "size_t param_value_size" << param_value_size
                  << "void* param_value" << param_value
                  << "size_t* param_value_size_ret" << param_value_size_ret;
        OutputParamsValueProvider provider(apiLogger);
        provider.AddParam("param_value_size_ret", param_value_size_ret, false, true);
        CALL_INSTRUMENTED_API_LOGGER(CONTEXT_MODULE, cl_int, GetKernelSubGroupInfo(kernel, device, param_name,
                                                                                   input_value_size, input_value,
                                                                                   param_value_size, param_value,
                                                                                   param_value_size_ret));
    }
    else
    {
        CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_int, GetKernelSubGroupInfo(kernel, device, param_name,
                                                                            input_value_size, input_value,
                                                                            param_value_size, param_value,
                                                                            param_value_size_ret));
    }
}
SET_ALIAS(clGetKernelSubGroupInfo);

cl_int CL_API_CALL clGetKernelSubGroupInfoKHR(cl_kernel kernel,
                                             cl_device_id device,
                                             cl_kernel_sub_group_info param_name,
                                             size_t input_value_size,
                                             const void* input_value,
                                             size_t param_value_size,
                                             void* param_value,
                                             size_t* param_value_size_ret)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
      ApiLogger apiLogger("clGetKernelSubGroupInfoKHR");
      apiLogger << "cl_kernel kernel" << kernel
                << "cl_device_id device" << device
                << "cl_kernel_sub_group_info param_name" << param_name
                << "size_t input_value_size" << input_value_size
                << "const void* input_value" << input_value
                << "size_t param_value_size" << param_value_size
                << "void* param_value" << param_value
                << "size_t* param_value_size_ret" << param_value_size_ret;
      OutputParamsValueProvider provider(apiLogger);
      provider.AddParam("param_value_size_ret", param_value_size_ret, false, true);
      // Intentionally call non-KHR version as we want to re-use it's implementation.
      CALL_INSTRUMENTED_API_LOGGER(CONTEXT_MODULE, cl_int, GetKernelSubGroupInfo(kernel, device, param_name,
                                                                                 input_value_size, input_value,
                                                                                 param_value_size, param_value,
                                                                                 param_value_size_ret));
    }
    else
    {
        // Intentionally call non-KHR version as we want to re-use it's implementation.
        CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_int, GetKernelSubGroupInfo(kernel, device, param_name,
                                                                            input_value_size, input_value,
                                                                            param_value_size, param_value,
                                                                            param_value_size_ret));
    }
}
SET_ALIAS(clGetKernelSubGroupInfoKHR);
REGISTER_EXTENSION_FUNCTION(clGetKernelSubGroupInfoKHR, clGetKernelSubGroupInfoKHR);

void* CL_API_CALL
clMapHostPipeIntelFPGA(cl_mem pipe, cl_map_flags flags,
                       size_t requestedSize, size_t* pMappedSize,
                       cl_int* pError)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clMapHostPipeIntelFPGA);
        apiLogger << "cl_mem pipe " << pipe
                  << ", cl_map_flags flags " << flags
                  << ", size_t requestedSize " << requestedSize
                  << ", size_t* pMappedSize " << pMappedSize;
        CALL_INSTRUMENTED_API_LOGGER(CONTEXT_MODULE, void*,
            MapHostPipeIntelFPGA(pipe, flags, requestedSize,
                                 pMappedSize, pError));
    }
    else
    {
        CALL_INSTRUMENTED_API(CONTEXT_MODULE, void*,
            MapHostPipeIntelFPGA(pipe, flags, requestedSize,
                                 pMappedSize, pError));
    }
}
SET_ALIAS(clMapHostPipeIntelFPGA);
REGISTER_EXTENSION_FUNCTION(clMapHostPipeIntelFPGA, clMapHostPipeIntelFPGA);

cl_int CL_API_CALL
clUnmapHostPipeIntelFPGA(cl_mem pipe, void* pMappedPtr,
                         size_t sizeToUnmap,
                         size_t* pUnmappedSize)

{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clUnmapHostPipeIntelFPGA);
        apiLogger << "cl_mem pipe " << pipe
                  << ", void* pMappedPtr " << pMappedPtr
                  << ", size_t sizeToUnmap"
                  << ", size_t* pUnmappedSize" << pUnmappedSize;
        CALL_INSTRUMENTED_API_LOGGER(CONTEXT_MODULE, cl_int,
            UnmapHostPipeIntelFPGA(pipe, pMappedPtr, sizeToUnmap, pUnmappedSize));
    }
    else
    {
        CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_int,
            UnmapHostPipeIntelFPGA(pipe, pMappedPtr, sizeToUnmap, pUnmappedSize));
    }
}
SET_ALIAS(clUnmapHostPipeIntelFPGA);
REGISTER_EXTENSION_FUNCTION(clUnmapHostPipeIntelFPGA, clUnmapHostPipeIntelFPGA);

cl_int CL_API_CALL clReadPipeIntelFPGA(cl_mem pipe, void* ptr)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clReadPipeIntelFPGA);
        apiLogger << "cl_mem pipe " << pipe << ", void* ptr " << ptr;
        CALL_INSTRUMENTED_API_LOGGER(CONTEXT_MODULE, cl_int, ReadPipeIntelFPGA(pipe, ptr));
    }
    else
    {
        CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_int, ReadPipeIntelFPGA(pipe, ptr));
    }
}
SET_ALIAS(clReadPipeIntelFPGA);
REGISTER_EXTENSION_FUNCTION(clReadPipeIntelFPGA, clReadPipeIntelFPGA);

cl_int CL_API_CALL clWritePipeIntelFPGA(cl_mem pipe, const void* ptr)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clWritePipeIntelFPGA);
        apiLogger << "cl_mem pipe " << pipe << ", const void* ptr " << ptr;
        CALL_INSTRUMENTED_API_LOGGER(CONTEXT_MODULE, cl_int, WritePipeIntelFPGA(pipe, ptr));
    }
    else
    {
        CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_int, WritePipeIntelFPGA(pipe, ptr));
    }
}
SET_ALIAS(clWritePipeIntelFPGA);
REGISTER_EXTENSION_FUNCTION(clWritePipeIntelFPGA, clWritePipeIntelFPGA);

cl_int CL_API_CALL clGetProfileDataDeviceIntelFPGA(
    cl_device_id device_id, cl_program program, cl_bool read_enqueue_kernels,
    cl_bool read_auto_enqueued, cl_bool clear_counters_after_readback,
    size_t param_value_size, void* param_value, size_t* param_value_size_ret,
    cl_int* errcode_ret)
{
    if (g_pUserLogger->IsApiLoggingEnabled())
    {
        START_LOG_API(clGetProfileDataDeviceIntelFPGA);
        apiLogger << "cl_device_id device_id " << device_id
                  << ", cl_program program " << program
                  << ", cl_bool read_enqueue_kernels " << read_enqueue_kernels
                  << ", cl_bool read_auto_enqueued " << read_auto_enqueued
                  << ", cl_bool clear_counters_after_readback "
                  << clear_counters_after_readback
                  << ", size_t param_value_size " << param_value_size
                  << ", void* param_value " << param_value
                  << ", size_t* param_value_size_ret " << param_value_size_ret
                  << ", cl_int* errcode_ret " << errcode_ret;
        CALL_INSTRUMENTED_API_LOGGER(CONTEXT_MODULE, cl_int,
                                     GetProfileDataDeviceIntelFPGA(
              device_id, program, read_enqueue_kernels, read_auto_enqueued,
              clear_counters_after_readback, param_value_size, param_value,
              param_value_size_ret, errcode_ret));
    }
    else
    {
        CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_int,
                              GetProfileDataDeviceIntelFPGA(
              device_id, program, read_enqueue_kernels, read_auto_enqueued,
              clear_counters_after_readback, param_value_size, param_value,
              param_value_size_ret, errcode_ret));
    }
}
SET_ALIAS(clGetProfileDataDeviceIntelFPGA);
REGISTER_EXTENSION_FUNCTION(clGetProfileDataDeviceIntelFPGA,
                            clGetProfileDataDeviceIntelFPGA);

cl_int CL_API_CALL clGetDeviceFunctionPointerINTEL(cl_device_id device,
    cl_program program, const char* func_name, cl_ulong* func_pointer_ret)
{
  if (g_pUserLogger->IsApiLoggingEnabled())
    {
      ApiLogger apiLogger("clGetDeviceFunctionPointerINTEL");
      apiLogger << "cl_device_id device" << device
                << "cl_program program" << program
                << "const char* func_name" << func_name
                << "cl_ulong* func_pointer_ret" << func_pointer_ret;
      OutputParamsValueProvider provider(apiLogger);
      provider.AddParam("func_pointer_ret", func_pointer_ret, false, true);
      CALL_INSTRUMENTED_API_LOGGER(CONTEXT_MODULE, cl_int,
          GetDeviceFunctionPointer(device, program, func_name,
              func_pointer_ret));
    }
    else
    {
        CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_int,
            GetDeviceFunctionPointer(device, program, func_name,
                func_pointer_ret));
    }
}
SET_ALIAS(clGetDeviceFunctionPointerINTEL);

#if defined( __GNUC__ ) && __GNUC__ > 7
# pragma GCC diagnostic pop
#endif

REGISTER_EXTENSION_FUNCTION(clGetDeviceFunctionPointerINTEL,
    clGetDeviceFunctionPointerINTEL);
