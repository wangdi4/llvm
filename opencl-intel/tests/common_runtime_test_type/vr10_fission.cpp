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

#include "vr10.h"
#include <sstream>

class VR10_Fission : public FissionWrapper {};

//|  TEST: VR10_Fission.CPUGPUbinaries (TC-121)
//|
//|  Purpose
//|  -------
//|
//|  Verify that it is possible to create a program object for both CPU
// subdevice and GPU devices |  and loads the binary bits specified into program
// object
//|
//|  Method
//|  ------
//|
//|  1.  Create a program object for both CPU subdevice and GPU root
// device, and load the binary |      bits specified into a program object
//|  2.  Build (compiles & links) a program executable from the program
// binary for |      both the CPU and the GPU devices.
//|
//|  Pass criteria
//|  -------------
//|
//|  A valid non-zero program object should be returned and the program
// should be built successfully.
//|
TEST_F(VR10_Fission, CPUGPUbinaries) {
  ASSERT_NO_FATAL_FAILURE(this->createAndMergeWithGPU(ocl_descriptor));
  ASSERT_NO_FATAL_FAILURE(testCPUGPUbinariesBody(ocl_descriptor));
}