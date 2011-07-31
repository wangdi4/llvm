/* ************************************************************************* *\
                  INTEL CORPORATION PROPRIETARY INFORMATION
      This software is supplied under the terms of a license agreement or 
      nondisclosure agreement with Intel Corporation and may not be copied 
      or disclosed except in accordance with the terms of that agreement. 
          Copyright (C) 2010-2011 Intel Corporation. All Rights Reserved.
\* ************************************************************************* */
#ifndef COIBUFFER_SOURCE_H
#define COIBUFFER_SOURCE_H
/** @ingroup COIBuffer
 *  @addtogroup COIBufferSource
@{

* @file source\COIBuffer_source.h 
*/
#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include<common/COITypes_common.h>
#include<common/COIResult_common.h>
#endif // DOXYGEN_SHOULD_SKIP_THIS

#ifdef __cplusplus
extern "C" {
#endif 


//////////////////////////////////////////////////////////////////////////////
/// The valid buffer types that may be created using COIBufferCreate.
/// Please see the COI_VALID_BUFFER_TYPES_AND_FLAGS matrix
/// below which describes the valid combinations of buffer types and flags.
///
typedef enum COI_BUFFER_TYPE
{
    /// Normal buffers exist as a single physical buffer in either Source or
    /// Sink physical memory. Mapping the buffer may stall the pipelines.
    COI_BUFFER_NORMAL = 1,
    
    /// A streaming buffer creates new versions each time it is passed to
    /// Runfunction. These new versions are consumed by run functions.

    ///  To_SINK buffers are used to send data from SOURCE to SINK
    ///  These buffers are SOURCE write only buffers. If read, won't
    ///  get Data written by SINK
    COI_BUFFER_STREAMING_TO_SINK,

    ///  To_SOURCE buffers are used to get data from SINK to SOURCE
    ///  These buffers are SOURCE Read only buffers. If written, data
    ///  won't get reflected on SINK side.
    COI_BUFFER_STREAMING_TO_SOURCE,

    /// A pinned buffer exists in a shared memory region and is always
    /// available for read or write operations.
    COI_BUFFER_PINNED
   
} COI_BUFFER_TYPE;


/// @name COIBUFFER creation flags.
/// Please see the COI_VALID_BUFFER_TYPES_AND_FLAGS matrix
/// below which describes the valid combinations of buffer types and flags.
//@{

/// Create the buffer such that it has the same virtual address on all of the 
/// sink processes with which it is associated.
#define COI_SAME_ADDRESS_SINKS             0x00000001

/// Create the buffer such that it has the same virtual address on all of the 
/// sink processes with which it is associated and in the source process.  
#define COI_SAME_ADDRESS_SINKS_AND_SOURCE  0x00000002

/// Hint to the runtime that the source will frequently read the buffer
#define COI_OPTIMIZE_SOURCE_READ           0x00000004

/// Hint to the runtime that the source will frequently write the buffer
#define COI_OPTIMIZE_SOURCE_WRITE          0x00000008

/// Hint to the runtime that the sink will frequently read the buffer
#define COI_OPTIMIZE_SINK_READ             0x00000010

/// Hint to the runtime that the sink will frequently write the buffer
#define COI_OPTIMIZE_SINK_WRITE            0x00000020

/// Causes the buffer to be allocated from memory that is accessible by the 
/// SPU and TXS devices on MIC.  This flag is only valid for buffers used in 
/// run functions that execute on the MIC device.
/// Note that SPU/TXS buffers are not currently supported
#define COI_SPU_TXS_MEMORY                 0x00000040

//@}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// Make the flag mask
#ifdef F
#undef F
#endif
#define F 0
#ifdef T
#undef T
#endif
#define T 1
#define MTM(_BUFFER, B1, B2, B3, B4, B5, B6, B7) \
  (B1 | B2 << 1 | B3 << 2 | B4 << 3 | B5 << 4 | B6 << 5 | B7 << 6)
#endif

/// This matrix shows the valid combinations of buffer types and buffer flags
/// that may be passed in to COIBufferCreate and COIBufferCreateFromMemory.
/// \code
static const uint64_t
COI_VALID_BUFFER_TYPES_AND_FLAGS[COI_BUFFER_PINNED+1] = {
/*                             | SAME  |      
                       | SAME  | ADDR  | OPT   | OPT   | OPT   | OPT   | SPU  |
                       | ADDR  | SINK  | SRC   | SRC   | SINK  | SINK  | TXS  |
                       | SINKS | SRC   | READ  | WRITE | READ  | WRITE | MEM  |
                       +-------+-------+-------+-------+-------+-------+------*/
MTM(INVALID            ,   F   ,   F   ,   F   ,   F   ,   F   ,   F   ,   F  ),
MTM(NORMAL             ,   T   ,   T   ,   T   ,   T   ,   T   ,   T   ,   T  ),
MTM(STREAMING_TO_SINK  ,   T   ,   F   ,   F   ,   T   ,   T   ,   T   ,   F  ),
MTM(STREAMING_TO_SOURCE,   T   ,   F   ,   T   ,   T   ,   F   ,   T   ,   F  ),
MTM(PINNED             ,   T   ,   T   ,   T   ,   T   ,   T   ,   T   ,   F  )
};
///\endcode
#undef MTM

//////////////////////////////////////////////////////////////////////////////
/// These flags control how the buffer will be accessed on the source after
/// it is mapped.
/// Please see the COI_VALID_BUFFER_TYPES_AND_MAP matrix below for the
/// valid buffer type and map operation combinations.
typedef enum COI_MAP_TYPE
{
    /// Allows the application to read and write the contents of the buffer
    /// after it is mapped.
    COI_MAP_READ_WRITE = 1,

    /// If this flag is set then the application must only read from the
    /// buffer after it is mapped. If the application writes to the buffer
    /// the contents will not be reflected back to the sink or stored for
    /// the next time the buffer is mapped on the source.
    /// This allows the runtime to make significant performance optimizations 
    /// in buffer handling.
    COI_MAP_READ_ONLY,

    /// Setting this flag means that the source will overwrite the entire
    /// buffer once it is mapped. The app must not read from the buffer and
    /// must not expect the contents of the buffer to be synchronized from
    /// the sink side during the map operation.
    /// This allows the runtime to make significant performance optimizations 
    /// in buffer handling.
    COI_MAP_WRITE_ENTIRE_BUFFER
} COI_MAP_TYPE;

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// Make the flag mask
#define MMM(_BUFFER, B1, B2, B3) \
  {  F  , B1, B2, B3}
#endif

/// This matrix shows the valid combinations of buffer types and map
/// operations that may be passed in to COIBufferMap.
/// \code
static const uint64_t
COI_VALID_BUFFER_TYPES_AND_MAP
[COI_BUFFER_PINNED+1][COI_MAP_WRITE_ENTIRE_BUFFER+1] = {
/*                      | MAP   | MAP   | MAP
                        | READ  | READ  | WRITE
                        | WRITE | ONLY  | ENTIRE|
                        +-------+-------+-------*/
MMM(INVALID             ,   F   ,   F   ,   F   ),
MMM(NORMAL              ,   T   ,   T   ,   T   ),
MMM(STREAMING_TO_SINK   ,   F   ,   F   ,   T   ),
MMM(STREAMING_TO_SOURCE ,   F   ,   T   ,   F   ),
MMM(PINNED              ,   T   ,   T   ,   T   )
};
///\endcode
#undef MMM
#ifndef DOXYGEN_SHOULD_SKIP_THIS
#undef F
#undef T
#endif

//////////////////////////////////////////////////////////////////////////////
/// The valid copy operation types for the COIBufferWrite, COIBufferRead,
/// and COIBufferCopy APIs.
///
typedef enum COI_COPY_TYPE
{
    /// The runtime can pick the best suitable way to copy the data.
    COI_COPY_UNSPECIFIED = 0,

    /// The runtime should use DMA to copy the data.
    COI_COPY_USE_DMA, 

    /// The runtime should use a CPU copy to copy the data.
    COI_COPY_USE_CPU

} COI_COPY_TYPE;


//////////////////////////////////////////////////////////////////////////////
/// The buffer states are used to indicate whether a buffer is available for
/// access in a COIPROCESS. This is used with COIBufferSetState.
typedef enum {
    COI_BUFFER_VALID = 0,    // Buffer is valid and up-to-date on the process
    COI_BUFFER_INVALID       // Buffer is not valid, need valid data
} COI_BUFFER_STATE;

//////////////////////////////////////////////////////////////////////////////
/// The buffer move flags are used to indicate when a buffer should be moved
/// when it's state is changed. This is used with COIBufferSetState.
typedef enum {
    COI_BUFFER_MOVE_DATA = 0, // Dirty data is moved if state change requires it
    COI_BUFFER_NO_MOVE_DATA   // Change state without moving data
} COI_BUFFER_MOVE_FLAG;

//////////////////////////////////////////////////////////////////////////////
///
/// Creates a buffer that can be used in RunFunctions that are queued in
/// pipelines. The address space for the buffer is reserved when it is
/// created although the memory may not be committed until the buffer is
/// used for the first time.
///
/// @param  in_Size
///         [in] The number of bytes to allocate for the buffer. If in_Size
///         is not page aligned, it will be rounded up. 
///
/// @param  in_Type
///         [in] The type of the buffer to create.
///
/// @param  in_Flags
///         [in] A bitmask of attributes for the newly created buffer.
///         Some of these flags are required for correctness while others
///         are provided as hints to the runtime system so it can make
///         certain performance optimizations.
///
/// @param  in_pInitData
///         [in] If non-NULL the buffer will be initialized with the data 
///         pointed to by pInitData. The memory at in_pInitData must hold
///         at least in_Size bytes.
///
/// @param  in_NumProcesses
///         [in] The number of processes with which this buffer might be used.
///
/// @param  in_pProcesses
///         [in] An array of COIPROCESS handles identifying the processes with 
///         which this buffer might be used.
///
/// @param  out_pBuffer
///         [out] Pointer to a buffer handle. The handle will be filled in
///         with a value that uniquely identifies the newly created buffer.
///         This handle should be disposed of via COIBufferDestroy()
///         once it is no longer needed.
/// 
/// @return COI_SUCCESS if the buffer was created
///
/// @return COI_ARGUMENT_MISMATCH if the in_Type and in_Flags parameters 
///         are not compatible with one another. Please see the 
///         COI_VALID_BUFFER_TYPES_AND_FLAGS map above for information about
///         which flags and types are compatible.
///
/// @return COI_OUT_OF_RANGE if in_Size is zero, if the bits set in
///         the in_Flags parameter are not recognized flags, or if 
///         in_NumProcesses is zero.
///
/// @return COI_INVALID_POINTER if the in_pProcesses or out_pBuffer parameter
///         is NULL.
///
/// @return COI_INVALID_HANDLE if one of the COIPROCESS handles in the
///         in_pProcesses array does not identify a valid process.
///
/// @return COI_OUT_OF_MEMORY if allocating the buffer fails.
///
/// @return COI_RESOURCE_EXHAUSTED if the device is out of buffer memory.
///
COIRESULT 
COIBufferCreate(
            uint64_t            in_Size,
            COI_BUFFER_TYPE     in_Type,
            uint32_t            in_Flags,
    const   void*               in_pInitData,
            uint32_t            in_NumProcesses,
    const   COIPROCESS*         in_pProcesses,
            COIBUFFER*          out_pBuffer);

//////////////////////////////////////////////////////////////////////////////
///
/// Creates a buffer from some existing memory that can be used in 
/// RunFunctions that are queued in pipelines. The memory provided is used
/// as backing store for the buffer on the source and must not be freed 
/// before the buffer is destroyed. The runtime system may also reserve memory
/// on the sink for the buffer to optimize performance. 
/// While the user still owns the memory passed in they must use COIBufferMap
/// calls to get access to the memory so that the runtime knows when the
/// memory has been modified. If the user just writes directly to the memory
/// location then those changes may not be visible on the sink when the
/// corresponding buffer is accessed.
/// Whatever values are already present in the memory location when this call
/// is made is preserved. The memory values are also preserved when
/// COIBufferDestroy is called.
///
///
/// @param  in_Size
///         [in] The size of in_Memory in bytes. If in_Size
///         is not page aligned, it will be rounded up. 
///
/// @param  in_Type
///         [in] The type of the buffer to create. Note that streaming buffers
///         can not be created from user memory. Only COI_BUFFER_NORMAL and
///         COI_BUFFER_PINNED buffer types are supported.
///
/// @param  in_Flags
///         [in] A bitmask of attributes for the newly created buffer.
///         Some of these flags are required for correctness while others
///         are provided as hints to the runtime system so it can make
///         certain performance optimizations. Note that the flag
///         COI_SAME_ADDRESS_SINKS_AND_SOURCE is still valid but may fail
///         if the same address as in_Memory can not be allocated on the sink.
///
/// @param  in_Memory
///         [in] A pointer to an already allocated memory region on the source
///         that should be turned into a COIBUFFER. Although the user still
///         owns this memory they should not free it before calling
///         COIBufferDestroy. They must also only access the memory using
///         COIBUFFER semantics, for example using COIBufferMap/COIBufferUnmap
///         when they wish to read or write the data. There are no alignment
///         or size requirements for this memory region.
///
/// @param  in_NumProcesses
///         [in] The number of processes with which this buffer might be used.
///
/// @param  in_pProcesses
///         [in] An array of COIPROCESS handles identifying the processes with 
///         which this buffer might be used.
///
/// @param  out_pBuffer
///         [out] Pointer to a buffer handle. The handle will be filled in
///         with a value that uniquely identifies the newly created buffer.
///         This handle should be disposed of via COIBufferDestroy()
///         once it is no longer needed.
/// 
/// @return COI_SUCCESS if the buffer was created
///
/// @return COI_NOT_SUPPORTED if the in_Type value is not COI_BUFFER_NORMAL or
///         COI_BUFFER_PINNED.
///
/// @return COI_ARGUMENT_MISMATCH if the in_Type and in_Flags parameters 
///         are not compatible with one another. Please see the 
///         COI_VALID_BUFFER_TYPES_AND_FLAGS map above for information about
///         which flags and types are compatible.
///
/// @return COI_OUT_OF_RANGE if in_Size is zero, if the bits set in
///         the in_Flags parameter are not recognized flags, or if 
///         in_NumProcesses is zero.
///
/// @return COI_INVALID_POINTER if in_Memory, in_pProcesses or 
///         out_pBuffer parameter is NULL.
///
/// @return COI_INVALID_HANDLE if one of the COIPROCESS handles in the
///         in_pProcesses array does not identify a valid process.
///
COIRESULT 
COIBufferCreateFromMemory(
            uint64_t            in_Size,
            COI_BUFFER_TYPE     in_Type,
            uint32_t            in_Flags,
            void*               in_Memory,
            uint32_t            in_NumProcesses,
    const   COIPROCESS*         in_pProcesses,
            COIBUFFER*          out_pBuffer);


//////////////////////////////////////////////////////////////////////////////
///
/// Destroys a buffer.  Will block on completion of any operations on the
/// buffer, such as COIPipelineRunFunction or COIBufferCopy.  Will block until
/// all COIBufferAddRef calls have had a matching COIBufferReleaseRef call
/// made.  Will not block on an outstanding COIBufferUnmap but will instead
/// return COI_RETRY.
///
/// @param  in_Buffer
///         [in] Handle of the buffer to destroy.
///
/// @return COI_SUCCESS if the buffer was destroyed.
///
/// @return COI_INVALID_HANDLE if the buffer handle was invalid.
///
/// @return COI_RETRY if the buffer is currently mapped. The buffer must 
///         first be unmapped before it can be destroyed.
///
COIRESULT
COIBufferDestroy(
            COIBUFFER           in_Buffer);


//////////////////////////////////////////////////////////////////////////////
///
/// This call initiates a request to access a region of a buffer. Multiple
/// overlapping (or non overlapping) regions can be mapped simultaneously for
/// any given buffer.  If a completion barrier is specified this call will
/// queue a request for the data which will be satisfied when the buffer is
/// available.  Once all conditions are met the completion barrier will be
/// signaled and the user can access the data at out_ppData.  The user can call
/// COIBarrierWait with out_pCompletion to find out when the map operation has
/// completed. If the user accesses the data before the map operation is
/// complete the results are undefined.  If out_pCompletion is NULL then this
/// call blocks until the map operation completes and when this call returns
/// out_ppData can be safely accessed.  This call returns a map instance handle
/// in an out parameter which must be passed into COIBufferUnmap when the user
/// no longer needs access to that region of the buffer.
///
/// Note that different types of buffers behave differently when mapped.
/// For instance, mapping a COI_BUFFER_NORMAL for write must stall if the 
/// buffer is currently being written to by a run function. Mapping a
/// COI_BUFFER_STREAMING_TO_SINK will create a new physical copy of the buffer
/// and make it available immediately.  Mapping a COI_BUFFER_PINNED buffer will
/// not affect other functions that use that buffer since a COI_BUFFER_PINNED
/// buffer can be mapped at any time.
/// The asynchronous operation of COIBufferMap will likely be most useful when
/// paired with a COI_BUFFER_NORMAL.
///
/// @param  in_Buffer
///         [in] Handle for the buffer to map.
///
/// @param  in_Offset
///         [in] Offset into the buffer that a pointer should be returned
///         for.  The value 0 can be passed in to signify that the mapped
///         region should start at the beginning of the buffer.
///
/// @param  in_Length
///         [in] Length of the buffer area to map. This parameter, in
///         combination with in_Offset, allows the caller to specify
///         that only a subset of an entire buffer need be mapped.  A
///         value of 0 can be passed in to signify that the mapped
///         region should reach the end of the buffer.  Thus, for a
///         4096 byte buffer, if in_Offset is 2048, and in_Length is 0,
///         then the bytes in positions 2049-4095 will be mapped.
///
/// @param  in_Type
///         [in] The access type that is needed by the application. This will
///         affect how the data can be accessed once the map operation
///         completes. See the COI_MAP_TYPE enum for more details.
///
/// @param  in_NumDependencies
///         [in] The number of dependencies specified in the in_pDependencies
///         array. This may be 0 if the caller does not want the map
///         call initiation to wait for any barriers to be signaled before 
///         starting the map operations.
///
/// @param  in_pDependencies
///         [in] An optional array of handles to previously created COIBARRIER 
///         objects that this map operation will wait for before starting.
///         This allows the user to create dependencies between asynchronous
///         map calls and other operations such as run functions or other 
///         asynchronous map calls. The user may pass in NULL if they do not
///         wish to wait for any dependencies to complete before initiating map
///         operations.
///
/// @param  out_pCompletion
///         [out] An optional pointer to a COIBARRIER object 
///         that will be signaled when a map call with the passed in buffer
///         would complete immediately, that is, the buffer memory has been
///         allocated on the host and its contents updated.  The user may pass 
///         in NULL if the user wants COIBufferMap to perform a blocking map
///         operation.
///
/// @param  out_pMapInstance
///         [out] A pointer to a COIMAPINSTANCE which represents this mapping
///         of the buffer and must be passed in to COIBufferUnmap when access
///         to this region of the buffer data is no longer needed.
///
/// @param  out_ppData
///         [out] Pointer to the buffer data. The data will only be valid
///         when the completion object is signaled, or for a synchronous
///         map operation with the call to map returns.
///
///
/// @return COI_SUCCESS if the map request succeeds.
///
/// @return COI_OUT_OF_RANGE if in_Offset is beyond the end of the buffer.
///
/// @return COI_OUT_OF_RANGE if in_Offset + in_Length exceeds the size of
///         the buffer.
///
/// @return COI_ARGUMENT_MISMATCH if in_NumDependencies is non-zero while
///         in_pDependencies was passed in as NULL.
///
/// @return COI_ARGUMENT_MISMATCH if in_pDependencies is non-NULL but
///         in_NumDependencies is zero.
///
/// @return COI_INVALID_HANDLE if in_Buffer is not a valid buffer handle.
///
/// @return COI_INVALID_POINTER if out_pMapInstance or out_ppData is NULL.
///
COIRESULT 
COIBufferMap(
            COIBUFFER           in_Buffer,
            uint64_t            in_Offset,
            uint64_t            in_Length,
            COI_MAP_TYPE        in_Type,
            uint32_t            in_NumDependencies,
    const   COIBARRIER*         in_pDependencies,
            COIBARRIER*         out_pCompletion,
            COIMAPINSTANCE*     out_pMapInstance,
            void**              out_ppData);

//////////////////////////////////////////////////////////////////////////////
///
/// Disables Source access to the region of the buffer that was provided
/// through the corresponding call to COIBufferMap.  The number of calls to
/// COIBufferUnmap() should always match the number of calls made to
/// COIBufferMap().  The data pointer returned from the COIBufferMap() call
/// will be invalid after this call.
///
/// @param  in_MapInstance
///         [in] buffer map instance handle to unmap.
///
/// @param  in_NumDependencies
///         [in] The number of dependencies specified in the in_pDependencies
///         array. This may be 0 if the caller does not want the unmap call to
///         wait for any barriers to be signaled before performing the unmap
///         operation.
///
/// @param  in_pDependencies
///         [in] An optional array of handles to previously created COIBARRIER
///         objects that this unmap operation will wait for before starting.
///         This allows the user to create dependencies between asynchronous
///         unmap calls and other operations such as run functions or other
///         asynchronous unmap calls. The user may pass in NULL if they do not
///         wish to wait for any dependencies to complete before initiating
///         unmap operations.
///
/// @param  out_pCompletion
///         [out] An optional pointer to a COIBARRIER object that will be
///         signaled when the unmap is complete.  The user may pass in NULL if
///         the user wants COIBufferUnmap to perform a blocking unmap
///         operation.
///
/// @return COI_SUCCESS upon successful unmapping of the buffer instance.
///
/// @return COI_INVALID_HANDLE if the passed in map instance handle was NULL.
///
/// @return COI_ARGUMENT_MISMATCH if the in_pDependencies is non NULL but
///         in_NumDependencies is 0.
///
/// @return COI_ARGUMENT_MISMATCH if in_pDependencies is NULL but
///         in_NumDependencies is not 0.
///
COIRESULT 
COIBufferUnmap(
            COIMAPINSTANCE      in_MapInstance,
            uint32_t            in_NumDependencies,
    const   COIBARRIER*         in_pDependencies,
            COIBARRIER*         out_pCompletion);

//////////////////////////////////////////////////////////////////////////////
///
/// Gets the Sink's virtual address of the buffer.  This is the same
/// address that is passed to the run function on the Sink.  
/// This address is only valid on the Sink and should not be dereferenced on 
/// the Source (except for the special case of buffers created with the
/// COI_SAME_ADDRESS flag).
///
/// @param  in_Buffer
///         [in] Buffer handle
///
/// @param  out_pAddress
///         [out] pointer to a uint64_t* that will be filled with the address.
///
/// @return COI_SUCCESS upon successful return of the buffer's address.
///
/// @return COI_INVALID_HANDLE if the passed in buffer handle was invalid.
///
/// @return COI_INVALID_POINTER if the out_pAddress parameter was invalid.
///
COIRESULT 
COIBufferGetSinkAddress( 
            COIBUFFER           in_Buffer,
            uint64_t*           out_pAddress);

//////////////////////////////////////////////////////////////////////////////
///
/// Copy data from a normal virtual address into an existing COIBUFFER.
///
/// @param  in_DestBuffer
///         [in] Buffer to write into.
///
/// @param  in_Offset
///         [in] Location in the buffer to start writing to.  Currently must
///         be a page aligned value.  For now, all buffers start on a page
///         boundary.
///
/// @param  in_pSourceData
///         [in] A pointer to local memory that should be copied into the
///         provided buffer.
///
/// @param  in_Length
///         [in] The number of bytes to write from in_pSourceData into
///         in_DestBuffer. Must not be larger than the size of in_DestBuffer
///         and must not over run in_DestBuffer if an in_Offset is provided.
///         Currently must be a page aligned value.
///
/// @param  in_Type
///         [in] The type of copy operation to use, one of either 
///         COI_COPY_UNSPECIFIED, COI_COPY_USE_DMA, COI_COPY_USE_CPU.
///
/// @param  in_NumDependencies
///         [in] The number of dependencies specified in the in_pDependencies
///         array. This may be 0 if the caller does not want the write call to
///         wait for any additional barriers to be signaled before starting the
///         write operation.
///
/// @param  in_pDependencies
///         [in] An optional array of handles to previously created COIBARRIER
///         objects that this write operation will wait for before starting.
///         This allows the user to create dependencies between buffer write
///         calls and other operations such as run functions and map calls. The
///         user may pass in NULL if they do not wish to wait for any
///         additional dependencies to complete before doing the write.
/// 
/// @param  out_pCompletion
///         [out] An optional barrier to be signaled when the copy has
///         completed. This barrier can be used as a dependency to order
///         the copy with regard to future operations.
///         If no completion barrier is passed in then the copy is
///         synchronous and will block until the transfer is complete.
///
/// @return COI_SUCCESS if the buffer was copied successfully.
///
/// @return COI_INVALID_HANDLE if the buffer handle was invalid.
///
/// @return COI_OUT_OF_RANGE if in_Offset is beyond the end of the buffer.
///
/// @return COI_ARGUMENT_MISMATCH if the in_pDependencies is non NULL but
///         in_NumDependencies is 0.
///
/// @return COI_ARGUMENT_MISMATCH if in_pDependencies is NULL but
///         in_NumDependencies is not 0.
///
/// @return COI_OUT_OF_RANGE in_Offset or in_Length are not page aligned.
///
/// @return COI_INVALID_POINTER if the in_pSourceData pointer is NULL.
///
/// @return COI_OUT_OF_RANGE if in_Offset + in_Length exceeds the size of
///         the buffer.
///
/// @return COI_OUT_OF_RANGE if in_Length is 0.
///
/// @return COI_RETRY if in_DestBuffer is mapped and is not a COI_BUFFER_PINNED
///         buffer.
///
COIRESULT
COIBufferWrite(
            COIBUFFER           in_DestBuffer,
            uint64_t            in_Offset,
    const   void*               in_pSourceData,
            uint64_t            in_Length,
            COI_COPY_TYPE       in_Type,
            uint32_t            in_NumDependencies,
    const   COIBARRIER*         in_pDependencies,
            COIBARRIER*         out_pCompletion);

//////////////////////////////////////////////////////////////////////////////
///
/// Copy data from a buffer into local memory.
///
/// @param  in_SourceBuffer
///         [in] Buffer to write into.
///
/// @param  in_Offset
///         [in] Location in the buffer to start reading from.  Currently must
///         be a page aligned value.  For now, all buffers start on a page
///         boundary.
///
/// @param  in_pDestData
///         [in] A pointer to local memory that should be written into from 
///         the provided buffer.
///
/// @param  in_Length
///         [in] The number of bytes to write from in_SourceBuffer into
///         in_pDestData. Must not be larger than the size of in_SourceBuffer
///         and must not over run in_SourceBuffer if an in_Offset is provided.
///         Currently must be a page aligned value.
///
/// @param  in_Type
///         [in] The type of copy operation to use, one of either 
///         COI_COPY_UNSPECIFIED, COI_COPY_USE_DMA, COI_COPY_USE_CPU.
/// 
/// @param  in_NumDependencies
///         [in] The number of dependencies specified in the in_pDependencies
///         array. This may be 0 if the caller does not want the read call to
///         wait for any additional barriers to be signaled before starting the
///         read operation.
///
/// @param  in_pDependencies
///         [in] An optional array of handles to previously created COIBARRIER
///         objects that this read operation will wait for before starting.
///         This allows the user to create dependencies between buffer read
///         calls and other operations such as run functions and map calls. The
///         user may pass in NULL if they do not wish to wait for any
///         additional dependencies to complete before doing the read.
/// 
/// @param  out_pCompletion
///         [out] An optional barrier to be signaled when the copy has
///         completed. This barrier can be used as a dependency to order
///         the copy with regard to future operations.
///         If no completion barrier is passed in then the copy is
///         synchronous and will block until the transfer is complete.
///
/// @return COI_SUCCESS if the buffer was copied successfully.
///
/// @return COI_INVALID_HANDLE if the buffer handle was invalid.
///
/// @return COI_OUT_OF_RANGE if in_Offset is beyond the end of the buffer.
///
/// @return COI_ARGUMENT_MISMATCH if the in_pDependencies is non NULL but
///         in_NumDependencies is 0.
///
/// @return COI_ARGUMENT_MISMATCH if in_pDependencies is NULL but
///         in_NumDependencies is not 0.
///
/// @return COI_OUT_OF_RANGE if in_Offset + in_Length exceeds the size of
///         the buffer.
///
/// @return COI_OUT_OF_RANGE in_Offset or in_Length are not page aligned.
///
/// @return COI_OUT_OF_RANGE if in_Length is 0.
///
/// @return COI_INVALID_POINTER if the in_pDestData pointer is NULL.
///
/// @return COI_RETRY if in_SourceBuffer is mapped and is not a
///         COI_BUFFER_PINNED buffer.
///
COIRESULT
COIBufferRead(
            COIBUFFER           in_SourceBuffer,
            uint64_t            in_Offset,
            void*               in_pDestData,
            uint64_t            in_Length,
            COI_COPY_TYPE       in_Type,
            uint32_t            in_NumDependencies,
    const   COIBARRIER*         in_pDependencies,
            COIBARRIER*         out_pCompletion);

//////////////////////////////////////////////////////////////////////////////
///
/// Copy data between two different buffers.
///
/// @param  in_DestBuffer
///         [in] Buffer to copy into.
///
/// @param  in_SourceBuffer
///         [in] Buffer to copy from.
///
/// @param  in_DestOffset
///         [in] Location in the destination buffer to start writing to.
///         Currently must be a page aligned value.  For now, all buffers
///         start on a page
///
/// @param  in_SourceOffset
///         [in] Location in the source buffer to start reading from.
///         Currently must be a page aligned value.  For now, all buffers
///         start on a page
///
/// @param  in_Length
///         [in] The number of bytes to copy from in_SourceBuffer into
///         in_DestinationBuffer.
///         If the length is specified as zero then the entire buffer will
///         be copied.
///         Must not be larger than the size of in_SourceBuffer or 
///         in_DestBuffer and must not over run in_SourceBuffer or
///         in_DestBuffer if offsets are specified.
///         Currently must be a page aligned value.
///
/// @param  in_Type
///         [in] The type of copy operation to use, one of either 
///         COI_COPY_UNSPECIFIED, COI_COPY_USE_DMA, COI_COPY_USE_CPU.
/// 
/// @param  in_NumDependencies
///         [in] The number of dependencies specified in the in_pDependencies
///         array. This may be 0 if the caller does not want the copy call to
///         wait for any additional barriers to be signaled before starting the
///         copy operation.
///
/// @param  in_pDependencies
///         [in] An optional array of handles to previously created COIBARRIER
///         objects that this copy operation will wait for before starting.
///         This allows the user to create dependencies between buffer copy
///         calls and other operations such as run functions and map calls. The
///         user may pass in NULL if they do not wish to wait for any
///         additional dependencies to complete before doing the copy.
/// 
/// @param  out_pCompletion
///         [out] An optional barrier to be signaled when the copy has
///         completed. This barrier can be used as a dependency to order
///         the copy with regard to future operations.
///         If no completion barrier is passed in then the copy is
///         synchronous and will block until the transfer is complete.
///
/// @return COI_SUCCESS if the buffer was copied successfully.
///
/// @return COI_INVALID_HANDLE if either buffer handle was invalid.
///
/// @return COI_DUPLICATE_OBJECT if in_SourceBuffer and in_DestBuffer refer
///         to the same object.
///
/// @return COI_OUT_OF_RANGE if in_DestOffset is is beyond the end of
///         in_DestBuffer 
///
/// @return COI_OUT_OF_RANGE if in_SourceOffset is beyond the end of
///         in_SourceBuffer.
///
/// @return COI_OUT_OF_RANGE if in_DestOffset + in_Length exceeds the size of
///         the in_DestBuffer 
///
/// @return COI_OUT_OF_RANGE if in_SourceOffset + in_Length exceeds 
///         the size of in_SourceBuffer.
///
/// @return COI_OUT_OF_RANGE in_DestOffset, in_SourceOffset or in_Length
///         are not page aligned.
///
/// @return COI_ARGUMENT_MISMATCH if the in_pDependencies is non NULL but
///         in_NumDependencies is 0.
///
/// @return COI_ARGUMENT_MISMATCH if in_pDependencies is NULL but
///         in_NumDependencies is not 0.
///
/// @return COI_NOT_SUPPORTED if either buffer is of type 
///         COI_BUFFER_STREAMING_TO_SINK or COI_BUFFER_STREAMING_TO_SOURCE.
///
/// @return COI_RETRY if in_DestBuffer or in_SourceBuffer are mapped and not
///         COI_BUFFER_PINNED buffers.
///
COIRESULT
COIBufferCopy(
            COIBUFFER           in_DestBuffer,
            COIBUFFER           in_SourceBuffer,
            uint64_t            in_DestOffset,
            uint64_t            in_SourceOffset,
            uint64_t            in_Length,
            COI_COPY_TYPE       in_Type,
            uint32_t            in_NumDependencies,
    const   COIBARRIER*         in_pDependencies,
            COIBARRIER*         out_pCompletion);


//////////////////////////////////////////////////////////////////////////////
///
/// This API allows an experienced COI developer to set where a COIBUFFER is
/// located and when the COIBUFFER's data is moved. This functionality is
/// useful when the developer knows when and where a buffer is going to be
/// accessed. It allows the data movement to happen sooner than if the COI
/// runtime tried to manage the buffer placement itself. The advantage of
/// this API is that the developer knows much more about their own
/// application's data access patterns and can therefore optimize the data
/// access to be much more efficient than the COI runtime. Using this API
/// may yield better memory utilization, lower latency and overall improved
/// workload throughput.
/// This API does respect implicit dependencies for buffer read/write hazards.
/// For example, if the buffer is being written in one COIPROCESS and the user 
/// requests the buffer be placed in another COIPROCESS then this API will wait
/// for the first access to complete before moving the buffer.
/// This API is not required for program correctness. It is intended solely
/// for advanced COI developers who wish to fine tune their application
/// performance.
///
/// @param  in_Buffer
///         [in] The buffer to modify.
///
/// @param  in_Process
///         [in] The process where the state is being modified for this
///         buffer.
///
/// @param  in_State
///         [in] The new state for the buffer. The buffer's state could be
///         set to invalid on one of the sink processes where it is being
///         used.
///
/// @param  in_DataMove
///         [in] A flag to indicate if the buffer's data should be moved
///         when the state is changed. For instance, a buffer's state may
///         be set to valid on a process and the data move flag may be set to
///         COI_BUFFER_MOVE_DATA which would cause the buffer contents to be
///         copied to the process where it is now valid.
///
/// @return COI_SUCCESS if the buffer's state was changed successfully.
///
/// @return COI_INVALID_HANDLE if in_Buffer or in_Process is invalid.
///
COIRESULT 
COIBufferSetState( 
            COIBUFFER               in_Buffer,
            COIPROCESS              in_Process,
            COI_BUFFER_STATE        in_State,
            COI_BUFFER_MOVE_FLAG    in_DataMove);

#ifdef __cplusplus
} // extern "C"
#endif 
#endif // COIBUFFER_SOURCE_H

/*! @} */
