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

/**
 * 
 */
 //#pragma OPENCL EXTENSION cl_khr_fp64 : enable

 __kernel void image3d_2_buffer(
	 read_only image3d_t srcBuf, 
	 sampler_t samp,
	 __global float4 *dstBuf)
{

	size_t x=get_global_id(0);
	size_t y=get_global_id(1);
	size_t z=get_global_id(2);

	size_t x_sz=get_global_size(0);
	size_t y_sz=get_global_size(1);
	size_t z_sz=get_global_size(2);

	float4 coords={x,y,z,0};
	int idx=z*x_sz*y_sz+y*x_sz+x;
	dstBuf[idx]=read_imagef(srcBuf,samp,coords);

}