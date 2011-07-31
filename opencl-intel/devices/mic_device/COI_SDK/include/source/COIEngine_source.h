/* ************************************************************************* *\
                  INTEL CORPORATION PROPRIETARY INFORMATION
      This software is supplied under the terms of a license agreement or 
      nondisclosure agreement with Intel Corporation and may not be copied 
      or disclosed except in accordance with the terms of that agreement. 
          Copyright (C) 2010-2011 Intel Corporation. All Rights Reserved.
\* ************************************************************************* */
#ifndef COIENGINE_SOURCE_H
#define COIENGINE_SOURCE_H
/** @ingroup COIEngine
 *  @addtogroup COIEngineSource
@{

* @file source\COIEngine_source.h 
*/
#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include<common/COITypes_common.h>
#include<common/COIResult_common.h>
#endif // DOXYGEN_SHOULD_SKIP_THIS

#ifdef __cplusplus
extern "C" {
#endif 



#define COI_MAX_DRIVER_VERSION_STR_LEN 255

#define COI_MAX_HW_THREADS 1024

#define COI_MAX_ISA_x86_64_DEVICES 1
#define COI_MAX_ISA_KNF_DEVICES 32
#define COI_MAX_ISA_KNC_DEVICES 32

typedef enum
{
    COI_ISA_INVALID = 0,
    COI_ISA_x86_64,
    COI_ISA_KNF,
    COI_ISA_KNC
} COI_ISA_TYPE;


///////////////////////////////////////////////////////////////////////////////
/// This structure returns information about a MIC engine.  A pointer to
/// this structure is passed into the COIGetEngineInfo() function, which fills
/// in the data before returning to the caller.
///
typedef struct COI_ENGINE_INFO
{
    /// The version string identifying the driver.
    wchar_t     DriverVersion[COI_MAX_DRIVER_VERSION_STR_LEN];

    /// The ISA supported by the engine.  
    COI_ISA_TYPE ISA;

    /// The number of cores on the engine.
    uint32_t    NumCores;              

    /// The number of texture samplers on the engine.
    uint32_t    NumTXS;

    /// The number of hardware threads on the engine.
    uint32_t    NumThreads;

    /// The maximum frequency (in MHz) of the cores on the engine.
    uint32_t    CoreMaxFrequency;

    /// The load percentage for each of the hardware threads on the engine.
    uint32_t    Load[COI_MAX_HW_THREADS];

    /// The amount of physical memory managed by the OS.
    uint64_t    PhysicalMemory;

    /// The amount of free physical memory in the OS.
    uint64_t    PhysicalMemoryFree;

    /// The amount of swap memory managed by the OS.
    uint64_t    SwapMemory;

    /// The amount of free swap memory in the OS.
    uint64_t    SwapMemoryFree;
} COI_ENGINE_INFO;

///////////////////////////////////////////////////////////////////////////////
/// 
/// Returns information related to a specified engine.
/// 
/// 
/// @param  in_EngineHandle
///         [in] The COIENGINE structure as provided from COIEngineGetHandle() 
///         which to query for device level information.
/// 
/// @param  out_pEngineInfo
///         [out] The address of a user allocated COI_ENGINE_INFO structure. 
///         Upon success, the contents of the structure will be updated 
///         to contain information related to the specified engine. 
///          
/// 
/// @return  COI_SUCCESS if the function completed without error. 
/// 
/// @return  COI_INVALID_HANDLE if the in_EngineHandle handle is not valid.
///
/// @return  COI_INVALID_POINTER if the out_pEngineInfo pointer is NULL.
/// 
COIRESULT
COIEngineGetInfo(
            COIENGINE           in_EngineHandle,
            COI_ENGINE_INFO*    out_pEngineInfo);


///////////////////////////////////////////////////////////////////////////////
///
/// Returns the number of engines in the system that match the provided ISA.
///
///
/// @param  in_ISA
///         [in] The bitmask specifying the ISA of the engines the caller 
///         would like to enumerate.  Only the number of engines that 
///         match a subset of the specified bitmask will be returned to 
///         the user.
///
/// @param  out_pNumEngines
///         [out] The number of engines available. This can be used to index 
///         into the engines using COIEngineGetHandle().
///
/// @return COI_SUCCESS if the function completed without error. 
///
/// @return COI_DOES_NOT_EXIST if the in_ISA parameter is not valid.
/// 
/// @return COI_INVALID_POINTER if the out_pNumEngines parameter is NULL.
///
COIRESULT
COIEngineGetCount(
            COI_ISA_TYPE    in_ISA,
            uint32_t*       out_pNumEngines);


///////////////////////////////////////////////////////////////////////////////
///
/// Returns the handle of a user specified engine.
///
///
/// @param  in_ISA
///         [in] The bitmask specifying the ISA of the engine. Only an engine
///         that matches a subset of the specified bitmask will be returned
///         to the user.
///
/// @param  in_EngineIndex
///         A unsigned integer which specifies the zero-based position of the 
///         engine in a collection of engines.  The makeup of this collection 
///         is defined by the in_ISA parameter.
///
/// @param  out_pEngineHandle
///         The address of an COIENGINE handle.
///
/// @return COI_SUCCESS if the function completed without error. 
///
/// @return COI_DOES_NOT_EXIST if the in_ISA parameter is not valid.
/// 
/// @return COI_OUT_OF_RANGE if in_EngineIndex is greater than or equal to 
///         the number of engines that match the in_ISA parameter.
///
/// @return COI_INVALID_POINTER if the out_pEngineHandle parameter is NULL.
///
COIRESULT
COIEngineGetHandle(
            COI_ISA_TYPE    in_ISA,
            uint32_t        in_EngineIndex,
            COIENGINE*      out_pEngineHandle);

#ifdef __cplusplus
} // extern "C"
#endif 
#endif // COIENGINE_SOURCE_H

/*! @} */
