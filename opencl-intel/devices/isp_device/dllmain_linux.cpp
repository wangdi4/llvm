// Copyright (c) 2013 Intel Corporation
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
//  dllmain.cpp
///////////////////////////////////////////////////////////

#include "stdafx.h"

#include "isp_device.h"
#include "cl_sys_info.h"

#include <libgen.h>

using namespace Intel::OpenCL::ISPDevice;

extern char clISPDEVICE_CFG_PATH[];

void __attribute__ ((constructor)) dll_init(void);
void __attribute__ ((destructor)) dll_fini(void);

void dll_init(void)
{
	char tBuff[PATH_MAX];
	Intel::OpenCL::Utils::GetModulePathName((void*)(ptrdiff_t)dll_init, tBuff, PATH_MAX-1);
	Intel::OpenCL::Utils::safeStrCpy(clISPDEVICE_CFG_PATH, MAX_PATH-1, dirname(tBuff));
	Intel::OpenCL::Utils::safeStrCat(clISPDEVICE_CFG_PATH, MAX_PATH-1, "/cl.cfg");
}

void dll_fini(void)
{
}
