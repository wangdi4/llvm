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

#include "vr9.h"

//|	TEST: CommonRuntime.CPUSubDevices (TC-120)
//|
//|	Purpose
//|	-------
//|	
//|	Verify that the host can recieve events from CPU sub-devices 
//| for all types of commands
//|	Method
//|	------
//|
//|	1. Use the root CPU device to create an array of n CPU sub-devices
//| 2. Create a user-event to control commands execution
//|	3. Queue all types of queue-commands to a CPU sub-device  
//|	   commands-queue with returning event object (for each command).
//|	4. In the host waitForEvent from all the queued commands.
//|	5. Setting the user-event to start the commands execution 
//     (should be done from other thread).
//|	6. The list of queued commands is:
//|			- clEnqueueReadBuffer()
//|			- clEnqueueWriteBuffer()
//|			- clEnqueueReadBufferRect()
//|			- clEnqueueWriteBufferRect()
//|			- clEnqueueCopyBuffer()
//|			- clEnqueueBufferRect()
//|			- clEnqueueMapBuffer()
//|			- clEnqueueMapReadImage()
//|			- clEnqueueWriteImage()
//|			- clEnqueueCopyImage()
//|			- clEnqueueCopyImageToBuffer()
//|			- clEnqueueCopyBufferToImage()
//|			- clEnqueueMapImage()
//|			- clEnqueueUnmapMemObject()
//|			- clEnqueueNDRangeKernel()
//|			- clEnqueueTask()
//|			- clEnqueueNativeKernel()
//|			- clEnqueueMarker()
//|	Pass criteria
//|	-------------
//|
//|	The host should receive CL_COMPLETE events from all the queued commands


class VR9_Fission: public FissionWrapper{};


TEST_F(VR9_Fission, CPUSubDeviceCmds)
{
	ASSERT_NO_FATAL_FAILURE(createAndMergeWithGPU(ocl_descriptor));
	ASSERT_NO_FATAL_FAILURE(waitOnBothDevices(ocl_descriptor));
}
