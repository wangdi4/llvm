/* ************************************************************************* *\
                  INTEL CORPORATION PROPRIETARY INFORMATION
      This software is supplied under the terms of a license agreement or 
      nondisclosure agreement with Intel Corporation and may not be copied 
      or disclosed except in accordance with the terms of that agreement. 
          Copyright (C) 2010-2011 Intel Corporation. All Rights Reserved.
\* ************************************************************************* */

#ifndef _COIPROCESS_SOURCE_H
#define _COIPROCESS_SOURCE_H
/** @ingroup COIProcess
 *  @addtogroup COIProcessSource
@{
* @file source/COIProcess_source.h
*/
#ifndef DOXYGEN_SHOULD_SKIP_THIS

#include <common/COITypes_common.h>
#include <common/COIResult_common.h>

#ifdef __cplusplus
extern "C" {
#endif 
#endif // DOXYGEN_SHOULD_SKIP_THIS


///////////////////////////////////////////////////////////////////////////////
/// This is a special COIPROCESS handle that can be used to indicate that
/// the source process should be used for an operation.
///
#define COI_PROCESS_SOURCE  ((COIPROCESS)-1)

typedef enum COI_SHUTDOWN_REASON
{
    COI_SHUTDOWN_OK = 0,
    COI_SHUTDOWN_SIGTERM,
    COI_SHUTDOWN_SEGFAULT
} COI_SHUTDOWN_REASON;

#define COI_MAX_FILE_NAME_LENGTH 256

///////////////////////////////////////////////////////////////////////////////
///
/// Create a remote process on the Sink and start executing it's main()
/// function. This will also automatically load any dependent shared objects
/// on to the device. Once the process is created, remote calls can be 
/// initiated by using the RunFunction mechanism found in the COIPipeline APIs.
/// For more information on how dependencies are loaded, see the
/// COIProcessLoadLibrary APIs.
///
/// @param  in_Engine 
///         [in] A handle retrieved via a call to COIEngineGetHandle() that
///         indicates which device to create the process on.  This is
///         necessary because there can be more than one device
///         within the system.
///
/// @param  in_pBinaryName
///         [in] Pointer to a null-terminated string that contains the
///         path to the program binary to be instantiated as a process on
///         the sink device.  The file name will be accessed via 
///         fopen and fread, as such, the passed in binary name must
///         be locatable via these commands. Also, the file name (without
///         directory information) will be used automatically by the system
///         to create the argv[0] of the new process.
///
/// @param  in_Argc
///         [in] The number of arguments being passed in to the process in the
///         in_ppArgv parameter.
///
/// @param  in_ppArgv
///         [in] An array of strings that represent the arguments being passed
///         in. The system will auto-generate argv[0] using in_pBinaryName and
///         thus that parameter cannot be passed in using in_ppArgv. Instead,
///         in_ppArgv contains the rest of the parameters being passed in.
///
/// @param  in_DupEnv
///         [in] A boolean that indicates whether the process that is being
///         created should inherit the environment of the caller.
///
/// @param  in_ppAdditionalEnv
///         [in] An array of strings that represent additional environment
///         variables. This parameter must terminate the array with a NULL
///         string. For convenience it is also allowed to be NULL if there are
///         no additional environment variables that need adding. Note that
///         any environment variables specified here will be in addition to
///         but override those that were inherited via in_DupEnv.
///
/// @param  in_ProxyActive
///         [in] A boolean that specifies whether the process that is to be
///         created wants I/O proxy support.
///
/// @param  in_ProxyRoot
///         [in] If proxy support was requested this string indicates the
///         root directory that will be prepended to any proxy file I/O.
///         If proxy support was requested passing NULL will set the proxy
///         root to the default value of "/".
///
/// @param  in_BufferSpace
///         [in] The most memory (in bytes) that will ever be allocated for
///         buffers that will be used by pipelines associated with this process.
///         If the buffer space specified is 0, then all buffers will be
///         allocated in process space upon allocation, without limit.
///         If it is specified then:
///         * Buffer memory is allocated by the sink process as part of the
///         process creation.
///         * Buffer creations that specify a buffer size larger than the buffer
///         space of one of its processes will fail.
///         * Run functions that specify a COIBuffer collection larger than this
///         limit will fail.
///         * Run functions whose buffer space, when combined with AddRef’d
///         buffers and buffers from other pipelines,  exceed the buffer space
///         limit will stall until enough buffer space is released.
///
/// @param  out_pProcess
///         [out] Handle returned to uniquely identify the process that was
///         created for use in later API calls.
///
/// @return COI_SUCCESS if the remote process was successfully created.
///
/// @return COI_INVALID_HANDLE if the in_Engine handle passed in was invalid.
///
/// @return COI_INVALID_POINTER if out_pProcess was NULL.
///
/// @return COI_INVALID_POINTER if in_pBinaryName was NULL.
///
/// @return COI_DOES_NOT_EXIST if in_pBinaryName cannot be found.
///
/// @return COI_BINARY_AND_HARDWARE_MISMATCH if in_pBinaryName is an invalid
///         executable on the engine specified.
///
/// @return COI_RESOURCE_EXHAUSTED if no more COIProcesses can be created.
///
/// @return COI_ARGUMENT_MISMATCH if in_Argc is 0 and in_ppArgv is not NULL.
///
/// @return COI_ARGUMENT_MISMATCH if in_Argc is greater than 0 and in_ppArgv is
///         NULL.
///
/// @return COI_OUT_OF_RANGE if in_Argc is less than 0.
///
/// @return COI_OUT_OF_RANGE if the length of in_pBinaryName is greater than or 
///         equal to COI_MAX_FILE_NAME_LENGTH.
///
/// @return COI_ARGUMENT_MISMATCH if in_ProxyActive is false and in_ProxyRoot 
///         is not NULL.
///
/// @return COI_DOES_NOT_EXIST if in_ProxyRoot is not NULL and does not exist.
///
/// @return COI_TIME_OUT_REACHED if establishing the communication channel with
///         the remote process timed out.
///
COIRESULT
COIProcessCreateFromFile(
            COIENGINE           in_Engine,
    const   char*               in_pBinaryName,
            int                 in_Argc,
    const   char**              in_ppArgv,
            uint8_t             in_DupEnv,
    const   char**              in_ppAdditionalEnv,
            uint8_t             in_ProxyActive,
    const   char*               in_ProxyRoot,
            uint64_t            in_BufferSpace,
            COIPROCESS*         out_pProcess);

///////////////////////////////////////////////////////////////////////////////
///
/// Create a remote process on the Sink and start executing it's main()
/// function. This will also automatically load any dependent shared objects
/// on to the device. Once the process is created, remote calls can be
/// initiated by using the RunFunction mechanism found in the COIPipeline APIs.
/// For more information on how dependencies are loaded, see the
/// COIProcessLoadLibrary APIs.
///
/// @param  in_Engine
///         [in] A handle retrieved via a call to COIEngineGetHandle() that
///         indicates which device to create the process on.  This is
///         necessary because there can be more than one device
///         within the system.
///
/// @param  in_pBinaryName
///         [in] Pointer to a null-terminated string that contains the name to
///         give the process that will be created. Note that the final name
///         will strip out any directory information from in_pBinaryName and
///         use the file information to generate an argv[0] for the new
///         process.
///
/// @param  in_pBinaryBuffer
///         [in] Pointer to a buffer whose contents represent the sink-side
///         process that we want to create.
///
/// @param  in_BinaryBufferLength
///         [in] Number of bytes in in_pBinaryBuffer.
///
/// @param  in_Argc
///         [in] The number of arguments being passed in to the process in the
///         in_ppArgv parameter.
///
/// @param  in_ppArgv
///         [in] An array of strings that represent the arguments being passed
///         in. The system will auto-generate argv[0] using in_pBinaryName and
///         thus that parameter cannot be passed in using in_ppArgv. Instead,
///         in_ppArgv contains the rest of the parameters being passed in.
///
/// @param  in_DupEnv
///         [in] A boolean that indicates whether the process that is being
///         created should inherit the environment of the caller.
///
/// @param  in_ppAdditionalEnv
///         [in] An array of strings that represent additional environment
///         variables. This parameter must terminate the array with a NULL
///         string. For convenience it is also allowed to be NULL if there are
///         no additional environment variables that need adding. Note that
///         any environment variables specified here will be in addition to
///         but override those that were inherited via in_DupEnv.
///
/// @param  in_ProxyActive
///         [in] A boolean that specifies whether the process that is to be
///         created wants I/O proxy support.
///
/// @param  in_ProxyRoot
///         [in] If proxy support was requested this string indicates the
///         root directory that will be prepended to any proxy file I/O.
///         If proxy support was requested passing NULL will set the proxy
///         root to the default value of "/".
///
/// @param  in_BufferSpace
///         [in] The most memory (in bytes) that will ever be allocated for
///         buffers that will be used by pipelines associated with this process.
///         If the buffer space specified is 0, then all buffers will be
///         allocated in process space upon allocation, without limit.
///         If it is specified then:
///         * Buffer memory is allocated by the sink process as part of the
///         process creation.
///         * Buffer creations that specify a buffer size larger than the buffer
///         space of one of its processes will fail.
///         * Run functions that specify a COIBuffer collection larger than this
///         limit will fail.
///         * Run functions whose buffer space, when combined with AddRef’d
///         buffers and buffers from other pipelines,  exceed the buffer space
///         limit will stall until enough buffer space is released.
///
/// @param  out_pProcess
///         [out] Handle returned to uniquely identify the process that was
///         created for use in later API calls.
///
/// @return COI_SUCCESS if the remote process was successfully created.
///
/// @return COI_INVALID_HANDLE if the in_Engine handle passed in was invalid.
///
/// @return COI_INVALID_POINTER if out_pProcess was NULL.
///
/// @return COI_INVALID_POINTER if in_pBinaryName or in_pBinaryBuffer was NULL.
///
/// @return COI_BINARY_AND_HARDWARE_MISMATCH if in_pBinaryName is an invalid
///         executable on the engine specified.
///
/// @return COI_RESOURCE_EXHAUSTED if no more COIProcesses can be created.
///
/// @return COI_ARGUMENT_MISMATCH if in_Argc is 0 and in_ppArgv is not NULL.
///
/// @return COI_ARGUMENT_MISMATCH if in_Argc is greater than 0 and in_ppArgv is
///         NULL.
///
/// @return COI_OUT_OF_RANGE if in_Argc is less than 0.
///
/// @return COI_OUT_OF_RANGE if the length of in_pBinaryName is greater than or
///         equal to COI_MAX_FILE_NAME_LENGTH.
///
/// @return COI_OUT_OF_RANGE if in_BinaryBufferLength is 0.
///
/// @return COI_ARGUMENT_MISMATCH if in_ProxyActive is false and in_ProxyRoot
///         is not NULL.
///
/// @return COI_DOES_NOT_EXIST if in_ProxyRoot is not NULL and does not exist.
///
/// @return COI_TIME_OUT_REACHED if establishing the communication channel with
///         the remote process timed out.
///
COIRESULT
COIProcessCreateFromMemory(
            COIENGINE           in_Engine,
    const   char*               in_pBinaryName,
    const   void*               in_pBinaryBuffer,
            uint64_t            in_BinaryBufferLength,
            int                 in_Argc,
    const   char**              in_ppArgv,
            uint8_t             in_DupEnv,
    const   char**              in_ppAdditionalEnv,
            uint8_t             in_ProxyActive,
    const   char*               in_ProxyRoot,
            uint64_t            in_BufferSpace,
            COIPROCESS*         out_pProcess);

//////////////////////////////////////////////////////////////////////////////
///
/// Destroys the indicated process, releasing its resources.
///
/// @param  in_Process
///         [in] Process to destroy.
///
/// @param  in_WaitForMainTimeout
///         [in] The number of milliseconds to wait for the main() function
///         to return in the sink process before timing out.  If 0 is passed
///         in this function polls and immediately returns.  If -1 is passed 
///         in this function waits indefinitely for main() to return before 
///         freeing the process resources.
///
/// @param  in_ForceDestroy
///         [in] If this flag is set to true then the sink process will be
///         forcibly terminated after the timeout has been reached. A timeout
///         value of 0 will kill the process immediately, while a timeout of
///         -1 is invalid.  If the flag is set to false then a message will 
///         be sent to the sink process requesting a clean shutdown. In
///         most cases this flag should be set to false. If a sink process
///         is not responding then it may be necessary to set this flag to
///         true.
///
/// @param  out_pProcessReturn
///         [out] The value returned from the main() function executing in
///         the sink process. This is an optional parameter. If the caller
///         is not interested in the return value from the remote process
///         they may pass in NULL for this parameter.
///
/// @param  out_pReason
///         [out] This parameter specifies the shutdown reason. This may
///         be COI_SHUTDOWN_OK if the remote process exited cleanly or
///         some other value if the process exited abnormally. This is an
///         optional parameter and the caller may pass in NULL if they
///         are not interested in the shutdown reason.
///
/// @return COI_SUCCESS if the process was destroyed.
///
/// @return COI_INVALID_HANDLE if the process handle passed in was invalid.
///
/// @return COI_OUT_OF_RANGE for any negative in_WaitForMainTimeout value 
///         except -1.
///
/// @return COI_ARGUMENT_MISMATCH if in_WaitForMainTimeout is -1 and
///         in_ForceDestroy is true.
///
/// @return COI_TIME_OUT_REACHED if the sink process is still running after
///         waiting in_WaitForMainTimeout milliseconds and in_ForceDestroy
///         is false.  This is true even if in_WaitForMainTimeout was 0.
///         In this case, out_pProcessReturn and out_pReason are undefined.
///
COIRESULT
COIProcessDestroy(
            COIPROCESS              in_Process,
            int32_t                 in_WaitForMainTimeout,
            uint8_t                 in_ForceDestroy,
            int8_t*                 out_pProcessReturn,
            COI_SHUTDOWN_REASON*    out_pReason);


#define COI_MAX_FUNCTION_NAME_LENGTH 256

//////////////////////////////////////////////////////////////////////////////
///
/// Given a loaded native process, gets an array of function handles that can
/// be used to schedule run functions on a pipeline associated with that 
/// process.  See the documentation for COIPipelineRunFunction() for 
/// additional information.  All functions that are to be retrieved in this  
/// fashion must have the define COINATIVEPROCESSEXPORT preceeding their type  
/// specification.  For functions that are written in C++, either the entries
/// in in_pFunctionNameArray in must be pre-mangled, or the functions must be 
/// declared as extern "C".  It is possible for this call to successfully find 
/// function handles for some of the names passed in but not all of them. If 
/// this occurs COI_DOES_NOT_EXIST will return and any handles not found will
/// be returned as NULL.
///
/// @param  in_Process
///         [in] Process handle previously returned via COIProcessCreate()
///
/// @param  in_NumFunctions
///         [in] Number of function names passed in to the in_pFunctionNames
///         array.
///
/// @param  in_ppFunctionNameArray
///         [in] Pointer to an array of null-terminated strings that match
///         the name of functions present in the code of the binary 
///         previously loaded via COIProcessCreate().  Note that if a C++ 
///         function is used, then the string passed in must already be 
///         properly name-mangled, or extern "C" must be used for where
///         the function is declared.
///
/// @param  out_pFunctionHandleArray
///         [in out] Pointer to a location created by the caller large 
///         enough to hold an array of COIFUNCTION sized elements that has
///         in_numFunctions entries in the array.
///
/// @return COI_SUCCESS if all function names indicated were found.
///
/// @return COI_INVALID_HANDLE if the in_Process handle passed in was invalid.
///
/// @return COI_OUT_OF_RANGE if in_NumFunctions is zero.
///
/// @return COI_INVALID_POINTER if the in_ppFunctionNameArray or 
///         out_pFunctionHandleArray pointers was NULL.
///
/// @return COI_DOES_NOT_EXIST if one or more function names were not
///         found. To determine the function names that were not found, 
///         check which elements in the out_pFunctionHandleArray
///         are set to NULL.
///
/// @return COI_OUT_OF_RANGE if any of the null-terminated strings passed in 
///         via in_ppFunctionNameArray were more than 
///         COI_MAX_FUNCTION_NAME_LENGTH characters in length including 
///         the null.
///
/// @warning This operation can take several milliseconds so it is recommended 
///          that it only be be done at load time.
///
COIRESULT
COIProcessGetFunctionHandles(
            COIPROCESS          in_Process,
            uint32_t            in_NumFunctions,
    const   char**              in_ppFunctionNameArray,
            COIFUNCTION*        out_pFunctionHandleArray);


//////////////////////////////////////////////////////////////////////////////
///
/// Loads a shared library into the specified remote process, akin to using
/// dlopen() on a local process in Linux or LoadLibrary() in Windows.
/// Dependencies for this library that are not listed with absolute paths
/// are searched for first in current working directory, then in the
/// colon-delimited paths in the environment variable SINK_LD_LIBRARY_PATH,
/// and finally in the MPSS-installed library paths.
///
/// @param  in_Process
///         [in] Process to load the library into.
///
/// @param  in_pLibraryBuffer
///         [in] The memory buffer containing the shared library to load.
///
/// @param  in_LibraryBufferLength
///         [in] The number of bytes in the memory buffer in_pLibraryBuffer.
///
/// @param  in_pLibraryName
///         [in] Name for the shared library. This optional parameter can
///         be specified in case the dynamic library doesn't have an
///         SO_NAME field. If specified, it will take precedence over
///         the SO_NAME if it exists. If it is not specified then
///         the library must have a valid SO_NAME field.
///
/// @param  out_pLibrary
///         [out] If COI_SUCCESS or COI_ALREADY_EXISTS is returned, the handle
///         that uniquely identifies the loaded library.
///
/// @return COI_SUCCESS if the library was successfully loaded.
///
/// @return COI_INVALID_HANDLE if the process handle passed in was invalid.
///
/// @return COI_OUT_OF_RANGE if in_LibraryBufferLength is 0.
///
/// @return COI_INVALID_FILE if in_pLibraryBuffer does not represent a valid
///         shared library file.
///
/// @return COI_ARGUMENT_MISMATCH if the shared library is missing an SONAME
///         and in_pLibraryName is NULL.
///
/// @return COI_ARGUMENT_MISMATCH if in_pLibraryName is the same as that of
///         any of the dependencies (recursive) of the library being loaded.
///
/// @return COI_ALREADY_EXISTS if there is an existing COILIBRARY handle
///         that identifies this library, and this COILIBRARY hasn't been
///         unloaded yet.
///
/// @return COI_BINARY_AND_HARDWARE_MISMATCH if the binary's target machine
///         does not match the engine associated with in_Process.
///
/// @return COI_INVALID_POINTER if out_pLibrary is NULL.
///
COIRESULT
COIProcessLoadLibraryFromMemory(
            COIPROCESS          in_Process,
    const   void*               in_pLibraryBuffer,
            uint64_t            in_LibraryBufferLength,
    const   char*               in_pLibraryName,
            COILIBRARY*         out_pLibrary);

//////////////////////////////////////////////////////////////////////////////
///
/// Loads a shared library into the specified remote process, akin to using
/// dlopen() on a local process in Linux or LoadLibrary() in Windows.
/// Dependencies for this library that are not listed with absolute paths
/// are searched for first in current working directory, then in the
/// colon-delimited paths in the environment variable SINK_LD_LIBRARY_PATH,
/// and finally in the MPSS-installed library paths.
///
/// @param  in_Process
///         [in] Process to load the library into.
///
/// @param  in_pFileName
///         [in] The name of the shared library file on the source's file
///         system that is being loaded. If the file name is not an absolute
///         path, the file is searched for in the same manner as dependencies.
///
/// @param  in_pLibraryName
///         [in] Name for the shared library. This optional parameter can
///         be specified in case the dynamic library doesn't have an
///         SO_NAME field. If specified, it will take precedence over
///         the SO_NAME if it exists. If it is not specified then
///         the library must have a valid SO_NAME field.
///
/// @param  out_pLibrary
///         [out] If COI_SUCCESS or COI_ALREADY_EXISTS is returned, the handle
///         that uniquely identifies the loaded library.
///
/// @return COI_SUCCESS if the library was successfully loaded.
///
/// @return COI_INVALID_HANDLE if the process handle passed in was invalid.
///
/// @return COI_INVALID_POINTER if in_pFileName is NULL.
///
/// @return COI_DOES_NOT_EXIST if in_pFileName cannot be found.
///
/// @return COI_INVALID_FILE if the file is not a valid shared library.
///
/// @return COI_ARGUMENT_MISMATCH if the shared library is missing an SONAME
///         and in_pLibraryName is NULL.
///
/// @return COI_ARGUMENT_MISMATCH if in_pLibraryName is the same as that of
///         any of the dependencies (recursive) of the library being loaded.
///
/// @return COI_ALREADY_EXISTS if there is an existing COILIBRARY handle
///         that identifies this library, and this COILIBRARY hasn't been
///         unloaded yet.
///
/// @return COI_BINARY_AND_HARDWARE_MISMATCH if the binary's target machine
///         does not match the engine associated with in_Process.
///
/// @return COI_INVALID_POINTER if out_pLibrary is NULL.
///
COIRESULT
COIProcessLoadLibraryFromFile(
            COIPROCESS          in_Process,
    const   char*               in_pFileName,
    const   char*               in_pLibraryName,
            COILIBRARY*         out_pLibrary);

//////////////////////////////////////////////////////////////////////////////
///
/// Unloads a a previously loaded shared library from the specified
/// remote process.
///
/// @param  in_Process
///         [in] Process that we are unloading a library from.
///
/// @param  in_Library
///         [in] Library that we want to unload.
///
/// @return COI_SUCCESS if the library was successfully loaded.
///
/// @return COI_INVALID_HANDLE if the process or library handle were invalid.
///
COIRESULT
COIProcessUnLoadLibrary(
            COIPROCESS          in_Process,
            COILIBRARY          in_Library);



#ifdef __cplusplus
} // extern "C"
#endif
#endif //_COIPROCESS_SOURCE_H

/*! @} */
