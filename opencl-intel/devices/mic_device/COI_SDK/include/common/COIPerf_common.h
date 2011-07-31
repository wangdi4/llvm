/* ************************************************************************* *\
                  INTEL CORPORATION PROPRIETARY INFORMATION
      This software is supplied under the terms of a license agreement or 
      nondisclosure agreement with Intel Corporation and may not be copied 
      or disclosed except in accordance with the terms of that agreement. 
      Copyright (C) 2007, 2010, 2011 Intel Corporation. All Rights Reserved.
\* ************************************************************************* */
#ifndef COIPERF_COMMON_H
#define COIPERF_COMMON_H
/** @ingroup COIPerf
 *  @addtogroup COIPerfCommon
@{

* @file common/COIPerf_common.h 
* Performance Analysis API */
#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include <common/COITypes_common.h>

#ifdef __cplusplus
extern "C" {
#endif

#endif // DOXYGEN_SHOULD_SKIP_THIS

///////////////////////////////////////////////////////////////////////////////
///
/// Returns a performance counter value
///
/// This function returns a performance counter value that increments
/// at a constant rate for all time and is coherent across all cores.   
///
/// @return Current performance counter value or 0 if no performance counter 
///         is available
///
///
__inline uint64_t COIPerfGetCycleCounter(void)
{
    uint32_t tsc_low;
    uint32_t tsc_high;
    __asm__ __volatile__("rdtsc" : "=a"(tsc_low), "=d"(tsc_high));
    return (((uint64_t)tsc_high) << 32) | tsc_low;
}

///////////////////////////////////////////////////////////////////////////////
///
/// Returns the calculated system frequency in hertz.
///
/// @return Current system frequency in hertz.
///
uint64_t COIPerfGetCycleFrequency(void);

#ifdef __cplusplus
} // extern "C"
#endif 
/*! @} */

#endif /* COIPERF_COMMON_H */
