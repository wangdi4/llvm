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

#include <stdio.h>
#include "common_runtime_tests.h"
#include "global_environment.h"

int main(int argc, char** argv)
{
	setSecondDeviceType(CL_DEVICE_TYPE_GPU);	
	for(int i=0; i<argc; ++i)
	{
		if(0==strcmp("ACC",argv[i]))
		{
			std::cout << "SECOND DEVICE: ACC" << std::endl;
			setSecondDeviceType(CL_DEVICE_TYPE_ACCELERATOR);	
			break;
		}
	}
	if(CL_DEVICE_TYPE_GPU==getSecondDeviceType())
	{
		std::cout << "SECOND DEVICE: GPU" << std::endl;
	}
	::testing::AddGlobalTestEnvironment(new EnvironemntCommonRuntimeTestType());
	::testing::InitGoogleTest(&argc, argv);
    int rc = RUN_ALL_TESTS();

    if (rc == 0) 
	{
        printf("\n==============\nTEST SUCCEDDED\n==============\n");
        return 0;
    }
    else 
	{
        printf("\n==============\nTEST FAILED\n==============\n");
        return 1;
    }
	return 0;
}
