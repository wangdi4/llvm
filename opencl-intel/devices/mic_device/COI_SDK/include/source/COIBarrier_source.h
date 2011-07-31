/* ************************************************************************* *\
                  INTEL CORPORATION PROPRIETARY INFORMATION
      This software is supplied under the terms of a license agreement or
      nondisclosure agreement with Intel Corporation and may not be copied
      or disclosed except in accordance with the terms of that agreement.
          Copyright (C) 2010-2011 Intel Corporation. All Rights Reserved.
\* ************************************************************************* */
#ifndef _COIBARRIER_SOURCE_H
#define _COIBARRIER_SOURCE_H
/** @ingroup COIBarrier
 *  @addtogroup COIBarrierSource
@{
* @file source/COIBarrier_source.h
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
/// Wait for an arbitrary number of COIBARRIERs to be signaled as completed,
/// eg when the run function or asynchronous map call associated with a barrier
/// has finished execution.
/// If the user sets in_WaitForAll = True and not all of the barriers are
/// signaled when the timeout period is reached then COI_TIME_OUT_REACHED will
/// be returned.
/// If the user sets in_WaitForAll = False then if at least one barrier is
/// signaled when the timeout is reached then COI_SUCCESS is returned.
///
/// @param  in_NumBarriers
///         [in] The number of barriers to wait for.
///
/// @param  in_pBarriers
///         [in] The array of COIBARRIER handles to wait for.
///
/// @param  in_Timeout
///         [in] The time in milliseconds to wait for the barrier. 0 polls
///         and returns immediately, -1 blocks indefinitely.
///
/// @param  in_WaitForAll
///         [in] Boolean value specifying behavior.  If true, wait for all
///         barriers to be signaled, or for timeout, whichever happens first.
///         If false, return when any barrier is signaled, or at timeout.
///
/// @param  out_pNumSignaled
///         [out] The number of barriers that were signaled. If in_NumBarriers
///         is 1 or in_WaitForAll = True, this parameter is optional.
///
/// @param  out_pSignaledIndices
///         [out] Pointer to an array of indicies into the original barrier
///         array.  Those denoted have been signaled.  The user must provide
///         an array that is no smaller than the in_Barriers array. If
///         in_NumBarriers is 1 or in_WaitForAll = True, this parameter is
///         optional.
///
/// @return COI_SUCCESS once a barrier has been signaled completed.
///
/// @return COI_TIME_OUT_REACHED if the barriers are still in use when the
///         timeout is reached or timeout is zero (a poll).
///
/// @return COI_OUT_OF_RANGE if a negative value other than -1 is passed in to
///         the in_Timeout parameter.
///
/// @return COI_OUT_OF_RANGE if in_NumBarriers is 0.
///
/// @return COI_INVALID_POINTER if in_pBarriers is NULL.
///
/// @return COI_ARGUMENT_MISMATCH if in_NumBarriers > 1 and and if
///         in_WaitForAll is not true and out_pSignaled or
///         out_pSignaledIndicies are NULL .
///
/// @return COI_ARGUMENT_MISMATCH if out_pNumSignaled is not NULL
///         and out_pSignaledIndices is NULL (or vice versa.)
///
/// @return COI_BARRIER_CANCELED if while waiting on a user barrier, it gets
///         unregistered this returns COI_BARRIER_CANCELED
///
COIRESULT
COIBarrierWait(
            uint16_t        in_NumBarriers,
    const   COIBARRIER*     in_pBarriers,
            int32_t         in_Timeout,
            uint8_t         in_WaitForAll,
            uint32_t*       out_pNumSignaled,
            uint32_t*       out_pSignaledIndices);



///////////////////////////////////////////////////////////////////////////////
///
/// Register a User COIBARRIER so that it can be fired. Registered barrier is
/// a one shot User barrier in other words once signaled it cannot be used
/// again for signaling. You have to unregister and register again to enable
/// signaling. A barrier will be reset if it is re-registered without
/// unregistering, resulting in loss of all outstanding signals.
///
/// @param  out_pBarrier
///         Pointer to COIBARRIER handle being Registered
///
/// @return COI_SUCCESS a barrier is successfully registered
///
/// @return COI_INVALID_POINTER if out_pBarrier is NULL
///
COIRESULT
COIBarrierRegisterUserBarrier(
            COIBARRIER* out_pBarrier);


///////////////////////////////////////////////////////////////////////////////
///
/// Unregister a User COIBARRIER. Unregistering a unsignaled barrier is similar
/// to firing a barrier. Except Calling COIBarrierWait on a barrier that is
/// being unregistered returns COI_BARRIER_CANCELED
///
/// @param  in_Barrier
///         Barrier Handle to be unregistered.
///
/// @return COI_SUCCESS a barrier is successfully registered
///
COIRESULT
COIBarrierUnregisterUserBarrier(
            COIBARRIER in_Barrier);

#ifdef __cplusplus
} // extern "C"
#endif
#endif //_COIBARRIER_SOURCE_H

/*! @} */
