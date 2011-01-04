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

#pragma once

#include <cl_types.h>
#include <cl_device_api.h>
#include <logger.h>
#include "cl_object.h"
#include "observer.h"
#include "Device.h"
#include "queue_event.h"
#include <list>

namespace Intel { namespace OpenCL { namespace Framework {

	#define CL_COMMAND_RUNTIME			0
	#define CL_COMMAND_BARRIER			1
	#define CL_COMMAND_WAIT_FOR_EVENTS	2
    // Forward declarations
    class QueueEvent;
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
    class Command : public ICmdStatusChangedObserver
    {

    public:
		Command( IOclCommandQueueBase* cmdQueue, ocl_entry_points * pOclEntryPoints );
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
		// set command type (some commands like ReadBuffer/ReadImage change to MARKER on their Execute() if decided not to go to device)
		//
		void SetCommandType(cl_command_type newCmdType) { m_commandType = newCmdType; }
		// Returns True if command is: Marker || Barrier || WaitForEvents
		//
		virtual bool isControlCommand() const  { return false; }
		//
		// Returns the success status of the command
		//
		virtual cl_int GetReturnCode() const { return m_returnCode; }
        //
        // Returns whether a command is going to be executed on the device or not.
        //
        virtual ECommandExecutionType GetExecutionType() const = 0;

        // ICmdStatusChangedObserver function
        cl_err_code NotifyCmdStatusChanged(cl_dev_cmd_id clCmdId, cl_int iCmdStatus, cl_int iCompletionResult, cl_ulong ulTimer);

        // Command general functions
        QueueEvent*     GetEvent    ()                                      { return &m_Event; }   
        void            SetDevCmdListId    (cl_dev_cmd_list clDevCmdListId) { m_clDevCmdListId = clDevCmdListId; }
        cl_dev_cmd_list GetDevCmdListId    () const                         { return m_clDevCmdListId; }
		void            SetDevice(Device* pDevice)                          { m_pDevice = pDevice; }
		Device*         GetDevice() const                                   { return m_pDevice; }

        // Debug functions
        virtual const char*     GetCommandName() const                              { return "UNKNOWN"; }

    protected:
		Command(const Command& O) : m_Event(NULL, NULL) {}

        QueueEvent                  m_Event;                    // An associated event object
        cl_dev_cmd_desc             m_DevCmd;                   // Device command descriptor struct
        cl_dev_cmd_list             m_clDevCmdListId;           // An handle of the device command list that this command should be queued on
		Device*                     m_pDevice;                  // A pointer to the device executing the command
		IOclCommandQueueBase*       m_pCommandQueue;            // A pointer to the command queue on which the command resides
		cl_int                      m_returnCode;               // The result of the completed command. Can be CL_SUCCESS or one of the errors defined by the spec. 
		cl_int                      m_iId;                      // The command's ID
		cl_command_type				m_commandType;				// Command type

		DECLARE_LOGGER_CLIENT;
    };

	class MemoryCommand : public Command
	{
	public:
		MemoryCommand( IOclCommandQueueBase* cmdQueue, ocl_entry_points * pOclEntryPoints ) : Command(cmdQueue, pOclEntryPoints) {}
		cl_err_code		PrepareOnDevice(
			MemoryObject* pSrcMemObj, 						
			const size_t* pSrcOrigin, 						
			const size_t* pRegion,							
			QueueEvent**  pEvent);

	protected:
		cl_err_code CopyFromHost(
			void* pSrcData,
			MemoryObject* pSrcMemObj, 
			const size_t* pSrcOrigin,
			const size_t* pDstOrigin,
			const size_t* pRegion,
			const size_t  szSrcRowPitch,
			const size_t  szSrcSlicePitch,
			const size_t  szDstRowPitch,
			const size_t  szDstSlicePitch,
			QueueEvent**  pEvent);

		cl_err_code CopyToHost(
			MemoryObject*	pSrcMemObj, 												
			QueueEvent**		pEvent);

		cl_dev_cmd_param_rw m_rwParams;

	};

    /******************************************************************
     *
     ******************************************************************/
	class ReadMemObjCommand : public MemoryCommand
    {
    public:
        ReadMemObjCommand(
			IOclCommandQueueBase* cmdQueue, 
			ocl_entry_points *    pOclEntryPoints,
            MemoryObject*  		  pMemObj,
			const size_t*  		  pszOrigin,
			const size_t*  		  pszRegion,
			size_t         		  szRowPitch, 
			size_t         		  szSlicePitch, 
			void*          		  pDst,
			const size_t*	      pszDstOrigin	= NULL,
			const size_t          szDstRowPitch	= 0,
			const size_t          szDstSlicePitch = 0);

        virtual ~ReadMemObjCommand();

		virtual cl_command_type         GetCommandType() const  { return CL_COMMAND_READ_MEM_OBJECT; }
        virtual ECommandExecutionType GetExecutionType() const
			{ return m_commandType != CL_COMMAND_MARKER ? DEVICE_EXECUTION_TYPE : RUNTIME_EXECUTION_TYPE; }
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
		size_t			m_szDstOrigin[3];
		size_t			m_szDstRowPitch;
		size_t			m_szDstSlicePitch;
    };

    class ReadBufferRectCommand : public ReadMemObjCommand
    {
    public:
        ReadBufferRectCommand(
			IOclCommandQueueBase* cmdQueue, 
			ocl_entry_points *    pOclEntryPoints,
			MemoryObject*  		  pBuffer, 
			const size_t   		  szBufferOrigin[3],
			const size_t   		  szDstOrigin[3],
			const size_t  		  szRegion[3],
			const size_t  		  szBufferRowPitch,
			const size_t  		  szBufferSlicePitch,
			const size_t	      szDstRowPitch,
			const size_t          szDstSlicePitch,			            
            void*                 pDst
            );
        virtual ~ReadBufferRectCommand();
        
        cl_command_type         GetCommandType() const  { return m_commandType; }
        const char*             GetCommandName() const  { return "CL_COMMAND_READ_BUFFER_RECT"; }
    };
	

	class ReadBufferCommand : public ReadMemObjCommand
    {
    public:
        ReadBufferCommand(
			IOclCommandQueueBase* cmdQueue, 
			ocl_entry_points *    pOclEntryPoints,
            MemoryObject*         pBuffer, 
            const size_t          pszOffset[3],
            const size_t          pszCb[3], 
            void*                 pDst
            );
        virtual ~ReadBufferCommand();
        
        cl_command_type         GetCommandType() const  { return m_commandType; }
        const char*             GetCommandName() const  { return "CL_COMMAND_READ_BUFFER"; }
    };

    /******************************************************************
     *
     ******************************************************************/
    class ReadImageCommand : public ReadMemObjCommand
    {

    public:
        ReadImageCommand(
			IOclCommandQueueBase* cmdQueue, 
			ocl_entry_points *    pOclEntryPoints,
            MemoryObject*         pImage,
            const size_t*         pszOrigin,
            const size_t*         pszRegion,
            size_t                szRowPitch, 
            size_t                szSlicePitch, 
            void*                 pDst
            );
        virtual ~ReadImageCommand();
     
        cl_command_type         GetCommandType() const  { return m_commandType; }
        const char*             GetCommandName() const  { return "CL_COMMAND_READ_IMAGE"; }
    };

	/******************************************************************
    *
    ******************************************************************/
	class WriteMemObjCommand : public MemoryCommand
    {

    public:
        WriteMemObjCommand(
			IOclCommandQueueBase* cmdQueue, 
			ocl_entry_points *    pOclEntryPoints,
            MemoryObject*  		  pMemObj, 
            const size_t*  		  pszOrigin,
            const size_t*  		  pszRegion,
            size_t         		  szRowPitch,
            size_t         		  szSlicePitch,
            const void *   		  cpSrc,
			const size_t*  		  pszSrcOrigin	= NULL,
			const size_t   		  szSrcRowPitch	= 0,
			const size_t   		  szSrcSlicePitch = 0
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
		size_t			m_szSrcOrigin[3];
		size_t			m_szSrcRowPitch;
		size_t			m_szSrcSlicePitch;
    };

    /******************************************************************
     *
     ******************************************************************/
    class WriteBufferCommand : public WriteMemObjCommand
    {

    public:
        WriteBufferCommand(
			IOclCommandQueueBase* cmdQueue, 
			ocl_entry_points *    pOclEntryPoints,
            MemoryObject*		  pBuffer, 
            const size_t		  pszOffset[3], 
            const size_t		  pszCb[3],
            const void*			  cpSrc
            );

        virtual ~WriteBufferCommand();


        cl_command_type         GetCommandType() const  { return CL_COMMAND_WRITE_BUFFER; }
        const char*             GetCommandName() const  { return "CL_COMMAND_WRITE_BUFFER"; }
    };

	class WriteBufferRectCommand : public WriteMemObjCommand
    {
    public:
        WriteBufferRectCommand(
			IOclCommandQueueBase* cmdQueue, 
			ocl_entry_points *    pOclEntryPoints,
            MemoryObject*		  pBuffer, 
            const size_t     	  szBufferOrigin[3],
			const size_t     	  szSrcOrigin[3],
			const size_t	 	  szRegion[3],
			const size_t	 	  szBufferRowPitch,
			const size_t	 	  szBufferSlicePitch,
			const size_t	 	  szDstRowPitch,
			const size_t	 	  szDstSlicePitch,			            
            const void*      	  pDst
            );
        virtual ~WriteBufferRectCommand();
        
        cl_command_type         GetCommandType() const  { return m_commandType; }
        const char*             GetCommandName() const  { return "CL_COMMAND_WRITE_BUFFER_RECT"; }
    };

    /******************************************************************
     *
     ******************************************************************/
    class WriteImageCommand : public WriteMemObjCommand
    {

    public:
        WriteImageCommand(
			IOclCommandQueueBase* cmdQueue, 
			ocl_entry_points *    pOclEntryPoints,
            MemoryObject*		  pImage, 
            const size_t*		  pszOrigin,
            const size_t*		  pszRegion,
            size_t				  szRowPitch,
            size_t				  szSlicePitch,
            const void *		  cpSrc
            );

        virtual ~WriteImageCommand();

        cl_command_type         GetCommandType() const  { return CL_COMMAND_WRITE_IMAGE; }
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
     * This is an abstract class that is used for all copy memory object commands
     ******************************************************************/
    class CopyMemObjCommand : public MemoryCommand
    {

    public:
        CopyMemObjCommand(
			IOclCommandQueueBase* cmdQueue, 
			ocl_entry_points *    pOclEntryPoints,
            MemoryObject*		  pSrcMemObj, 
            MemoryObject*		  pDstMemObj, 
            const size_t*		  szSrcOrigin, 
            const size_t*		  szDstOrigin,
            const size_t*		  szRegion,
			const size_t		  szSrcRowPitch,
			const size_t		  szSrcSlicePitch,
			const size_t		  szDstRowPitch,
			const size_t		  szDstSlicePitch
            );        
        virtual ~CopyMemObjCommand();

        virtual cl_err_code Init();
        virtual cl_err_code Execute();    
        virtual cl_err_code CommandDone();

        virtual ECommandExecutionType GetExecutionType() const
			{return m_commandType != CL_COMMAND_MARKER ? DEVICE_EXECUTION_TYPE : RUNTIME_EXECUTION_TYPE;}


    protected:
        MemoryObject*   m_pSrcMemObj;
        MemoryObject*   m_pDstMemObj;
        size_t          m_szSrcOrigin[MAX_WORK_DIM];
        size_t          m_szDstOrigin[MAX_WORK_DIM];
        size_t          m_szRegion[MAX_WORK_DIM];
        cl_uint         m_uiSrcNumDims;     // The dimensions represent the memory object type, 1,2,3 
                                            // respectively are BUFFER/2D/3D. The private member is used only for ease of use.
        cl_uint         m_uiDstNumDims;
		cl_dev_cmd_param_copy m_copyParams;

		size_t	m_szSrcRowPitch;
		size_t	m_szSrcSlicePitch;
		size_t	m_szDstRowPitch;
		size_t	m_szDstSlicePitch;
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
			IOclCommandQueueBase* cmdQueue, 
			ocl_entry_points *    pOclEntryPoints,
            MemoryObject*		  pSrcBuffer, 
            MemoryObject*		  pDstBuffer, 
            const size_t		  szSrcOrigin[3],
            const size_t		  szDstOrigin[3],
            const size_t		  szRegion[3]
            );        
        virtual ~CopyBufferCommand();

        cl_command_type         GetCommandType() const  { return m_commandType; }
        const char*             GetCommandName() const  { return "CL_COMMAND_COPY_BUFFER"; }

    };

	/******************************************************************
     *
     ******************************************************************/
    class CopyBufferRectCommand : public CopyMemObjCommand
    {

    public:
        CopyBufferRectCommand(
			IOclCommandQueueBase* cmdQueue, 
			ocl_entry_points *    pOclEntryPoints,
            MemoryObject*		  pSrcBuffer, 
            MemoryObject*		  pDstBuffer, 
            const size_t		  szSrcOrigin[3],
            const size_t		  szDstOrigin[3],
            const size_t		  szRegion[3],
			const size_t		  szSrcRowPitch,
			const size_t		  szSrcSlicePitch,
			const size_t		  szDstRowPitch,
			const size_t		  szDstSlicePitch
            );        
        virtual ~CopyBufferRectCommand();

        cl_command_type         GetCommandType() const  { return m_commandType; }
        const char*             GetCommandName() const  { return "CL_COMMAND_COPY_BUFFER_RECT"; }

    };

    /******************************************************************
     *
     ******************************************************************/
    class CopyImageCommand : public CopyMemObjCommand
    {

    public:
        CopyImageCommand(
			IOclCommandQueueBase* cmdQueue, 
			ocl_entry_points *    pOclEntryPoints,
            MemoryObject*		  pSrcImage,
            MemoryObject*		  pDstImage,
            const size_t*		  pszSrcOrigin,
            const size_t*		  pszDstOrigin,
            const size_t*		  pszRegion
        );
        virtual ~CopyImageCommand();

        cl_command_type         GetCommandType() const  { return m_commandType; }
        const char*             GetCommandName() const  { return "CL_COMMAND_COPY_IMAGE"; }
    };

    /******************************************************************
     *
     ******************************************************************/
    class CopyImageToBufferCommand : public CopyMemObjCommand
    {
    public:
        CopyImageToBufferCommand(
			IOclCommandQueueBase* cmdQueue, 
			ocl_entry_points *    pOclEntryPoints,
            MemoryObject*		  pSrcImage, 
            MemoryObject*		  pDstBuffer, 
            const size_t*		  pszSrcOrigin, 
            const size_t*		  pszSrcRegion,
            size_t				  pszDstOffset[3]
        );
        virtual ~CopyImageToBufferCommand();

        cl_command_type         GetCommandType() const  { return m_commandType; }
        const char*             GetCommandName() const  { return "CL_COMMAND_COPY_IMAGE_TO_BUFFER"; }
    };

    /******************************************************************
     *
     ******************************************************************/
    class CopyBufferToImageCommand : public CopyMemObjCommand
    {

    public:
        CopyBufferToImageCommand(
			IOclCommandQueueBase* cmdQueue, 
			ocl_entry_points *    pOclEntryPoints,
            MemoryObject*		  pSrcBuffer, 
            MemoryObject*		  pDstImage, 
            size_t				  pszSrcOffset[3], 
            const size_t*		  pszDstOrigin, 
            const size_t*		  pszDstRegion
        );

        virtual ~CopyBufferToImageCommand();

        cl_command_type         GetCommandType() const  { return m_commandType; }
        const char*             GetCommandName() const  { return "CL_COMMAND_COPY_BUFFER_TO_IMAGE"; }

    };


    /******************************************************************
     *
     ******************************************************************/
    class MapMemObjCommand : public Command
    {
    public:
        MapMemObjCommand(
			IOclCommandQueueBase* cmdQueue, 
			ocl_entry_points *    pOclEntryPoints,
            MemoryObject*		  pMemObj,
            cl_map_flags		  clMapFlags, 
            const size_t*		  pOrigin, 
            const size_t*		  pRegion,
            size_t*				  pszImageRowPitch,
            size_t*				  pszImageSlicePitch
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
			IOclCommandQueueBase* cmdQueue, 
			ocl_entry_points *    pOclEntryPoints,
            MemoryObject*		  pBuffer, 
            cl_map_flags		  clMapFlags, 
            size_t				  szOffset, 
            size_t				  szCb
            );
        virtual ~MapBufferCommand();

        cl_command_type         GetCommandType() const  { return CL_COMMAND_MAP_BUFFER; }
        ECommandExecutionType   GetExecutionType() const{ return DEVICE_EXECUTION_TYPE; }
        const char*             GetCommandName() const  { return "CL_COMMAND_MAP_BUFFER"; }
	protected:
		size_t m_origin;
		size_t m_region;
    };

    /******************************************************************
     *
     ******************************************************************/
    class MapImageCommand : public MapMemObjCommand
    {
    public:
        MapImageCommand(
			IOclCommandQueueBase* cmdQueue, 
			ocl_entry_points *    pOclEntryPoints,
            MemoryObject*		  pImage,
            cl_map_flags		  clMapFlags, 
            const size_t*		  pOrigin, 
            const size_t*		  pRegion,
            size_t*				  pszImageRowPitch,
            size_t*				  pszImageSlicePitch
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
			IOclCommandQueueBase* cmdQueue, 
			ocl_entry_points *    pOclEntryPoints,
            MemoryObject*		  pMemObject,
            void*				  pMappedRegion
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
        NDRangeKernelCommand(IOclCommandQueueBase* cmdQueue, ocl_entry_points* pOclEntryPoints, Kernel* pKernel, cl_uint uWorkDim, const size_t* szGlobalWorkOffset, const size_t* szGlobalWorkSize, const size_t* szLocalWorkSize);
        virtual ~NDRangeKernelCommand();

        virtual cl_err_code     Init();
        virtual cl_err_code     Execute();    
        virtual cl_err_code     CommandDone();
        virtual cl_command_type GetCommandType() const  { return CL_COMMAND_NDRANGE_KERNEL; }
        ECommandExecutionType   GetExecutionType() const{ return DEVICE_EXECUTION_TYPE;     }
        const char*             GetCommandName() const  { return "CL_COMMAND_NDRANGE_KERNEL"; }


    protected:         
		cl_dev_cmd_param_kernel m_kernelParams;
        // Private members
        Kernel*         m_pKernel;
        cl_uint         m_uiWorkDim;
        const size_t*   m_cpszGlobalWorkOffset;
        const size_t*   m_cpszGlobalWorkSize;
        const size_t*   m_cpszLocalWorkSize;

        // Intermediate data
        std::list<OCLObject<_cl_mem_int>*>  m_MemOclObjects;
		std::list<OCLObject<_cl_mem_int>*>  m_NonMemOclObjects;
    };

    /******************************************************************
     *
     ******************************************************************/
    class TaskCommand : public NDRangeKernelCommand
    {

    public:
        TaskCommand( IOclCommandQueueBase* cmdQueue, ocl_entry_points* pOclEntryPoints, Kernel* pKernel );
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
			IOclCommandQueueBase* cmdQueue, 
			ocl_entry_points *    pOclEntryPoints,
            void                (*pUserFnc)(void *), 
            void*                 pArgs, 
            size_t                szCbArgs,
            cl_uint               uNumMemObjects,
            MemoryObject**        ppMemObjList,
            const void**          ppArgsMemLoc
            );
        virtual ~NativeKernelCommand();

        cl_err_code             Init();
        cl_err_code             Execute();    
        cl_err_code             CommandDone();
        cl_command_type         GetCommandType() const  { return CL_COMMAND_NATIVE_KERNEL;  }
        ECommandExecutionType   GetExecutionType() const{ return DEVICE_EXECUTION_TYPE;     }
        const char*             GetCommandName() const  { return "CL_COMMAND_NATIVE_KERNEL"; }

	protected:
		cl_dev_cmd_param_native m_nativeParams;

    private:

        void*			m_pUserFnc;
        void*			m_pArgs;
        size_t			m_szCbArgs;
        cl_uint			m_uNumMemObjects;
        MemoryObject**	m_ppMemObjList;
        const void**	m_ppArgsMemLoc;
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
		RuntimeCommand( IOclCommandQueueBase* cmdQueue, ocl_entry_points * pOclEntryPoints ) : Command(cmdQueue, pOclEntryPoints) {}
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
        MarkerCommand( IOclCommandQueueBase* cmdQueue, ocl_entry_points * pOclEntryPoints ) : RuntimeCommand(cmdQueue, pOclEntryPoints) {}
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
        WaitForEventsCommand( IOclCommandQueueBase* cmdQueue, ocl_entry_points * pOclEntryPoints ) : RuntimeCommand(cmdQueue, pOclEntryPoints) {}
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
        BarrierCommand( IOclCommandQueueBase* cmdQueue, ocl_entry_points * pOclEntryPoints ) : RuntimeCommand(cmdQueue, pOclEntryPoints) {}
        virtual ~BarrierCommand() {}

        cl_command_type         GetCommandType() const  { return CL_COMMAND_BARRIER; }
        const char*             GetCommandName() const  { return "CL_COMMAND_BARRIER"; }
    };

}}};    // Intel::OpenCL::Framework
