/* ************************************************************************* *\
                  INTEL CORPORATION PROPRIETARY INFORMATION
      This software is supplied under the terms of a license agreement or 
      nondisclosure agreement with Intel Corporation and may not be copied 
      or disclosed except in accordance with the terms of that agreement. 
          Copyright (C) 2010-2011 Intel Corporation. All Rights Reserved.
\* ************************************************************************* */
#ifndef COIBUFFER_SINK_H
#define COIBUFFER_SINK_H
/** @ingroup COIBuffer
 *  @addtogroup COIBufferSink
@{

* @file sink\COIBuffer_sink.h 
*/
#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include <common/COITypes_common.h>
#include <common/COIResult_common.h>
#endif // DOXYGEN_SHOULD_SKIP_THIS

#ifdef __cplusplus
extern "C" {
#endif 

//////////////////////////////////////////////////////////////////////////////
///
/// Adds a reference to the memory of a buffer.  The memory of the buffer
/// will remain on the device until both a corresponding COIBufferReleaseRef() 
/// call is made and the run function that delivered the buffer returns.
///
/// @warning It is possible for enqueued run functions to be unable to 
///          execute due to all card memory being occupied by addref'ed
///          buffers. As such, it is important that whenever a buffer is
///          addref'd that there be no dependencies on future run functions
///          for progress to be made towards releasing the buffer.
///
/// @param  in_pBuffer
///         [in] Pointer to the start of a buffer being addref'ed, that was
///         passed in at the start of the run function.
/// 
/// @return COI_SUCCESS if the buffer ref count was successfully incremented.
///
/// @return COI_INVALID_POINTER if the buffer pointer was invalid.
///
COIRESULT
COIBufferAddRef(
            void*           in_pBuffer);


//////////////////////////////////////////////////////////////////////////////
///
/// Removes a reference to the memory of a buffer.  The memory of the buffer
/// will be eligible for being freed on the device when the following
/// conditions are met: the run function that delivered the buffer
/// returns, and the number of calls to COIBufferReleaseRef() matches the 
/// number of calls to COIBufferAddRef().
///
/// @param  in_pBuffer
///         [in] Pointer to the start of a buffer previously addref'ed, that
///         was passed in at the start of the run function.
/// 
/// @return COI_SUCCESS if the buffer refcount was successfully decremented.
///
/// @return COI_INVALID_POINTER if the buffer pointer was invalid.
///
/// @return COI_OUT_OF_RANGE if the buffer did not have COIBufferAddRef() 
///         previously called on it.
///
COIRESULT
COIBufferReleaseRef(
            void*           in_pBuffer);


#ifdef __cplusplus
} // extern "C"
#endif 
#endif // COIBUFFER_SINK_H

/*! @} */
