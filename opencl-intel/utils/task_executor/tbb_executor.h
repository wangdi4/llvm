// Copyright (c) 2006-2009 Intel Corporation
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

/*
*
* File tbb_executor.h
*		Implements interface required for task execution on XNTask sub-system
*
*/
#pragma once

#include "task_executor.h"

#include "tbb/task_group.h"

namespace Intel { namespace OpenCL { namespace TaskExecutor {

	class TBBTaskExecutor : public ITaskExecutor
	{
	public:
		TBBTaskExecutor();
		virtual ~TBBTaskExecutor();
		int	Init(unsigned int uiNumThreads);
		unsigned int GetNumWorkingThreads() const;
		ITaskList* CreateTaskList(bool OOO = false);
		unsigned int	Execute(ITaskBase * pTask);
		bool			WaitForCompletion();

		void Close(bool bCancel);

		void ReleasePerThreadData();
	protected:
		long		m_lRefCount;
		// Independent tasks will be executed by this task group
		static tbb::task_group*				sTBB_executor;
	};

}}}