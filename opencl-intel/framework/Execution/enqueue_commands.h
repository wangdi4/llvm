// Copyright (c) 2008-2012 Intel Corporation
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
#include <Logger.h>
#include "cl_object.h"
#include "observer.h"
#include "Device.h"
#include "queue_event.h"
#include "kernel.h"
#include <ocl_itt.h>
#include <list>
#include "ocl_object_base.h"
#include "task_executor.h"
#include "MemoryObject.h"

namespace Intel { namespace OpenCL { namespace Framework {
    
    #define CL_COMMAND_RUNTIME          0
    #define CL_COMMAND_WAIT_FOR_EVENTS  2
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
    class Command : public OCLObjectBase, public ICmdStatusChangedObserver
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
        // The function that is called when the command is popped out from the queue and ready for the device
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
        
        virtual IOclCommandQueueBase* GetCommandQueue() { return m_pCommandQueue; }
        
        // ICmdStatusChangedObserver function
        cl_err_code NotifyCmdStatusChanged(cl_dev_cmd_id clCmdId, cl_int iCmdStatus, cl_int iCompletionResult, cl_ulong ulTimer);
        
        // Command general functions
        QueueEvent*     GetEvent    ()                                      { return &m_Event; }
        void            SetDevCmdListId    (cl_dev_cmd_list clDevCmdListId) { m_clDevCmdListId = clDevCmdListId; }
        cl_dev_cmd_list GetDevCmdListId    () const                         { return m_clDevCmdListId; }
        void            SetDevice(FissionableDevice* pDevice)               { m_pDevice = pDevice; }
        FissionableDevice* GetDevice() const                                { return m_pDevice; }
        
		// wrapper above Enqueue command to allow pre/post-fix commands
		// pEvent is an external user pointer that will point to the user-wisible command which completion means user command completion
		// Note: this may disapper during Enqueue if it was successful!
		virtual cl_err_code EnqueueSelf(cl_bool bBlocking, cl_uint uNumEventsInWaitList, const cl_event* cpEeventWaitList, cl_event* pEvent);

		// Prefix and Postfix Runtime commands
		// Each command may schedule prefix and postfix runtime commands for itself. Such commands are invisible for users
		// and are logical part of the main command that should be executed by RunTime.
		//   Prefix command is executed Before main command is scheduled to device agent
		//   Postfix command is executed After main command signals completion
		// This commands may be long and are executed by task executor
		virtual cl_err_code	PrefixExecute()  { return CL_SUCCESS; }
		virtual cl_err_code	PostfixExecute() { return CL_SUCCESS; }

        // Returns whether this command has been created dependent on events that need to complete before it can be executed
        virtual bool IsDependentOnEvents() const { return false; }

        // Debug functions
        virtual const char*     GetCommandName() const                              { return "UNKNOWN"; }
        
        // GPA related functions
        virtual ocl_gpa_command* GPA_GetCommand() { return m_pGpaCommand; }
        virtual void             GPA_InitCommand();
        virtual void             GPA_DestroyCommand();
        virtual void             GPA_WriteCommandMetadata() {}
        virtual const char*      GPA_GetCommandName() const { return NULL; }
        
    protected:
        Command(const Command& O) : OCLObjectBase("Command"), ICmdStatusChangedObserver(), m_Event(NULL, NULL) {}
        
        // retrieve device specific descriptor of the memory object.
        // If descriptor is not ready on a device:
        //  1. The descriptor value will be set with NULL
        //  2. additional event will be added to dependency list
        //  3. On resolution the provided memory location will be update with device descriptor value
        cl_err_code GetMemObjectDescriptor(MemoryObject* pMemObj, IOCLDevMemoryObject* *ppDevMemObj);

        // AcquireMemoryObjects() brings required memory objects to the target device and lock them there
        // Must be called from Execute() and accompanied by call to RelinquishMemoryObjects during CommandDone().
        // If memory objects are not ready, adds new events to dependency and returns CL_NOT_READY
        // Subsequent calls to AcquireMemoryObjects() will do nothing and always return CL_SUCCESS
        struct MemoryObjectArg 
        {
            MemoryObject* pMemObj;
            MemoryObject::MemObjUsage access_rights;

            MemoryObjectArg( MemoryObject* a, MemoryObject::MemObjUsage b ) : pMemObj(a), access_rights(b) {};
        };

        typedef list<MemoryObjectArg>   MemoryObjectArgList;

        cl_err_code AcquireMemoryObjects( MemoryObjectArgList& mem_objs, FissionableDevice* pDev = NULL ) 
        { 
            return AcquireMemoryObjectsInt( &mem_objs, NULL, pDev ); 
        };
        
        cl_err_code AcquireMemoryObjects( MemoryObject* pMemObj, MemoryObject::MemObjUsage access_rights, FissionableDevice* pDev = NULL  )
        { 
            MemoryObjectArg arg( pMemObj, access_rights ); 
            return AcquireMemoryObjectsInt( NULL, &arg, pDev ); 
        };
        
        void        RelinquishMemoryObjects( MemoryObjectArgList& mem_objs, FissionableDevice* pDev = NULL ) 
        { 
            RelinquishMemoryObjectsInt( &mem_objs, NULL, pDev ); 
        };
        
        void        RelinquishMemoryObjects( MemoryObject* pMemObj, FissionableDevice* pDev = NULL ) 
        { 
            RelinquishMemoryObjectsInt( NULL, pMemObj, pDev );  
        };
        
        QueueEvent                  m_Event;                    // An associated event object
        cl_dev_cmd_desc             m_DevCmd;                   // Device command descriptor struct
        cl_dev_cmd_list             m_clDevCmdListId;           // An handle of the device command list that this command should be queued on
        FissionableDevice*          m_pDevice;                  // A pointer to the device executing the command
        IOclCommandQueueBase*       m_pCommandQueue;            // A pointer to the command queue on which the command resides
        cl_int                      m_returnCode;               // The result of the completed command. Can be CL_SUCCESS or one of the errors defined by the spec.
        cl_int                      m_iId;                      // The command's ID
        cl_command_type             m_commandType;              // Command type
        
        ocl_gpa_command*            m_pGpaCommand;
        
        DECLARE_LOGGER_CLIENT;
    private:

        // return true if ready
        bool AcquireSingleMemoryObject( MemoryObjectArg& arg, FissionableDevice* pDev );
        cl_err_code AcquireMemoryObjectsInt( MemoryObjectArgList* pList, MemoryObjectArg* pSingle, FissionableDevice* pDev );
        void RelinquishMemoryObjectsInt( MemoryObjectArgList* pList, MemoryObject* pSingle, FissionableDevice* pDev );
                
        bool                        m_memory_objects_acquired;
       
    };
    
    class MemoryCommand : public Command
    {
    public:
        MemoryCommand( IOclCommandQueueBase* cmdQueue, ocl_entry_points * pOclEntryPoints ) : Command(cmdQueue, pOclEntryPoints) {}
    protected:        
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
            MemoryObject*   pMemObj,
            const size_t*   pszOrigin,
            const size_t*   pszRegion,
            size_t          szRowPitch,
            size_t          szSlicePitch,
            void*           pDst,
            const size_t*   pszDstOrigin    = NULL,
            const size_t    szDstRowPitch   = 0,
            const size_t    szDstSlicePitch = 0);
        
        virtual ~ReadMemObjCommand();
        
        virtual cl_command_type         GetCommandType() const  { return CL_COMMAND_READ_MEM_OBJECT; }
        virtual ECommandExecutionType GetExecutionType() const
        { return m_commandType != CL_COMMAND_MARKER ? DEVICE_EXECUTION_TYPE : RUNTIME_EXECUTION_TYPE; }
        virtual const char*             GetCommandName() const  { return "CL_COMMAND_READ_MEM_OBJECT"; }
        
        virtual cl_err_code             Init();
        virtual cl_err_code             Execute();
        virtual cl_err_code             CommandDone();
        
    protected:
        MemoryObject*   m_pMemObj;
        size_t          m_szOrigin[MAX_WORK_DIM];
        size_t          m_szRegion[MAX_WORK_DIM];
        size_t          m_szMemObjRowPitch;
        size_t          m_szMemObjSlicePitch;
        void*           m_pDst;
        size_t          m_szDstOrigin[MAX_WORK_DIM];
        size_t          m_szDstRowPitch;
        size_t          m_szDstSlicePitch;
    };
    
    /******************************************************************
     * 
     ******************************************************************/
    class ReadBufferRectCommand : public ReadMemObjCommand
    {
    public:
        ReadBufferRectCommand(
            IOclCommandQueueBase* cmdQueue,
            ocl_entry_points *    pOclEntryPoints,
            MemoryObject*     pBuffer,
            const size_t      szBufferOrigin[MAX_WORK_DIM],
            const size_t      szDstOrigin[MAX_WORK_DIM],
            const size_t      szRegion[MAX_WORK_DIM],
            const size_t      szBufferRowPitch,
            const size_t      szBufferSlicePitch,
            const size_t      szDstRowPitch,
            const size_t      szDstSlicePitch,
            void*             pDst
        );
        virtual ~ReadBufferRectCommand();
        
        cl_command_type         GetCommandType() const  { return m_commandType; }
        const char*             GetCommandName() const  { return "CL_COMMAND_READ_BUFFER_RECT"; }
        
        // GPA related functions
        virtual const char*     GPA_GetCommandName() const { return "Read Buffer Rect"; }
    };
    
    
    class ReadBufferCommand : public ReadMemObjCommand
    {
    public:
        ReadBufferCommand(
            IOclCommandQueueBase* cmdQueue,
            ocl_entry_points *    pOclEntryPoints,
            MemoryObject*     pBuffer,
            const size_t      pszOffset[MAX_WORK_DIM],
            const size_t      pszCb[MAX_WORK_DIM],
            void*             pDst
        );
        virtual ~ReadBufferCommand();
        
        cl_command_type         GetCommandType() const  { return m_commandType; }
        const char*             GetCommandName() const  { return "CL_COMMAND_READ_BUFFER"; }
        
        // GPA related functions
        virtual const char*     GPA_GetCommandName() const { return "Read Buffer"; }
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
            MemoryObject*   pImage,
            const size_t*   pszOrigin,
            const size_t*   pszRegion,
            size_t          szRowPitch,
            size_t          szSlicePitch,
            void*           pDst
        );
        virtual ~ReadImageCommand();
        
        cl_command_type         GetCommandType() const  { return m_commandType; }
        const char*             GetCommandName() const  { return "CL_COMMAND_READ_IMAGE"; }
        
        // GPA related functions
        virtual const char*     GPA_GetCommandName() const { return "Read Image"; }
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
            cl_bool         bBlocking,
            MemoryObject*   pMemObj,
            const size_t*   pszOrigin,
            const size_t*   pszRegion,
            size_t          szMemObjRowPitch,
            size_t          szMemObjSlicePitch,
            const void *    cpSrc,
            const size_t*   pszSrcOrigin    = NULL,
            const size_t    szSrcRowPitch   = 0,
            const size_t    szSrcSlicePitch = 0
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
        size_t          m_szOrigin[MAX_WORK_DIM];
        size_t          m_szRegion[MAX_WORK_DIM];
        cl_bool         m_bBlocking;
        cl_bool         m_bDiscardPreviousData;
        size_t          m_szMemObjRowPitch;
        size_t          m_szMemObjSlicePitch;
        const void*     m_cpSrc;
        size_t          m_szSrcOrigin[MAX_WORK_DIM];
        size_t          m_szSrcRowPitch;
        size_t          m_szSrcSlicePitch;
        void*           m_pTempBuffer;          // This buffer is used when command is blocking
    };
    
    
    /******************************************************************
     * 
     ******************************************************************/
    class FillMemObjCommand : public Command
    {
        
    public:
    	/**
    	 * Multi-dimmensional CTOR, for images.
    	 *
    	 * @param cmdQueue
    	 * @param pOclEntryPoints
    	 * @param pMemObj
    	 * @param pszOffset
    	 * @param pszRegion
    	 * @param numOfDimms
    	 * @param pattern
    	 * @param pattern_size
    	 */
        FillMemObjCommand(
            IOclCommandQueueBase* cmdQueue,
            ocl_entry_points *    pOclEntryPoints,
            MemoryObject*   pMemObj,
            const size_t*   pszOffset,
            const size_t*   pszRegion,
            const cl_uint   numOfDimms,
            const void*     pattern,
            const size_t    pattern_size
        );

        /**
         * 1D CTOR, for buffers.
         *
         * @param cmdQueue
         * @param pOclEntryPoints
         * @param pMemObj
         * @param pszOffset
         * @param pszRegion
         * @param pattern
         * @param pattern_size
         */
        FillMemObjCommand(
            IOclCommandQueueBase* cmdQueue,
            ocl_entry_points *    pOclEntryPoints,
            MemoryObject*   pMemObj,
            const size_t    pszOffset,
            const size_t    pszRegion,
            const void*     pattern,
            const size_t    pattern_size
        );

        virtual ~FillMemObjCommand();
        
        virtual cl_command_type         GetCommandType() const  { return CL_COMMAND_FILL_MEM_OBJECT; }
        virtual ECommandExecutionType   GetExecutionType() const{ return DEVICE_EXECUTION_TYPE;   }
        virtual const char*             GetCommandName() const  { return "CL_COMMAND_FILL_MEM_OBJECT"; }
        
        
        virtual cl_err_code   Init();
        virtual cl_err_code   Execute();
        virtual cl_err_code   CommandDone();
        
    protected:
        MemoryObject*   m_pMemObj;
        size_t          m_szOffset[MAX_WORK_DIM];
        size_t          m_szRegion[MAX_WORK_DIM];
        cl_uint         m_numOfDimms;

        char            m_pattern[MAX_PATTERN_SIZE]; /* pattern for fill */
		size_t          m_pattern_size; /* fill pattern size in bytes */

        const void*     m_cpSrc;

        cl_dev_cmd_param_fill m_fillCmdParams;

    private:
        cl_err_code     m_internalErr; /* error logger for CTOR */
        bool            m_bDiscardPreviousData;
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
            cl_bool         bBlocking,
            MemoryObject*   pBuffer,
            const size_t    pszOffset[MAX_WORK_DIM],
            const size_t    pszCb[MAX_WORK_DIM],
            const void*     cpSrc
        );
        
        virtual ~WriteBufferCommand();
        
        
        cl_command_type         GetCommandType() const  { return CL_COMMAND_WRITE_BUFFER; }
        const char*             GetCommandName() const  { return "CL_COMMAND_WRITE_BUFFER"; }
        
        // GPA related functions
        virtual const char*     GPA_GetCommandName() const { return "Write Buffer"; }
    };
    
    class FillBufferCommand : public FillMemObjCommand
    {
    public:
        FillBufferCommand(
            IOclCommandQueueBase* cmdQueue,
            ocl_entry_points *    pOclEntryPoints,
            MemoryObject*   pBuffer,
            const void *pattern,
            size_t pattern_size,
            size_t offset,
            size_t size);
        
        virtual ~FillBufferCommand();
        
        
        cl_command_type         GetCommandType() const  { return CL_COMMAND_FILL_BUFFER; }
        const char*             GetCommandName() const  { return "CL_COMMAND_FILL_BUFFER"; }
        
        // GPA related functions
        virtual const char*     GPA_GetCommandName() const { return "Fill Buffer"; }
    };
    
    class WriteBufferRectCommand : public WriteMemObjCommand
    {
    public:
        WriteBufferRectCommand(
            IOclCommandQueueBase* cmdQueue,
            ocl_entry_points *    pOclEntryPoints,
            cl_bool         bBlocking,
            MemoryObject*     pBuffer,
            const size_t      szBufferOrigin[MAX_WORK_DIM],
            const size_t      szSrcOrigin[MAX_WORK_DIM],
            const size_t      szRegion[MAX_WORK_DIM],
            const size_t      szBufferRowPitch,
            const size_t      szBufferSlicePitch,
            const size_t      szDstRowPitch,
            const size_t      szDstSlicePitch,
            const void*       pDst
        );
        virtual ~WriteBufferRectCommand();
        
        cl_command_type         GetCommandType() const  { return m_commandType; }
        const char*             GetCommandName() const  { return "CL_COMMAND_WRITE_BUFFER_RECT"; }
        
        // GPA related functions
        virtual const char*     GPA_GetCommandName() const { return "Write Buffer Rect"; }
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
            cl_bool         bBlocking,
            MemoryObject*   pImage,
            const size_t*   pszOrigin,
            const size_t*   pszRegion,
            size_t          szRowPitch,
            size_t          szSlicePitch,
            const void *    cpSrc
        );
        
        virtual ~WriteImageCommand();
        
        cl_command_type         GetCommandType() const  { return CL_COMMAND_WRITE_IMAGE; }
        const char*             GetCommandName() const  { return "CL_COMMAND_WRITE_IMAGE"; }
        
        // GPA related functions
        virtual const char*     GPA_GetCommandName() const { return "Write Image"; }
        
    private:
        MemoryObject*   m_pImage;
        size_t          m_szOrigin[MAX_WORK_DIM];
        size_t          m_szRegion[MAX_WORK_DIM];
        size_t          m_szRowPitch;
        size_t          m_szSlicePitch;
        const void*     m_cpSrc;
    };
    
    
    class FillImageCommand : public FillMemObjCommand
    {
    public:
    	FillImageCommand(
            IOclCommandQueueBase* cmdQueue,
            ocl_entry_points *    pOclEntryPoints,
            MemoryObject*   pImg,
            const void *pattern,
            size_t pattern_size,
            const cl_uint num_of_dimms,
            const size_t *offset,
            const size_t *size);

        virtual ~FillImageCommand();


        cl_command_type         GetCommandType() const  { return CL_COMMAND_FILL_IMAGE; }
        const char*             GetCommandName() const  { return "CL_COMMAND_FILL_IMAGE"; }

        // GPA related functions
        virtual const char*     GPA_GetCommandName() const { return "Fill Image"; }
    };

    /******************************************************************
     * This is an abstrct class that is used for all copy memory object commands
     ******************************************************************/
    class CopyMemObjCommand : public MemoryCommand
    {
        
    public:
        CopyMemObjCommand(
            IOclCommandQueueBase* cmdQueue,
            ocl_entry_points *    pOclEntryPoints,
            MemoryObject*   pSrcMemObj,
            MemoryObject*   pDstMemObj,
            const size_t*   szSrcOrigin,
            const size_t*   szDstOrigin,
            const size_t*   szRegion,
            const size_t    szSrcRowPitch,
            const size_t    szSrcSlicePitch,
            const size_t    szDstRowPitch,
            const size_t    szDstSlicePitch
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
        
        size_t  m_szSrcRowPitch;
        size_t  m_szSrcSlicePitch;
        size_t  m_szDstRowPitch;
        size_t  m_szDstSlicePitch;

        MemoryObjectArgList m_objs;
        
        // Private functions
        cl_err_code CopyOnDevice    (FissionableDevice* pDevice);
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
            MemoryObject*   pSrcBuffer,
            MemoryObject*   pDstBuffer,
            const size_t    szSrcOrigin[MAX_WORK_DIM],
            const size_t    szDstOrigin[MAX_WORK_DIM],
            const size_t    szRegion[MAX_WORK_DIM]
        );
        virtual ~CopyBufferCommand();
        
        cl_command_type         GetCommandType() const  { return m_commandType; }
        const char*             GetCommandName() const  { return "CL_COMMAND_COPY_BUFFER"; }
        
        // GPA related functions
        virtual const char*     GPA_GetCommandName() const { return "Copy Buffer"; }
        
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
            MemoryObject*   pSrcBuffer,
            MemoryObject*   pDstBuffer,
            const size_t    szSrcOrigin[MAX_WORK_DIM],
            const size_t    szDstOrigin[MAX_WORK_DIM],
            const size_t    szRegion[MAX_WORK_DIM],
            const size_t    szSrcRowPitch,
            const size_t    szSrcSlicePitch,
            const size_t    szDstRowPitch,
            const size_t    szDstSlicePitch
        );
        virtual ~CopyBufferRectCommand();
        
        cl_command_type         GetCommandType() const  { return m_commandType; }
        const char*             GetCommandName() const  { return "CL_COMMAND_COPY_BUFFER_RECT"; }
        
        // GPA related functions
        virtual const char*     GPA_GetCommandName() const { return "Copy Buffer Rect"; }
        
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
            MemoryObject*   pSrcImage,
            MemoryObject*   pDstImage,
            const size_t*   pszSrcOrigin,
            const size_t*   pszDstOrigin,
            const size_t*   pszRegion
        );
        virtual ~CopyImageCommand();
        
        cl_command_type         GetCommandType() const  { return m_commandType; }
        const char*             GetCommandName() const  { return "CL_COMMAND_COPY_IMAGE"; }
        
        // GPA related functions
        virtual const char*     GPA_GetCommandName() const { return "Copy Image"; }
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
            MemoryObject*   pSrcImage,
            MemoryObject*   pDstBuffer,
            const size_t*   pszSrcOrigin,
            const size_t*   pszSrcRegion,
            size_t          pszDstOffset[MAX_WORK_DIM]
        );
        virtual ~CopyImageToBufferCommand();
        
        cl_command_type         GetCommandType() const  { return m_commandType; }
        const char*             GetCommandName() const  { return "CL_COMMAND_COPY_IMAGE_TO_BUFFER"; }
        
        // GPA related functions
        virtual const char*     GPA_GetCommandName() const { return "Copy Image To Buffer"; }
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
            MemoryObject*   pSrcBuffer,
            MemoryObject*   pDstImage,
            size_t          pszSrcOffset[MAX_WORK_DIM],
            const size_t*   pszDstOrigin,
            const size_t*   pszDstRegion
        );
        
        virtual ~CopyBufferToImageCommand();
        
        cl_command_type         GetCommandType() const  { return m_commandType; }
        const char*             GetCommandName() const  { return "CL_COMMAND_COPY_BUFFER_TO_IMAGE"; }
        
        // GPA related functions
        virtual const char*     GPA_GetCommandName() const { return "Copy Buffer To Image"; }
        
    };
    
    
    /******************************************************************
     * 
     ******************************************************************/
	class PrePostFixRuntimeCommand;
    class MapMemObjCommand : public Command
    {
    public:
        MapMemObjCommand(
            IOclCommandQueueBase* cmdQueue,
            ocl_entry_points *    pOclEntryPoints,
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
        
		virtual cl_err_code EnqueueSelf(cl_bool bBlocking, cl_uint uNumEventsInWaitList, const cl_event* cpEeventWaitList, cl_event* pEvent);
		virtual cl_err_code	PostfixExecute();

        // Object only function
        void*           GetMappedPtr() const { return m_pHostDataPtr; }
        
    protected:
        MemoryObject*           m_pMemObj;
        cl_map_flags            m_clMapFlags;
        size_t                  m_szOrigin[MAX_WORK_DIM];
        size_t                  m_szRegion[MAX_WORK_DIM];
        size_t*                 m_pszImageRowPitch;
        size_t*                 m_pszImageSlicePitch;
        cl_dev_cmd_param_map*   m_pMappedRegion;
        void*                   m_pHostDataPtr;
        FissionableDevice*      m_pActualMappingDevice;

		// postfix-related. Created in init, pointer zeroed at enqueue.
		ocl_entry_points *      m_pOclEntryPoints;
		PrePostFixRuntimeCommand* m_pPostfixCommand;
        bool                    m_bDiscardPreviousData;
        bool                    m_bResourcesAllocated;
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
            MemoryObject*   pBuffer,
            cl_map_flags    clMapFlags,
            size_t          szOffset,
            size_t          szCb
        );
        virtual ~MapBufferCommand();
        
        cl_command_type         GetCommandType() const  { return CL_COMMAND_MAP_BUFFER; }
        ECommandExecutionType   GetExecutionType() const{ return DEVICE_EXECUTION_TYPE; }
        const char*             GetCommandName() const  { return "CL_COMMAND_MAP_BUFFER"; }
        
        // GPA related functions
        virtual const char*     GPA_GetCommandName() const { return "Map Buffer"; }
    protected:
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
        
        // GPA related functions
        virtual const char*     GPA_GetCommandName() const { return "Map Image"; }
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

		cl_err_code				EnqueueSelf(cl_bool bBlocking, cl_uint uNumEventsInWaitList, const cl_event* cpEeventWaitList, cl_event* pEvent);
		cl_err_code				PrefixExecute();

        cl_command_type         GetCommandType() const  { return CL_COMMAND_UNMAP_MEM_OBJECT; }
        ECommandExecutionType   GetExecutionType() const{ return DEVICE_EXECUTION_TYPE;       }
        const char*             GetCommandName() const  { return "CL_COMMAND_UNMAP_MEM_OBJECT"; }
        
        // GPA related functions
        virtual const char*     GPA_GetCommandName() const { return "Unmap"; }
        
	private:
        MemoryObject*           m_pMemObject;
        void*                   m_pMappedPtr;
        cl_dev_cmd_param_map*   m_pMappedRegion;       
        FissionableDevice*      m_pActualMappingDevice;

		// prefix-related. Created in init, pointer zeroed at enqueue.
		PrePostFixRuntimeCommand* m_pPrefixCommand;
		ocl_entry_points *      m_pOclEntryPoints;
        bool                    m_bDiscardPreviousData;
        bool                    m_bResourcesAllocated;
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
        
        // GPA related functions
        virtual const char*     GPA_GetCommandName() const { return m_pKernel->GetName(); }
        virtual void            GPA_WriteCommandMetadata();
        
    protected:
        cl_dev_cmd_param_kernel m_kernelParams;
        // Private members
        Kernel*         m_pKernel;
        cl_uint         m_uiWorkDim;
        const size_t*   m_cpszGlobalWorkOffset;
        const size_t*   m_cpszGlobalWorkSize;
        const size_t*   m_cpszLocalWorkSize;
        
        // Intermediate data
        MemoryObjectArgList                 m_MemOclObjects;
        std::list<OCLObject<_cl_mem_int>*>  m_NonMemOclObjects;
        #if defined (USE_GPA)
        void GPA_WriteWorkMetadata(const size_t* pWorkMetadata, __itt_string_handle* stringHandle) const;
        #endif
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
        typedef void (CL_CALLBACK*pUserFnc_t)(void *);
        NativeKernelCommand(
            IOclCommandQueueBase* cmdQueue,
            ocl_entry_points *    pOclEntryPoints,
            pUserFnc_t          pUserFnc,
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
        
    protected:
        cl_dev_cmd_param_native m_nativeParams;
        
    private:
        
        pUserFnc_t           m_pUserFnc;
        void*                m_pArgs;
        size_t               m_szCbArgs;
        cl_uint              m_uNumMemObjects;
        MemoryObject**       m_ppMemObjList;
        const void**         m_ppArgsMemLoc;
        MemoryObjectArgList  m_MemOclObjects;
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
		RuntimeCommand( IOclCommandQueueBase* cmdQueue, ocl_entry_points * pOclEntryPoints,
            bool bIsDependentOnEvents = false) : Command(cmdQueue, pOclEntryPoints),
            m_bIsDependentOnEvents(bIsDependentOnEvents) {}
        virtual ~RuntimeCommand()                               {}
        virtual cl_err_code             Init()                  { return CL_SUCCESS; }
        virtual cl_err_code             Execute();
        virtual cl_err_code             CommandDone()           { return CL_SUCCESS; }
        virtual cl_command_type         GetCommandType() const  { return CL_COMMAND_RUNTIME; }
        virtual ECommandExecutionType   GetExecutionType() const{ return RUNTIME_EXECUTION_TYPE;  }
        virtual const char*             GetCommandName() const  { return "CL_COMMAND_RUNTIME"; }
        virtual bool isControlCommand() const { return true; }
        virtual bool IsDependentOnEvents() const { return m_bIsDependentOnEvents; }

    private:

        const bool m_bIsDependentOnEvents;
    };
    
    /******************************************************************
     * 
     ******************************************************************/
    class MarkerCommand : public RuntimeCommand
    {
        
    public:
        MarkerCommand( IOclCommandQueueBase* cmdQueue, ocl_entry_points * pOclEntryPoints, bool bIsDependentOnEvents ) :
          RuntimeCommand(cmdQueue, pOclEntryPoints, bIsDependentOnEvents) {}
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
        WaitForEventsCommand( IOclCommandQueueBase* cmdQueue, ocl_entry_points * pOclEntryPoints, bool bIsDependentOnEvents ) :
          RuntimeCommand(cmdQueue, pOclEntryPoints, bIsDependentOnEvents) {}
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
        BarrierCommand( IOclCommandQueueBase* cmdQueue, ocl_entry_points * pOclEntryPoints, bool bIsDependentOnEvents ) :
          RuntimeCommand(cmdQueue, pOclEntryPoints, bIsDependentOnEvents) {}
        virtual ~BarrierCommand() {}
        
        cl_command_type         GetCommandType() const  { return CL_COMMAND_BARRIER; }
        const char*             GetCommandName() const  { return "CL_COMMAND_BARRIER"; }
    };
    
    /******************************************************************
     *
	 * Special internal Runtime commands to perform some async action before/after normal command
	 *
     ******************************************************************/
	class ErrorQueueEvent : public OclEvent
	{
	public:
		ErrorQueueEvent() : m_owner(NULL) {};
		void Init( PrePostFixRuntimeCommand* owner ) { m_owner = owner; }

		//Override to notify my command about failed events it depended on
		virtual cl_err_code ObservedEventStateChanged(OclEvent* pEvent, cl_int returnCode);
		// Get the context to which the event belongs.
		virtual cl_context GetContextHandle() const;
		// Get the return code of the command associated with the event.
		virtual cl_int     GetReturnCode() const; 

		virtual cl_err_code	GetInfo(cl_int iParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet) const;

	private:
		PrePostFixRuntimeCommand* m_owner;
	};

	class RuntimeCommandTask : public Intel::OpenCL::TaskExecutor::ITask
	{
	public:
		RuntimeCommandTask() : m_owner(NULL) {};
		void Init( PrePostFixRuntimeCommand* owner ) { m_owner = owner; }

		// ITask interface
		bool Execute();
		long Release(); 

	private:
		PrePostFixRuntimeCommand*			m_owner;
	};

    class PrePostFixRuntimeCommand : public RuntimeCommand
    {
    public:
		enum Mode { PREFIX_MODE = 0, POSTFIX_MODE };

        PrePostFixRuntimeCommand(Command* relatedUserCommand,    Mode working_mode, 
								 IOclCommandQueueBase* cmdQueue, ocl_entry_points * pOclEntryPoints ); 
        virtual ~PrePostFixRuntimeCommand() {}

		cl_err_code             Init();
		cl_err_code				Execute();
		cl_err_code             CommandDone();

		// called possibly from another thread
		void					DoAction();

		// called by "related" command if enqueue was unsuccessful 
		void					ErrorDone();
		void					ErrorEnqueue(cl_event* intermediate_pEvent, cl_event* user_pEvent, cl_err_code err_to_force_return );
		cl_err_code				GetForcedErrorCode() const { return m_force_error_return; };

        cl_command_type         GetCommandType() const  { return m_relatedUserCommand->GetCommandType(); };
        const char*             GetCommandName() const  { return m_relatedUserCommand->GetCommandName(); };

	private:
		 Command*			m_relatedUserCommand;
		 Mode				m_working_mode;
		 cl_err_code		m_force_error_return;
		 ErrorQueueEvent	m_error_event;
		 RuntimeCommandTask m_task;
    };

}}}    // Intel::OpenCL::Framework
