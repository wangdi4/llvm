// Copyright (c) 2006-2008 Intel Corporation
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

#pragma once

#include "cl_device_api.h"
#include "notification_port.h"

namespace Intel { namespace OpenCL { namespace MICDevice {

class CommandList;
class InOrderCommandList;
class OutOfOrderCommandList;

class Command : public NotificationPort::CallBack
{

public:

    virtual cl_dev_err_code execute(CommandList* pCommandList) = 0;

protected:

    Command();

    COIBARRIER m_completionBarrier;

};


class NDRange1 : public Command
{

public:

    static cl_dev_err_code Create(bool isInOrder, Command** pOutCommand);

protected:

    NDRange1();
};


class InOrderNDRange : public NDRange1
{

public:

    InOrderNDRange();

	cl_dev_err_code execute(CommandList* pCommandList);

	void fireCallBack(void* arg);
};

class OutOfOrderNDRange : public NDRange1
{

public:

    OutOfOrderNDRange();

	cl_dev_err_code execute(CommandList* pCommandList);

	void fireCallBack(void* arg);
};


class FailureNotification : public Command
{

public:

    FailureNotification();

	cl_dev_err_code execute(CommandList* pCommandList);

	void fireCallBack(void* arg);
};

}}}