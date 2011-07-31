/* ************************************************************************* *\
                  INTEL CORPORATION PROPRIETARY INFORMATION
      This software is supplied under the terms of a license agreement or 
      nondisclosure agreement with Intel Corporation and may not be copied 
      or disclosed except in accordance with the terms of that agreement. 
          Copyright (C) 2010-2011 Intel Corporation. All Rights Reserved.
\* ************************************************************************* */
#ifndef _COIPIPELINE_SINK_H
#define _COIPIPELINE_SINK_H
/** @ingroup COIPipeline
 *  @addtogroup COIPipelineSink
@{
* @file sink/COIPipeline_sink.h
*/
#ifndef DOXYGEN_SHOULD_SKIP_THIS

#include<common/COITypes_common.h>
#include<common/COIResult_common.h>

#ifdef __FreeBSD__
#define COINATIVELIBEXPORT_VISIBILITY "extern"
#else
#define COINATIVELIBEXPORT_VISIBILITY "default"
#endif

#ifdef __cplusplus
#define COINATIVELIBEXPORT \
    extern "C" __attribute__ ((visibility(COINATIVELIBEXPORT_VISIBILITY)))
#else
#define COINATIVELIBEXPORT \
    __attribute__ ((visibility(COINATIVELIBEXPORT_VISIBILITY)))
#endif

#ifdef __cplusplus
extern "C" {
#endif 
#endif // DOXYGEN_SHOULD_SKIP_THIS

//////////////////////////////////////////////////////////////////////////////
///
/// This is the prototype that run functions should follow.
///
/// @param   in_BufferCount
///          The number of buffers passed to the run function.
///
/// @param   in_ppBufferPointers
///          An array that is in_BufferCount in length that contains the
///          sink side virtual addresses for each buffer passed in to
///          the run function.
///
/// @param   in_pBufferLengths
///          An array that is in_BufferCount in length of uint32_t integers
///          describing the length of each passed in buffer in bytes.
///
/// @param   in_pMiscData
///          Pointer to the MiscData passed in when the run function
///          was enqueued on the source.
///
/// @param   in_MiscDataLen
///          Length in bytes of the MiscData passed in when the run function
///          was enqueued on the source.
///
/// @param   in_pReturnValue
///          Pointer to the location where the return value from this run
///          function will be stored.
///
/// @param   in_ReturnValueLength
///          Length in bytes of the user-allocated ReturnValue pointer.
///
/// @return  A uint64_t that can be retrieved in the out_UserData parameter
///          from the COIPipelineWaitForBarrier function.
///
typedef void 
(*RunFunctionPtr_t)(
            uint32_t        in_BufferCount,
            void**          in_ppBufferPointers,
            uint64_t*       in_pBufferLengths,
            void*           in_pMiscData,
            uint16_t        in_MiscDataLength,
            void*           in_pReturnValue,
            uint16_t        in_ReturnValueLength);

///////////////////////////////////////////////////////////////////////////////
///
/// Start processing pipelines on the Sink. This should be done after any
/// required initialization in the Sink's application has finished. No
/// run functions will actually be executed (although they may be queued)
/// until this function is called.
///
///
/// @return COI_SUCCESS if the pipelines were successfully started.
///
COIRESULT
COIPipelineStartExecutingRunFunctions();


#ifdef __cplusplus
} // extern "C"
#endif
#endif //_COIPIPELINE_SINK_H

/*! @} */
