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
#pragma once

#include <map>
#include <vector>
#include <string>

#include <ICLDevBackendProgram.h>
#include <cl_device_api.h>
#include <task_executor.h>


using namespace Intel::OpenCL::DeviceBackend;

namespace Intel { namespace OpenCL { namespace CPUDevice {

class TaskDispatcher;

class BuiltInProgram : public ICLDevBackendProgram_
{
public:	
	BuiltInProgram() {};

	cl_dev_err_code ParseFunctionList(const char* szBuiltInKernelList);

	unsigned long long int GetProgramID() const {return (unsigned long long int)this;}
	const char* GetBuildLog() const	{return NULL;}
    const ICLDevBackendCodeContainer* GetProgramCodeContainer() const {return NULL;}

	cl_dev_err_code GetKernelByName(
        const char* pKernelName, 
        const ICLDevBackendKernel_** ppKernel) const;

    int GetKernelsCount() const { return (int)m_mapKernels.size(); }

	virtual cl_dev_err_code	GetKernel(
        int kernelIndex, 
        const ICLDevBackendKernel_** pKernel) const;

    virtual const ICLDevBackendProgramJITCodeProperties* GetProgramJITCodeProperties() const {return NULL;}

protected:
	// Stores a list of MKL kernels perticipated in the Built-In kernel program
	typedef std::map<std::string, ICLDevBackendKernel_*> BIKernelsMap_t;
	BIKernelsMap_t	m_mapKernels;
	std::vector<ICLDevBackendKernel_*>	m_listKernels;
};

class IBuiltInKernel : public ICLDevBackendKernel_
{
public:

	virtual cl_dev_err_code CreateBIKernelTask(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd, Intel::OpenCL::TaskExecutor::ITaskBase* *pTask) = 0;
};

typedef cl_dev_err_code fn_BuiltInFunctionCreate(IBuiltInKernel* *ppBIKernel);

class BuiltInKernelRegistry
{
public:
	static	BuiltInKernelRegistry* GetInstance();

	BuiltInKernelRegistry():m_stKernelNameStrLength(0) {}
	void	RegisterBuiltInKernel(const char* szBIKernelName, fn_BuiltInFunctionCreate* pCreator);

	void	GetBuiltInKernelList(char* szBIKernelList, size_t stSize) const;
	size_t	GetBuiltInKernelListSize() const {return m_stKernelNameStrLength+1;}

	cl_dev_err_code CreateBuiltInProgram(const char* szKernelList, ICLDevBackendProgram_* *ppProgram);

protected:
	friend class BuiltInProgram;
	cl_dev_err_code CreateBuiltInKernel(const char* szMKLFuncName, IBuiltInKernel* *pMKLExecutor) const;

	typedef std::map<std::string, fn_BuiltInFunctionCreate*>	KernelCreatorMap_t;
	
	KernelCreatorMap_t	m_mapKernelCreators;
	size_t				m_stKernelNameStrLength;			// Holds the total size of the list of built-in functions

	static BuiltInKernelRegistry*	g_pMKLRegistery;
};

#define REGISTER_BUILTIN_KERNEL(BI_KENREL_NAME,BI_CREATOR_FUNCTION) \
	struct BI_KENREL_NAME##CreatorClassRegister\
	{\
		BI_KENREL_NAME##CreatorClassRegister()\
		{\
			BuiltInKernelRegistry::GetInstance()->RegisterBuiltInKernel(#BI_KENREL_NAME, BI_CREATOR_FUNCTION);\
		}\
	};\
	BI_KENREL_NAME##CreatorClassRegister class##BI_KENREL_NAME##CreatorClassRegister;

}}}