
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#if !defined(_WIN32)
#include <stdbool.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>

#include "procs.h"

int test_simple_write_image_pitch(cl_device_id device, cl_context cl_context_, cl_command_queue q, int num_elements)
{
  cl_int err = CL_SUCCESS;
  
  size_t imageW = 143;
  size_t imageH = 151;
  size_t bufferW = 151*4;
  size_t bufferH = 151;
  
  size_t pixel_bytes = 4;
  size_t image_bytes = imageW * imageH * pixel_bytes;
  
  size_t buffer_bytes = bufferW * bufferH;

  PASSIVE_REQUIRE_IMAGE_SUPPORT( device );
  
  char* host_image = (char*)malloc(image_bytes);
  memset(host_image,0x0,image_bytes);
  
  cl_image_format fmt = { 0 };
  fmt.image_channel_order     = CL_RGBA;
  fmt.image_channel_data_type = CL_UNORM_INT8;
  
  cl_image_desc desc = { 0 };
  desc.image_type         = CL_MEM_OBJECT_IMAGE2D;
  desc.image_width        = imageW;
  desc.image_height       = imageH;
  
	cl_mem image;
	image = clCreateImage(cl_context_, CL_MEM_COPY_HOST_PTR|CL_MEM_READ_WRITE, NULL, &desc, host_image, &err);
  image = clCreateImage(cl_context_, CL_MEM_COPY_HOST_PTR|CL_MEM_READ_WRITE, &fmt, &desc, host_image, &err);
  test_error(err,"clCreateImage");
  
  char* host_buffer = (char*)malloc(buffer_bytes);
  memset(host_buffer,0xa,buffer_bytes);
  
  // Test reading from the image
  size_t origin[] = { 0, 0, 0 };
  size_t region[] = { imageW, imageH, 1 };
  
  err = clEnqueueWriteImage(q, NULL, CL_TRUE, origin, region, bufferW, 0, host_buffer, 0, NULL, NULL);
  err = clEnqueueWriteImage(q, image, CL_TRUE, origin, region, bufferW, 0, host_buffer, 0, NULL, NULL);
  test_error(err,"clEnqueueWriteImage");
  
  size_t mapped_pitch = 0;
  char* mapped_image;
  mapped_image = (char*)clEnqueueMapImage(q, NULL, CL_TRUE, CL_MAP_READ, origin, region, &mapped_pitch, NULL, 0, NULL, NULL, &err);
  mapped_image = (char*)clEnqueueMapImage(q, image, CL_TRUE, CL_MAP_READ, origin, region, &mapped_pitch, NULL, 0, NULL, NULL, &err);
  test_error(err,"clEnqueueMapImage");
  
  size_t errors = 0;
  for (size_t j=0;j<imageH;++j) {
    for (size_t i=0;i<mapped_pitch;++i) {
      char val = mapped_image[j*mapped_pitch+i];
      if ((i<imageW*pixel_bytes) && (val != 0xa)) {
        log_error("Bad value %x in image at (byte: %lu, row: %lu)\n",val,i,j);
        ++errors;
      }
    }
  }

  err = clEnqueueUnmapMemObject(q, NULL, (void *)mapped_image, 0, 0, 0);
  err = clEnqueueUnmapMemObject(q, image, (void *)mapped_image, 0, 0, 0);
  test_error(err,"clEnqueueUnmapMemObject");
  
  test_error(clReleaseMemObject(image),"clReleaseMemObject");
  free(host_image);
  free(host_buffer);
  
  return CL_SUCCESS;
}
