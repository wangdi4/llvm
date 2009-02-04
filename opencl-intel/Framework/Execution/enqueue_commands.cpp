// Copyright (c) 2008-2009 Intel Corporation
// All rights reserved.
// 
// WARRANTY DISCLAIMER
// 
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

///////////////////////////////////////////////////////////
//  enqueue_commands.cpp
//  Implementation of the Class ReadBufferCommand
//  Created on:      16-Dec-2008 10:11:31 AM
//  Original author: Peleg, Arnon
///////////////////////////////////////////////////////////
#include "enqueue_commands.h"
#include "queue_event.h"
//For debug
#include <stdio.h>
#include <windows.h>
#include <process.h>
using namespace Intel::OpenCL::Framework;

/******************************************************************
 *
 ******************************************************************/
Command::Command():
    m_pQueueEvent(NULL),
    m_pReceiver(NULL)
{
}

/******************************************************************
 *
 ******************************************************************/
Command::~Command()
{
    // The command delets its event
    if ( NULL != m_pQueueEvent) delete m_pQueueEvent;
    m_pQueueEvent =  NULL;
    m_pReceiver =    NULL;
}

/******************************************************************
 *
 ******************************************************************/
CopyBufferToImageCommand::CopyBufferToImageCommand(
    MemoryObject*   srcBuffer, 
    MemoryObject*   dstImage, 
    size_t          srcOffset, 
    const size_t*   dstOrigin[3], 
    const size_t*   region[3]
    )
{
}

/******************************************************************
 *
 ******************************************************************/
CopyBufferToImageCommand::~CopyBufferToImageCommand()
{
}

/******************************************************************
 *
 ******************************************************************/
CopyBufferCommand::CopyBufferCommand(MemoryObject* srcBuffer, MemoryObject* dstBuffer, size_t srcOffset, size_t dstOffset, size_t cb)
{
}


/******************************************************************
 *
 ******************************************************************/
CopyBufferCommand::~CopyBufferCommand(){

}

/******************************************************************
 *
 ******************************************************************/
CopyImageCommand::CopyImageCommand(MemoryObject* image, const size_t* origin[3], const size_t* region[3], size_t row_pitch, size_t slice_pitch, void* dst){

}

/******************************************************************
 *
 ******************************************************************/
CopyImageCommand::~CopyImageCommand(){

}

/******************************************************************
 *
 ******************************************************************/
CopyImageToBufferCommand::CopyImageToBufferCommand(MemoryObject* srcImage, MemoryObject* dstBuffer, const size_t* srcOrigin[3], const size_t* region[3], size_t dstOffset){

}

/******************************************************************
 *
 ******************************************************************/
CopyImageToBufferCommand::~CopyImageToBufferCommand(){

}

/******************************************************************
 *
 ******************************************************************/
MapBufferCommand::MapBufferCommand(MemoryObject* buffer, cl_map_flags map_flags, size_t offset, size_t cb){

}

/******************************************************************
 *
 ******************************************************************/
MapBufferCommand::~MapBufferCommand(){

}

/******************************************************************
 *
 ******************************************************************/
MapImageCommand::MapImageCommand(MemoryObject* image, cl_map_flags map_flags, const size_t* origion[3], const size_t* region[3], size_t* image_row_pitch, size_t* image_slice_pitch){

}

/******************************************************************
 *
 ******************************************************************/
MapImageCommand::~MapImageCommand(){

}

/******************************************************************
 *
 ******************************************************************/
MarkerCommand::MarkerCommand(){

}

/******************************************************************
 *
 ******************************************************************/
MarkerCommand::~MarkerCommand(){

}

/******************************************************************
 *
 ******************************************************************/
BarrierCommand::BarrierCommand(){

}

/******************************************************************
 *
 ******************************************************************/
BarrierCommand::~BarrierCommand(){

}

/******************************************************************
 *
 ******************************************************************/
NativeKernelCommand::NativeKernelCommand(void* usrfunc, void* args, size_t cbArgs, cl_uint numMemObjects, const MemoryObject* memObjList, const void** args_mem_loc){

}

/******************************************************************
 *
 ******************************************************************/
NativeKernelCommand::~NativeKernelCommand(){

}

/******************************************************************
 *
 ******************************************************************/
NDRangeKernelCommand::NDRangeKernelCommand(Kernel* kernel, cl_uint work_dim, const size_t* global_work_offset, const size_t* global_work_size, const size_t* local_work_size){

}

/******************************************************************
 *
 ******************************************************************/
NDRangeKernelCommand::~NDRangeKernelCommand(){

}

/******************************************************************
 *
 ******************************************************************/
ReadBufferCommand::ReadBufferCommand(MemoryObject* buffer, size_t offset, size_t cb, void* dst){

}

/******************************************************************
 *
 ******************************************************************/
ReadBufferCommand::~ReadBufferCommand(){

}

/******************************************************************
 *
 ******************************************************************/
ReadImageCommand::ReadImageCommand(MemoryObject* image, const size_t* origin[3], const size_t* region[3], size_t row_pitch, size_t slice_pitch, void* dst){

}

/******************************************************************
 *
 ******************************************************************/
ReadImageCommand::~ReadImageCommand(){

}

/******************************************************************
 *
 ******************************************************************/
TaskCommand::TaskCommand(Kernel* kernel){

}

/******************************************************************
 *
 ******************************************************************/
TaskCommand::~TaskCommand(){

}

/******************************************************************
 *
 ******************************************************************/
UnmapMemObjectCommand::UnmapMemObjectCommand(MemoryObject* memObject, void* mapped_ptr){

}

/******************************************************************
 *
 ******************************************************************/
UnmapMemObjectCommand::~UnmapMemObjectCommand(){

}

/******************************************************************
 *
 ******************************************************************/
WaitForEventsCommand::WaitForEventsCommand(){

}


/******************************************************************
 *
 ******************************************************************/
WaitForEventsCommand::~WaitForEventsCommand(){

}

/******************************************************************
 *
 ******************************************************************/
WriteBufferCommand::WriteBufferCommand(MemoryObject* buffer, size_t offset, size_t cb, const void* src){

}


/******************************************************************
 *
 ******************************************************************/
WriteBufferCommand::~WriteBufferCommand(){

}

/******************************************************************
 *
 ******************************************************************/
WriteImageCommand::WriteImageCommand(MemoryObject* image, const size_t* origin[3], const size_t* region[3], size_t row_pitch, size_t slice_pitch, const void* dst){

}

/******************************************************************
 *
 ******************************************************************/
WriteImageCommand::~WriteImageCommand(){

}


/******************************************************************
 *
 ******************************************************************/
unsigned int __stdcall DummyCommandThreadEntryPoint(void* threadObject)
{
    Sleep(300);
    Command* pCommand = (Command*)threadObject;
    pCommand->GetEvent()->SetEventColor(QueueEvent::EVENT_STATE_BLACK);
    return 1;
}


cl_err_code DummyCommand::Execute()
{
    // Start execution thread;
    _beginthreadex(NULL, 0, DummyCommandThreadEntryPoint, this, 0, NULL);
    return CL_SUCCESS;
}
