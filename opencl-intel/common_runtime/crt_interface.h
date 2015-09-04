// Copyright (c) 2006-2007 Intel Corporation
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
//
//  Original author: rjiossy
///////////////////////////////////////////////////////////

#pragma once

#include "crt_types.h"
#include "export/crt_dispatch_table.h"

using namespace CRT_ICD_DISPATCH;
// vendor dispatch table structure

struct _crt_object
{
    SOCLCRTDispatchTable*       crtDispatch;
    void * object;
};

struct _cl_platform_id_crt : public _cl_platform_id, public _crt_object
{
    _cl_platform_id_crt();
};

struct _cl_device_id_crt : public _cl_device_id, public _crt_object
{
    _cl_device_id_crt();
};

struct _cl_context_crt : public _cl_context, public _crt_object
{
    _cl_context_crt();
};

struct _cl_command_queue_crt : public _cl_command_queue, public _crt_object
{
    _cl_command_queue_crt();
};

struct _cl_mem_crt : public _cl_mem, public _crt_object
{
    _cl_mem_crt();
};

struct _cl_program_crt : public _cl_program, public _crt_object
{
    _cl_program_crt();
};

struct _cl_kernel_crt : public _cl_kernel, public _crt_object
{
    _cl_kernel_crt();
};

struct _cl_event_crt : public _cl_event, public _crt_object
{
    _cl_event_crt();
};

struct _cl_sampler_crt : public _cl_sampler, public _crt_object
{
    _cl_sampler_crt();
};

