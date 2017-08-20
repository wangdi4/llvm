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
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES{ } LOSS OF USE, DATA, OR
// PROFITS{ } OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

#include "pipe.h"

using namespace Intel::OpenCL::Framework;

cl_err_code Pipe::Initialize(cl_uint uiPacketSize, cl_uint uiMaxPackets, void* pHostPtr)
{
    m_uiPacketSize = uiPacketSize;
    m_uiMaxPackets = uiMaxPackets;

    const size_t szDim = CalcPipeSize(uiPacketSize, uiMaxPackets);
    cl_err_code err = CL_SUCCESS;
    if (nullptr == pHostPtr)
    {
        err = GenericMemObject::Initialize(CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS, nullptr, 1, &szDim, nullptr, nullptr, 0);
    }
    else
    {
        err = GenericMemObject::Initialize(CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, nullptr, 1, &szDim, nullptr, pHostPtr, 0);
    }
    if (CL_FAILED(err))
    {
        return err;
    }


    IOCLDevBackingStore* pBS;
    err = GetBackingStore(CL_DEV_BS_GET_ALWAYS, &pBS);
    assert(CL_SUCCEEDED(err) && "GetBackingStore failed");

#ifdef BUILD_FPGA_EMULATOR
    pipe_init(pBS->GetRawData(), uiPacketSize, uiMaxPackets);
#else
    pipe_control_intel_t* pipeCtrl = (pipe_control_intel_t*)pBS->GetRawData();
    memset(pipeCtrl, 0, INTEL_PIPE_HEADER_RESERVED_SPACE);
    // one extra packet is required by the pipe implementation
    pipeCtrl->pipe_max_packets_plus_one = uiMaxPackets + 1;
#endif

    return CL_SUCCESS;
}

cl_int Pipe::GetPipeInfo(cl_pipe_info paramName, size_t szParamValueSize, void* pParamValue, size_t* pszParamValueSizeRet)
{
    if (nullptr != pParamValue && szParamValueSize < sizeof(cl_uint))
    {
        return CL_INVALID_VALUE;
    }
    if (nullptr != pszParamValueSizeRet)
    {
        *pszParamValueSizeRet = sizeof(cl_uint);
    }
    switch (paramName)
    {
    case CL_PIPE_PACKET_SIZE:
        if (nullptr != pParamValue)
        {
            *(cl_uint*)pParamValue = m_uiPacketSize;
        }
        break;
    case CL_PIPE_MAX_PACKETS:
        if (nullptr != pParamValue)
        {
            *(cl_uint*)pParamValue = m_uiMaxPackets;
        }
        break;
    default:
        return CL_INVALID_VALUE;
    }
    return CL_SUCCESS;
}
