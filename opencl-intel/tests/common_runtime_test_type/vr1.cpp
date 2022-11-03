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

#include "common_runtime_tests.h"

class VR1 : public CommonRuntime {};

//|  TEST: VR1.SinglePlatform (TC-1)
//|
//|  Purpose
//|  -------
//|
//|  Test that the list of Intel platforms available is 1 and only 1
//|
//|  Method
//|  ------
//|
//|  Call clGetPlatformIDs() with num_entries greater than 1
//|  and num_platfroms and platfroms not NULL
//|
//|  Pass criteria
//|  -------------
//|
//|  The list of Intel platforms returned is 1
//|
TEST_F(VR1, SinglePlatform) {
  // set num_entries to value greater than 1
  cl_uint num_entries = 2;
  cl_uint num_platforms = 0;
  cl_platform_id platforms[] = {0, 0, 0};

  // expect return of a single platform
  ASSERT_NO_FATAL_FAILURE(
      getPlatformIDs(num_entries, platforms, &num_platforms));
  ASSERT_EQ(1, num_platforms) << "num_platforms is not 1";

  EXPECT_NE((cl_platform_id)0, platforms[0]);
  EXPECT_EQ((cl_platform_id)0, platforms[1]);
}
