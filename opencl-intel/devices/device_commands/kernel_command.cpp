// Copyright (c) 2006-2012 Intel Corporation
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

#include "kernel_command.h"

using namespace Intel::OpenCL::DeviceCommands;
using Intel::OpenCL::Utils::OclAutoMutex;

void KernelCommand::AddChildKernel(const SharedPtr<KernelCommand>& child, kernel_enqueue_flags_t flags)
{
	ASSERT_RET(CLK_ENQUEUE_FLAGS_NO_WAIT == flags || CLK_ENQUEUE_FLAGS_WAIT_KERNEL == flags || CLK_ENQUEUE_FLAGS_WAIT_WORK_GROUP == flags,
		"CLK_ENQUEUE_FLAGS_NO_WAIT == flags || CLK_ENQUEUE_FLAGS_WAIT_KERNEL == flags || CLK_ENQUEUE_FLAGS_WAIT_WORK_GROUP == flags");

	if (flags != CLK_ENQUEUE_FLAGS_NO_WAIT)
	{
		child->AddDependency();
		if (CLK_ENQUEUE_FLAGS_WAIT_KERNEL == flags)
		{
			// we'll move the children in this list to m_waitingChildrenForKernel when the work-group finishes execution - this saves us a lot of contention
			GetWaitingChildrenForParentInWg().push_back(child);
		}
		else
		{
			GetWaitingChildrenForWG().push_back(child);
		}
	}
}

void KernelCommand::WaitForChildrenCompletion()
{ 
	// since new children can't be added anymore at this pointer, there is no need for synchronization
	if (m_waitingChildrenForKernel.empty())
	{
		return;
	}
	for (std::vector<SharedPtr<KernelCommand> >::iterator iter = m_waitingChildrenForKernel.begin(); iter != m_waitingChildrenForKernel.end(); iter++)
	{
		(*iter)->NotifyCommandFinished(GetError());
	}
	m_waitingChildrenForKernel.clear();
	m_childrenTaskGroup->WaitForAll();
}

void KernelCommand::WgFinishedExecution()
{
	// move waiting children for parent from WG list to the global list
	std::vector<SharedPtr<KernelCommand> >& waitingChildrenForParentInWg = GetWaitingChildrenForParentInWg();
	if (!waitingChildrenForParentInWg.empty())
	{
		OclAutoMutex mutex(&m_muChildrenForKernel);
		std::vector<SharedPtr<KernelCommand> >::iterator listEnd = m_waitingChildrenForKernel.end();
		m_waitingChildrenForKernel.resize(m_waitingChildrenForKernel.size() + waitingChildrenForParentInWg.size());	
		std::copy(waitingChildrenForParentInWg.begin(), waitingChildrenForParentInWg.end(), listEnd);
		waitingChildrenForParentInWg.clear();
	}
	
	// since new children can't be added anymore at this pointer, there is no need for synchronization
	std::vector<SharedPtr<KernelCommand> >& waitingChildrenForWg = GetWaitingChildrenForWG();
	if (!waitingChildrenForWg.empty())
	{
		for (std::vector<SharedPtr<KernelCommand> >::const_iterator iter = waitingChildrenForWg.begin(); iter != waitingChildrenForWg.end(); iter++)
		{
			(*iter)->NotifyCommandFinished(GetError());
		}
		waitingChildrenForWg.clear();
	}	
}

void KernelCommand::Launch()
{
    assert(GetParentTaskGroup() != NULL && "Launch shouldn't be called for KernelCommands enqueued from the host");
    GetList()->Spawn(GetMyTaskBase(), *GetParentTaskGroup());
}
