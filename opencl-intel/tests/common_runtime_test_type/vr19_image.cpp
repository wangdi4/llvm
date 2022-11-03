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

#include "vr19_image.h"
#include <sstream>

class VR19Image : public CommonRuntime {};

//|  TEST: VR19Image.ReadImage2DShared (TC-92)
//|
//|  Purpose
//|  -------
//|
//|  Verify the ability of different devices (CPU and GPU) to work in
// parallel on the same | shared memory using Read Image commands for 2D images
//|
//|  Method
//|  ------
//|
//|  1. Initialize array of size arraySize in host with CPU pattern
//|  2. Initialize array of size arraySize in host with GPU pattern
//|  3. Initialize array of size 2*arraySize in host with pattern different
// from CPU and GPU patterns |  4. Create 2 OpenCL 2D imaged with
// CL_MEM_ALLOC_HOST_PTR of size arraySize*sizeof(type of arrays in steps 1, 2
// and 3) |  5. Write array from step 1 to one of the images |  6. Write
// array from step 2 to the other image |  7. Create user event |  8.
// Enqueue command read from image in step 5 to lower half of array in step 3 in
// queue of CPU device |    Command is dependant on status of event
// in step 7 |  9. Enqueue command read from image in step 6 to ther other half
// of array in step 3 in queue of GPU device |    Command is dependant on
// status of event in step 7 |  10. Wait |  11. Verify that commands in
// steps 8 and 9 are enqueued |  12. Set status of event in step 7 to
// CL_COMPLETE
//|  13. Wait for commands to finish
//|
//|  Pass criteria
//|  -------------
//|
//|  Verify that first half of array in step 3 is filled with CPU pattern,
// and the second with GPU pattern
//|
TEST_F(VR19Image, ReadImage2DShared) {
  ASSERT_NO_FATAL_FAILURE(testReadImage2DSharedBody(ocl_descriptor));
}

//|  TEST: VR19Image.ReadImage3DShared (TC-93)
//|
//|  Purpose
//|  -------
//|
//|  Verify the ability of different devices (CPU and GPU) to work in
// parallel on the same | shared memory using Read Image commands for 3D images
//|
//|  Method
//|  ------
//|
//|  1. Initialize array of size arraySize in host with CPU pattern
//|  2. Initialize array of size arraySize in host with GPU pattern
//|  3. Initialize array of size 2*arraySize in host with pattern different
// from CPU and GPU patterns |  4. Create 2 OpenCL 3D imaged with
// CL_MEM_ALLOC_HOST_PTR of size arraySize*sizeof(type of arrays in steps 1, 2
// and 3) |  5. Write array from step 1 to one of the images |  6. Write
// array from step 2 to the other image |  7. Create user event |  8.
// Enqueue command read from image in step 5 to lower half of array in step 3 in
// queue of CPU device |    Command is dependant on status of event
// in step 7 |  9. Enqueue command read from image in step 6 to ther other half
// of array in step 3 in queue of GPU device |    Command is dependant on
// status of event in step 7 |  10. Wait |  11. Verify that commands in
// steps 8 and 9 are enqueued |  12. Set status of event in step 7 to
// CL_COMPLETE
//|  13. Wait for commands to finish
//|
//|  Pass criteria
//|  -------------
//|
//|  Verify that first half of array in step 3 is filled with CPU pattern,
// and the second with GPU pattern
//|
TEST_F(VR19Image, ReadImage3DShared) {
  ASSERT_NO_FATAL_FAILURE(testReadImage3DSharedBody(ocl_descriptor));
}
