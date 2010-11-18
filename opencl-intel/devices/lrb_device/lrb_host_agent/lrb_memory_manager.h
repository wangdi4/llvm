// Copyright (c) 2008-2009 Intel Corporation
// All rights reserved.
// 
// WARRANTY DISCLAIMER
// 
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING ANY WAY OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

///////////////////////////////////////////////////////////
//  lrb_memory_manager.h
//  
//  Created on:      1-JuLY-2009
//  Original author: Peleg, Arnon
///////////////////////////////////////////////////////////
#if !defined(__LRB_MEMORY_MANAGER_H__)
#define __LRB_MEMORY_MANAGER_H__

#include "lrb_mem_transactor.h"

#include <cl_types.h>
#include <cl_table.h>
#include <cl_device_api.h>

using namespace Intel::OpenCL::Utils;
//using namespace std;

namespace Intel { namespace OpenCL { namespace LRBAgent {

    //
    // Forward declaration
    //
    class XNWrapper;
    class LrbCommandExecuter;
    struct CommandEntry;
    
    /**********************************************************************************************
     * Struct: LrbMemObject
     *
    /**********************************************************************************************/    
    struct LrbMemObject
    {
        // TODO: save description for image
        void*               pHostBuf;   // Pointer to host buffer if needed (in case of OpenCL map/unmap)
        void*               bufHndl;    // Handle of the device buffer
        cl_dev_mem          hMemObj;	// Handle object to use by runtime. The object holds it since it need to free it
    };

    /**********************************************************************************************
     * Class name:    LrbMemoryManager
     *
     * Description:    
     *      This class is responsible to allocate resources for the OpenCL Memory object.
     *      In addition, it is up to the manager to synchronize accesses to any memory object.
     *
     * Author:        Arnon Peleg
     * Date:          June 2009
    /**********************************************************************************************/    
    class LrbMemoryManager
    {
    public:
        LrbMemoryManager(cl_uint uiDevId, XNWrapper* pXnWrapper, fn_clDevCmdStatusChanged* pclDevCmdStatusChanged);
        virtual ~LrbMemoryManager();
        
        void    SetCmdDoneListener  (LrbCommandExecuter* pCmdExecuter)  { m_pCmdExecuter = pCmdExecuter; };
        cl_int  CreateMemObject     (cl_dev_mem_flags clFlags, const cl_image_format* pclFormat, cl_uint uiDimsCount, const size_t* pszDims, void* pMemBuf, const size_t* pszPitch, cl_dev_host_ptr_flags clHostFlags, cl_dev_mem* pMemObj);
        cl_int	DeleteMemObject     ( cl_dev_mem memObj );
        cl_int  CreateMappedRegion  ( cl_dev_cmd_param_map* pMapParams);
        cl_int  ReleaseMappedRegion ( cl_dev_cmd_param_map* pMapParams);
        void*   GetMemObjectHndl    ( cl_dev_mem memObj );

        // Enqueue functions
        cl_int  ReadMemoryCommand(cl_dev_cmd_desc* pclCmd);
        cl_int  WriteMemoryCommand(cl_dev_cmd_desc* pclCmd);

        // Memory execution model - Memory Transactor
        cl_int StartMemoryTransactor    ();
        cl_int QueueMemoryTransaction   ( CommandEntry* pBlockingCmd );
        cl_int ProcessMemoryTransaction ( CommandEntry* pCommand );
        cl_int SignalMemoryTransactor   ()      { return m_pMemTransactor->Signal(); };
                
    private:
        cl_uint                   m_uiDevId;                // The device id of this object.
        XNWrapper*                m_pXnWrapper;             // This object is used for all accesses to LRB HW device
        LrbCommandExecuter*       m_pCmdExecuter;           // Pointer to the execution module. Used to notify on command done
        ClTable*                  m_pMemoryObjTable;        // Table that holds allocated memory objects.
        fn_clDevCmdStatusChanged* m_pclDevCmdStatusChanged; // Notify command status change.
        LrbMemTransactor*         m_pMemTransactor;         // Worker thread that process Read/Write command

        // Helper functions
        size_t CalcBufferSize(cl_uint uiImageType, const cl_image_format* pclFormat, const size_t* pszDims, const size_t* pszPitch);
    };

}}};    // Intel::OpenCL::LRBAgent
#endif  // !defined(__LRB_MEMORY_MANAGER_H__)
