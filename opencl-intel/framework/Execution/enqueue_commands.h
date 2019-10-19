// INTEL CONFIDENTIAL
//
// Copyright 2008-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#pragma once

#include "cl_object.h"
#include "observer.h"
#include "Device.h"
#include "queue_event.h"
#include "kernel.h"
#include "task_executor.h"
#include "MemoryObject.h"

#include <CL/cl.h>
#include <cl_types.h>
#include <cl_device_api.h>
#include <Logger.h>
#include <ocl_itt.h>
#include <list>

namespace Intel { namespace OpenCL { namespace Framework {
    
    #define CL_COMMAND_RUNTIME          0
    #define CL_COMMAND_WAIT_FOR_EVENTS  2
    // Forward declarations
    class QueueEvent;
    class MemoryObject;
    class Kernel;
    class IOclCommandQueueBase;
    class ContextModule;
    class OclCommandQueue;
    
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
        Command( const SharedPtr<IOclCommandQueueBase>& cmdQueue );
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
        // The function that is called when the command is popped out from the queue and ready for the device
        // INSTEAD of Execute. Default implementation does the generic part - if more is required override it and add more.
        //
        virtual cl_err_code     Cancel();

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
        // Returns the return code of the command
        //
        virtual cl_int GetReturnCode() const { return m_returnCode; }
        //
        // Sets the return code of the command
        //
        virtual void SetReturnCode(cl_int returnCode) { m_returnCode = returnCode; }
        //
        // Returns whether a command is going to be executed on the device or not.
        //
        virtual ECommandExecutionType GetExecutionType() const = 0;
        
        virtual const SharedPtr<IOclCommandQueueBase>& GetCommandQueue() { return m_pCommandQueue; }
        
        // ICmdStatusChangedObserver function
        cl_err_code NotifyCmdStatusChanged(cl_dev_cmd_id clCmdId, cl_int iCmdStatus, cl_int iCompletionResult, cl_ulong ulTimer);
        
        // Command general functions
        const SharedPtr<QueueEvent>& GetEvent()                             { return m_Event; }
        void            SetDevCmdListId    (cl_dev_cmd_list clDevCmdListId) { m_clDevCmdListId = clDevCmdListId; }
        cl_dev_cmd_list GetDevCmdListId    () const                         { return m_clDevCmdListId; }
        void            SetDevice(const SharedPtr<FissionableDevice>& pDevice)               { m_pDevice = pDevice; }
        const SharedPtr<FissionableDevice>& GetDevice() const                                { return m_pDevice; }

        cl_dev_cmd_desc* GetDeviceCommandDescriptor();

        // wrapper above Enqueue command to allow pre/post-fix commands
        // pEvent is an external user pointer that will point to the user-wisible command which completion means user command completion
        // Note: this may disapper during Enqueue if it was successful!
        virtual cl_err_code EnqueueSelf(cl_bool bBlocking, cl_uint uNumEventsInWaitList, const cl_event* cpEeventWaitList, cl_event* pEvent, ApiLogger* apiLogger);

        // Prefix and Postfix Runtime commands
        // Each command may schedule prefix and postfix runtime commands for itself. Such commands are invisible for users
        // and are logical part of the main command that should be executed by RunTime.
        //   Prefix command is executed Before main command is scheduled to device agent
        //   Postfix command is executed After main command signals completion
        // This commands may be long and are executed by task executor
        virtual cl_err_code    PrefixExecute()  { return CL_SUCCESS; }
        virtual cl_err_code    PostfixExecute() { return CL_SUCCESS; }

        // Returns whether this command has been created dependent on events that need to complete before it can be executed
        virtual bool IsDependentOnEvents() const { return false; }

        // Debug functions
        virtual const char*     GetCommandName() const =0;
        
        // GPA related functions
        virtual ocl_gpa_command* GPA_GetCommand() { return m_pGpaCommand; }
        virtual void             GPA_InitCommand();
        virtual void             GPA_DestroyCommand();
        virtual void             GPA_WriteCommandMetadata() {}
        virtual const char*      GPA_GetCommandName() const { return nullptr; }

        /**
         * @return whether this Command is already being deleted (useful for m_Event, which when destroyed deletes its Command, which is usually the one that contains it)
         */
        bool IsBeingDeleted() const { return m_bIsBeingDeleted; }

    protected:

        // call this to break event<>command sharedPtr loop and initiate command deletion
        void DetachEventSharedPtr();
        
        // retrieve device specific descriptor of the memory object.
        // If descriptor is not ready on a device:
        //  1. The descriptor value will be set with NULL
        //  2. additional event will be added to dependency list
        //  3. On resolution the provided memory location will be update with device descriptor value
        cl_err_code GetMemObjectDescriptor(const SharedPtr<MemoryObject>& pMemObj, IOCLDevMemoryObject* *ppDevMemObj);

        // AcquireMemoryObjects() brings required memory objects to the target device and lock them there
        // Must be called from Execute() and accompanied by call to RelinquishMemoryObjects during CommandDone().
        // If memory objects are not ready, adds new events to dependency and returns CL_NOT_READY
        // Subsequent calls to AcquireMemoryObjects() will do nothing and always return CL_SUCCESS
        struct MemoryObjectArg 
        {
            SharedPtr<MemoryObject> pMemObj;
            MemoryObject::MemObjUsage access_rights;
            MemoryObject::MemObjUsage access_rights_realy_used;

            MemoryObjectArg( const SharedPtr<MemoryObject>& a, MemoryObject::MemObjUsage b ) : pMemObj(a), access_rights(b), access_rights_realy_used(b) {};
            MemoryObjectArg() : pMemObj(nullptr), access_rights(MemoryObject::MEMOBJ_USAGES_COUNT), access_rights_realy_used(MemoryObject::MEMOBJ_USAGES_COUNT) {};
            MemoryObjectArg(const MemoryObjectArg& other) : pMemObj(other.pMemObj), access_rights(other.access_rights), access_rights_realy_used(other.access_rights_realy_used) { }
        };

        typedef vector<MemoryObjectArg>   MemoryObjectArgList;

        static void AddToMemoryObjectArgList( MemoryObjectArgList& argList, MemoryObject* pMemObj, MemoryObject::MemObjUsage access_rights )
        {
            argList.resize( argList.size() + 1 );
            MemoryObjectArg& arg = argList.back();
            arg.pMemObj          = pMemObj;
            arg.access_rights    = access_rights;
        }
        
        static void AddToMemoryObjectArgList( MemoryObjectArgList& argList, const SharedPtr<MemoryObject>& pMemObj, MemoryObject::MemObjUsage access_rights )
        {
            AddToMemoryObjectArgList( argList, pMemObj.GetPtr(), access_rights );
        }
        
        cl_err_code AcquireMemoryObjects( MemoryObjectArgList& argList, const SharedPtr<FissionableDevice>& pDev = nullptr );
 
        void        RelinquishMemoryObjects( MemoryObjectArgList& argList, const SharedPtr<FissionableDevice>& pDev = nullptr );

        void prepare_command_descriptor( cl_dev_cmd_type type, void* params, size_t params_size );
        
        SharedPtr<QueueEvent>           m_Event;    // An associated event object

        cl_dev_cmd_desc                 m_DevCmd;                   // Device command descriptor struct
        cl_dev_cmd_list                 m_clDevCmdListId;           // An handle of the device command list that this command should be queued on
        SharedPtr<FissionableDevice>    m_pDevice;                  // A pointer to the device executing the command
        SharedPtr<IOclCommandQueueBase> m_pCommandQueue;            // A pointer to the command queue on which the command resides
        cl_int                          m_returnCode;               // The result of the completed command. Can be CL_SUCCESS or one of the errors defined by the spec.
        cl_command_type                 m_commandType;              // Command type
        
        ocl_gpa_command*                m_pGpaCommand;
        bool                            m_bIsBeingDeleted;          // Command destructor is active - to be check during internal event destructor 
        volatile bool                   m_bEventDetached;           // event already detached from the command
        

        // Intermediate data
        MemoryObjectArgList             m_MemOclObjects;
        
        DECLARE_LOGGER_CLIENT;
    private:
        
        // disable possibility to create two instances of Command with the same logger pointer.
        Command(const Command& s);
        Command& operator=(const Command& s);
        // return CL_SUCCESS if ready and succeeded, CL_NOT_READY if not ready yet and succeeded, other error code in case of error
        cl_err_code AcquireSingleMemoryObject( MemoryObjectArg& arg, const SharedPtr<FissionableDevice>& pDev );
                
        bool                        m_memory_objects_acquired;
       
    };
    
    /**
     * This class represents a shared pointer for Command objects. It exposes an interface of SmartPtr<COMMAND_TYPE>, but performs reference of its QueueEvent.
     * 
     * @param COMMAND_TYPE the type of pointed to Command
     */
    template<typename COMMAND_TYPE = Command>
    class CommandSharedPtr : public SmartPtr<COMMAND_TYPE>
    {

    public:

        /**
         * Constructor
         * @param pCommand a pointer to the Command
         */
        CommandSharedPtr(COMMAND_TYPE* pCommand) : SmartPtr<COMMAND_TYPE>(pCommand), m_pQueueEvent(pCommand ? pCommand->GetEvent().GetPtr() : nullptr)
        {
        }
        /**
         * Copy constructor
         */
        CommandSharedPtr(const CommandSharedPtr& command) : SmartPtr<COMMAND_TYPE>(command.GetPtr()), m_pQueueEvent(command ? command->GetEvent().GetPtr() : nullptr)
        {
        }

        /**
         * Assignment operator
         */
        CommandSharedPtr& operator=(const CommandSharedPtr& command)
        {
            this->m_ptr = command.GetPtr();
            m_pQueueEvent = (command ? command->GetEvent().GetPtr() : nullptr);
            return *this;
        }

    private:

        SharedPtr<QueueEvent> m_pQueueEvent;

    };

    class MemoryCommand : public Command
    {
    public:
        MemoryCommand( const SharedPtr<IOclCommandQueueBase>& cmdQueue ) : Command(cmdQueue) {}
        
    protected:        
        cl_dev_cmd_param_rw m_rwParams;

        void create_dev_cmd_rw( cl_uint                uiDimCount,
                                void*               pPtr,
                                const size_t*       pszMemObjOrigin,
                                const size_t*       pszPtrOrigin,
                                const size_t*       pszRegion,
                                size_t              szPtrRowPitch,
                                size_t              szPtrSlicePitch,
                                size_t              szMemObjRowPitch,
                                size_t              szMemObjSlicePitch,
                                cl_dev_cmd_type     clCmdType );
        
    };
    
    /******************************************************************
     * 
     ******************************************************************/
    class ReadMemObjCommand : public MemoryCommand
    {
    public:
        ReadMemObjCommand(
            const SharedPtr<IOclCommandQueueBase>& cmdQueue,
            ocl_entry_points *    pOclEntryPoints,
            const SharedPtr<MemoryObject>&   pMemObj,
            const size_t*   pszOrigin,
            const size_t*   pszRegion,
            size_t          szRowPitch,
            size_t          szSlicePitch,
            void*           pDst,
            const size_t*   pszDstOrigin    = nullptr,
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
            const SharedPtr<IOclCommandQueueBase>& cmdQueue,
            ocl_entry_points *    pOclEntryPoints,
            const SharedPtr<MemoryObject>&     pBuffer,
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
            const SharedPtr<IOclCommandQueueBase>& cmdQueue,
            ocl_entry_points *    pOclEntryPoints,
            const SharedPtr<MemoryObject>&     pBuffer,
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

    class ReadSvmBufferCommand : public ReadBufferCommand
    {
    public:        
        ReadSvmBufferCommand(
            const SharedPtr<IOclCommandQueueBase>& cmdQueue,
            ocl_entry_points *    pOclEntryPoints,
            const SharedPtr<MemoryObject>&     pBuffer,
            const size_t      pszOffset[MAX_WORK_DIM],
            const size_t      pszCb[MAX_WORK_DIM],
            void*             pDst
            ) : ReadBufferCommand(cmdQueue, pOclEntryPoints, pBuffer, pszOffset, pszCb, pDst) { }

        cl_command_type         GetCommandType() const  { return CL_COMMAND_SVM_MEMCPY; }
        const char*             GetCommandName() const  { return "CL_COMMAND_SVM_MEMCPY"; }
        virtual const char*     GPA_GetCommandName() const { return "SVM memcpy"; }
    };
    
    class ReadUsmBufferCommand : public ReadBufferCommand
    {
    public:
        ReadUsmBufferCommand(
            const SharedPtr<IOclCommandQueueBase>& cmdQueue,
            ocl_entry_points *    pOclEntryPoints,
            const SharedPtr<MemoryObject>&     pBuffer,
            const size_t      pszOffset[MAX_WORK_DIM],
            const size_t      pszCb[MAX_WORK_DIM],
            void*             pDst
            ) : ReadBufferCommand(cmdQueue, pOclEntryPoints, pBuffer, pszOffset,
                                  pszCb, pDst) {}

        cl_command_type         GetCommandType() const {
            return CL_COMMAND_MEMCPY_INTEL;
        }
        const char*             GetCommandName() const {
            return "CL_COMMAND_MEMCPY_INTEL";
        }
        virtual const char*     GPA_GetCommandName() const {
            return "USM memcpy";
        }
    };

    /******************************************************************
     * 
     ******************************************************************/
    class ReadImageCommand : public ReadMemObjCommand
    {
        
    public:
        ReadImageCommand(
            const SharedPtr<IOclCommandQueueBase>& cmdQueue,
            ocl_entry_points *    pOclEntryPoints,
            const SharedPtr<MemoryObject>&   pImage,
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
            const SharedPtr<IOclCommandQueueBase>& cmdQueue,
            ocl_entry_points *    pOclEntryPoints,
            cl_bool         bBlocking,
            const SharedPtr<MemoryObject>&   pMemObj,
            const size_t*   pszOrigin,
            const size_t*   pszRegion,
            size_t          szMemObjRowPitch,
            size_t          szMemObjSlicePitch,
            const void *    cpSrc,
            const size_t*   pszSrcOrigin    = nullptr,
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
        size_t          m_szOrigin[MAX_WORK_DIM];
        size_t          m_szRegion[MAX_WORK_DIM];
        cl_bool         m_bBlocking;
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
            const SharedPtr<IOclCommandQueueBase>& cmdQueue,
            ocl_entry_points *    pOclEntryPoints,
            const SharedPtr<MemoryObject>&   pMemObj,
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
            const SharedPtr<IOclCommandQueueBase>& cmdQueue,
            ocl_entry_points *    pOclEntryPoints,
            const SharedPtr<MemoryObject>&   pMemObj,
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
        size_t          m_szOffset[MAX_WORK_DIM];
        size_t          m_szRegion[MAX_WORK_DIM];
        cl_uint         m_numOfDimms;

        char            m_pattern[MAX_PATTERN_SIZE]; /* pattern for fill */
        size_t          m_pattern_size; /* fill pattern size in bytes */

        cl_dev_cmd_param_fill m_fillCmdParams;

    private:
        cl_err_code     m_internalErr; /* error logger for CTOR */
    };
    
    /******************************************************************
     * 
     ******************************************************************/
    class WriteBufferCommand : public WriteMemObjCommand
    {
        
    public:
        WriteBufferCommand(
            const SharedPtr<IOclCommandQueueBase>& cmdQueue,
            ocl_entry_points *    pOclEntryPoints,
            cl_bool         bBlocking,
            const SharedPtr<MemoryObject>&   pBuffer,
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

    class WriteSvmBufferCommand : public WriteBufferCommand
    {
    public:

        WriteSvmBufferCommand(
            const SharedPtr<IOclCommandQueueBase>& cmdQueue,
            ocl_entry_points *    pOclEntryPoints,
            cl_bool         bBlocking,
            const SharedPtr<MemoryObject>&   pBuffer,
            const size_t    pszOffset[MAX_WORK_DIM],
            const size_t    pszCb[MAX_WORK_DIM],
            const void*     cpSrc
            ) : WriteBufferCommand(cmdQueue, pOclEntryPoints, bBlocking, pBuffer, pszOffset, pszCb, cpSrc) { }

        cl_command_type         GetCommandType() const  { return CL_COMMAND_SVM_MEMCPY; }
        const char*             GetCommandName() const  { return "CL_COMMAND_SVM_MEMCPY"; }        
        virtual const char*     GPA_GetCommandName() const { return "SVM memcpy"; }
    };
    
    class WriteUsmBufferCommand : public WriteBufferCommand
    {
    public:

        WriteUsmBufferCommand(
            const SharedPtr<IOclCommandQueueBase>& cmdQueue,
            ocl_entry_points *    pOclEntryPoints,
            cl_bool         bBlocking,
            const SharedPtr<MemoryObject>&   pBuffer,
            const size_t    pszOffset[MAX_WORK_DIM],
            const size_t    pszCb[MAX_WORK_DIM],
            const void*     cpSrc
            ) : WriteBufferCommand(cmdQueue, pOclEntryPoints, bBlocking,
                                   pBuffer, pszOffset, pszCb, cpSrc) {}

        cl_command_type     GetCommandType() const {
            return CL_COMMAND_MEMCPY_INTEL;
        }
        const char*         GetCommandName() const {
            return "CL_COMMAND_MEMCPY_INTEL";
        }
        virtual const char* GPA_GetCommandName() const { return "USM memcpy"; }
    };

    class FillBufferCommand : public FillMemObjCommand
    {
    public:
        FillBufferCommand(
            const SharedPtr<IOclCommandQueueBase>& cmdQueue,
            ocl_entry_points *    pOclEntryPoints,
            const SharedPtr<MemoryObject>&   pBuffer,
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
    
    class FillSvmBufferCommand : public FillBufferCommand
    {
    public:
        FillSvmBufferCommand(
            const SharedPtr<IOclCommandQueueBase>& cmdQueue,
            ocl_entry_points *    pOclEntryPoints,
            const SharedPtr<MemoryObject>&   pBuffer,
            const void *pattern,
            size_t pattern_size,
            size_t offset,
            size_t size
        ) : FillBufferCommand(cmdQueue, pOclEntryPoints, pBuffer, pattern, pattern_size, offset, size) { }

        cl_command_type         GetCommandType() const  { return CL_COMMAND_SVM_MEMFILL; }
        const char*             GetCommandName() const  { return "CL_COMMAND_SVM_MEMFILL"; }        
        virtual const char*     GPA_GetCommandName() const { return "Fill SVM Buffer"; }
    };

    class MemsetUsmBufferCommand : public FillBufferCommand
    {
    public:
        MemsetUsmBufferCommand(
            const SharedPtr<IOclCommandQueueBase>& cmdQueue,
            ocl_entry_points *    pOclEntryPoints,
            const SharedPtr<MemoryObject>&   pBuffer,
            const void *pattern,
            size_t pattern_size,
            size_t offset,
            size_t size
        ) : FillBufferCommand(cmdQueue, pOclEntryPoints, pBuffer, pattern,
                              pattern_size, offset, size) {}

        cl_command_type         GetCommandType() const {
            return CL_COMMAND_MEMSET_INTEL;
        }
        const char*             GetCommandName() const {
            return "CL_COMMAND_MEMSET_INTEL";
        }
        virtual const char*     GPA_GetCommandName() const {
            return "Memset USM Buffer";
        }
    };

    class WriteBufferRectCommand : public WriteMemObjCommand
    {
    public:
        WriteBufferRectCommand(
            const SharedPtr<IOclCommandQueueBase>& cmdQueue,
            ocl_entry_points *    pOclEntryPoints,
            cl_bool         bBlocking,
            const SharedPtr<MemoryObject>&     pBuffer,
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
            const SharedPtr<IOclCommandQueueBase>& cmdQueue,
            ocl_entry_points *    pOclEntryPoints,
            cl_bool         bBlocking,
            const SharedPtr<MemoryObject>&   pImage,
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
    };
    
    
    class FillImageCommand : public FillMemObjCommand
    {
    public:
        FillImageCommand(
            const SharedPtr<IOclCommandQueueBase>& cmdQueue,
            ocl_entry_points *    pOclEntryPoints,
            const SharedPtr<MemoryObject>&   pImg,
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
            const SharedPtr<IOclCommandQueueBase>& cmdQueue,
            ocl_entry_points *    pOclEntryPoints,
            const SharedPtr<MemoryObject>&   pSrcMemObj,
            const SharedPtr<MemoryObject>&   pDstMemObj,
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
        SharedPtr<MemoryObject>   m_pSrcMemObj;
        SharedPtr<MemoryObject>   m_pDstMemObj;
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
        
        // Private functions
        cl_err_code CopyOnDevice    (const SharedPtr<FissionableDevice>& pDevice);
    };
    
    /******************************************************************
     * 
     ******************************************************************/
    class CopyBufferCommand : public CopyMemObjCommand
    {
        
    public:
        CopyBufferCommand(
            const SharedPtr<IOclCommandQueueBase>& cmdQueue,
            ocl_entry_points *    pOclEntryPoints,
            const SharedPtr<MemoryObject>&   pSrcBuffer,
            const SharedPtr<MemoryObject>&   pDstBuffer,
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

    class CopySvmBufferCommand : public CopyBufferCommand
    {
    public:

        CopySvmBufferCommand(
            const SharedPtr<IOclCommandQueueBase>& cmdQueue,
            ocl_entry_points *    pOclEntryPoints,
            const SharedPtr<MemoryObject>&   pSrcBuffer,
            const SharedPtr<MemoryObject>&   pDstBuffer,
            const size_t    szSrcOrigin[MAX_WORK_DIM],
            const size_t    szDstOrigin[MAX_WORK_DIM],
            const size_t    szRegion[MAX_WORK_DIM]
        ) : CopyBufferCommand(cmdQueue, pOclEntryPoints, pSrcBuffer, pDstBuffer, szSrcOrigin, szDstOrigin, szRegion) { }

        cl_command_type         GetCommandType() const  { return CL_COMMAND_SVM_MEMCPY; }
        const char*             GetCommandName() const  { return "CL_COMMAND_SVM_MEMCPY"; }        
        virtual const char*     GPA_GetCommandName() const { return "SVM memcpy"; }
    };
    
    class CopyUsmBufferCommand : public CopyBufferCommand
    {
    public:

        CopyUsmBufferCommand(
            const SharedPtr<IOclCommandQueueBase>& cmdQueue,
            ocl_entry_points *    pOclEntryPoints,
            const SharedPtr<MemoryObject>&   pSrcBuffer,
            const SharedPtr<MemoryObject>&   pDstBuffer,
            const size_t    szSrcOrigin[MAX_WORK_DIM],
            const size_t    szDstOrigin[MAX_WORK_DIM],
            const size_t    szRegion[MAX_WORK_DIM]
        ) : CopyBufferCommand(cmdQueue, pOclEntryPoints, pSrcBuffer, pDstBuffer,
                              szSrcOrigin, szDstOrigin, szRegion) {}

        cl_command_type     GetCommandType() const {
            return CL_COMMAND_MEMCPY_INTEL;
        }
        const char*         GetCommandName() const {
            return "CL_COMMAND_MEMCPY_INTEL";
        }
        virtual const char* GPA_GetCommandName() const { return "USM memcpy"; }
    };

    /******************************************************************
     * 
     ******************************************************************/
    class CopyBufferRectCommand : public CopyMemObjCommand
    {
        
    public:
        CopyBufferRectCommand(
            const SharedPtr<IOclCommandQueueBase>& cmdQueue,
            ocl_entry_points *    pOclEntryPoints,
            const SharedPtr<MemoryObject>&   pSrcBuffer,
            const SharedPtr<MemoryObject>&   pDstBuffer,
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
            const SharedPtr<IOclCommandQueueBase>& cmdQueue,
            ocl_entry_points *    pOclEntryPoints,
            const SharedPtr<MemoryObject>&   pSrcImage,
            const SharedPtr<MemoryObject>&   pDstImage,
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
            const SharedPtr<IOclCommandQueueBase>& cmdQueue,
            ocl_entry_points *    pOclEntryPoints,
            const SharedPtr<MemoryObject>&   pSrcImage,
            const SharedPtr<MemoryObject>&   pDstBuffer,
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
            const SharedPtr<IOclCommandQueueBase>& cmdQueue,
            ocl_entry_points *    pOclEntryPoints,
            const SharedPtr<MemoryObject>&   pSrcBuffer,
            const SharedPtr<MemoryObject>&   pDstImage,
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
            const SharedPtr<IOclCommandQueueBase>& cmdQueue,
            ocl_entry_points *    pOclEntryPoints,
            const SharedPtr<MemoryObject>&   pMemObj,
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

        ECommandExecutionType   GetExecutionType() const{ return m_ExecutionType; }
        
        virtual cl_err_code EnqueueSelf(cl_bool bBlocking, cl_uint uNumEventsInWaitList, const cl_event* cpEeventWaitList, cl_event* pEvent, ApiLogger* apiLogger);
        virtual cl_err_code    PostfixExecute();

        // Object only function
        void*           GetMappedPtr() const { return m_pHostDataPtr; }
        
    protected:
        cl_map_flags            m_clMapFlags;
        size_t                  m_szOrigin[MAX_WORK_DIM];
        size_t                  m_szRegion[MAX_WORK_DIM];
        size_t*                 m_pszImageRowPitch;
        size_t*                 m_pszImageSlicePitch;
        cl_dev_cmd_param_map*   m_pMappedRegion;
        void*                   m_pHostDataPtr;
        SharedPtr<FissionableDevice>      m_pActualMappingDevice;
        ECommandExecutionType   m_ExecutionType;

        // postfix-related. Created in init, pointer zeroed at enqueue.
        ocl_entry_points *      m_pOclEntryPoints;
        PrePostFixRuntimeCommand* m_pPostfixCommand;
        bool                    m_bResourcesAllocated;
    private:
        MapMemObjCommand(const MapMemObjCommand&);
        MapMemObjCommand& operator=(const MapMemObjCommand&);
    };
    
    /******************************************************************
     * 
     ******************************************************************/
    class MapBufferCommand : public MapMemObjCommand
    {
        
    public:
        MapBufferCommand(
            const SharedPtr<IOclCommandQueueBase>& cmdQueue,
            ocl_entry_points *    pOclEntryPoints,
            const SharedPtr<MemoryObject>&   pBuffer,
            cl_map_flags    clMapFlags,
            size_t          szOffset,
            size_t          szCb
        );
        virtual ~MapBufferCommand();
        
        cl_command_type         GetCommandType() const  { return CL_COMMAND_MAP_BUFFER; }
        const char*             GetCommandName() const  { return "CL_COMMAND_MAP_BUFFER"; }
        
        // GPA related functions
        virtual const char*     GPA_GetCommandName() const { return "Map Buffer"; }
    protected:
    };

    class MapSvmBufferCommand : public MapBufferCommand
    {
    public:

        MapSvmBufferCommand(
            const SharedPtr<IOclCommandQueueBase>& cmdQueue,
            ocl_entry_points *    pOclEntryPoints,
            const SharedPtr<MemoryObject>&   pBuffer,
            cl_map_flags    clMapFlags,
            size_t          szOffset,
            size_t          szCb
            ) : MapBufferCommand(cmdQueue, pOclEntryPoints, pBuffer, clMapFlags, szOffset, szCb) { }

        cl_command_type         GetCommandType() const  { return CL_COMMAND_SVM_MAP; }
        const char*             GetCommandName() const  { return "CL_COMMAND_SVM_MAP"; }
        virtual const char*     GPA_GetCommandName() const { return "Map SVM Buffer"; }
    };
    
    /******************************************************************
     * 
     ******************************************************************/
    class MapImageCommand : public MapMemObjCommand
    {
    public:
        MapImageCommand(
            const SharedPtr<IOclCommandQueueBase>& cmdQueue,
            ocl_entry_points *    pOclEntryPoints,
            const SharedPtr<MemoryObject>&   pImage,
            cl_map_flags    clMapFlags,
            const size_t*   pOrigin,
            const size_t*   pRegion,
            size_t*         pszImageRowPitch,
            size_t*         pszImageSlicePitch
        );
        virtual ~MapImageCommand();
        
        cl_command_type         GetCommandType() const  { return CL_COMMAND_MAP_IMAGE; }
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
            const SharedPtr<IOclCommandQueueBase>& cmdQueue,
            ocl_entry_points *    pOclEntryPoints,
            const SharedPtr<MemoryObject>&          pMemObject,
            void*                  pMappedRegion
        );
        virtual ~UnmapMemObjectCommand();
        
        cl_err_code             Init();
        cl_err_code             Execute();
        cl_err_code             CommandDone();

        cl_err_code                EnqueueSelf(cl_bool bBlocking, cl_uint uNumEventsInWaitList, const cl_event* cpEeventWaitList, cl_event* pEvent, ApiLogger* apiLogger);
        cl_err_code                PrefixExecute();

        cl_command_type         GetCommandType() const  { return CL_COMMAND_UNMAP_MEM_OBJECT; }
        ECommandExecutionType   GetExecutionType() const{ return m_ExecutionType; }
        const char*             GetCommandName() const  { return "CL_COMMAND_UNMAP_MEM_OBJECT"; }
        
        // GPA related functions
        virtual const char*     GPA_GetCommandName() const { return "Unmap"; }
        
    private:
        void*                   m_pMappedPtr;
        cl_dev_cmd_param_map*   m_pMappedRegion;       
        SharedPtr<FissionableDevice>      m_pActualMappingDevice;
        ECommandExecutionType   m_ExecutionType;

        // prefix-related. Created in init, pointer zeroed at enqueue.
        PrePostFixRuntimeCommand* m_pPrefixCommand;
        ocl_entry_points *      m_pOclEntryPoints;
        bool                    m_bResourcesAllocated;
        UnmapMemObjectCommand(const UnmapMemObjectCommand&);
        UnmapMemObjectCommand& operator=(const UnmapMemObjectCommand&);
    };
    
    class UnmapSvmBufferCommand : public UnmapMemObjectCommand
    {
    public:

        UnmapSvmBufferCommand(
            const SharedPtr<IOclCommandQueueBase>& cmdQueue,
            ocl_entry_points *    pOclEntryPoints,
            const SharedPtr<MemoryObject>&          pMemObject,
            void*                  pMappedRegion
            ) : UnmapMemObjectCommand(cmdQueue, pOclEntryPoints, pMemObject, pMappedRegion) { }

        cl_command_type         GetCommandType() const  { return CL_COMMAND_SVM_UNMAP; }
        const char*             GetCommandName() const  { return "CL_COMMAND_SVM_UNMAP"; }
        virtual const char*     GPA_GetCommandName() const { return "Unmap SVM Buffer"; }
    };

    /******************************************************************
     * 
     ******************************************************************/
    class NDRangeKernelCommand : public Command
    {
    public:
        NDRangeKernelCommand(const SharedPtr<IOclCommandQueueBase>& cmdQueue, ocl_entry_points* pOclEntryPoints, const SharedPtr<Kernel>& pKernel, cl_uint uWorkDim, const size_t* szGlobalWorkOffset, const size_t* szGlobalWorkSize, const size_t* szLocalWorkSize);
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
        SharedPtr<Kernel>       m_pKernel;
        const DeviceKernel*     m_pDeviceKernel;
        cl_uint                 m_uiWorkDim;
        const size_t*           m_cpszGlobalWorkOffset;
        const size_t*           m_cpszGlobalWorkSize;
        const size_t*           m_cpszLocalWorkSize;
        
        std::vector<IOCLDevMemoryObject*>   m_nonArgSvmBuffersVec;        
        std::vector<IOCLDevMemoryObject*>   m_nonArgUsmBuffersVec;
#if defined (USE_ITT)
        void GPA_WriteWorkMetadata(const size_t* pWorkMetadata, __itt_string_handle* keyStrHandle) const;
#endif
    };
    
    /******************************************************************
     * 
     ******************************************************************/
    class TaskCommand : public NDRangeKernelCommand
    {
        
    public:
        TaskCommand( const SharedPtr<IOclCommandQueueBase>& cmdQueue, ocl_entry_points* pOclEntryPoints, const SharedPtr<Kernel>& pKernel );
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
            const SharedPtr<IOclCommandQueueBase>&  cmdQueue,
            ocl_entry_points*                       pOclEntryPoints,
            pUserFnc_t                              pUserFnc,
            void*                                   pArgs,
            size_t                                  szCbArgs,
            cl_uint                                 uNumMemObjects,
            SharedPtr<MemoryObject>*                ppMemObjList,
            const void**                            ppArgsMemLoc
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
        
        pUserFnc_t                  m_pUserFnc;
        void*                       m_pArgs;
        size_t                      m_szCbArgs;
        cl_uint                     m_uNumMemObjects;
        SharedPtr<MemoryObject>*    m_ppMemObjList;
        const void**                m_ppArgsMemLoc;
    };


    class MigrateSVMMemCommand : public Command
    {

    public:
        MigrateSVMMemCommand(
            const SharedPtr<IOclCommandQueueBase>&  cmdQueue,
            ContextModule*         pContextModule,
            cl_mem_migration_flags clFlags,
            cl_uint                uNumMemObjects,
            const void**           pMemObject,
            const size_t*          sizes
        );

        virtual                         ~MigrateSVMMemCommand();

        virtual cl_command_type         GetCommandType()   const  { return CL_COMMAND_MIGRATE_SVM_MEM_OBJECTS; }
        virtual ECommandExecutionType   GetExecutionType() const  { return DEVICE_EXECUTION_TYPE; }
        virtual const char*             GetCommandName()   const  { return "CL_COMMAND_MIGRATE_SVM_MEM_OBJECTS"; }

        virtual cl_err_code             Init();
        virtual cl_err_code             Execute();
        virtual cl_err_code             CommandDone();

        // GPA related functions
        virtual const char*             GPA_GetCommandName() const { return "Migrate SVM Memory Object"; }

    protected:

        const void**                m_pMemObjects;      // used temporary to pass info from contructor to init()
        const size_t*               m_pSizes;
        ContextModule*              m_pContextModule;

        cl_dev_cmd_param_migrate    m_migrateCmdParams;
    };

    /******************************************************************
     * 
     ******************************************************************/
    class MigrateMemObjCommand : public Command
    {
        
    public:
        MigrateMemObjCommand(
            const SharedPtr<IOclCommandQueueBase>&  cmdQueue,
            ocl_entry_points *     pOclEntryPoints,
            ContextModule*         pContextModule,
            cl_mem_migration_flags clFlags,
            cl_uint                uNumMemObjects,
            const cl_mem*          pMemObjects
        );

        virtual                         ~MigrateMemObjCommand();
        
        virtual cl_command_type         GetCommandType() const  { return CL_COMMAND_MIGRATE_MEM_OBJECTS; }
        virtual ECommandExecutionType   GetExecutionType() const{ return DEVICE_EXECUTION_TYPE;   }
        virtual const char*             GetCommandName() const  { return "CL_COMMAND_MIGRATE_MEM_OBJECTS"; }
        
        virtual cl_err_code             Init();
        virtual cl_err_code             Execute();
        virtual cl_err_code             CommandDone();
        
        // GPA related functions
        virtual const char*             GPA_GetCommandName() const { return "Migrate Memory Object"; }

    protected:

        const cl_mem*               m_pMemObjects;      // used temporary to pass info from contructor to init()
        ContextModule*              m_pContextModule;

        cl_dev_cmd_param_migrate    m_migrateCmdParams;
    };

    /******************************************************************
     * MigrateUSMMemCommand explicitly migrates a region of a shared
     * USM allocation to the device associated with the command_queue.
     ******************************************************************/
    class MigrateUSMMemCommand : public Command
    {

    public:
        MigrateUSMMemCommand(
            const SharedPtr<IOclCommandQueueBase>& cmdQueue,
            ContextModule* contextModule,
            cl_mem_migration_flags clFlags,
            const void* ptr,
            size_t size
        );

        virtual                         ~MigrateUSMMemCommand() {}

        virtual cl_command_type         GetCommandType() const {
            return CL_COMMAND_MIGRATEMEM_INTEL;
        }
        virtual ECommandExecutionType   GetExecutionType() const {
            return DEVICE_EXECUTION_TYPE;
        }
        virtual const char*             GetCommandName() const {
            return "CL_COMMAND_MIGRATEMEM_INTEL";
        }

        virtual cl_err_code             Init();
        virtual cl_err_code             Execute();
        virtual cl_err_code             CommandDone();

        // GPA related functions
        virtual const char*             GPA_GetCommandName() const {
            return "Migrate USM Memory Object";
        }

    protected:

        const void*                  m_ptr;
        ContextModule*               m_contextModule;

        cl_dev_cmd_param_migrate_usm m_migrateCmdParams;
    };

    /******************************************************************
     *
     ******************************************************************/
    class AdviseUSMMemCommand : public Command
    {
    public:
        AdviseUSMMemCommand(
            const SharedPtr<IOclCommandQueueBase>& cmdQueue,
            ContextModule* pContextModule,
            const void* ptr,
            size_t size,
            cl_mem_advice_intel advice
        );

        virtual                         ~AdviseUSMMemCommand() {}

        virtual cl_command_type         GetCommandType() const {
            return CL_COMMAND_MEMADVISE_INTEL;
        }
        virtual ECommandExecutionType   GetExecutionType() const {
            return DEVICE_EXECUTION_TYPE;
        }
        virtual const char*             GetCommandName() const {
            return "CL_COMMAND_MEMADVISE_INTEL";
        }

        virtual cl_err_code             Init();
        virtual cl_err_code             Execute();
        virtual cl_err_code             CommandDone();

        // GPA related functions
        virtual const char*             GPA_GetCommandName() const {
            return "Advise USM Memory Object";
        }

    protected:

        const void*                 m_ptr;
        ContextModule*              m_contextModule;

        cl_dev_cmd_param_advise_usm m_adviseCmdParams;
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
        RuntimeCommand( const SharedPtr<IOclCommandQueueBase>& cmdQueue,
            bool bIsDependentOnEvents = false) : Command(cmdQueue),
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
        MarkerCommand( const SharedPtr<IOclCommandQueueBase>& cmdQueue, bool bIsDependentOnEvents ) :
          RuntimeCommand(cmdQueue, bIsDependentOnEvents) {}
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
        WaitForEventsCommand( const SharedPtr<IOclCommandQueueBase>& cmdQueue, bool bIsDependentOnEvents ) :
          RuntimeCommand(cmdQueue, bIsDependentOnEvents) {}
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
        BarrierCommand( const SharedPtr<IOclCommandQueueBase>& cmdQueue, bool bIsDependentOnEvents ) :
          RuntimeCommand(cmdQueue, bIsDependentOnEvents) {}
        virtual ~BarrierCommand() {}
        
        cl_command_type         GetCommandType() const  { return CL_COMMAND_BARRIER; }
        const char*             GetCommandName() const  { return "CL_COMMAND_BARRIER"; }
    };

    /******************************************************************
     * The command is used when user map pointers which have not been
     * allocated by clSVMAlloc, e.g. SVM_FINE_GRAIN_SYSTEM.
     * In this case we should just handle event dependency.
     ******************************************************************/
    class SVMMAP_Command_NOOP : public RuntimeCommand
    {

    public:
        SVMMAP_Command_NOOP( const SharedPtr<IOclCommandQueueBase>& cmdQueue, bool bIsDependentOnEvents ) :
          RuntimeCommand(cmdQueue, bIsDependentOnEvents) {}
        virtual ~SVMMAP_Command_NOOP() {}

        cl_command_type         GetCommandType()     const { return CL_COMMAND_SVM_MAP;   }
        const char*             GetCommandName()     const { return "CL_COMMAND_SVM_MAP"; }
        virtual const char*     GPA_GetCommandName() const { return "Map SVM Buffer";     }
    };

    /******************************************************************
     * The command is used when user map pointers which have not been
     * allocated by clSVMAlloc, e.g. SVM_FINE_GRAIN_SYSTEM.
     * In this case we should just handle event dependency.
     ******************************************************************/
    class SVMUNMAP_Command_NOOP : public RuntimeCommand
    {

    public:
        SVMUNMAP_Command_NOOP( const SharedPtr<IOclCommandQueueBase>& cmdQueue, bool bIsDependentOnEvents ) :
          RuntimeCommand(cmdQueue, bIsDependentOnEvents) {}
        virtual ~SVMUNMAP_Command_NOOP() {}

        cl_command_type         GetCommandType()     const { return CL_COMMAND_SVM_UNMAP;   }
        const char*             GetCommandName()     const { return "CL_COMMAND_SVM_UNMAP"; }
        virtual const char*     GPA_GetCommandName() const { return "Unmap SVM Buffer";     }
    };
    
    /******************************************************************
     *
     * Special internal Runtime commands to perform some async action before/after normal command
     *
     ******************************************************************/
    class ErrorQueueEvent : public OclEvent
    {
    public:

        PREPARE_SHARED_PTR(ErrorQueueEvent)

        static SharedPtr<ErrorQueueEvent> Allocate(_cl_context_int* context)
        {
            return SharedPtr<ErrorQueueEvent>(new ErrorQueueEvent(context));
        }
        
        void Init( PrePostFixRuntimeCommand* owner ) { m_owner = owner; }

        //Override to notify my command about failed events it depended on
        virtual cl_err_code ObservedEventStateChanged(const SharedPtr<OclEvent>& pEvent, cl_int returnCode);

        // Get the return code of the command associated with the event.
        virtual cl_int     GetReturnCode() const; 

        virtual cl_err_code    GetInfo(cl_int iParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet) const;

    private:

        ErrorQueueEvent(_cl_context_int* context) : OclEvent(context), m_owner(nullptr) {};

        PrePostFixRuntimeCommand* m_owner;
    };

    class RuntimeCommandTask : public Intel::OpenCL::TaskExecutor::ITask
    {
    public:

        PREPARE_SHARED_PTR(RuntimeCommandTask)

        static SharedPtr<RuntimeCommandTask> Allocate() { return SharedPtr<RuntimeCommandTask>(new RuntimeCommandTask()); }
        
        void Init( PrePostFixRuntimeCommand* owner ) { m_owner = owner; }

        // ITask interface
        bool    SetAsSyncPoint();
        bool    IsCompleted() const {return m_bIsCompleted;}
        bool    CompleteAndCheckSyncPoint();
        bool    Execute();
        void    Cancel();
        long    Release(); 

        Intel::OpenCL::TaskExecutor::TASK_PRIORITY   GetPriority() const 
                        { return Intel::OpenCL::TaskExecutor::TASK_PRIORITY_MEDIUM;}

        virtual Intel::OpenCL::TaskExecutor::IThreadLibTaskGroup* GetNDRangeChildrenTaskGroup() { return nullptr; }

    private:

        RuntimeCommandTask() : m_owner(nullptr) {};

        CommandSharedPtr<PrePostFixRuntimeCommand>            m_owner;
        bool                                m_bIsCompleted;
    };

    class PrePostFixRuntimeCommand : public RuntimeCommand
    {
    public:
        enum Mode { PREFIX_MODE = 0, POSTFIX_MODE };

        PrePostFixRuntimeCommand(Command* relatedUserCommand,
                                    Mode working_mode, 
                                    const SharedPtr<IOclCommandQueueBase>& cmdQueue );

        cl_err_code             Init();
        cl_err_code                Execute();
        cl_err_code             CommandDone();

        // called possibly from another thread
        void                    DoAction();
        void                    CancelAction();

        // called by "related" command if enqueue was unsuccessful 
        void                    ErrorDone();
        void                    ErrorEnqueue(cl_event* intermediate_pEvent, cl_event* user_pEvent, cl_err_code err_to_force_return );
        cl_err_code                GetForcedErrorCode() const { return m_force_error_return; };

        cl_command_type         GetCommandType() const  { return m_relatedUserCommand->GetCommandType(); };
        const char*             GetCommandName() const
        { 
            if (m_working_mode == PREFIX_MODE)
            {
                return "PreFixRuntimeCommand";
            }
            else
            {
                return "PostFixRuntimeCommand";
            }
        };

    private:
         CommandSharedPtr<> m_relatedUserCommand;
         Mode                m_working_mode;
         cl_err_code        m_force_error_return;
         SharedPtr<ErrorQueueEvent>    m_error_event;
         SharedPtr<RuntimeCommandTask> m_task;
    };

}}}    // Intel::OpenCL::Framework
