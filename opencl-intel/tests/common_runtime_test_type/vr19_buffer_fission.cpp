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

#include <sstream>
#include "vr19_buffer.h"

class VR19Buffer_Fission: public FissionWrapper{};

//|	TEST: VR19Buffer_Fission.ReadBufferShared (TC-126 TC-127)
//|
//|	Purpose
//|	-------
//|	
//|	Verify the ability of different devices (CPU subdevice and GPU) to work in parallel on the same
//| shared memory using Read Buffer commands
//|
//|	Method
//|	------
//|
//|	1. Initialize array of size arraySize in host with CPU subdevice  pattern
//|	2. Initialize array of size arraySize in host with GPU pattern
//|	3. Initialize array of size 2*arraySize in host with pattern different from CPU subdevice  and GPU patterns
//|	4. Create 2 OpenCL buffers with CL_MEM_ALLOC_HOST_PTR of size arraySize*sizeof(type of arrays in steps 1, 2 and 3)
//|	5. Write array from step 1 to one of the buffers
//|	6. Write array from step 2 to the other buffer
//|	7. Create user event
//|	8. Enqueue command read from buffer in step 5 to lower half of array in step 3 in queue of CPU subdevice 
//|		Command is dependant on status of event in step 7
//|	9. Enqueue command read from buffer in step 6 to ther other half of array in step 3 in queue of GPU device
//|		Command is dependant on status of event in step 7
//|	10. Wait
//|	11. Verify that commands in steps 8 and 9 are enqueued
//|	12. Set status of event in step 7 to CL_COMPLETE
//|	13. Wait for commands to finish
//|	
//|	Pass criteria
//|	-------------
//|
//|	Verify that first half of array in step 3 is filled with CPU pattern, and the second with GPU pattern
//|
TEST_F(VR19Buffer_Fission, ReadBufferShared) 
{
	ASSERT_NO_FATAL_FAILURE(createAndMergeWithGPU(ocl_descriptor));
	ASSERT_NO_FATAL_FAILURE(testVR19ReadBufferSharedBody(ocl_descriptor));
}

//|	TEST: VR19Buffer_Fission.ReadBufferRectShared (TC-126 TC-127)
//|
//|	Purpose
//|	-------
//|	
//|	Verify the ability of different devices (CPU subdevice and GPU) to work in parallel on the same
//| shared memory using Read Buffer Rect commands
//|
//|	Method
//|	------
//|
//|	1. Initialize array of size arraySize in host with CPU subdevice pattern
//|	2. Initialize array of size arraySize in host with GPU pattern
//|	3. Initialize array of size 2*arraySize in host with pattern different from CPU subdevice and GPU patterns
//|	4. Create 2 OpenCL buffers with CL_MEM_ALLOC_HOST_PTR of size arraySize*sizeof(type of arrays in steps 1, 2 and 3)
//|	5. Write array from step 1 to one of the buffers with Read Buffer Rect
//|	6. Write array from step 2 to the other buffer with Read Buffer Rect
//|	7. Create user event
//|	8. Enqueue command read from buffer in step 5 to lower half of array in step 3 in queue of CPU subdevice
//|		Command is dependant on status of event in step 7 with Write Buffer Rect
//|	9. Enqueue command read from buffer in step 6 to ther other half of array in step 3 in queue of GPU device
//|		Command is dependant on status of event in step 7 with Write Buffer Rect
//|	10. Wait
//|	11. Verify that commands in steps 8 and 9 are enqueued
//|	12. Set status of event in step 7 to CL_COMPLETE
//|	13. Wait for commands to finish
//|	
//|	Pass criteria
//|	-------------
//|
//|	Verify that first half of array in step 3 is filled with CPU pattern, and the second with GPU pattern
//|
TEST_F(VR19Buffer_Fission, ReadBufferRectShared)
{
	ASSERT_NO_FATAL_FAILURE(createAndMergeWithGPU(ocl_descriptor));
	ASSERT_NO_FATAL_FAILURE(testVR19ReadBufferRectSharedBody(ocl_descriptor));
}

//|	TEST: VR19Buffer_Fission.CopyBufferShared (TC-126 TC-127)
//|
//|	Purpose
//|	-------
//|	
//|	Verify the ability of different devices (CPU subdevice and GPU) to work in parallel on the same
//| shared memory using Copy Buffer commands
//|
//|	Method
//|	------
//|
//|	1. Initialize array of size arraySize in host with CPU subdevice pattern
//|	2. Initialize array of size arraySize in host with GPU pattern
//|	3. Initialize array of size 2*arraySize in host with pattern different from CPU subdevice and GPU patterns
//|		and OpenCL buffer of this size with CL_MEM_USE_HOST_PTR
//|	4. Create 2 OpenCL buffers with CL_MEM_ALLOC_HOST_PTR of size arraySize*sizeof(type of arrays in steps 1, 2 and 3)
//|	5. Write array from step 1 to one of the buffers with Read Buffer Rect
//|	6. Write array from step 2 to the other buffer with Read Buffer Rect
//|	7. Create user event
//|	8. Enqueue command copy from buffer in step 5 to lower half of buffer in step 3 in queue of CPU subdevice
//|		Command is dependant on status of event in step 7 with Write Buffer Rect
//|	9. Enqueue command copy from buffer in step 6 to ther other half of buffer in step 3 in queue of GPU device
//|		Command is dependant on status of event in step 7 with Write Buffer Rect
//|	10. enqueue enqueueMapBuffer dependant on events signaling completion of commands in steps 8 and 9 and wait
//|	11. Verify that commands in steps 8, 9 and 10 are enqueued
//|	12. Set status of event in step 7 to CL_COMPLETE
//|	13. Wait for commands to finish
//|	
//|	Pass criteria
//|	-------------
//|
//|	Verify that first half of array in step 3 is filled with CPU pattern, and the second with GPU pattern
//|
TEST_F(VR19Buffer_Fission, CopyBufferShared)
{
	ASSERT_NO_FATAL_FAILURE(createAndMergeWithGPU(ocl_descriptor));
	ASSERT_NO_FATAL_FAILURE(testVR19CopyBufferSharedBody(ocl_descriptor));
}