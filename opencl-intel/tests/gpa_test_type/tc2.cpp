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

#include "gpa_tests.h"

//|	TEST: GPA.TC2_CommanQueueInstrumentation (TC-2)
//|
//|	Purpose
//|	-------
//|	
//|	Test OpenCL command queue instrumentation
//|
//|	Method
//|	------
//|
//|	Create an OpenCL command queue.
//|	
//|	Pass criteria
//|	-------------
//|
//|	
//|
TEST_F(GPA, TC2_CommanQueueInstrumentation)
{	
	int arraySize = 2;
	int value = 5;
	// setup CPU device 
	ASSERT_NO_FATAL_FAILURE(getCPUDevice(&ocl_descriptor.platform, &ocl_descriptor.device));
	
	// create context
	cl_context_properties properties[] = {CL_CONTEXT_PLATFORM, (cl_context_properties)ocl_descriptor.platform, 0};
	ASSERT_NO_FATAL_FAILURE(createContext(&ocl_descriptor.context, properties, 1, &ocl_descriptor.device, NULL, NULL));
		
	// create queue
	ASSERT_NO_FATAL_FAILURE(createCommandQueue(&ocl_descriptor.queue, ocl_descriptor.context, 
		ocl_descriptor.device, CL_QUEUE_PROFILING_ENABLE));
}