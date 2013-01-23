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
#include "TaskExecutorTester.h"
#include "gtest/gtest.h"

using namespace std;

ITaskExecutor* TaskExecutorTester::m_pTaskExecutor = NULL;

static bool RunSomeTasks(void* pSubdevData, bool bOutOfOrder, ITaskExecutor& taskExecutor, bool bIsFullDevice)
{
    CommandListCreationParam cmdListCreationParam;

    cmdListCreationParam.isOOO = bOutOfOrder;
    cmdListCreationParam.isSubdevice = pSubdevData != NULL;
    SharedPtr<ITaskList> pTaskList = taskExecutor.CreateTaskList(&cmdListCreationParam, pSubdevData);
    if (NULL == pTaskList)
    {
        cerr << "TaskExecutor::CreateTaskList returned NULL" << endl;
        return false;
    }	
    for (unsigned int uiNumDims = 1; uiNumDims <= 3; uiNumDims++)
    {
        std::vector<SharedPtr<TesterTaskSet> > tasks(1000);
        for (size_t i = 0; i < tasks.size(); i++)
        {
            SharedPtr<TesterTaskSet> pTaskSet = TesterTaskSet::Allocate(uiNumDims);
            pTaskList->Enqueue(pTaskSet);
            tasks[i] = pTaskSet;
        }        
        if (!pTaskList->Flush())
        {
            cerr << "Flush failed failed" << endl;
            return false;
        }

		const bool bWaitShouldBeSupported = NULL == pSubdevData || bIsFullDevice;
        for (size_t i = 0; i < tasks.size(); i++)
        {
            const te_wait_result res = pTaskList->WaitForCompletion(tasks[i]);
			
            if ((!bWaitShouldBeSupported && res != TE_WAIT_NOT_SUPPORTED) || (bWaitShouldBeSupported && res != TE_WAIT_COMPLETED))
            {
                cerr << "WaitForCompletion doesn't return result as expected" << endl;
                return false;
            }
        }
        
        const te_wait_result res = pTaskList->WaitForCompletion(NULL);
        if ((!bWaitShouldBeSupported && res != TE_WAIT_NOT_SUPPORTED) || (bWaitShouldBeSupported && res != TE_WAIT_COMPLETED))
        {
            cerr << "WaitForCompletion doesn't return result as expected" << endl;
            return false;
        }
    }    
    return true;
}

// a value of 0 in uiSubdevSize designates running on the root device

static bool RunSubdeviceTest(unsigned int uiSubdevSize, ITaskExecutor& taskExecutor)
{
    CommandListCreationParam cmdListCreationParam;

    cmdListCreationParam.isOOO = false;
    cmdListCreationParam.isSubdevice = true;    
    taskExecutor.Activate();
    std::vector<unsigned int> legalCores;
    for (unsigned int i = 0; i < uiSubdevSize; i++)
    {
        legalCores.push_back(i);
    }
    TesterAffinityChangeObserver observer;
    void* const pSubdevData = uiSubdevSize > 0 ? taskExecutor.CreateSubdevice(uiSubdevSize, &legalCores[0], observer) : NULL;
    if (NULL == pSubdevData && uiSubdevSize > 0)
    {
        cerr << "CreateSubdevice returned NULL" << endl;
        return false;
    }
	const bool bResult = RunSomeTasks(pSubdevData, false, taskExecutor, uiSubdevSize == taskExecutor.GetNumWorkingThreads());

    if (0 == uiSubdevSize)   // subdevices with size 1 don't support WaitForCompletion
    {
        SharedPtr<TesterTaskSet> pTaskSet = TesterTaskSet::Allocate(1);
        taskExecutor.Execute(pTaskSet, pSubdevData);
        taskExecutor.WaitForCompletion(pTaskSet.GetPtr(), pSubdevData);
        if (!pTaskSet->IsCompleted())
        {
            cerr << "pTaskSet is not completed after taskExecutor.Execute" << endl;
            return false;
        }
    }
    taskExecutor.WaitUntilEmpty(pSubdevData);
    taskExecutor.ReleaseSubdevice(pSubdevData);
    taskExecutor.Deactivate();
    return bResult;
}

bool SubdeviceTest()
{
    TaskExecutorTester tester;
    return RunSubdeviceTest(tester.GetTaskExecutor().GetNumWorkingThreads() / 2, tester.GetTaskExecutor());
}

bool SubdeviceSize1Test()
{
    TaskExecutorTester tester;
    return RunSubdeviceTest(1, tester.GetTaskExecutor());
}

bool SubdeviceFullDevice()
{
	TaskExecutorTester tester;
	return RunSubdeviceTest(tester.GetTaskExecutor().GetNumWorkingThreads(), tester.GetTaskExecutor());
}

bool BasicTest()
{
    TaskExecutorTester tester;    

    ITaskExecutor& taskExecutor = tester.GetTaskExecutor();
    taskExecutor.Activate();
    const bool bResult = RunSubdeviceTest(0, taskExecutor);
    taskExecutor.Deactivate();
    return bResult;
}

bool OOOTest()
{
    TaskExecutorTester tester;

    ITaskExecutor& taskExecutor = tester.GetTaskExecutor();
    taskExecutor.Activate();
    const bool bResult = RunSomeTasks(NULL, true, taskExecutor, true);
    taskExecutor.Deactivate();
    return bResult;
}

TEST(TaskExecutorTestType, Test_Basic)
{
	EXPECT_TRUE(BasicTest());
}

TEST(TaskExecutorTestType, Test_Subdevices)
{
    EXPECT_TRUE(SubdeviceTest());
}

TEST(TaskExecutorTestType, Test_SubdeviceSize1)
{
    EXPECT_TRUE(SubdeviceSize1Test());
}

TEST(TaskExecutorTestType, Test_SubdeviceFullDevice)
{
	EXPECT_TRUE(SubdeviceFullDevice());
}

TEST(TaskExecutorTestType, Test_OOO)
{
    EXPECT_TRUE(OOOTest());
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
   return RUN_ALL_TESTS();
}
