// INTEL CONFIDENTIAL
//
// Copyright 2006-2018 Intel Corporation.
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

