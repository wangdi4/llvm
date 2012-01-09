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

#include "vr21.h"

class VR21_Fission: public FissionWrapper{};

//|	TEST: VR21_Fission.CPUGPUsampler (TC-129)
//|
//|	Purpose
//|	-------
//|	
//|	Verify that an OpenCL sampler can be used by all 
//| different devices in a context (VR-21).
//|	Method
//|	------
//|
//|	1. Create a 3D image object
//|	2. Create a sampler object for the 3D image object
//|	3. Create a shared kernel that gets as arguments an
//|	   image object, a sampler object, and a buffer object
//|	   and uses the sampler to read the image and write the read
//|	   data to the buffer object.
//|	4. Queue the kernel execution command to the CPU subdevice commands-queue.
//|	5. Queue the Kernel execution command to the GPU commands-queue 
//|	   (with another buffer object).
//|
//|	Pass criteria
//|	-------------
//|
//|	The content of the destination buffer object should match the
//| content of the source 3D image object.
//|

TEST_F(VR21_Fission, CPUGPUsampler)
{
	ASSERT_NO_FATAL_FAILURE(createAndMergeWithGPU(ocl_descriptor));
	ASSERT_NO_FATAL_FAILURE(testCPUGPUsamplerBody(ocl_descriptor));
}
