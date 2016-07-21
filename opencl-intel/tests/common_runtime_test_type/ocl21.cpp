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

class OCL21: public CommonRuntime{};

//|	TEST: OCL21.clCreateProgramWithIL01
//|
//|	Purpose
//|	-------
//|	
//| Verify the ability to create program with il for shared context 
//| Verify the ability to get IL from program
//|	
//|	Method
//|	------
//|
//|	1. Create a program with IL for shared context
//|	2. Build program
//| 3. Get IL from program
//|
//|	Pass criteria
//|	-------------
//|
//|	Verify that valid non-zero program object are returned and build successfull
//|

TEST_F(OCL21, clCreateProgramWithIL01)
{
    ASSERT_NO_FATAL_FAILURE(setUpContextProgramQueues(ocl_descriptor, "vector4_d.spir12_64"));

    const char * kernelSource = NULL;
	ASSERT_NO_FATAL_FAILURE(fileToBuffer(&kernelSource, "vector4_d.spir12_64"));

    void * il = NULL;
    size_t ret;

    ASSERT_NO_FATAL_FAILURE(ocl_descriptor.program, CL_PROGRAM_IL, sizeof(void *), &il, &ret);
    ASSERT_EQ(sizeof(void *), ret);
    ASSERT_EQ(sizeof(kernelSource), sizeof(il));

    if (kernelSource != NULL)
    {
        delete[] kernelSource;
        kernelSource = NULL;
    }
}
