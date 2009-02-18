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

#pragma once
///////////////////////////////////////////////////////////
//  enqueue_commands.h
//  Implementation of the Class ReadBufferCommand
//  Created on:      16-Dec-2008 10:11:31 AM
//  Original author: Peleg, Arnon
///////////////////////////////////////////////////////////

#if !defined(__OCL_ENQUEUE_COMMANDS__)
#define __OCL_ENQUEUE_COMMANDS__
    
#include <cl_types.h>
#include <cl_device_api.h>
#include "cl_object.h"
#include "observer.h"
#include <list>

namespace Intel { namespace OpenCL { namespace Framework {

    // Forward declration
    class QueueEvent;
    class ICommandReceiver;
    class MemoryObject;
    class Kernel;

    /******************************************************************
     *
     ******************************************************************/
    class Command : public OCLObject, public ICmdStatusChangedObserver
    {

    public:
	    Command();
	    virtual ~Command();

        //
        // Use this function to initiate the command local data set by a command constructor
        //
        virtual cl_err_code     Init() = 0;

        //
        // This function is called when CL_COMPLETED is notified. 
        // The command use this function to update its data respectively such as the buffers.
        //
        virtual cl_err_code     CommandDone() = 0;

        //
        // The function that is called when the command is poped out from the queue and ready for the device
        // Each command implements its local logic within this function.
        //
	    virtual cl_err_code     Execute() = 0;	

        //
        // Returns the command type for GetInfo requests.
        //
        virtual cl_command_type GetCommandType() const = 0;

        // ICmdStatusChangedObserver function
        cl_err_code NotifyCmdStatusChanged(cl_dev_cmd_id clCmdId, cl_int iCmdStatus, cl_int iCompletionResult);

        // Command general functions
        virtual void            SetEvent    (QueueEvent* queueEvent)        { m_pQueueEvent = queueEvent; }
        virtual QueueEvent*     GetEvent    ()                              { return m_pQueueEvent; }   
        virtual void            SetReceiver (ICommandReceiver* receiver)    { m_pReceiver = receiver; }
        virtual void            SetCommandDeviceId (cl_device_id clDeviceId){ m_clDeviceId = clDeviceId; }

    protected:
	    QueueEvent*                 m_pQueueEvent;
	    ICommandReceiver*           m_pReceiver;
        cl_dev_cmd_desc*            m_pDevCmd;
        cl_device_id                m_clDeviceId;
        ICmdStatusChangedObserver*  m_pStatusChangeObserver;
    };

    /******************************************************************
     *
     ******************************************************************/
    class ReadBufferCommand : public Command
    {
    public:
	    ReadBufferCommand(
		    MemoryObject* 	pBuffer, 
		    size_t 			szOffset, 
		    size_t			szCb, 
		    void* 			pDst
		    );
        virtual ~ReadBufferCommand();

        cl_err_code     Init();
	    cl_err_code     Execute();	
        cl_err_code     CommandDone();
        cl_command_type GetCommandType() const { return CL_COMMAND_READ_BUFFER; };

    private:
        MemoryObject*   m_pBuffer;
	    size_t          m_szOffset;
	    size_t          m_szCb;
        void*           m_pDst;
    };



    /******************************************************************
     *
     ******************************************************************/
    class WriteBufferCommand : public Command
    {

    public:
	    WriteBufferCommand(
            MemoryObject*   pBuffer, 
            size_t          szOffset, 
            size_t          szCb,
            const void*     cpSrc
            );

	    virtual ~WriteBufferCommand();

        cl_err_code     Init();
	    cl_err_code     Execute();	
        cl_err_code     CommandDone();
        cl_command_type GetCommandType() const { return CL_COMMAND_WRITE_BUFFER; };

    private:
        MemoryObject*   m_pBuffer;
	    size_t          m_szOffset;
	    size_t          m_szCb;
        const void*     m_cpSrc;
    };

    /******************************************************************
     *
     ******************************************************************/
    class ReadImageCommand : public Command
    {

    public:
	    ReadImageCommand(
            MemoryObject*   image, 
            const size_t*   origin[3], 
            const size_t*   region[3], 
            size_t          row_pitch, 
            size_t          slice_pitch, 
            void*           dst
            );
	    virtual ~ReadImageCommand();

    private:
            MemoryObject*   m_image;
            const size_t*   m_origin[3]; 
            const size_t*   m_region[3]; 
            size_t          m_rowPitch;
            size_t          m_slicePitch; 
            void*           m_dst;
    };

    /******************************************************************
     *
     ******************************************************************/
    class WriteImageCommand : public Command
    {

    public:
	    WriteImageCommand(
            MemoryObject*   image, 
            const size_t*   origin[3], 
            const size_t*   region[3], 
            size_t          row_pitch, 
            size_t          slice_pitch, 
            const void*     dst       
            );
	    virtual ~WriteImageCommand();

        private:
            MemoryObject*   m_image;
            const size_t*   m_origin[3]; 
            const size_t*   m_region[3]; 
            size_t          m_rowPitch;
            size_t          m_slicePitch; 
            const void*     m_src;
    };

    /******************************************************************
     *
     ******************************************************************/
    class CopyBufferCommand : public Command
    {

    public:
		    CopyBufferCommand(
		    MemoryObject* 	srcBuffer, 
		    MemoryObject* 	dstBuffer, 
		    size_t 			srcOffset, 
		    size_t 			dstOffset,
		    size_t 			cb
		    );		
	    virtual ~CopyBufferCommand();

    private:
        MemoryObject* 	m_srcBuffer;
        MemoryObject* 	m_dstBuffer;
	    size_t 			m_Offset;
	    size_t 			m_srcOffset;
	    size_t 			m_cb;
    };

    /******************************************************************
     *
     ******************************************************************/
    class CopyImageCommand : public Command
    {

    public:
	    CopyImageCommand(
	 	    MemoryObject*   srcImage,
            MemoryObject*   dstimage, 
            const size_t*   srcOrigin[3], 
            const size_t*   dstOrigion[3],
            const size_t*   region[3]
	    );
	    virtual ~CopyImageCommand();
	    CopyImageCommand(MemoryObject* image, const size_t* origin[3], const size_t* region[3], size_t row_pitch, size_t slice_pitch, void* dst);

    private: 
        MemoryObject*   m_srcImage;
        MemoryObject*   m_dstImage;
        const size_t*   m_srcOrigin[3];
        const size_t*   m_dstOrigion[3];
        const size_t*   m_region[3];
	    MemoryObject* m_image;
	    const size_t* m_origin[3];
	    size_t m_rowPitch;
	    size_t m_slicePitch;
	    void* m_dst;   
    };

    /******************************************************************
     *
     ******************************************************************/
    class CopyImageToBufferCommand : public Command
    {
    public:
	    CopyImageToBufferCommand(
            MemoryObject*   srcImage, 
            MemoryObject*   dstBuffer, 
            const size_t*   srcOrigin[3], 
            const size_t*   region[3],
            size_t          dstOffset
        );
	    virtual ~CopyImageToBufferCommand();

    private: 
        MemoryObject*   m_srcImage;
        MemoryObject*   m_dstBuffer;
        const size_t*   m_srcOrigin[3];
        const size_t*   m_region[3];
        size_t          m_dstOffset;
	    MemoryObject *m_dsrBuffer; 

    };

    /******************************************************************
     *
     ******************************************************************/
    class CopyBufferToImageCommand : public Command
    {

    public:
	    CopyBufferToImageCommand(
            MemoryObject*   srcBuffer, 
            MemoryObject*   dstImage, 
            size_t          srcOffset, 
            const size_t*   dstOrigin[3], 
            const size_t*   region[3]
        );
	    virtual ~CopyBufferToImageCommand();

    private: 
        MemoryObject*   m_srcBuffer;
        MemoryObject*   m_dstImage;
        size_t          m_srcOffset; 
        const size_t*   m_dstOrigin[3];
        const size_t*   m_region[3];



    };

    /******************************************************************
     *
     ******************************************************************/
    class MapBufferCommand : public Command
    {

    public:
	    MapBufferCommand(MemoryObject* buffer, cl_map_flags map_flags, size_t offset, size_t cb);
	    virtual ~MapBufferCommand();

    private: 
        MemoryObject*   m_buffer;
	    cl_map_flags m_mapFlags;
        size_t          m_offset; 
        size_t		   	m_cb;
    };

    /******************************************************************
     *
     ******************************************************************/
    class MapImageCommand : public Command
    {
    public:
	    MapImageCommand(MemoryObject* image, cl_map_flags map_flags, const size_t* origion[3], const size_t* region[3], size_t* image_row_pitch, size_t* image_slice_pitch);
	    virtual ~MapImageCommand();
    private: 
	    MemoryObject*   m_image;
        cl_map_flags m_map_flags;
        const size_t*   m_origion[3];
        const size_t*   m_region[3]; 	
        size_t* 		m_image_row_pitch;
        size_t* 		m_image_slice_pitch;
    	
    };

    /******************************************************************
     *
     ******************************************************************/
    class UnmapMemObjectCommand : public Command
    {

    public:
	    UnmapMemObjectCommand(
		    MemoryObject*   memObject,
    	    void* 			mapped_ptr
		    );
	    virtual ~UnmapMemObjectCommand();

    private: 
	    MemoryObject*   m_memObject;
        void*			m_mappedPtr;
    };

    /******************************************************************
     *
     ******************************************************************/
    class NDRangeKernelCommand : public Command
    {
    public:
	    NDRangeKernelCommand(Kernel* kernel, cl_uint work_dim, const size_t* global_work_offset, const size_t* global_work_size, const size_t* local_work_size);
	    virtual ~NDRangeKernelCommand();

        cl_err_code     Init();
	    cl_err_code     Execute();	
        cl_err_code     CommandDone();
        cl_command_type GetCommandType() const { return CL_COMMAND_NDRANGE_KERNEL; };

    private: 
        // This enum includes the type of CL objects that can be pass
        // as arguments to the device
        enum EObjectArgType
        {
            KL_ARG_TYPE_BUFFER,
            KL_ARG_TYPE_IMAGE_2D,
            KL_ARG_TYPE_IMAGE_3D,
            KL_ARG_TYPE_SAMPLER
        };
        
        // Kerel object args stracture
        struct TObjectArg
        {
            EObjectArgType  type;       // The type of the object
            size_t          szOffset;   // The offset of the argument in the device argument bytestream
            size_t          szSize;     // The argument size in the device argument bytestream
            void*           pObject;    // void pointer to the actual object
        };

        // Private memebers
	    Kernel* 		m_pKernel;
	    cl_uint         m_uiWorkDim;
	    const size_t*	m_cpszGlobalWorkOffset;
	    const size_t*	m_cpszGlobalWorkSize;
	    const size_t*	m_cpszLocalWorkSize;        
        // Intermidate data
        std::list<TObjectArg>  m_ObjectArgList;
    };

    /******************************************************************
     *
     ******************************************************************/
    class TaskCommand : public Command
    {

    public:
	    TaskCommand(
		    Kernel* kernel
		    );
	    virtual ~TaskCommand();
    private:
	    Kernel* m_kernel;
    };

    /******************************************************************
     *
     ******************************************************************/
    class NativeKernelCommand : public Command
    {

    public:
	    NativeKernelCommand(void* usrfunc, void* args, size_t cbArgs, cl_uint numMemObjects, const MemoryObject* memObjList, const void** args_mem_loc);
	    virtual ~NativeKernelCommand();

    private:
	    void* 				m_usrfunc;
	    void*				m_args;
	    size_t 				m_cbArgs;
	    cl_uint m_numMemObjects;
	    const MemoryObject*	m_memObjList;
	    const void**		m_argsMemLoc;
    };

    /******************************************************************
     *
     ******************************************************************/
    class MarkerCommand : public Command
    {

    public:
	    MarkerCommand();
	    virtual ~MarkerCommand();

        cl_err_code     Init()                  { return CL_SUCCESS; }
	    cl_err_code     Execute();	
        cl_err_code     CommandDone()           { return CL_SUCCESS; } //Nothing to do, command is done on execution
        cl_command_type GetCommandType() const  { return CL_COMMAND_MARKER; }
    };

    /******************************************************************
     *
     ******************************************************************/
    class WaitForEventsCommand : public Command
    {

    public:
	    WaitForEventsCommand();
	    virtual ~WaitForEventsCommand();

        cl_err_code     Init()                  { return CL_SUCCESS; }
	    cl_err_code     Execute();	
        cl_err_code     CommandDone()           { return CL_SUCCESS; } //Nothing to do, command is done on execution
        cl_command_type GetCommandType() const  { return CL_COMMAND_WAIT_FOR_EVENTS; }
    };

    /******************************************************************
     *
     ******************************************************************/
    class BarrierCommand : public Command
    {

    public:
	    BarrierCommand();
	    virtual ~BarrierCommand();

        cl_err_code     Init()                  { return CL_SUCCESS; }
	    cl_err_code     Execute();	
        cl_err_code     CommandDone()           { return CL_SUCCESS; } //Nothing to do, command is done on execution
        cl_command_type GetCommandType() const  { return CL_COMMAND_BARRIER; }
    };



    /******************************************************************
     * DummyCommand - used for debugging
     * On execution the command prints its status, than change its status
     * to done... ??? will it work???

     ******************************************************************/
    class DummyCommand : public Command
    {

    public:
        DummyCommand()  {};
        virtual ~DummyCommand() {};

	    cl_err_code     Execute();	
        cl_command_type GetCommandType() const { return 0; };
    };




}}};    // Intel::OpenCL::Framework
#endif  // !defined(__OCL_ENQUEUE_COMMANDS__)