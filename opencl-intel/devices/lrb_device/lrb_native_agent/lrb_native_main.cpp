// Copyright (c) 2006-2009 Intel Corporation
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
//  lrb_native_main.cpp
///////////////////////////////////////////////////////////
// #include "lrb_agent_common.h"
#include "lrb_native_agent.h"
#include <stdio.h>

using namespace Intel::OpenCL::LRBAgent;

/******************************************************************
 * Init function is used to initiate the device agent.
 *
 ******************************************************************/
#ifdef __cplusplus
extern "C"
#endif
XNNATIVELIBEXPORT
void LrbInitNativeDevice(XN_BUFFER_LIST xnBufs, void* pMiscData, uint16_t uiMiscDataLen)
{ 
}


/******************************************************************
 * Release function is used to free resources in the device.
 *
 ******************************************************************/
#ifdef __cplusplus
extern "C"
#endif
XNNATIVELIBEXPORT
void ReleaseNativeDevice(XN_BUFFER_LIST xnBufs, void* pMiscData, uint16_t uiMiscDataLen)
{ 
}


/******************************************************************
 * This is the entry function for build
 *
 ******************************************************************/
#ifdef __cplusplus
extern "C"
#endif
XNNATIVELIBEXPORT
void NativeBuildProgram(XN_BUFFER_LIST xnBufs, void* pMiscData, uint16_t uiMiscDataLen)
{ 
    printf("LRB Native: NativeBuildProgram enter\n");
    LrbNativeAgent* agent = LrbNativeAgent::GetInstance();
    agent->BuildProgram(xnBufs, pMiscData, uiMiscDataLen);
    return;
}

/******************************************************************
 * This is the entry function for execution
 *
 ******************************************************************/
#ifdef __cplusplus
extern "C"
#endif
XNNATIVELIBEXPORT
void NativeExecuteCmds(XN_BUFFER_LIST xnBufs, void* pMiscData, uint16_t uiMiscDataLen)
{ 
    // TODO: update command status on the host
    printf("LRB Native: NativeExecuteCmds enter\n");
    LrbNativeAgent* agent = LrbNativeAgent::GetInstance();
    agent->ExecuteCommands(xnBufs, pMiscData, uiMiscDataLen);
    return;
}

/******************************************************************
 * Native main function, creates the native device
 *
 ******************************************************************/
XNNATIVELIBEXPORT int main()
{
    printf("LRB Native: Main entered\n");
    LrbNativeAgent* agent = LrbNativeAgent::GetInstance();
    agent->Initialize();
    // Native main function waits until end
    // When native application is unloaded, the host agent is expected to send terminate message.
    // When message is sent, This function release the LrbNativeAgent and exit.
    agent->WaitForCompletion();

    // Destroy the agent
    LrbNativeAgent::Destroy();
    printf("LRB Native: Main exit\n");
    return 0;
}

