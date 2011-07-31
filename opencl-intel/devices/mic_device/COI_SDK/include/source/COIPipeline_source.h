/* ************************************************************************* *\
                  INTEL CORPORATION PROPRIETARY INFORMATION
      This software is supplied under the terms of a license agreement or 
      nondisclosure agreement with Intel Corporation and may not be copied 
      or disclosed except in accordance with the terms of that agreement. 
          Copyright (C) 2010-2011 Intel Corporation. All Rights Reserved.
\* ************************************************************************* */
#ifndef _COIPIPELINE_SOURCE_H
#define _COIPIPELINE_SOURCE_H
/** @ingroup COIPipeline
 *  @addtogroup COIPipelineSource
@{
* @file source/COIPipeline_source.h
*/
#ifndef DOXYGEN_SHOULD_SKIP_THIS

#include <common/COITypes_common.h>
#include <common/COIResult_common.h>

#ifdef __cplusplus
extern "C" {
#endif 
#endif // DOXYGEN_SHOULD_SKIP_THIS



//////////////////////////////////////////////////////////////////////////////
/// These flags specify how a buffer will be used within a run function.  They 
/// allow COI to make optimizations in how it moves data around the system.  
/// These flags can affect the correctness of an application, so they must be 
/// set properly.  For example, if a buffer is used in a run function with the 
/// COI_SINK_READ flag and then mapped on the source, COI may use a previously 
/// cached version of the buffer instead of retrieving data from the sink.
typedef enum COI_ACCESS_FLAGS
{
    /// Specifies that the run function will only read the associated buffer.
    COI_SINK_READ = 1,

    /// Specifies that the run function will only write the associated buffer.
    COI_SINK_WRITE,

    /// Specifies that the run function will overwrite the entire associated
    /// buffer and therefore the buffer will not be synchronized with the
    /// source before execution.
    COI_SINK_WRITE_ENTIRE
} COI_ACCESS_FLAGS;

#define COI_PIPELINE_MAX_IN_BUFFERS 32768
#define COI_PIPELINE_MAX_IN_MISC_DATA_LEN 32768


///////////////////////////////////////////////////////////////////////////////
///
/// Create a pipeline assoiated with a remote process. This pipeline can
/// then be used to execute remote functions and to share data using COIBuffers.
///
/// @param  in_Process
///         [in] A handle to an already existing process that the pipeline
///         will be associated with.
///
/// @param  in_Mask
///         [in] An optional mask of the set of hardware threads on which the 
///         sink pipeline command processing thread could run.
///
/// @param  in_StackSize
///         [in] An optional value that will be used when the pipeline
///         processing thread is created on the sink. If the user passes in
///         0 the OS default stack size will be used.
///
/// @param  out_pPipeline
///         [out] Handle returned to uniquely identify the pipeline that was
///         created for use in later API calls.
///
///
/// @return COI_SUCCESS if the pipeline was successfully created.
///
/// @return COI_INVALID_HANDLE if the in_Process handle passed in was invalid.
///
/// @return COI_INVALID_POINTER if the out_pPipeline pointer was NULL.
///
/// @return COI_RESOURCE_EXHAUSTED if no more COIPipelines can be created.
///
/// @return COI_TIME_OUT_REACHED if establishing the communication channel with
///         the remote pipeline timed out.
///
COIRESULT
COIPipelineCreate(
            COIPROCESS          in_Process,
            COI_CPU_MASK        in_Mask,
            uint32_t            in_StackSize,
            COIPIPELINE*        out_pPipeline);

///////////////////////////////////////////////////////////////////////////////
///
/// Destroys the inidicated pipeline, releasing its resources.
///
/// @param  in_Pipeline
///         [in] Pipeline to destroy.
///
///
/// @return COI_SUCCESS if the pipeline was destroyed
///
/// @return COI_INVALID_HANDLE if the in_Pipeline handle passed in was invalid.
///
COIRESULT
COIPipelineDestroy(
            COIPIPELINE         in_Pipeline);


//////////////////////////////////////////////////////////////////////////////
///
/// Enqueues a function in the remote process binary to be executed. The
/// function execution is asynchronous in regards to the Source and all
/// run functions enqueued on a pipeline are executed in-order. The run
/// function will only execute when all of the required buffers are present
/// in the Sink's memory.
///
/// @param  in_Pipeline
///         [in] Handle to a previously created pipeline that this run
///         function should be enqueued to.   
///
/// @param  in_Function
///         [in] Previously returned handle from a call to 
///         COIPipelineGetFunctionHandle() that represents a function in the
///         application running on the Sink process.
///
/// @param  in_NumBuffers
///         [in] The number of buffers that are being passed to the run
///         function. This number must match the number of buffers in the
///         in_Buffers and in_pBufferAccessFlags arrays. Must be less than
///         COI_PIPELINE_MAX_IN_BUFFERS.
///
/// @param  in_Buffers
///         [in] An array of COIBUFFER handles that the function is expected 
///         to use during its execution. Each buffer when it arrives at the 
///         Sink process will be at least 4k page aligned, thus, using a very 
///         large number of small buffers is memory inefficient and should be 
///         avoided.
///
/// @param  in_pBufferAccessFlags
///         [in] An array of flag values which correspond to the buffers
///         passed in the in_Buffers parameter. These flags are used to
///         track dependendencies between different run functions being
///         executed from different pipelines. 
///
/// @param  in_NumDependencies
///         [in] The number of dependencies specified in the in_pDependencies
///         array. This may be 0 if the caller does not want the run function
///         to wait for any dependencies.
///
/// @param  in_pDependencies
///         [in] An optional array of COIBARRIER objects that this run 
///         function will wait for before executing. This allows the user to 
///         create dependencies between run functions in different pipelines. 
///         The user may pass in NULL if they do not wish to wait for any 
///         dependencies to complete.
///
/// @param  in_pMiscData
///         [in] Pointer to user defined data, typically used to pass 
///         parameters to Sink side functions. Should only be used for small 
///         amounts data since the data will be placed directly in the 
///         Driver's command buffer.  COIBuffers should be used to pass large 
///         amounts of data.
///
/// @param  in_MiscDataLen
///         [in] Size of the in_pMiscData in bytes. Must be less than
///         COI_PIPELINE_MAX_IN_MISC_DATA_LEN, and should usually be much 
///         smaller, see documentation for the parameter in_pMiscData.
///
/// @param  out_pAsyncReturnValue 
///         [out] Pointer to user-allocated memory where the return value from
///         the run function will be placed.  This memory should not be read
///         until out_pCompletion has been signalled.
///
/// @param  in_AsyncReturnValueLen
///         [in] Size of the out_pAsyncReturnValue in bytes.
///
/// @param  out_pCompletion
///         [out] An optional pointer to a COIBARRIER object 
///         that will be signaled when this run function has completed 
///         execution. The user may pass in NULL if they do not wish to signal
///         any COIBARRIERs when this run function completes.
///
/// @return COI_SUCCESS if the function was successfully placed in a 
///         pipeline for future execution.  Note that the actual
///         execution of the function will occur in the future.
///
/// @return COI_OUT_OF_RANGE if in_NumBuffers is greater than
///         COI_PIPELINE_MAX_IN_BUFFERS or if in_MiscDataLen is greater than
///         COI_PIPELINE_MAX_IN_MISC_DATA_LEN.
///
/// @return COI_INVALID_HANDLE if the pipeline handle passed in was invalid.
///
/// @return COI_INVALID_HANDLE if the function handle passed in was invalid.
///
/// @return COI_INVALID_HANDLE if any of the buffers passed in are invalid.
///
/// @return COI_ARGUMENT_MISMATCH if in_NumDependencies is non-zero while
///         in_pDependencies was passed in as NULL.
///
/// @return COI_ARGUMENT_MISMATCH if in_pDependencies is non-NULL but
///         in_NumDependencies is zero.
///
/// @return COI_ARGUMENT_MISMATCH if in_MiscDataLen is non-zero while
///         in_pMiscData was passed in as NULL.
///
/// @return COI_ARGUMENT_MISMATCH if in_pMiscData is non-NULL but 
///         in_MiscDataLen is zero.
///
/// @return COI_ARGUMENT_MISMATCH if in_NumBuffers is non-zero and in_Buffers
///         or in_pBufferAccessFlags are NULL.
///
/// @return COI_ARGUMENT_MISMATCH if in_pBufferAccessFlags is non-NULL but 
///         in_NumBuffers is zero.
///
/// @return COI_ARGUMENT_MISMATCH if in_ReturnValueLen is non-zero while
///         in_pReturnValue was passed in as NULL.
///
/// @return COI_ARGUMENT_MISMATCH if in_pReturnValue is non-NULL but
///         in_ReturnValueLen is zero.
///
/// @return COI_RETRY if any input buffers, which are not pinned buffers,
///         are still mapped when passed to the run function.
///
COIRESULT
COIPipelineRunFunction(
            COIPIPELINE         in_Pipeline,
            COIFUNCTION         in_Function,
            uint32_t            in_NumBuffers,
    const   COIBUFFER*          in_Buffers,
    const   COI_ACCESS_FLAGS*   in_pBufferAccessFlags,
            uint32_t            in_NumDependencies,
    const   COIBARRIER*         in_pDependencies,
    const   void*               in_pMiscData,
            uint16_t            in_MiscDataLen,
            void*               out_pAsyncReturnValue,
            uint16_t            in_AsyncReturnValueLen,
            COIBARRIER*         out_pCompletion);


//////////////////////////////////////////////////////////////////////////////
///
/// Flushes the commands in the pipeline, causing the Sink to start
/// processing run functions from the indicated pipeline, and waits for all
/// run functions that have been queued on the pipeline before the flush
/// to finish before the call returns.
///
/// @param  in_Pipeline
///         [in] Pipeline to flush.
///
/// @param  in_Timeout
///         [in] The time in milliseconds to wait for the flush to complete. 
///         -1 waits indefinitely, 0 does not wait at all and returns
///         immediately.
///
/// @return COI_SUCCESS if the pipeline was flushed.
/// 
/// @return COI_INVALID_HANDLE if the pipeline handle passed in was invalid.
/// 
/// @return COI_OUT_OF_RANGE if in_Timeout is less than -1.
///
/// @return COI_TIME_OUT_REACHED if the flush has not completed when the 
///         timeout is reached.
///
COIRESULT
COIPipelineFlush(
            COIPIPELINE         in_Pipeline,
            int32_t             in_Timeout);


//////////////////////////////////////////////////////////////////////////////
///
/// Retrieve the engine that the pipeline is associated with.
///
/// @param  in_Pipeline
///         [in] Pipeline to query.
///
/// @param  out_pEngine
///         [out] The handle of the Engine.
///
/// @return COI_SUCCESS if the engine was retrieved.
/// 
/// @return COI_INVALID_HANDLE if the pipeline handle passed in was invalid.
///
/// @return COI_INVALID_POINTER if the out_pEngine parameter is NULL.
/// 
COIRESULT
COIPipelineGetEngine(
            COIPIPELINE         in_Pipeline,
            COIENGINE*          out_pEngine);

//////////////////////////////////////////////////////////////////////////////
///
/// Set a given mask to a particular core:thread pair.
///
/// @param  in_Process
///         [in] A handle to an already existing process that the pipeline
///         will be associated with.
///
/// @param  in_CoreID
///         [in] Core to affinitize to; must be less than the number of cores
///         on the device. 
///
/// @param  in_ThreadID
///         [in] Thread on the core to affinitize to (0 - 3).
///
/// @param  out_pMask
///         [out] Pointer to the mask to set.
///
/// @return COI_SUCCESS if the mask was set.
/// 
/// @return COI_OUT_OF_RANGE if the in_CoreID or in_ThreadID is out of range.
/// 
/// @return COI_INVALID_POINTER if out_pMask is invalid.
///
/// @return COI_INVALID_HANDLE if in_Process is invalid.
///
COIRESULT
COIPipelineSetCPUMask(
            COIPROCESS          in_Process,
            uint32_t            in_CoreID,
            uint8_t             in_ThreadID,
            COI_CPU_MASK*       out_pMask);

//////////////////////////////////////////////////////////////////////////////
///
/// Clears a given mask.
///
/// @param  in_Mask
///         [in] Pointer to the mask to clear.
///
/// @return COI_SUCCESS if the mask was cleared.
/// 
/// @return COI_INVALID_POINTER if in_Mask is invalid.
/// 
COIRESULT
COIPipelineClearCPUMask(
            COI_CPU_MASK*       in_Mask);

#ifdef __cplusplus
} // extern "C"
#endif
#endif //_COIPIPELINE_SOURCE_H

/*! @} */
