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

#include "vr9.h"

class VR9: public CommonRuntime{};

//|	TEST: VR9.CPUwaitOnEvents (TC-57)
//|
//|	Purpose
//|	-------
//|	
//|	Verify that the host can recieve events from CPU device
//|
//|	Method
//|	------
//|
//|	1. Enqueue on a devices queue commands from VR-9, all commands depend on user event
//|	2. User event is set as CL_COMPLETED by another thread
//|	3. Each enqueued commands returns an event upon completion
//|	4. Wait on all events
//|	
//|	Pass criteria
//|	-------------
//|
//|	Verify CL_COMPLETED status for all commands' completion events on CPU
//|
TEST_F(VR9, CPUwaitOnEvents)
{
	ASSERT_NO_FATAL_FAILURE(waitOnSingleDevice(ocl_descriptor, CL_DEVICE_TYPE_CPU));
}

//|	TEST: VR9.GPUwaitOnEvents (TC-58)
//|
//|	Purpose
//|	-------
//|	
//|	Verify that the host can recieve events from GPU device
//|
//|	Method
//|	------
//|
//|	1. Enqueue on a devices queue commands from VR-9, all commands depend on user event
//|	2. User event is set as CL_COMPLETED by another thread
//|	3. Each enqueued commands returns an event upon completion
//|	4. Wait on all events
//|	
//|	Pass criteria
//|	-------------
//|
//|	Verify CL_COMPLETED status for all commands' completion events on GPU
//|
TEST_F(VR9, GPUwaitOnEvents)
{
	ASSERT_NO_FATAL_FAILURE(waitOnSingleDevice(ocl_descriptor, CL_DEVICE_TYPE_GPU));
}

//|	TEST: CommonRuntime.CPUGPUwaitOnEvents (TC-59)
//|
//|	Purpose
//|	-------
//|	
//|	Verify that the host can recieve events from both CPU and GPU device
//|
//|	Method
//|	------
//|
//|	1. Enqueue on each devices queue commands from VR-9, all commands depend on user event
//|	2. User event is set as CL_COMPLETED by another thread
//|	3. Each enqueued commands returns an event upon completion
//|	4. Wait on all events
//|	
//|	Pass criteria
//|	-------------
//|
//|	Verify CL_COMPLETED status for all commands' completion events on CPU and GPU
//|
TEST_F(VR9, CPUGPUwaitOnEvents)
{
	ASSERT_NO_FATAL_FAILURE(waitOnBothDevices(ocl_descriptor));
}

//|	TEST: CommonRuntime.CPU100KernelswaitOnEvents (TC-60)
//|
//|	Purpose
//|	-------
//|	
//|	Verify that the host can recieve many events (100) from CPU device
//|
//|	Method
//|	------
//|
//|	1. Enqueue on CPU device an instance of a kernel 100 times
//|	   Each enqueued kernel is dependant of user event, and returns an event upon completion
//|	2. User event is set as CL_COMPLETED by another thread
//|	3. Wait on all kernel completion events
//|	
//|	Pass criteria
//|	-------------
//|
//|	Verify CL_COMPLETED status for all kernels' completion events on CPU
//|
TEST_F(VR9, CPU100KernelswaitOnEvents)
{
	ASSERT_NO_FATAL_FAILURE(executeKernelOnSingleDevice(ocl_descriptor, CL_DEVICE_TYPE_CPU));
}

//|	TEST: VR9.GPU100KernelswaitOnEvents (TC-61)
//|
//|	Purpose
//|	-------
//|	
//|	Verify that the host can recieve many events (100) from GPU device
//|
//|	Method
//|	------
//|
//|	1. Enqueue on CPU device an instance of a kernel 100 times
//|	   Each enqueued kernel is dependant of user event, and returns an event upon completion
//|	2. User event is set as CL_COMPLETED by another thread
//|	3. Wait on all kernel completion events
//|	
//|	Pass criteria
//|	-------------
//|
//|	Verify CL_COMPLETED status for all kernels' completion events on GPU
//|
TEST_F(VR9, GPU100KernelswaitOnEvents)
{
	ASSERT_NO_FATAL_FAILURE(executeKernelOnSingleDevice(ocl_descriptor, CL_DEVICE_TYPE_GPU));
}

//|	TEST: VR9.CPUGPU200KernelswaitOnEvents (TC-62)
//|
//|	Purpose
//|	-------
//|	
//|	Verify that the host can recieve many events (200) from CPU and GPU devices (100 from each device)
//|
//|	Method
//|	------
//|
//|	1. Enqueue on each device an instance of a kernel 100 times
//|	   Each enqueued kernel is dependant of user event, and returns an event upon completion
//|	2. User event is set as CL_COMPLETED by another thread
//|	3. Wait on all kernel completion events
//|	
//|	Pass criteria
//|	-------------
//|
//|	Verify CL_COMPLETED status for all kernels' completion events on both CPU and GPU
//|
TEST_F(VR9, CPUGPU200KernelswaitOnEvents)
{
	ASSERT_NO_FATAL_FAILURE(executeKernelOnBothDevices(ocl_descriptor));
}