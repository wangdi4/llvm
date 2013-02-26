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

struct DeviceAuto
{
    SharedPtr<ITEDevice> deviceHandle;

    // root device constructor
    DeviceAuto( TaskExecutorTester& taskExecutorTester ) : is_root(true)
    {
        ITaskExecutor* taskExecutor = taskExecutorTester.GetTaskExecutor();
        deviceHandle = taskExecutor->CreateRootDevice(RootDeviceCreationParam(TE_AUTO_THREADS, TE_ENABLE_MASTERS_JOIN, 0),NULL, &taskExecutorTester);
    };

    // sub device constructor
    DeviceAuto( const SharedPtr<ITEDevice> root, unsigned int units ) : is_root( false )
    {
        deviceHandle = root->CreateSubDevice(units);
    }

    ~DeviceAuto()
    {
        if (NULL != deviceHandle)
        {
            //deviceHandle->WaitUntilEmpty();
            if (is_root)
            {
                deviceHandle->ResetObserver();
            }
        }
    }
private:
    bool is_root;
};

static bool RunSomeTasks(const SharedPtr<ITEDevice>& pSubdevData, bool bOutOfOrder, bool bIsFullDevice)
{
    SharedPtr<ITaskList> pTaskList = pSubdevData->CreateTaskList( bOutOfOrder ? TE_CMD_LIST_OUT_OF_ORDER : TE_CMD_LIST_IN_ORDER );
    if (NULL == pTaskList)
    {
        cerr << "TaskExecutor::CreateTaskList returned NULL" << endl;
        return false;
    }	
	const bool bWaitShouldBeSupported = bIsFullDevice;
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

static bool RunSubdeviceTest(unsigned int uiSubdevSize, TaskExecutorTester& taskExecutorTester)
{
    DeviceAuto  rootDeviceHandle( taskExecutorTester );
    bool use_subdevice = (uiSubdevSize > 0);
    DeviceAuto  subDevHandle( rootDeviceHandle.deviceHandle, uiSubdevSize );
    SharedPtr<ITEDevice> pSubdevData = use_subdevice ? subDevHandle.deviceHandle : rootDeviceHandle.deviceHandle;
    if (NULL == pSubdevData && use_subdevice)
    {
        cerr << "CreateSubdevice returned NULL" << endl;
        return false;
    }
	const bool bResult = RunSomeTasks(pSubdevData, false, !use_subdevice);

    if (0 == uiSubdevSize)   // subdevices with size 1 don't support WaitForCompletion
    {
        SharedPtr<TesterTaskSet> pTaskSet = TesterTaskSet::Allocate(1);
        SharedPtr<ITaskList>     pTaskList = pSubdevData->CreateTaskList( TE_CMD_LIST_IN_ORDER );

        pTaskList->Enqueue(pTaskSet);
        pTaskList->Flush();
        pTaskList->WaitForCompletion(pTaskSet.GetPtr());
        if (!pTaskSet->IsCompleted())
        {
            cerr << "pTaskSet is not completed after taskExecutor.Execute" << endl;
            return false;
        }
    }
    return bResult;
}

bool SubdeviceTest()
{
    TaskExecutorTester tester;
    return RunSubdeviceTest(tester.GetTaskExecutor()->GetMaxNumOfConcurrentThreads() / 2, tester);
}

bool SubdeviceSize1Test()
{
    TaskExecutorTester tester;
    return RunSubdeviceTest(1, tester);
}

bool SubdeviceFullDevice()
{
	TaskExecutorTester tester;
	return RunSubdeviceTest(tester.GetTaskExecutor()->GetMaxNumOfConcurrentThreads(), tester);
}

bool BasicTest()
{
    TaskExecutorTester tester;    

    DeviceAuto rootDeviceHandle( tester );
    const bool bResult = RunSubdeviceTest(0, tester);
    return bResult;
}

bool OOOTest()
{
    TaskExecutorTester tester;

    DeviceAuto rootDeviceHandle( tester );
    const bool bResult = RunSomeTasks(rootDeviceHandle.deviceHandle, true, true);
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
