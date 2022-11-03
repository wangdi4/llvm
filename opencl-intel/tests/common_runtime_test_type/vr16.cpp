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

#include "vr16.h"
#include <sstream>

class VR16 : public CommonRuntime {};

//|  TEST: VR16.SharedKernels (TC-83)
//|
//|  Purpose
//|  -------
//|
//|  Verify the ability to queue commands to execute a shared kernel on CPU
// device and a |  different shared kernel on GPU device.
//|
//|  Method
//|  ------
//|
//|  1. Create shared 2 kernels (shared on CPU and GPU devices)
//|  2. Enqueue one of the kernels on CPU and another on GPU queues, both
// kernels are waiting for user event |  3. Check that both kernels statuses are
// CL_QUEUED |  3. Set user to CL_COMPLETE |  4. Wait for kernels to complete
// their execution
//|
//|  Pass criteria
//|  -------------
//|
//|  Verify that both kernels were in CL_QUEUED status after NDRange and
// before their execution
//|

TEST_F(VR16, SharedKernels) {
  // set up OpenCL context, program and queues
  ASSERT_NO_FATAL_FAILURE(testSharedKernelsBody(ocl_descriptor));
}
