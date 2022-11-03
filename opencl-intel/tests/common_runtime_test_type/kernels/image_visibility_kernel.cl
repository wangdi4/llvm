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

__kernel void copy_image1D(read_only image1d_t input,
                           write_only image1d_t output) {
  int coord = 0;
  const sampler_t samplerNearest =
      CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_NEAREST | CLK_ADDRESS_CLAMP;
  write_imagei(output, coord, read_imagei(input, samplerNearest, coord));
}
__kernel void copy_image1D_float(read_only image1d_t input,
                                 write_only image1d_t output) {
  int coord = 0;
  const sampler_t samplerNearest =
      CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_NEAREST | CLK_ADDRESS_CLAMP;
  write_imagef(output, coord, read_imagef(input, samplerNearest, coord));
}

__kernel void copy_image1D_buffer(read_only image1d_buffer_t input,
                                  write_only image1d_t output) {
  int coord = 0;
  const sampler_t samplerNearest =
      CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_NEAREST | CLK_ADDRESS_CLAMP;
  write_imagei(output, coord, read_imagei(input, coord));
}
__kernel void copy_image1D_buffer_float(read_only image1d_buffer_t input,
                                        write_only image1d_t output) {
  int coord = 0;
  write_imagef(output, coord, read_imagef(input, coord));
}
__kernel void copy_image2D(read_only image2d_t input,
                           write_only image2d_t output) {
  int2 coord = (int2)(0, 0);

  const sampler_t samplerNearest =
      CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_NEAREST | CLK_ADDRESS_CLAMP;
  write_imagei(output, coord, read_imagei(input, samplerNearest, coord));
}
__kernel void copy_image2D_float(read_only image2d_t input,
                                 write_only image2d_t output) {
  int2 coord = (int2)(0, 0);
  const sampler_t samplerNearest =
      CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_NEAREST | CLK_ADDRESS_CLAMP;
  write_imagef(output, coord, read_imagef(input, samplerNearest, coord));
}

// int case of 3d image we can not write, so will have to use 1d image
__kernel void copy_image3D(read_only image3d_t input,
                           write_only image1d_t output) {
  int4 coord = (int4)(0, 0, 0, 0);

  const sampler_t samplerNearest =
      CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_NEAREST | CLK_ADDRESS_CLAMP;
  int coord2 = 0;
  write_imagei(output, coord2, read_imagei(input, samplerNearest, coord));
}
__kernel void copy_image3D_float(read_only image3d_t input,
                                 write_only image1d_t output) {
  int4 coord = (int4)(0, 0, 0, 0);
  const sampler_t samplerNearest =
      CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_NEAREST | CLK_ADDRESS_CLAMP;
  int coord2 = 0;
  write_imagei(output, coord2, read_imagei(input, samplerNearest, coord));
}

__kernel void copy_image_array_1D(read_only image1d_array_t input,
                                  write_only image1d_array_t output) {
  int2 coord = (int2)(0, 0);

  const sampler_t samplerNearest =
      CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_NEAREST | CLK_ADDRESS_CLAMP;
  write_imagei(output, coord, read_imagei(input, samplerNearest, coord));
}
__kernel void copy_image_array_1D_float(read_only image1d_array_t input,
                                        write_only image1d_array_t output) {
  int2 coord = (int2)(0, 0);
  const sampler_t samplerNearest =
      CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_NEAREST | CLK_ADDRESS_CLAMP;
  write_imagef(output, coord, read_imagef(input, samplerNearest, coord));
}

__kernel void copy_image_array_2D(read_only image2d_array_t input,
                                  write_only image2d_array_t output) {
  int4 coord = (int4)(0, 0, 0, 0);

  const sampler_t samplerNearest =
      CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_NEAREST | CLK_ADDRESS_CLAMP;
  write_imagei(output, coord, read_imagei(input, samplerNearest, coord));
}
__kernel void copy_image_array_2D_float(read_only image2d_array_t input,
                                        write_only image2d_array_t output) {
  int4 coord = (int4)(0, 0, 0, 0);
  const sampler_t samplerNearest =
      CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_NEAREST | CLK_ADDRESS_CLAMP;
  write_imagef(output, coord, read_imagef(input, samplerNearest, coord));
}

__kernel void read_image1D_float4(read_only image1d_t input,
                                  __global int *result) {

  int coord = 0;
  const sampler_t samplerNearest =
      CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_NEAREST | CLK_ADDRESS_NONE;
  float4 input_element = read_imagef(input, samplerNearest, coord);
  result[0] = input_element.x;
  result[1] = input_element.y;
  result[2] = input_element.z;
  result[3] = input_element.w;
}
__kernel void read_image2D_float4(read_only image2d_t input,
                                  __global int *result) {

  int2 coord = (int2)(0, 0);
  const sampler_t samplerNearest =
      CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_NEAREST | CLK_ADDRESS_NONE;
  float4 input_element = read_imagef(input, samplerNearest, coord);
  result[0] = input_element.z;
}

__kernel void read_write_image1D_int4(read_only image1d_t input,
                                      write_only image1d_t output) {

  int coord = 0;
  const sampler_t samplerNearest =
      CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_NEAREST | CLK_ADDRESS_NONE;
  float4 input_element = read_imagef(input, samplerNearest, coord);
  if (input_element.x == 0) {
    input_element.x = 1;
  } else {
    input_element.x = 0;
  }
  if (input_element.y == 0) {
    input_element.y = 1;
  } else {
    input_element.y = 0;
  }
  if (input_element.z == 0) {
    input_element.z = 1;
  } else {
    input_element.z = 0;
  }
  if (input_element.w == 0) {
    input_element.w = 1;
  } else {
    input_element.w = 0;
  }
  write_imagef(output, coord, input_element);
}
__kernel void read_write_image2D_int4(read_only image2d_t input,
                                      write_only image2d_t output) {

  int2 coord = (int2)(0, 0);
  const sampler_t samplerNearest =
      CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_NEAREST | CLK_ADDRESS_NONE;
  int4 input_element = read_imagei(input, samplerNearest, coord);
  if (input_element.x == 0) {
    input_element.x = 1;
  } else {
    input_element.x = 0;
  }
  if (input_element.y == 0) {
    input_element.y = 1;
  } else {
    input_element.y = 0;
  }
  if (input_element.z == 0) {
    input_element.z = 1;
  } else {
    input_element.z = 0;
  }
  if (input_element.w == 0) {
    input_element.w = 1;
  } else {
    input_element.w = 0;
  }
  write_imagei(output, coord, input_element);
}

/* canot write to 3d image yet...
__kernel void read_write_image3D_int4(read_only image3d_t input, write_only
image3d_t output)
{

        int4 coord = (int4)(0,0,0,0);
        const sampler_t samplerNearest = CLK_NORMALIZED_COORDS_FALSE |
CLK_FILTER_NEAREST | CLK_ADDRESS_NONE; int4 input_element = read_imagei( input,
samplerNearest, coord ); if(input_element.x == 0){ input_element.x = 1; }else{
                input_element.x = 0;
        }
        if(input_element.y == 0){
                input_element.y = 1;
        }else{
                input_element.y = 0;
        }
        if(input_element.z == 0){
                input_element.z = 1;
        }else{
                input_element.z = 0;
        }
        if(input_element.w == 0){
                input_element.w = 1;
        }else{
                input_element.w = 0;
        }
        write_imagei(output,coord,input_element);
}
*/
__kernel void write_image1D_int4(write_only image1d_t input) {

  int coord = 0;
  int4 value = (int4)(1, 1, 1, 1);
  write_imagei(input, coord, value);
}

__kernel void write_image1D_int4_CPU(write_only image1d_array_t input) {

  int2 coord = (int2)(0, 0);
  int4 value = (int4)(1, 1, 1, 1);
  write_imagei(input, coord, value);
}
__kernel void write_image1D_int4_GPU(write_only image1d_array_t input) {

  int2 coord = (int2)(0, 1);
  int4 value = 1;
  write_imagei(input, coord, value);
}

__kernel void write_image2D_int4_CPU(write_only image2d_array_t input) {

  int4 coord = (int4)(0, 0, 0, 0);
  int4 value = 1;
  write_imagei(input, coord, value);
}
__kernel void write_image2D_int4_GPU(write_only image2d_t input) {

  int2 coord = (int2)(0, 1);
  int4 value = 1;
  write_imagei(input, coord, value);
}

__kernel void change_1_to_2_image1D_int4(read_only image1d_t input,
                                         write_only image1d_t output) {

  int coord = 0;
  const sampler_t samplerNearest =
      CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_NEAREST | CLK_ADDRESS_NONE;
  int4 input_element = read_imagei(input, samplerNearest, coord);

  int4 value = 1;

  if (value.x == input_element.x && value.y == input_element.y &&
      value.z == input_element.z && value.w == input_element.w) {
    int4 value = 2;
    write_imagei(output, coord, value);
  }
}

__kernel void
change_1_to_2_image_array_1D_int4(read_only image1d_array_t input,
                                  write_only image1d_array_t output) {

  int2 coord = (int2)(0, 0);
  const sampler_t samplerNearest =
      CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_NEAREST | CLK_ADDRESS_NONE;
  int4 input_element = read_imagei(input, samplerNearest, coord);

  int4 value = 1;

  if (value.x == input_element.x && value.y == input_element.y &&
      value.z == input_element.z && value.w == input_element.w) {
    int4 value = 2;
    write_imagei(output, coord, value);
  }
}

__kernel void write_image2D_int4(write_only image2d_t input) {

  int2 coord = (int2)(0, 0);
  int4 value = (int4)(1, 1, 1, 1);
  write_imagei(input, coord, value);
}

__kernel void change_1_to_2_image2D_int4(read_only image2d_t input,
                                         write_only image2d_t output) {

  int2 coord = (int2)(0, 0);
  const sampler_t samplerNearest =
      CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_NEAREST | CLK_ADDRESS_NONE;
  int4 input_element = read_imagei(input, samplerNearest, coord);

  int4 value = 1;

  if (value.x == input_element.x && value.y == input_element.y) {
    int4 value = 2;
    write_imagei(output, coord, value);
  }
}

__kernel void write_image2D_int4_array(write_only image2d_array_t input) {

  int4 coord = (int4)(0, 0, 0, 0);
  int4 value = (int4)(1, 1, 1, 1);
  write_imagei(input, coord, value);
}

__kernel void
change_1_to_2_image_array_2D_int4(read_only image2d_array_t input,
                                  write_only image2d_array_t output) {

  int4 coord = (int4)(0, 0, 0, 0);
  const sampler_t samplerNearest =
      CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_NEAREST | CLK_ADDRESS_NONE;
  int4 input_element = read_imagei(input, samplerNearest, coord);

  int4 value = 1;

  if (value.x == input_element.x && value.y == input_element.y) {
    int4 value = 2;
    write_imagei(output, coord, value);
  }
}

/* writing to 3d images is not suported yet..
__kernel void write_image3D_int4(write_only image3d_t input)
{

        int4 coord = (int4)(0,0,0,0);
        int4 value = (int4)(1,1,1,1);
        write_imagei(input,coord,value);
}

__kernel void change_1_to_2_image3D_int4(read_only image3d_t input, write_only
image3d_t output)
{

        int4 coord = int4(0,0,0,0);
        const sampler_t samplerNearest = CLK_NORMALIZED_COORDS_FALSE |
CLK_FILTER_NEAREST | CLK_ADDRESS_NONE; int4 input_element = read_imagei(input,
samplerNearest, coord );

        int4 value = 1;

        if( value == input_element){
                int4 value = 2;
                write_imagei(input,coord,value);
        }
}
*/