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
 * kernel_n - kernels intended for kernels' compilation, will not be enqueued
 */
 
 __kernel void kernel_0(__global int* input, int input_size)
{
	input[0] = 0;			
}

__kernel void kernel_1(__global int* input, int input_size)
{
	input[0] = 1;			
}

__kernel void kernel_2(__global int* input, int input_size)
{
	input[0] = 2;			
}

__kernel void kernel_3(__global int* input, int input_size)
{
	input[0] = 3;			
}

__kernel void kernel_4(__global int* input, int input_size)
{
	input[0] = 4;			
}

__kernel void kernel_5(__global int* input, int input_size)
{
	input[0] = 5;			
}

__kernel void kernel_6(__global int* input, int input_size)
{
	input[0] = 6;			
}

__kernel void kernel_7(__global int* input, int input_size)
{
	input[0] = 7;			
}

__kernel void kernel_8(__global int* input, int input_size)
{
	input[0] = 8;			
}

__kernel void kernel_9(__global int* input, int input_size)
{
	input[0] = 9;			
}

#if FOR_CPU
__kernel void kernel_ifdef(__global int* input, int input_size, int input_size2)
{
	input[0] = 0;			
}
#else
__kernel void kernel_ifdef(__global int* input, int input_size)
{
	input[0] = 1;			
}
#endif




