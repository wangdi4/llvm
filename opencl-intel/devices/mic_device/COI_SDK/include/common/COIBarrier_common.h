/* ************************************************************************* *\
                  INTEL CORPORATION PROPRIETARY INFORMATION
      This software is supplied under the terms of a license agreement or 
      nondisclosure agreement with Intel Corporation and may not be copied 
      or disclosed except in accordance with the terms of that agreement. 
          Copyright (C) 2010 Intel Corporation. All Rights Reserved.
\* ************************************************************************* */
#ifndef _COIBARRIER_COMMONH_H
#define _COIBARRIER_COMMON_H
/** @ingroup COIBarrier
 *  @addtogroup COIBarriercommon
@{
* @file common/COIBarrier_common.h
*/
#ifndef DOXYGEN_SHOULD_SKIP_THIS

#include <common/COITypes_common.h>
#include <common/COIResult_common.h>

#ifdef __cplusplus
extern "C" {
#endif 
#endif // DOXYGEN_SHOULD_SKIP_THIS


///////////////////////////////////////////////////////////////////////////////
///
/// Signal  one shot User barrier. User barriers created on source can be
/// signaled from both sink and source. This fires the barrier and wakes up
/// threads waiting on COIBarrierWait.
///
/// Note: For barriers that are not registered or already signaled this call
///       will behave as a NOP. Users need to make sure that they pass valid
///       barriers on the sink side.
///
/// @param  in_Barrier
///         Barrier Handle to be signaled.
///
/// @return COI_SUCCESS
///
COIRESULT COIBarrierSignalUserBarrier(COIBARRIER in_Barrier);
///
///
#ifdef __cplusplus
} // extern "C"
#endif
#endif //_COIBARRIER_COMMON_H

/*! @} */
