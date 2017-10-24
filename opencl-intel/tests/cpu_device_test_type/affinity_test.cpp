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

#include <iostream>
#include <algorithm>
#include <vector>
#include "cpu_dev_test.h"
#include "../framework_test_type/test_utils.h"
#include "cl_synch_objects.h"
#include "cl_sys_info.h"
#include "task_executor.h"

using Intel::OpenCL::Utils::OclBinarySemaphore;
using std::vector;
using std::count;

enum EProcessorOccupied
{
    NOT_OCCUPIED,
    OCCUPIED
};

struct NativeKernelParams
{

    NativeKernelParams(unsigned long ulNumProcessors, unsigned long ulNumProcessorsInDev, unsigned int uiMasterCpuId, affinityMask_t* pMask) : m_ulNumProcessors(ulNumProcessors),m_ulNumKernels(ulNumProcessorsInDev), m_processorsOccupied(ulNumProcessors),
        m_bDoubleAffinity(false), m_uiMasterCpuId(uiMasterCpuId), m_pMask(pMask)
    {
        std::fill(m_processorsOccupied.begin(), m_processorsOccupied.end(), NOT_OCCUPIED);
    }

    unsigned long                       m_ulNumProcessors;
    unsigned long                       m_ulNumKernels;
    vector<EProcessorOccupied>          m_processorsOccupied;
    volatile bool                       m_bDoubleAffinity;
    volatile unsigned int               m_uiMasterCpuId;
    affinityMask_t*                     m_pMask;
    Intel::OpenCL::Utils::AtomicCounter m_cnt;
};

static void CL_CALLBACK NativeKernel(void* ptr)
{
	NativeKernelParams* params = *(NativeKernelParams**)ptr;
	const unsigned int uiCpuId = Intel::OpenCL::Utils::GetCpuId();

	if (uiCpuId != params->m_uiMasterCpuId && params->m_processorsOccupied[uiCpuId])
	{
		cout << "HW thread " << uiCpuId << " is already occupied" << endl;
		params->m_bDoubleAffinity = true;
	}
	else
	{
		params->m_processorsOccupied[uiCpuId] = OCCUPIED;
	}		
	params->m_cnt++;
	int count = 1;
	while (params->m_cnt < (long)params->m_ulNumKernels)
	{
	  if ( ++count == 10000 )
	  {
	      printf("HW thread %d, num joined threads=%d\n", uiCpuId, (int) params->m_cnt);fflush(0);
	  }
	}	
}

extern CPUTestCallbacks g_dev_callbacks;

class AffinityTestCallback : public IOCLFrameworkCallbacks
{
public:

	AffinityTestCallback(unsigned long ulNumKernels, OclBinarySemaphore* pSem) : m_ulNumKernels(ulNumKernels), m_pSem(pSem) { }

	void clDevCmdStatusChanged(cl_dev_cmd_id cmd_id, void* data, cl_int cmd_status, cl_int completion_result, cl_ulong timer)
	{
		if (NULL != m_pSem && CL_COMPLETE == cmd_status)
		{
			if (++m_cnt == (long)m_ulNumKernels)
			{
				m_pSem->Signal();
			}
		}
	}	

    Intel::OpenCL::TaskExecutor::ITaskExecutor* clDevGetTaskExecutor() { return Intel::OpenCL::TaskExecutor::GetTaskExecutor(); }

private:

	const unsigned long m_ulNumKernels;
	Intel::OpenCL::Utils::AtomicCounter m_cnt;
	OclBinarySemaphore* const m_pSem;
	
};

static bool AffinityTestForDevice(cl_dev_subdevice_id dev, NativeKernelParams* params, bool bMasterJoinsWork)
{
	bool bRes = true;
	OclBinarySemaphore sem;
	AffinityTestCallback callback(params->m_ulNumKernels, bMasterJoinsWork ? NULL : &sem);

	g_dev_callbacks.AddUserCallback(callback);
	try
	{			
		cl_dev_err_code err;
		cl_dev_cmd_list list;		
		
		err = dev_entry->clDevCreateCommandList(CL_DEV_LIST_ENABLE_OOO, dev, &list);
		CheckException("clDevCreateCommandList", CL_DEV_SUCCESS, err);					

		cl_dev_cmd_desc* cmdDescArr = new cl_dev_cmd_desc[params->m_ulNumKernels];

		cl_dev_cmd_param_native param_native;
		param_native.func_ptr = NativeKernel;
		param_native.mem_num = 0;
		param_native.args = sizeof(void*);
		param_native.argv = (void*)&params;

        for (unsigned long i = 0; i < params->m_ulNumKernels; i++)
        {
            cmdDescArr[i].type = CL_DEV_CMD_EXEC_NATIVE;
            cmdDescArr[i].params = &param_native;
            cmdDescArr[i].param_size = sizeof(param_native);
        }

        for (unsigned long i = 0; i < params->m_ulNumKernels; i++)
        {
            cl_dev_cmd_desc* cmdDescParam[] = { &cmdDescArr[i]};
            err = dev_entry->clDevCommandListExecute(list, cmdDescParam, 1);
            CheckException("clDevCommandListExecute", CL_DEV_SUCCESS, err);
        }

		err = dev_entry->clDevFlushCommandList(list);
		CheckException("clDevFlushCommandList", CL_DEV_SUCCESS, err);

		if (!bMasterJoinsWork)
		{
			sem.Wait();
			// we can't ensure that the commands have really finished their execution here, so we can't release them or the list
		}
		else
		{
			err = dev_entry->clDevCommandListWaitCompletion(list, NULL);
			CheckException("clDevCommandListWaitCompletion", CL_DEV_SUCCESS, err);			
			for (unsigned long i = 0; i < params->m_ulNumKernels; i++)
			{
				err = dev_entry->clDevReleaseCommand(&cmdDescArr[i]);
				CheckException("clDevReleaseCommand", CL_DEV_SUCCESS, err);
			}
			err = dev_entry->clDevReleaseCommandList(list);
			CheckException("clDevReleaseCommandList", CL_DEV_SUCCESS, err);
		}
		delete[] cmdDescArr;
		
#ifndef _WIN32
        if (NULL != params->m_pMask)
        {
            const vector<EProcessorOccupied>::difference_type numOccupiedProcessors = count(params->m_processorsOccupied.begin(), params->m_processorsOccupied.end(), OCCUPIED);
            for (unsigned long i = 0; i < params->m_ulNumProcessors; ++i)
            {                
                bRes &= !params->m_processorsOccupied[i] || CPU_ISSET(i, params->m_pMask);                    
            }            
            bRes &= 
                // in root device we allow one proccesor in the mask to be not occupied, since the number of workers is the number of processors in the device - 1
                (!bMasterJoinsWork || CPU_COUNT(params->m_pMask) == numOccupiedProcessors || CPU_COUNT(params->m_pMask) - 1 == numOccupiedProcessors) &&
                 (bMasterJoinsWork || CPU_COUNT(params->m_pMask) == numOccupiedProcessors);
        }
#endif
	}
	catch (const exception&)
	{
		bRes = false;
	}
	g_dev_callbacks.RemoveUserCallback(callback);
	if (bRes)
	{
		bRes = !params->m_bDoubleAffinity;
	}
	return bRes;
}

bool AffinityRootDeviceTest(affinityMask_t* pMask)
{
    unsigned long numProcessors = 0;
    unsigned long numUsedProcessors = numProcessors; 
    unsigned long myCPU = 0;
#ifndef _WIN32
    numProcessors = sysconf(_SC_NPROCESSORS_CONF);
    numUsedProcessors = numProcessors;
    if (NULL != pMask) 
    {
        numUsedProcessors = CPU_COUNT(pMask);

        //use the first CPU set in mask
        //mask is guaranteed to have at least one set bit, so no risk of infinite loop
        do
        {
            if (CPU_ISSET(myCPU, pMask)) break;
            ++myCPU;
        } while (true); 
    } 
#endif
    clSetThreadAffinityToCore(myCPU, clMyThreadId());
    NativeKernelParams rootDevParams(numProcessors, numUsedProcessors, myCPU, pMask);

    const bool res = AffinityTestForDevice(NULL, &rootDevParams, true);
    clResetThreadAffinityMask(clMyThreadId());
    return res;
}

#define NUM_SUB_DEVS 2

bool AffinitySubDeviceTest(affinityMask_t* pMask)
{
    if (NULL != pMask)
    {
        printf("Not running AffinitySubDeviceTest due to affinity mask being set. Don't mix fission and numactl or similar APIs\n");fflush(0);
        return true;
    }
    const unsigned long ulNumProcessors = Intel::OpenCL::Utils::GetNumberOfProcessors();
    const unsigned long ulSubDevSize = (ulNumProcessors-1) / NUM_SUB_DEVS;

    if ( 0 == ulSubDevSize )
    {
        printf("Not enough available threads. Test skipped\n");fflush(0);
        return true;
    }

    clSetThreadAffinityToCore(Intel::OpenCL::Utils::GetCpuId(), clMyThreadId());

    printf("AffinitySubDeviceTest: NumProcessors=%d, SubDeviceSize=%d, NUM_SUB_DEVS=%d\n",
        (int)ulNumProcessors, (int)ulSubDevSize, (int) NUM_SUB_DEVS);fflush(0);

    try
    {
        cl_dev_subdevice_id subDevs[NUM_SUB_DEVS];
        cl_uint uiNumDevices = 1;
        cl_dev_err_code err;

        for (size_t i = 0; i < NUM_SUB_DEVS; i++)
        {
            std::vector<size_t> requestedUnits;

            for (size_t j = i * ulNumProcessors / NUM_SUB_DEVS; j < (i + 1) * (ulNumProcessors / NUM_SUB_DEVS); j++)
            {
              assert(j < ulNumProcessors);
              requestedUnits.push_back(j);
            }

            err = dev_entry->clDevPartition(CL_DEV_PARTITION_BY_NAMES, 1, NULL, &uiNumDevices, &requestedUnits, &subDevs[i]);
            CheckException("clDevPartition", CL_DEV_SUCCESS, err);
        }

        bool res = true;
        for (size_t i = 0; i < NUM_SUB_DEVS; i++)
        {
            NativeKernelParams subDevParams(ulNumProcessors, ulSubDevSize, Intel::OpenCL::Utils::GetCpuId(), pMask);
            if (!AffinityTestForDevice(subDevs[i], &subDevParams, false))
            {
              res = false;
            }
        }

        for (size_t i = 0; i < NUM_SUB_DEVS; i++)
        {
            err = dev_entry->clDevReleaseSubdevice(subDevs[i]);
            CheckException("clDevReleaseSubdevice", CL_DEV_SUCCESS, err);
        }

        clResetThreadAffinityMask(clMyThreadId());
        return res;
    }
    catch (const exception&)
    {
        return false;
    }
    return true;
}
