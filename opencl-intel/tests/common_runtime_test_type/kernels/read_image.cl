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

#pragma OPENCL EXTENSION cl_khr_3d_image_writes : enable

/**
 * read_image_* - type of kernels are used for validation that in OpenCL images are being read by both devices
 * *_first - only reads first element in float5/int4 image element
 */


__kernel void read_image2D_float4(read_only image2d_t input, write_only __global float* sum)
{
	if(get_global_id(0)>0){
		return;
	}
	for(int x=0; x<get_image_width(input); ++x){
		for(int y=0; y<get_image_height(input); ++y){
			int2 coord = (int2)(x,y);
			const sampler_t samplerNearest = CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_NEAREST | CLK_ADDRESS_NONE;
			float4 tmp_sum = read_imagef( input, samplerNearest, coord );
			sum[0] += (tmp_sum.x + tmp_sum.y + tmp_sum.z + tmp_sum.w);
		}	
	}
}

__kernel void read_image3D_float4(read_only image3d_t input, __global float* sum)
{
	if(get_global_id(0)>0){
		return;
	}
	for(int x=0; x<get_image_width(input); ++x){
		for(int y=0; y<get_image_height(input); ++y){
			for(int h=0; h<get_image_depth(input); ++h){
				int4 coord = (int4)(x, y, h, 0);
				const sampler_t samplerNearest = CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_NEAREST | CLK_ADDRESS_NONE;
				float4 tmp_sum = read_imagef( input, samplerNearest, coord );
				sum[0] += (tmp_sum.x + tmp_sum.y + tmp_sum.z + tmp_sum.w);
			}
		}	
	}
}

__kernel void read_image2D_int4(read_only image2d_t input, write_only __global float* sum)
{
	if(get_global_id(0)>0){
		return;
	}
	for(int x=0; x<get_image_width(input); ++x){
		for(int y=0; y<get_image_height(input); ++y){
			int2 coord = (int2)(x, y);
			const sampler_t samplerNearest = CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_NEAREST | CLK_ADDRESS_NONE;
			int4 tmp_sum = read_imagei( input, samplerNearest, coord );
			sum[0] += (float)(tmp_sum.x + tmp_sum.y + tmp_sum.z + tmp_sum.w);
		}	
	}
}

__kernel void read_image3D_int4(read_only image3d_t input, __global float* sum)
{
	if(get_global_id(0)>0){
		return;
	}
	for(int x=0; x<get_image_width(input); ++x){
		for(int y=0; y<get_image_height(input); ++y){
			for(int h=0; h<get_image_depth(input); ++h){
				int4 coord = (int4)(x, y, h, 0);
				const sampler_t samplerNearest = CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_NEAREST | CLK_ADDRESS_NONE;
				int4 tmp_sum = read_imagei( input, samplerNearest, coord );
				sum[0] += (float)(tmp_sum.x + tmp_sum.y + tmp_sum.z + tmp_sum.w);
			}
		}	
	}
}

__kernel void read_image2D_float4_first(read_only image2d_t input, write_only __global float* sum)
{
	if(get_global_id(0)>0){
		return;
	}
	for(int x=0; x<get_image_width(input); ++x){
		for(int y=0; y<get_image_height(input); ++y){
			int2 coord = (int2)(x, y);
			const sampler_t samplerNearest = CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_NEAREST | CLK_ADDRESS_NONE;
			float4 tmp_sum = read_imagef( input, samplerNearest, coord );
			sum[0] += (tmp_sum.x);
		}	
	}
}

__kernel void read_image3D_float4_first(read_only image3d_t input, __global float* sum)
{
	if(get_global_id(0)>0){
		return;
	}
	for(int x=0; x<get_image_width(input); ++x){
		for(int y=0; y<get_image_height(input); ++y){
			for(int h=0; h<get_image_depth(input); ++h){
				int4 coord = (int4)(x, y, h, 0);
				const sampler_t samplerNearest = CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_NEAREST | CLK_ADDRESS_NONE;
				float4 tmp_sum = read_imagef( input, samplerNearest, coord );
				sum[0] += (tmp_sum.x);
			}
		}	
	}
}

__kernel void read_image2D_int4_first(read_only image2d_t input, write_only __global float* sum)
{
	if(get_global_id(0)>0){
		return;
	}
	for(int x=0; x<get_image_width(input); ++x){
		for(int y=0; y<get_image_height(input); ++y){
			int2 coord = (int2)(x, y);
			const sampler_t samplerNearest = CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_NEAREST | CLK_ADDRESS_NONE;
			int4 tmp_sum = read_imagei( input, samplerNearest, coord );
			sum[0] += (float)(tmp_sum.x);
		}	
	}
}

__kernel void read_image3D_int4_first(read_only image3d_t input, __global float* sum)
{
	if(get_global_id(0)>0){
		return;
	}
	for(int x=0; x<get_image_width(input); ++x){
		for(int y=0; y<get_image_height(input); ++y){
			for(int h=0; h<get_image_depth(input); ++h){
				int4 coord = (int4)(x, y, h, 0);
				const sampler_t samplerNearest = CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_NEAREST | CLK_ADDRESS_NONE;
				int4 tmp_sum = read_imagei( input, samplerNearest, coord );
				sum[0] += (float)(tmp_sum.x);
			}
		}	
	}
}

__kernel void read_image2D_float4_last(read_only image2d_t input, write_only __global float* sum)
{
	if(get_global_id(0)>0){
		return;
	}
	for(int x=0; x<get_image_width(input); ++x){
		for(int y=0; y<get_image_height(input); ++y){
			int2 coord = (int2)(x, y);
			const sampler_t samplerNearest = CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_NEAREST | CLK_ADDRESS_NONE;
			float4 tmp_sum = read_imagef( input, samplerNearest, coord );
			sum[0] += (tmp_sum.w);
		}	
	}
}

__kernel void read_image3D_float4_last(read_only image3d_t input, __global float* sum)
{
	if(get_global_id(0)>0){
		return;
	}
	for(int x=0; x<get_image_width(input); ++x){
		for(int y=0; y<get_image_height(input); ++y){
			for(int h=0; h<get_image_depth(input); ++h){
				int4 coord = (int4)(x, y, h, 0);
				const sampler_t samplerNearest = CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_NEAREST | CLK_ADDRESS_NONE;
				float4 tmp_sum = read_imagef( input, samplerNearest, coord );
				sum[0] += (tmp_sum.w);
			}
		}	
	}
}

__kernel void read_image2D_int4_last(read_only image2d_t input, write_only __global float* sum)
{
	if(get_global_id(0)>0){
		return;
	}
	for(int x=0; x<get_image_width(input); ++x){
		for(int y=0; y<get_image_height(input); ++y){
			int2 coord = (int2)(x, y);
			const sampler_t samplerNearest = CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_NEAREST | CLK_ADDRESS_NONE;
			int4 tmp_sum = read_imagei( input, samplerNearest, coord );
			sum[0] += (float)(tmp_sum.w);
		}	
	}
}

__kernel void read_image3D_int4_last(read_only image3d_t input, __global float* sum)
{
	if(get_global_id(0)>0){
		return;
	}
	for(int x=0; x<get_image_width(input); ++x){
		for(int y=0; y<get_image_height(input); ++y){
			for(int h=0; h<get_image_depth(input); ++h){
				int4 coord = (int4)(x, y, h, 0);
				const sampler_t samplerNearest = CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_NEAREST | CLK_ADDRESS_NONE;
				int4 tmp_sum = read_imagei( input, samplerNearest, coord );
				sum[0] += (float)(tmp_sum.w);
			}
		}	
	}
}

__kernel void read_image2D_float4_first_two(read_only image2d_t input, write_only __global float* sum)
{
	if(get_global_id(0)>0){
		return;
	}
	for(int x=0; x<get_image_width(input); ++x){
		for(int y=0; y<get_image_height(input); ++y){
			int2 coord = (int2)(x,y );
			const sampler_t samplerNearest = CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_NEAREST | CLK_ADDRESS_NONE;
			float4 tmp_sum = read_imagef( input, samplerNearest, coord );
			sum[0] += (tmp_sum.x + tmp_sum.y);
		}			
	}
}

__kernel void read_image3D_float4_first_two(read_only image3d_t input, __global float* sum)
{
	if(get_global_id(0)>0){
		return;
	}
	for(int x=0; x<get_image_width(input); ++x){
		for(int y=0; y<get_image_height(input); ++y){
			for(int h=0; h<get_image_depth(input); ++h){
				int4 coord = (int4)(x, y, h, 0);
				const sampler_t samplerNearest = CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_NEAREST | CLK_ADDRESS_NONE;
				float4 tmp_sum = read_imagef( input, samplerNearest, coord );
				sum[0] += (tmp_sum.x + tmp_sum.y);
			}
		}	
	}
}

__kernel void read_image2D_int4_first_two(read_only image2d_t input, write_only __global float* sum)
{
	if(get_global_id(0)>0){
		return;
	}
	for(int x=0; x<get_image_width(input); ++x){
		for(int y=0; y<get_image_height(input); ++y){
			int2 coord = (int2)(x, y);
			const sampler_t samplerNearest = CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_NEAREST | CLK_ADDRESS_NONE;
			int4 tmp_sum = read_imagei( input, samplerNearest, coord );
			sum[0] += (float)(tmp_sum.x + tmp_sum.y);
		}	
	}
}

__kernel void read_image3D_int4_first_two(read_only image3d_t input, __global float* sum)
{
	if(get_global_id(0)>0){
		return;
	}
	for(int x=0; x<get_image_width(input); ++x){
		for(int y=0; y<get_image_height(input); ++y){
			for(int h=0; h<get_image_depth(input); ++h){
				int4 coord = (int4)(x, y, h, 0);
				const sampler_t samplerNearest = CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_NEAREST | CLK_ADDRESS_NONE;
				int4 tmp_sum = read_imagei( input, samplerNearest, coord );
				sum[0] += (float)(tmp_sum.x + tmp_sum.y);
			}	
		}	
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////

__kernel void read_write_image2D_float4(
	read_only image2d_t input, 
	write_only image2d_t output, 
	read_only image2d_t expected_input, 
	__global int* result, 
	int multiplyBy)
{
	if(get_global_id(0)>0){
		return;
	}
	for(int x=0; x<get_image_width(input); ++x){
		for(int y=0; y<get_image_height(input); ++y){
			int2 coord = (int2)(x,y);
			const sampler_t samplerNearest = CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_NEAREST | CLK_ADDRESS_NONE;
			float4 input_element = read_imagef( input, samplerNearest, coord );
			float4 expected_input_element = read_imagef( expected_input, samplerNearest, coord );
			if(fabs(input_element.x-expected_input_element.x)>0.1f||fabs(input_element.y-expected_input_element.y)>0.1f||fabs(input_element.z-expected_input_element.z)>0.1f||fabs(input_element.w-expected_input_element.w)>0.1f){
				// input image is defferent from expected_input
				result[0] = -5;
				return;
			}
			input_element*=multiplyBy;
			write_imagef (output, coord, input_element);
			//result[0] = input_element.z;
			//return;
		}	
	}
	// input image is equal to expected_input
	result[0] = 0;
}

__kernel void read_write_image3D_float4(
	read_only image3d_t input, 
	write_only image3d_t output, 
	read_only image3d_t expected_input, 
	__global int* result, 
	int multiplyBy)
{
	if(get_global_id(0)>0){
		return;
	}
	for(int x=0; x<get_image_width(input); ++x){
		for(int y=0; y<get_image_height(input); ++y){
			for(int h=0; h<get_image_depth(input); ++h){
				int4 coord = (int4)(x, y, h, 0);
				const sampler_t samplerNearest = CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_NEAREST | CLK_ADDRESS_NONE;
				float4 input_element = read_imagef( input, samplerNearest, coord );
				float4 expected_input_element = read_imagef( expected_input, samplerNearest, coord );
				if(fabs(input_element.x-expected_input_element.x)>0.1f||fabs(input_element.y-expected_input_element.y)>0.1f||fabs(input_element.z-expected_input_element.z)>0.1f||fabs(input_element.w-expected_input_element.w)>0.1f){
					// input image is defferent from expected_input
					result[0] = -5;
					return;
				}
				input_element*=multiplyBy;
				write_imagef (output, coord, input_element);
			}
		}	
	}
	// input image is equal to expected_input
	result[0] = 0;
}

__kernel void read_write_image2D_int4(
	read_only image2d_t input, 
	write_only image2d_t output, 
	read_only image2d_t expected_input, 
	__global int* result, 
	int multiplyBy)
{
	if(get_global_id(0)>0){
		return;
	}
	for(int x=0; x<get_image_width(input); ++x){
		for(int y=0; y<get_image_height(input); ++y){
			int2 coord = (int2)(x, y);
			const sampler_t samplerNearest = CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_NEAREST | CLK_ADDRESS_NONE;
			int4 input_element = read_imagei( input, samplerNearest, coord );
			int4 expected_input_element = read_imagei( expected_input, samplerNearest, coord );
			if(fabs(input_element.x-expected_input_element.x)>0.1f||fabs(input_element.y-expected_input_element.y)>0.1f||fabs(input_element.z-expected_input_element.z)>0.1f||fabs(input_element.w-expected_input_element.w)>0.1f){
				// input image is defferent from expected_input
				result[0] = -5;
				return;
			}
			input_element*=multiplyBy;
			write_imagei (output, coord, input_element);
		}	
	}
	// input image is equal to expected_input
	result[0] = 0;
}

__kernel void read_write_image3D_int4(
	read_only image3d_t input, 
	write_only image3d_t output, 
	read_only image3d_t expected_input, 
	__global int* result, 
	int multiplyBy)
{
	if(get_global_id(0)>0){
		return;
	}
	for(int x=0; x<get_image_width(input); ++x){
		for(int y=0; y<get_image_height(input); ++y){
			for(int h=0; h<get_image_depth(input); ++h){
				int4 coord = (int4)(x, y, h, 0);
				const sampler_t samplerNearest = CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_NEAREST | CLK_ADDRESS_NONE;
				int4 input_element = read_imagei( input, samplerNearest, coord );
				int4 expected_input_element = read_imagei( expected_input, samplerNearest, coord );
				if(fabs(input_element.x-expected_input_element.x)>0.1f||fabs(input_element.y-expected_input_element.y)>0.1f||fabs(input_element.z-expected_input_element.z)>0.1f||fabs(input_element.w-expected_input_element.w)>0.1f){
					// input image is defferent from expected_input
					result[0] = -5;
					return;
				}
				input_element*=multiplyBy;
				write_imagei (output, coord, input_element);
			}
		}	
	}
	// input image is equal to expected_input
	result[0] = 0;
}

__kernel void read_write_image2D_float4_first(
	read_only image2d_t input, 
	write_only image2d_t output, 
	read_only image2d_t expected_input, 
	__global int* result, 
	int multiplyBy)
{
	if(get_global_id(0)>0){
		return;
	}
	for(int x=0; x<get_image_width(input); ++x){
		for(int y=0; y<get_image_height(input); ++y){
			int2 coord = (int2)(x, y);
			const sampler_t samplerNearest = CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_NEAREST | CLK_ADDRESS_NONE;
			float4 input_element = read_imagef( input, samplerNearest, coord );
			float4 expected_input_element = read_imagef( expected_input, samplerNearest, coord );
			if(fabs(input_element.x-expected_input_element.x)>0.1f){
				// input image is defferent from expected_input
				result[0] = -5;
				return;
			}
			input_element*=multiplyBy;
			write_imagef (output, coord, input_element);
		}	
	}
	// input image is equal to expected_input
	result[0] = 0;
}

__kernel void read_write_image3D_float4_first(
	read_only image3d_t input, 
	write_only image3d_t output, 
	read_only image3d_t expected_input, 
	__global int* result, 
	int multiplyBy)
{
	if(get_global_id(0)>0){
		return;
	}
	for(int x=0; x<get_image_width(input); ++x){
		for(int y=0; y<get_image_height(input); ++y){
			for(int h=0; h<get_image_depth(input); ++h){
				int4 coord = (int4)(x, y, h, 0);
				const sampler_t samplerNearest = CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_NEAREST | CLK_ADDRESS_NONE;
				float4 input_element = read_imagef( input, samplerNearest, coord );
				float4 expected_input_element = read_imagef( expected_input, samplerNearest, coord );
				if(fabs(input_element.x-expected_input_element.x)>0.1f){
					// input image is defferent from expected_input
					result[0] = -5;
					return;
				}
				input_element*=multiplyBy;
				write_imagef (output, coord, input_element);
			}
		}	
	}
	// input image is equal to expected_input
	result[0] = 0;
}

__kernel void read_write_image2D_int4_first(
	read_only image2d_t input, 
	write_only image2d_t output, 
	read_only image2d_t expected_input, 
	__global int* result, 
	int multiplyBy)
{
	if(get_global_id(0)>0){
		return;
	}
	for(int x=0; x<get_image_width(input); ++x){
		for(int y=0; y<get_image_height(input); ++y){
			int2 coord = (int2)(x, y);
			const sampler_t samplerNearest = CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_NEAREST | CLK_ADDRESS_NONE;
			int4 input_element = read_imagei( input, samplerNearest, coord );
			int4 expected_input_element = read_imagei( expected_input, samplerNearest, coord );
			if(fabs(input_element.x-expected_input_element.x)>0.1f){
				// input image is defferent from expected_input
				result[0] = -5;
				return;
			}
			input_element*=multiplyBy;
			write_imagei (output, coord, input_element);
		}	
	}
	// input image is equal to expected_input
	result[0] = 0;
}

__kernel void read_write_image3D_int4_first(
	read_only image3d_t input, 
	write_only image3d_t output, 
	read_only image3d_t expected_input, 
	__global int* result, 
	int multiplyBy)
{
	if(get_global_id(0)>0){
		return;
	}
	for(int x=0; x<get_image_width(input); ++x){
		for(int y=0; y<get_image_height(input); ++y){
			for(int h=0; h<get_image_depth(input); ++h){
				int4 coord = (int4)(x, y, h, 0);
				const sampler_t samplerNearest = CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_NEAREST | CLK_ADDRESS_NONE;
				int4 input_element = read_imagei( input, samplerNearest, coord );
				int4 expected_input_element = read_imagei( expected_input, samplerNearest, coord );
				if(fabs(input_element.x-expected_input_element.x)>0.1f){
					// input image is defferent from expected_input
					result[0] = -5;
					return;
				}
				input_element*=multiplyBy;
				write_imagei (output, coord, input_element);
			}
		}	
	}
	// input image is equal to expected_input
	result[0] = 0;
}

__kernel void read_write_image2D_float4_last(
	read_only image2d_t input, 
	write_only image2d_t output, 
	read_only image2d_t expected_input, 
	__global int* result, 
	int multiplyBy)
{
	if(get_global_id(0)>0){
		return;
	}
	for(int x=0; x<get_image_width(input); ++x){
		for(int y=0; y<get_image_height(input); ++y){
			int2 coord = (int2)(x, y);
			const sampler_t samplerNearest = CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_NEAREST | CLK_ADDRESS_NONE;
			float4 input_element = read_imagef( input, samplerNearest, coord );
			float4 expected_input_element = read_imagef( expected_input, samplerNearest, coord );
			if(fabs(input_element.w-expected_input_element.w)>0.1f){
				// input image is defferent from expected_input
				result[0] = -5;
				return;
			}
			input_element*=multiplyBy;
			write_imagef (output, coord, input_element);
		}	
	}
	// input image is equal to expected_input
	result[0] = 0;
}

__kernel void read_write_image3D_float4_last(
	read_only image3d_t input, 
	write_only image3d_t output, 
	read_only image3d_t expected_input, 
	__global int* result, 
	int multiplyBy)
{
	if(get_global_id(0)>0){
		return;
	}
	for(int x=0; x<get_image_width(input); ++x){
		for(int y=0; y<get_image_height(input); ++y){
			for(int h=0; h<get_image_depth(input); ++h){
				int4 coord = (int4)(x, y, h, 0);
				const sampler_t samplerNearest = CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_NEAREST | CLK_ADDRESS_NONE;
				float4 input_element = read_imagef( input, samplerNearest, coord );
				float4 expected_input_element = read_imagef( expected_input, samplerNearest, coord );
				if(fabs(input_element.w-expected_input_element.w)>0.1f){
					// input image is defferent from expected_input
					result[0] = -5;
					return;
				}
				input_element*=multiplyBy;
				write_imagef (output, coord, input_element);
			}
		}	
	}
	// input image is equal to expected_input
	result[0] = 0;
}

__kernel void read_write_image2D_int4_last(
	read_only image2d_t input, 
	write_only image2d_t output, 
	read_only image2d_t expected_input, 
	__global int* result, 
	int multiplyBy)
{
	if(get_global_id(0)>0){
		return;
	}
	for(int x=0; x<get_image_width(input); ++x){
		for(int y=0; y<get_image_height(input); ++y){
			int2 coord = (int2)(x, y);
			const sampler_t samplerNearest = CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_NEAREST | CLK_ADDRESS_NONE;
			int4 input_element = read_imagei( input, samplerNearest, coord );
			int4 expected_input_element = read_imagei( expected_input, samplerNearest, coord );
			if(fabs(input_element.w-expected_input_element.w)>0.1f){
				// input image is defferent from expected_input
				result[0] = -5;
				return;
			}
			input_element*=multiplyBy;
			write_imagei (output, coord, input_element);
		}	
	}
	// input image is equal to expected_input
	result[0] = 0;
}

__kernel void read_write_image3D_int4_last(
	read_only image3d_t input, 
	write_only image3d_t output, 
	read_only image3d_t expected_input, 
	__global int* result, 
	int multiplyBy)
{
	if(get_global_id(0)>0){
		return;
	}
	for(int x=0; x<get_image_width(input); ++x){
		for(int y=0; y<get_image_height(input); ++y){
			for(int h=0; h<get_image_depth(input); ++h){
				int4 coord = (int4)(x, y, h, 0);
				const sampler_t samplerNearest = CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_NEAREST | CLK_ADDRESS_NONE;
				int4 input_element = read_imagei( input, samplerNearest, coord );
				int4 expected_input_element = read_imagei( expected_input, samplerNearest, coord );
				if(fabs(input_element.w-expected_input_element.w)>0.1f){
					// input image is defferent from expected_input
					result[0] = -5;
					return;
				}
				input_element*=multiplyBy;
				write_imagei (output, coord, input_element);
			}
		}	
	}
	// input image is equal to expected_input
	result[0] = 0;
}

__kernel void read_write_image2D_float4_first_two(
	read_only image2d_t input, 
	write_only image2d_t output, 
	read_only image2d_t expected_input, 
	__global int* result, 
	int multiplyBy)
{
	if(get_global_id(0)>0){
		return;
	}
	for(int x=0; x<get_image_width(input); ++x){
		for(int y=0; y<get_image_height(input); ++y){	
			int2 coord = (int2)(x,y );
			const sampler_t samplerNearest = CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_NEAREST | CLK_ADDRESS_NONE;
			float4 input_element = read_imagef( input, samplerNearest, coord );
			float4 expected_input_element = read_imagef( expected_input, samplerNearest, coord );
			if(fabs(input_element.x-expected_input_element.x)>0.1f||fabs(input_element.y-expected_input_element.y)>0.1f){
				// input image is defferent from expected_input
				result[0] = -5;
				return;
			}
			input_element*=multiplyBy;
			write_imagef (output, coord, input_element);
		}	
	}
	// input image is equal to expected_input
	result[0] = 0;
}

__kernel void read_write_image3D_float4_first_two(
	read_only image3d_t input, 
	write_only image3d_t output, 
	read_only image3d_t expected_input, 
	__global int* result, 
	int multiplyBy)
{
	if(get_global_id(0)>0){
		return;
	}
	for(int x=0; x<get_image_width(input); ++x){
		for(int y=0; y<get_image_height(input); ++y){
			for(int h=0; h<get_image_depth(input); ++h){
				int4 coord = (int4)(x, y, h, 0);
				const sampler_t samplerNearest = CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_NEAREST | CLK_ADDRESS_NONE;
				float4 input_element = read_imagef( input, samplerNearest, coord );
				float4 expected_input_element = read_imagef( expected_input, samplerNearest, coord );
				if(fabs(input_element.x-expected_input_element.x)>0.1f||fabs(input_element.y-expected_input_element.y)>0.1f){
					// input image is defferent from expected_input
					result[0] = -5;
					return;
				}
				input_element*=multiplyBy;
				write_imagef (output, coord, input_element);
			}	
		}	
	}
	// input image is equal to expected_input
	result[0] = 0;
}

__kernel void read_write_image2D_int4_first_two(
	read_only image2d_t input, 
	write_only image2d_t output, 
	read_only image2d_t expected_input, 
	__global int* result, 
	int multiplyBy)
{
	if(get_global_id(0)>0){
		return;
	}
	for(int x=0; x<get_image_width(input); ++x){
		for(int y=0; y<get_image_height(input); ++y){
			int2 coord = (int2)(x, y);
			const sampler_t samplerNearest = CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_NEAREST | CLK_ADDRESS_NONE;
			int4 input_element = read_imagei( input, samplerNearest, coord );
			int4 expected_input_element = read_imagei( expected_input, samplerNearest, coord );
			if(fabs(input_element.x-expected_input_element.x)>0.1f||fabs(input_element.y-expected_input_element.y)>0.1f){
				// input image is defferent from expected_input
				result[0] = -5;
				return;
			}			
			input_element*=multiplyBy;
			write_imagei (output, coord, input_element);
		}	
	}
	// input image is equal to expected_input
	result[0] = 0;
}

__kernel void read_write_image3D_int4_first_two(
	read_only image3d_t input, 
	write_only image3d_t output, 
	read_only image3d_t expected_input, 
	__global int* result, 
	int multiplyBy)
{
	if(get_global_id(0)>0){
		return;
	}
	for(int x=0; x<get_image_width(input); ++x){
		for(int y=0; y<get_image_height(input); ++y){
			for(int h=0; h<get_image_depth(input); ++h){
				int4 coord = (int4)(x, y, h, 0);
				const sampler_t samplerNearest = CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_NEAREST | CLK_ADDRESS_NONE;
				int4 input_element = read_imagei( input, samplerNearest, coord );
				int4 expected_input_element = read_imagei( expected_input, samplerNearest, coord );
				if(fabs(input_element.x-expected_input_element.x)>0.1f||fabs(input_element.y-expected_input_element.y)>0.1f){
					// input image is defferent from expected_input
					result[0] = -5;
					return;
				}
				input_element*=multiplyBy;
				write_imagei (output, coord, input_element);
			}
		}	
	}
	// input image is equal to expected_input
	result[0] = 0;
}