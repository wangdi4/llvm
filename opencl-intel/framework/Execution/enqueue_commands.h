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
//  enqueue_commands.h
//  Implementation of all commands classes
//  Created on:      16-Dec-2008 10:11:31 AM
//  Original author: Peleg, Arnon
///////////////////////////////////////////////////////////

#if !defined(__OCL_ENQUEUE_COMMANDS__)
#define __OCL_ENQUEUE_COMMANDS__
    
#include <cl_types.h>
#include <cl_device_api.h>
#include <logger.h>
#include "cl_object.h"
#include "observer.h"
#include "Device.h"
#include <list>

namespace Intel { namespace OpenCL { namespace Framework {

	#define CL_COMMAND_RUNTIME			0
	#define CL_COMMAND_BARRIER			1
	#define CL_COMMAND_WAIT_FOR_EVENTS	2
    // Forward declarations
    class OclEvent;
    class MemoryObject;
    class Kernel;
    class IOclCommandQueueBase;

    /******************************************************************
     * This enumeration is used to identify if a command is going to be
     * executed on a device, or whether it is used by runtime only, mainly
     * for synch objects such as Barriers and Markers.
     ******************************************************************/
    enum ECommandExecutionType
    {
        RUNTIME_EXECUTION_TYPE,     // Command is executed in the runtime only
        DEVICE_EXECUTION_TYPE       // The command is expected to be executed on the device
    };

    /******************************************************************
     *
     ******************************************************************/
    class Command : public OCLObject<_cl_event>, public ICmdStatusChangedObserver
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
        // Returns the command type for GetInfo requests and execution needs
        //
        virtual cl_command_type GetCommandType() const = 0;
		//
		// Returns True if command is: Marker || Barrier || WaitForEvents
		//
		virtual bool isControlCommand() const  { return false; }
        //
        // Returns whether a command is going to be executed on the device or not.
        //
        virtual ECommandExecutionType GetExecutionType() const = 0;

		//
		// Makes sure pSrcMemObj memory object is valid on the device where 
		// the command belongs.
		//
		cl_err_code		PrepareOnDevice(
							MemoryObject* pSrcMemObj, 						
							const size_t* pSrcOrigin, 						
							const size_t* pRegion,							
							OclEvent**	pEvent);

        // ICmdStatusChangedObserver function
        cl_err_code NotifyCmdStatusChanged(cl_dev_cmd_id clCmdId, cl_int iCmdStatus, cl_int iCompletionResult, cl_ulong ulTimer);

        // Command general functions
        void            SetEvent    (OclEvent* evt);
        OclEvent*       GetEvent    ()                                      { return m_pEvent; }   
        void            SetDevCmdListId    (cl_dev_cmd_list clDevCmdListId) { m_clDevCmdListId = clDevCmdListId; }
        cl_dev_cmd_list GetDevCmdListId    () const                         { return m_clDevCmdListId; }
		void            SetDevice(Device* pDevice)                          { m_pDevice = pDevice; }
		Device*         GetDevice() const                                   { return m_pDevice; }
		void            SetCommandQueue(IOclCommandQueueBase* pQueue)       { m_pCommandQueue = pQueue; }

        // Debug functions
        virtual const char*     GetCommandName() const                              { return "UNKNOWN"; }

    protected:
		cl_err_code CopyFromHost(
						void* pSrcData,
						MemoryObject* pSrcMemObj, 
						const size_t* pSrcOrigin, 						
						const size_t* pRegion,					
						OclEvent**	pEvent);

		cl_err_code CopyToHost(
						MemoryObject*	pSrcMemObj, 												
						OclEvent**		pEvent);

        OclEvent*                   m_pEvent;                   // Pointer to the related event object
        cl_dev_cmd_desc             m_DevCmd;                   // Device command descriptor struct
        cl_dev_cmd_list             m_clDevCmdListId;           // An handle of the device command list that this command should be queued on
		Device*                     m_pDevice;                  // A pointer to the device executing the command
		IOclCommandQueueBase*       m_pCommandQueue;            // A pointer to the command queue on which the command resides

    public:
        bool						m_bIsFlushed;				// Required only for Debug, TODO: Remove on stabilty
    };

    /******************************************************************
     *
     ******************************************************************/
	class ReadMemObjCommand : public Command
    {
    public:
        ReadMemObjCommand(
            MemoryObject*   pMemObj,
			const size_t*   pszOrigin,
			const size_t*   pszRegion,
			size_t          szRowPitch, 
			size_t          szSlicePitch, 
			void*           pDst);

        virtual ~ReadMemObjCommand();

		virtual cl_command_type         GetCommandType() const  { return CL_COMMAND_READ_MEM_OBJECT; }
        virtual ECommandExecutionType   GetExecutionType() const{ return DEVICE_EXECUTION_TYPE;   }
		virtual const char*             GetCommandName() const  { return "CL_COMMAND_READ_MEM_OBJECT"; }

        virtual cl_err_code             Init();
        virtual cl_err_code             Execute();    
        virtual cl_err_code             CommandDone();
        
    private:
		MemoryObject*   m_pMemObj;
        size_t          m_szOrigin[3]; 
        size_t          m_szRegion[3];
        size_t          m_szRowPitch;
        size_t          m_szSlicePitch; 
        void*           m_pDst;        
    };

    class ReadBufferCommand : public ReadMemObjCommand
    {
    public:
        ReadBufferCommand(
            MemoryObject*     pBuffer, 
            size_t            szOffset,
            size_t            szCb, 
            void*             pDst
            );
        virtual ~ReadBufferCommand();
        
        cl_command_type         GetCommandType() const  { return CL_COMMAND_READ_BUFFER; }
        ECommandExecutionType   GetExecutionType() const{ return DEVICE_EXECUTION_TYPE;  }
        const char*             GetCommandName() const  { return "CL_COMMAND_READ_BUFFER"; }
    };


    /******************************************************************
     *
     ******************************************************************/
    class ReadImageCommand : public ReadMemObjCommand
    {

    public:
        ReadImageCommand(
            MemoryObject*   pImage,
            const size_t*   pszOrigin,
            const size_t*   pszRegion,
            size_t          szRowPitch, 
            size_t          szSlicePitch, 
            void*           pDst
            );
        virtual ~ReadImageCommand();
     
        cl_command_type         GetCommandType() const  { return CL_COMMAND_READ_IMAGE; }
        ECommandExecutionType   GetExecutionType() const{ return DEVICE_EXECUTION_TYPE; }
        const char*             GetCommandName() const  { return "CL_COMMAND_READ_IMAGE"; }
    };

	/******************************************************************
    *
    ******************************************************************/
	class WriteMemObjCommand : public Command
    {

    public:
        WriteMemObjCommand(
            MemoryObject*   pImage, 
            const size_t*   pszOrigin,
            const size_t*   pszRegion,
            size_t          szRowPitch,
            size_t          szSlicePitch,
            const void *    cpSrc			
            );

        virtual ~WriteMemObjCommand();

		virtual cl_command_type         GetCommandType() const  { return CL_COMMAND_WRITE_MEM_OBJECT; }
        virtual ECommandExecutionType   GetExecutionType() const{ return DEVICE_EXECUTION_TYPE;   }
		virtual const char*             GetCommandName() const  { return "CL_COMMAND_WRITE_MEM_OBJECT"; }
		

        virtual cl_err_code   Init();
        virtual cl_err_code   Execute();    
        virtual cl_err_code   CommandDone();        

    private:
        MemoryObject*   m_pMemObj;
        size_t          m_szOrigin[3];
        size_t          m_szRegion[3]; 
        size_t          m_szRowPitch;
        size_t          m_szSlicePitch; 
        const void*     m_cpSrc;		
    };

    /******************************************************************
     *
     ******************************************************************/
    class WriteBufferCommand : public WriteMemObjCommand
    {

    public:
        WriteBufferCommand(
            MemoryObject*   pBuffer, 
            size_t          szOffset, 
            size_t          szCb,
            const void*     cpSrc
            );

        virtual ~WriteBufferCommand();


        cl_command_type         GetCommandType() const  { return CL_COMMAND_WRITE_BUFFER; }
        ECommandExecutionType   GetExecutionType() const{ return DEVICE_EXECUTION_TYPE;   }
        const char*             GetCommandName() const  { return "CL_COMMAND_WRITE_BUFFER"; }
    };

    /******************************************************************
     *
     ******************************************************************/
    class WriteImageCommand : public WriteMemObjCommand
    {

    public:
        WriteImageCommand(
            MemoryObject*   pImage, 
            const size_t*   pszOrigin,
            const size_t*   pszRegion,
            size_t          szRowPitch,
            size_t          szSlicePitch,
            const void *    cpSrc
            );

        virtual ~WriteImageCommand();

        cl_command_type         GetCommandType() const  { return CL_COMMAND_WRITE_IMAGE; }
        ECommandExecutionType   GetExecutionType() const{ return DEVICE_EXECUTION_TYPE; }
        const char*             GetCommandName() const  { return "CL_COMMAND_WRITE_IMAGE"; }


    private:
        MemoryObject*   m_pImage;
        size_t          m_szOrigin[3];
        size_t          m_szRegion[3]; 
        size_t          m_szRowPitch;
        size_t          m_szSlicePitch; 
        const void*     m_cpSrc;
	};
	
    /******************************************************************
     * This is an abstrct class that is used for all copy memory object commands
     ******************************************************************/
    class CopyMemObjCommand : public Command
    {

    public:
        CopyMemObjCommand(
            MemoryObject*   pSrcMemObj, 
            MemoryObject*   pDstMemObj, 
            const size_t*   szSrcOrigin, 
            const size_t*   szDstOrigin,
            const size_t*   szRegion
            );        
        virtual ~CopyMemObjCommand();

        virtual cl_err_code Init();
        virtual cl_err_code Execute();    
        virtual cl_err_code CommandDone();

    protected:
        MemoryObject*   m_pSrcMemObj;
        MemoryObject*   m_pDstMemObj;
        size_t          m_szSrcOrigin[MAX_WORK_DIM];
        size_t          m_szDstOrigin[MAX_WORK_DIM];
        size_t          m_szRegion[MAX_WORK_DIM];
        cl_uint         m_uiSrcNumDims;     // The dimensions represent the memory object type, 1,2,3 
                                            // respectively are BUFFER/2D/3D. The private member is used only for ease of use.
        cl_uint         m_uiDstNumDims;

        // Private functions        
        cl_err_code CopyOnDevice    (cl_device_id clDeviceId);        
    };

    /******************************************************************
     *
     ******************************************************************/
    class CopyBufferCommand : public CopyMemObjCommand
    {

    public:
        CopyBufferCommand(
            MemoryObject*   pSrcBuffer, 
            MemoryObject*   pDstBuffer, 
            size_t          szSrcOffset, 
            size_t          szDstOffset,
            size_t          szCb
            );        
        virtual ~CopyBufferCommand();

        cl_command_type         GetCommandType() const  { return CL_COMMAND_COPY_BUFFER; }
        ECommandExecutionType   GetExecutionType() const{ return DEVICE_EXECUTION_TYPE;  }
        const char*             GetCommandName() const  { return "CL_COMMAND_COPY_BUFFER"; }

    };

    /******************************************************************
     *
     ******************************************************************/
    class CopyImageCommand : public CopyMemObjCommand
    {

    public:
        CopyImageCommand(
            MemoryObject*   pSrcImage,
            MemoryObject*   pDstImage,
            const size_t*   pszSrcOrigin,
            const size_t*   pszDstOrigin,
            const size_t*   pszRegion
        );
        virtual ~CopyImageCommand();

        cl_command_type         GetCommandType() const  { return CL_COMMAND_COPY_IMAGE; }
        ECommandExecutionType   GetExecutionType() const{ return DEVICE_EXECUTION_TYPE;  }
        const char*             GetCommandName() const  { return "CL_COMMAND_COPY_IMAGE"; }
    };

    /******************************************************************
     *
     ******************************************************************/
    class CopyImageToBufferCommand : public CopyMemObjCommand
    {
    public:
        CopyImageToBufferCommand(
            MemoryObject*   pSrcImage, 
            MemoryObject*   pDstBuffer, 
            const size_t*   pszSrcOrigin, 
            const size_t*   pszSrcRegion,
            size_t          szDstOffset
        );
        virtual ~CopyImageToBufferCommand();

        cl_command_type         GetCommandType() const  { return CL_COMMAND_COPY_IMAGE_TO_BUFFER; }
        ECommandExecutionType   GetExecutionType() const{ return DEVICE_EXECUTION_TYPE;  }
        const char*             GetCommandName() const  { return "CL_COMMAND_COPY_IMAGE_TO_BUFFER"; }
    };

    /******************************************************************
     *
     ******************************************************************/
    class CopyBufferToImageCommand : public CopyMemObjCommand
    {

    public:
        CopyBufferToImageCommand(
            MemoryObject*   pSrcBuffer, 
            MemoryObject*   pDstImage, 
            size_t          szSrcOffset, 
            const size_t*   pszDstOrigin, 
            const size_t*   pszDstRegion
        );

        virtual ~CopyBufferToImageCommand();

        cl_command_type         GetCommandType() const  { return CL_COMMAND_COPY_BUFFER_TO_IMAGE; }
        ECommandExecutionType   GetExecutionType() const{ return DEVICE_EXECUTION_TYPE;  }
        const char*             GetCommandName() const  { return "CL_COMMAND_COPY_BUFFER_TO_IMAGE"; }

    };


    /******************************************************************
     *
     ******************************************************************/
    class MapMemObjCommand : public Command
    {
    public:
        MapMemObjCommand(
            MemoryObject*   pMemObj,
            cl_map_flags    clMapFlags, 
            const size_t*   pOrigin, 
            const size_t*   pRegion,
            size_t*         pszImageRowPitch,
            size_t*         pszImageSlicePitch
            );
        virtual ~MapMemObjCommand();

        virtual cl_err_code Init();
        virtual cl_err_code Execute();    
        virtual cl_err_code CommandDone();

        // Object only function
        void*           GetMappedRegion()       { return m_pMappedRegion; }

    protected: 
        MemoryObject*   m_pMemObj;
        cl_map_flags    m_clMapFlags;
        const size_t*   m_pOrigin;
        const size_t*   m_pRegion;
        size_t*         m_pszImageRowPitch;
        size_t*         m_pszImageSlicePitch;
        void*           m_pMappedRegion;    
    };

    /******************************************************************
     *
     ******************************************************************/
    class MapBufferCommand : public MapMemObjCommand
    {

    public:
        MapBufferCommand(
            MemoryObject*   pBuffer, 
            cl_map_flags    clMapFlags, 
            size_t          szOffset, 
            size_t          szCb
            );
        virtual ~MapBufferCommand();

        cl_command_type         GetCommandType() const  { return CL_COMMAND_MAP_BUFFER; }
        ECommandExecutionType   GetExecutionType() const{ return DEVICE_EXECUTION_TYPE; }
        const char*             GetCommandName() const  { return "CL_COMMAND_MAP_BUFFER"; }
    };

    /******************************************************************
     *
     ******************************************************************/
    class MapImageCommand : public MapMemObjCommand
    {
    public:
        MapImageCommand(
            MemoryObject*   pImage,
            cl_map_flags    clMapFlags, 
            const size_t*   pOrigin, 
            const size_t*   pRegion,
            size_t*         pszImageRowPitch,
            size_t*         pszImageSlicePitch
            );
        virtual ~MapImageCommand();
 
        cl_command_type         GetCommandType() const  { return CL_COMMAND_MAP_IMAGE; }
        ECommandExecutionType   GetExecutionType() const{ return DEVICE_EXECUTION_TYPE; }
        const char*             GetCommandName() const  { return "CL_COMMAND_MAP_IMAGE"; }

    };

    /******************************************************************
     *
     ******************************************************************/
    class UnmapMemObjectCommand : public Command
    {

    public:
        UnmapMemObjectCommand(
            MemoryObject*   pMemObject,
            void*             pMappedRegion
            );
        virtual ~UnmapMemObjectCommand();

        cl_err_code             Init();
        cl_err_code             Execute();    
        cl_err_code             CommandDone();
        cl_command_type         GetCommandType() const  { return CL_COMMAND_UNMAP_MEM_OBJECT; }
        ECommandExecutionType   GetExecutionType() const{ return DEVICE_EXECUTION_TYPE;       }
        const char*             GetCommandName() const  { return "CL_COMMAND_UNMAP_MEM_OBJECT"; }

    private: 
        MemoryObject* m_pMemObject;
        void*         m_pMappedRegion;
    };

    /******************************************************************
     *
     ******************************************************************/
    class NDRangeKernelCommand : public Command
    {
    public:
        NDRangeKernelCommand(Kernel* pKernel, cl_uint uWorkDim, const size_t* szGlobalWorkOffset, const size_t* szGlobalWorkSize, const size_t* szLocalWorkSize);
        virtual ~NDRangeKernelCommand();

        virtual cl_err_code     Init();
        virtual cl_err_code     Execute();    
        virtual cl_err_code     CommandDone();
        virtual cl_command_type GetCommandType() const  { return CL_COMMAND_NDRANGE_KERNEL; }
        ECommandExecutionType   GetExecutionType() const{ return DEVICE_EXECUTION_TYPE;     }
        const char*             GetCommandName() const  { return "CL_COMMAND_NDRANGE_KERNEL"; }


    protected:         
        // Private members
        Kernel*         m_pKernel;
        cl_uint         m_uiWorkDim;
        const size_t*   m_cpszGlobalWorkOffset;
        const size_t*   m_cpszGlobalWorkSize;
        const size_t*   m_cpszLocalWorkSize;

        // Intermediate data
        std::list<OCLObject<_cl_mem>*>  m_MemOclObjects;
		std::list<OCLObject<_cl_mem>*>  m_NonMemOclObjects;
    };

    /******************************************************************
     *
     ******************************************************************/
    class TaskCommand : public NDRangeKernelCommand
    {

    public:
        TaskCommand( Kernel* pKernel );
        virtual ~TaskCommand();

        // Override Init only to set a different device type
        cl_err_code             Init();
        cl_command_type         GetCommandType() const  { return CL_COMMAND_TASK;       }
        ECommandExecutionType   GetExecutionType() const{ return DEVICE_EXECUTION_TYPE; }
        const char*             GetCommandName() const  { return "CL_COMMAND_TASK"; }
        
    private:
        size_t m_szStaticWorkSize; // Set to 1 to support NDRangeKernel execution with work_size=1
    };

    /******************************************************************
     *
     ******************************************************************/
    class NativeKernelCommand : public Command
    {

    public:
        NativeKernelCommand(
            void              (*pUserFnc)(void *), 
            void*               pArgs, 
            size_t              szCbArgs,
            cl_uint             uNumMemObjects,
            MemoryObject**      ppMemObjList,
            const void**        ppArgsMemLoc
            );
        virtual ~NativeKernelCommand();

        cl_err_code             Init();
        cl_err_code             Execute();    
        cl_err_code             CommandDone();
        cl_command_type         GetCommandType() const  { return CL_COMMAND_NATIVE_KERNEL;  }
        ECommandExecutionType   GetExecutionType() const{ return DEVICE_EXECUTION_TYPE;     }
        const char*             GetCommandName() const  { return "CL_COMMAND_NATIVE_KERNEL"; }

    private:

        void*                 m_pUserFnc;
        void*                m_pArgs;
        size_t                 m_szCbArgs;
        cl_uint             m_uNumMemObjects;
        MemoryObject**        m_ppMemObjList;
        const void**        m_ppArgsMemLoc;
    };

    /******************************************************************
     * Runtime command is a command that was created by the runtime 
     * and is used for sync within the runtime.
     * The command does nothing but keep the event mechanism and therefore can be use for synch
     * Implementation may use it for Flush or Finish commands or marker/barrier etc.
     ******************************************************************/
    class RuntimeCommand : public Command
    {
    public:
        RuntimeCommand()                                        {}
        virtual ~RuntimeCommand()                               {}
        virtual cl_err_code             Init()                  { return CL_SUCCESS; }
        virtual cl_err_code             Execute();        
        virtual cl_err_code             CommandDone()           { return CL_SUCCESS; }
        virtual cl_command_type         GetCommandType() const  { return CL_COMMAND_RUNTIME; }
        virtual ECommandExecutionType   GetExecutionType() const{ return RUNTIME_EXECUTION_TYPE;  }
        virtual const char*             GetCommandName() const  { return "CL_COMMAND_RUNTIME"; }
		virtual bool isControlCommand()	const { return true; }
    };

    /******************************************************************
     *
     ******************************************************************/
    class MarkerCommand : public RuntimeCommand
    {

    public:
        MarkerCommand()          {}
        virtual ~MarkerCommand() {}

        cl_command_type         GetCommandType() const  { return CL_COMMAND_MARKER;  }
        const char*             GetCommandName() const  { return "CL_COMMAND_MARKER";}
    };

    /******************************************************************
     *
     ******************************************************************/
    class WaitForEventsCommand : public RuntimeCommand
    {

    public:
        WaitForEventsCommand()          {}
        virtual ~WaitForEventsCommand() {}

        cl_command_type GetCommandType() const  { return CL_COMMAND_WAIT_FOR_EVENTS; }
        const char*     GetCommandName() const  { return "CL_COMMAND_WAIT_FOR_EVENTS"; }

    };

    /******************************************************************
     *
     ******************************************************************/
    class BarrierCommand : public RuntimeCommand
    {

    public:
        BarrierCommand()          {}
        virtual ~BarrierCommand() {}

        cl_command_type         GetCommandType() const  { return CL_COMMAND_BARRIER; }
        const char*             GetCommandName() const  { return "CL_COMMAND_BARRIER"; }
    };

}}};    // Intel::OpenCL::Framework
#endif  // !defined(__OCL_ENQUEUE_COMMANDS__)