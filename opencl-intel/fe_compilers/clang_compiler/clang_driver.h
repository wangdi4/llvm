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

///////////////////////////////////////////////////////////
//  clang_driver.h
///////////////////////////////////////////////////////////

#pragma once

#include "task_executor.h"
#include "frontend_api.h"
#include "cl_synch_objects.h"

#include <string>
#include <list>

using namespace Intel::OpenCL::TaskExecutor;

namespace Intel { namespace OpenCL { namespace ClangFE {
	// Internal routines used during library initialization/destroy
	int InitClangDriver();
	int CloseClangDriver();

	typedef std::list<std::string> ArgListType;

	class CompileTask : public ITask
	{
	public:
		CompileTask(FEBuildProgramDesc* pDesc): m_pTask(pDesc){};
		void Execute();
		void Release()
		{
			delete this;
		}
	protected:
		bool OptDebugInfo;
		bool Opt_Disable;
		bool Denorms_Are_Zeros;
		bool Fast_Relaxed_Math;

		void PrepareArgumentList(ArgListType &list, ArgListType &ignored, const char *buildOpts);
		FEBuildProgramDesc* m_pTask;
		static Intel::OpenCL::Utils::OclMutex s_serializingMutex;
	};
}}}
