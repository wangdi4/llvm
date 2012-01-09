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

#include "vr7.h"

class VR7_Fission: public FissionWrapper{};

//|	TEST: VR7_Fission.CPUThenGPUBufferSynch (TC-112)
//|
//|	Purpose
//|	-------
//|	
//|	Verify that it is possible to synchronize between commands enqueued to CPU subdevice and 
//| commands enqueued to GPU queues
//|
//|	Method
//|	------
//|
//|	1. Initialize all OpenCL resources	
//|	2. Call helper function testDeviceBufferSynchBody to enqueue kernels in syncronized manner
//|
//|	Pass criteria
//|	-------------
//|
//|	Validate that it is possible to syncronize execution of CPU subdevice kernel before a GPU kernel
//|
TEST_F(VR7_Fission, CPUThenGPUBufferSynch)
{
	// initial_pattern - initial array element content
	int initial_pattern = 4;
	// all elements of array matching initial_pattern will be replaced with cpu_replacement in CPU
	int cpu_replacement = 3;
	// all elements of array matching cpu_replacement will be replaced with gpu_replacement in GPU
	int gpu_replacement = 5;

	ASSERT_NO_FATAL_FAILURE(createAndMergeWithGPU(ocl_descriptor));

	// allocate OpenCL objects and determined devices order and execute kernels	
	// execute kernels
	ASSERT_NO_FATAL_FAILURE(testDeviceBufferSynchBody(ocl_descriptor,
		0, initial_pattern, cpu_replacement, gpu_replacement));
}

//|	TEST: VR7_Fission.GPUThenCPUBufferSynch (TC-113)
//|
//|	Purpose
//|	-------
//|	
//|	Verify that it is possible to synchronize between commands enqueued to CPU subdevice and commands enqueued to GPU queues
//|
//|	Method
//|	------
//|
//|	1. Iinitialize all OpenCL resources	
//|	2. Call helper function testDeviceBufferSynchBody to enqueue kernels in syncronized manner 
//|
//|	Pass criteria
//|	-------------
//|
//|	Validate that it is possible to syncronize execution of GPU kernel before a CPU subdevice kernel
//|
TEST_F(VR7_Fission, GPUThenCPUBufferSynch)
{
	// initial_pattern - initial array element content
	int initial_pattern = 4;
	// all elements of array matching initial_pattern will be replaced with cpu_replacement in CPU
	int cpu_replacement = 3;
	// all elements of array matching cpu_replacement will be replaced with gpu_replacement in GPU
	int gpu_replacement = 5;

	ASSERT_NO_FATAL_FAILURE(createAndMergeWithGPU(ocl_descriptor));

	// allocate OpenCL objects and determined devices order and execute kernels	
	// execute kernels
	ASSERT_NO_FATAL_FAILURE(testDeviceBufferSynchBody(ocl_descriptor,
		1, initial_pattern, gpu_replacement, cpu_replacement));
}

//|	TEST: VR7_Fission.SharedBufferCPUGPU (TC-114)
//|
//|	Purpose
//|	-------
//|	
//|	Verify the ability of different devices (CPU and GPU) to work in synchronization on the same shared memory
//|
//|	Method   
//|	------
//|
//|	1. Create cl_mem buffer on a shared context (shared context for CPU subdevice and GPU) of size arraySize*2
//|	2. For each device create kernel with buffer from step 1 as a parameter
//| 3. Enqueue kernels on queues of each device while kernels are waiting for user event
//|	   Enqueue map buffer waiting for kernel en of execution events.
//|	4. Wait some time
//|	5. Verify that all events are Queued
//|	6. Set event from step 3 as COMPLETED
//|	7. Wait for kernels to finish execution (wait for events)
//|	
//|	CPU's kernel will set first half of the buffer with CPU's specific pattern, GPU will set second half of the buffer
//|	with it's own pattern.
//|
//|	Pass criteria
//|	-------------
//|
//|	Validate that first half of the buffer is filled with CPU's pattern and second with GPU's
//|
TEST_F(VR7_Fission, SharedBufferCPUGPU)
{
	ASSERT_NO_FATAL_FAILURE(createAndMergeWithGPU(ocl_descriptor));
	ASSERT_NO_FATAL_FAILURE(sharedBufferCPUGPUBody(ocl_descriptor));
}
