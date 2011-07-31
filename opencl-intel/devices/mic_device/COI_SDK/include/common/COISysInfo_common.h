/* ************************************************************************* *\
                  INTEL CORPORATION PROPRIETARY INFORMATION
      This software is supplied under the terms of a license agreement or 
      nondisclosure agreement with Intel Corporation and may not be copied 
      or disclosed except in accordance with the terms of that agreement. 
          Copyright (C) 2010,2011 Intel Corporation. All Rights Reserved.
\* ************************************************************************* */
#ifndef COISYSINFO_COMMON_H
#define COISYSINFO_COMMON_H
/** @ingroup COISysInfo
 *  @addtogroup COISysInfoCommon
@{
* @file common/COISysInfo_common.h 
* This interface allows developers to query the platform for system level
* information. */

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include <common/COITypes_common.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif 
#endif // DOXYGEN_SHOULD_SKIP_THIS

#define INITIAL_APIC_ID_BITS 0xFF000000   // EBX[31:24] unique APIC ID
#define NUMBER_HW_THREADS 128

///////////////////////////////////////////////////////////////////////////////
/// \fn uint32_t COISysGetAPICID(void)
/// @return The Advanced Programmable Interrupt Controller (APIC) ID of 
/// the hardware thread on which the caller is running.
/// 
/// @warning APIC IDs are unique to each hardware thread within a processor, 
/// but may not be sequential.
uint32_t COISysGetAPICID(void);

///////////////////////////////////////////////////////////////////////////////
/// 
/// @return The number of cores exposed by the processor on which the caller is 
/// running.
uint32_t COISysGetCoreCount(void);

///////////////////////////////////////////////////////////////////////////////
/// 
/// @return The number of hardware threads exposed by the processor on which 
/// the caller is running.
uint32_t COISysGetHardwareThreadCount(void);

///////////////////////////////////////////////////////////////////////////////
/// 
/// @return The index of the hardware thread on which the caller is running.
/// 
/// The indexes of neighboring hardware threads will differ by a value of one 
/// and are within the range zero through COISysGetHardwareThreadCount()-1.
uint32_t COISysGetHardwareThreadIndex(void);

///////////////////////////////////////////////////////////////////////////////
/// 
/// @return The index of the core on which the caller is running.
/// 
/// The indexes of neighboring cores will differ by a value of one and are 
/// within the range zero through COISysGetCoreCount()-1.
uint32_t COISysGetCoreIndex(void);

///////////////////////////////////////////////////////////////////////////////
/// 
/// @return The number of level 2 caches within the processor on which the 
/// caller is running.
uint32_t COISysGetL2CacheCount(void);

///////////////////////////////////////////////////////////////////////////////
/// 
/// @return The index of the level 2 cache on which the caller is running.
/// 
/// The indexes of neighboring cores will differ by a value of one and are 
/// within the range zero through COISysGetL2CacheCount()-1.
uint32_t COISysGetL2CacheIndex(void);

#ifdef __cplusplus
} // extern "C"
#endif 
/*! @} */
#endif /* COISYSINFO_COMMON_H */
