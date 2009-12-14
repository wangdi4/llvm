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
//  lrb_agent_common.h
//  Implementation of the Class LrbAgent
//  Created on:      16-June-2009
//  Original author: Peleg, Arnon
///////////////////////////////////////////////////////////
#if !defined(__LRB_AGENT_COMMON_H__)
#define __LRB_AGENT_COMMON_H__

#include "common/XN0Types_common.h"

#define COMMAND_STATUS_CHANGE_COMMUNICATOR "/XN/lrb_device/command_status_change_communicator"
#define XN_MESSAGE_MAX_SIZE     256
#define BUILD_OUTPUT_MAX_SISE   256*1024    // Max size of kernel * Max kernels
#define XN_OPTIONS_MAX_SISE     1024        //

#define STS_COMPLETE                                 0x0
#define STS_RUNNING                                  0x1
#define STS_BUILD_DONE                               0xfe

#define LRB_CMD_CPY                                  0x0
#define LRB_CMD_KERNEL                               0x1


/************************************************************************
 * Define of fixed prototype for kernel function.
 * TODO: Remove it when Dasher Backend API will be ready
 ************************************************************************/
typedef void (fn_lrbKernelFunction)(
                                    const float*    a, 
                                    const float*    b,
                                    float*          c, 
                                    int             tid
                                    );

//////////////////////////////////////////////////////////////////////////
namespace Intel { namespace OpenCL { namespace LRBAgent {

    /************************************************************************
     * Output message: Type of message that is transfered on the pinned memory
     * when command is done
     ************************************************************************/
    struct CommandDoneMessage
    {
        uint32_t    cmdHndl;
        uint8_t     status;  
    };


    //This structure defined the data that is in the commands buffer
    // This data is used as 
    struct execute_cmds_params
    {
        uint16_t    uiNumCmds;        // Number of commands.
        uint8_t     uiIsListInOrder;  // 0x1, is in order, 0x0 otherwise
    };


    //This structure holds a description of a command that is passed to the device for execution.
    struct lrb_cmd_desc
    {
        uint32_t    uiCmdAddr;
        uint32_t    uiCmdSize;
        uint16_t    uiNumMemObjUsed;
        uint8_t     uiCmdType;
        uint8_t     bProfiling;
        uint32_t    uiParamSize;    
        uint32_t*   pParams;
    };


    // Copy parameters
    struct lrb_cmd_param_copy
    {
        void*       pSrcMemObj;         // Handle to a source memory object from where the data to be read.
        void*       pDstMemObj;         // Handle to a source memory object to where the data to be written.
        uint8_t     uiSrcDimCount;      // A number of dimensions in the source memory object.
        uint8_t     uiDstDimCount;      // A number of dimensions in the destination memory object.
        uint32_t    uiSrcOrigins[3];    // Multi-dimensional offset in the source memory object.
        uint32_t    uiDstOrigins[3];    // Multi-dimensional offset in the destination memory object.
        uint32_t    uiRegion[3];        // Defines multi-dimensional region of the memory object to be copied. 
    };

    struct lrb_cmd_param_kernel
    {
        uint32_t    uiKernelAddr;    // The kernel object address
        uint8_t     uiWorkDim;       // The global work-items number of dimensions (1-3)
        uint32_t    uiGlbWrkOffs[3]; // Currently must be (0, 0, 0). 
        uint32_t    uiGlbWrkSize[3]; // The number of global work-items
        uint32_t    uiLclWrkSize[3]; // Describes the number of work-items in a work-group
        uint32_t    uiArgsSize;      // Size of the args data
        uint32_t*   pArgsValues;     // Pointer to kernel args
    };

}}};    // Intel::OpenCL::LRBAgent
#endif // __LRB_AGENT_COMMON_H__