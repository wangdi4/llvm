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

class VR24 : public CommonRuntime {};

//|  TEST: VR24.CL3DImage (TC-28 and TC-29)
//|
//|  Purpose
//|  -------
//|
//|  Verify that the eextension cl_khr_3d_image_writes is supported by GPU
//(TC-28) | and is not supported by CPU (TC-29)
//|
//|  Method
//|  ------
//|
//|  create compile and execute a GPU kernel with image3d_t and write_only
// access | creata and compile a CPU kernel with image3d_t and write_only access
//|
//|  Pass criteria
//|  -------------
//|
//|  should succeed on gpu only context, fail on cpu only
//|

/*simple kernel for writing to image3d (sets image to 1.0fs)*/
const char *clsrc[] = {
    "#pragma OPENCL EXTENSION cl_khr_3d_image_writes : enable\n\
            __kernel void volume_kernel(__write_only image3d_t volume, uint dimx, uint dimy, uint dimz)\n\
             {\n\
              int x = get_global_id(0);\n\
              int y = get_global_id(1);\n\
              int z = get_global_id(2);\n\
              if(x < dimx && y < dimy && z < dimz)\n\
                write_imagef(volume,(int4)(x,y,z,0),(float4)(1.0f,1.0f,1.0f,1.0f));\n\
                }\n"};

TEST_F(VR24, CL3DImage) {
  // simple 0 volume;
  int dimx = 8, dimy = 8, dimz = 8;
  float *vol = new float[dimx * dimy * dimz * 4];
  float *test = new float[dimx * dimy * dimz * 4];
  memset(vol, 0, dimz * dimy * dimz * 4 * sizeof(float));
  memset(test, 0, dimz * dimy * dimz * 4 * sizeof(float));

  // work size
  size_t globalWorkSize[] = {dimx, dimy, dimz};
  // Crreate OpenCL context
  cl_int err;
  // Get Platforms
  cl_uint num_entries = 2;
  cl_uint num_platforms = 0;
  cl_platform_id platforms[] = {0, 0, 0};
  ASSERT_NO_FATAL_FAILURE(
      getPlatformIDs(num_entries, platforms, &num_platforms));
  const int maxdevices = 4; // assume no more than 4 devices in platform
  cl_uint numdevices;
  // get gpu only devices
  cl_device_id devices[maxdevices];
  err = clGetDeviceIDs(platforms[0], getSecondDeviceType(), maxdevices, devices,
                       &numdevices);
  EXPECT_EQ(err, CL_SUCCESS) << "Error getting GPU ONLY DEVICE\n";
  EXPECT_GT(numdevices, (unsigned int)0) << "Error No Devices returned";
  // properties for context
  cl_context_properties ctxProps[] = {
      CL_CONTEXT_PLATFORM,
      (cl_context_properties)platforms[0], // assumes intel is first platform
      0};

  if (err == CL_SUCCESS && numdevices > 0 &&
      !isAccelerator()) // not supported on MIC
  {
    // create device and queue from first device
    cl_context devcontext =
        clCreateContext(ctxProps, 1, devices, NULL, NULL, &err);
    ASSERT_EQ(CL_SUCCESS, err) << "Error creating gpu device context";
    cl_command_queue clq =
        clCreateCommandQueue(devcontext, devices[0], 0, &err);
    ASSERT_EQ(CL_SUCCESS, err) << "Error creating gpu command queue";
    cl_program program;
    program = clCreateProgramWithSource(devcontext, 1, (const char **)&clsrc,
                                        NULL, &err);
    EXPECT_EQ(CL_SUCCESS, err) << "Failed to create program from source";
    err = clBuildProgram(program, 1, devices, "", NULL, NULL);
    EXPECT_EQ(CL_SUCCESS, err) << "Failed to build program";
    if (err != CL_SUCCESS) {
      //  prints build fail log
      cl_int logStatus;
      char *buildLog = NULL;
      size_t buildLogSize = 0;
      logStatus =
          clGetProgramBuildInfo(program, devices[0], CL_PROGRAM_BUILD_LOG,
                                buildLogSize, buildLog, &buildLogSize);

      buildLog = (char *)malloc(buildLogSize);
      memset(buildLog, 0, buildLogSize);

      logStatus =
          clGetProgramBuildInfo(program, devices[0], CL_PROGRAM_BUILD_LOG,
                                buildLogSize, buildLog, NULL);

      std::cout << " \n\t\t\tBUILD LOG\n";
      std::cout << " ************************************************\n";
      std::cout << buildLog << std::endl;
      std::cout << " ************************************************\n";
      free(buildLog);
    } else {
      // create kernel
      cl_kernel kernel = clCreateKernel(program, "volume_kernel", &err);
      ASSERT_EQ(CL_SUCCESS, err)
          << "Failed to create kernel for \"volume_kernel\"";
      // create 3d image
      cl_image_format volform;
      volform.image_channel_order = CL_RGBA;
      volform.image_channel_data_type = CL_FLOAT;
      cl_mem devvol =
          clCreateImage3D(devcontext, CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR,
                          &volform, dimx, dimy, dimz, 0, 0, vol, &err);
      ASSERT_EQ(CL_SUCCESS, err) << "Failed to create 3D Image";
      // set args
      err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &devvol);
      err |= clSetKernelArg(kernel, 1, sizeof(int), &dimx);
      err |= clSetKernelArg(kernel, 2, sizeof(int), &dimy);
      err |= clSetKernelArg(kernel, 3, sizeof(int), &dimz);
      ASSERT_EQ(CL_SUCCESS, err) << "Failed to set kernel args\n";
      err = clEnqueueNDRangeKernel(clq, kernel, 3, NULL, globalWorkSize, NULL,
                                   0, 0, 0);
      EXPECT_EQ(CL_SUCCESS, err) << "Failed to enqueue kernel";
      EXPECT_EQ(CL_SUCCESS, clFinish(clq)) << "Failed to finish kernel";
      // read in image and check values
      size_t origin[] = {0, 0, 0};
      size_t region[] = {dimx, dimy, dimz};
      for (int i = 0; i < dimx * dimy * dimz * 4; i++)
        test[i] = -1.0f;
      err = clEnqueueReadImage(clq, devvol, true, origin, region, 0, 0, test, 0,
                               0, 0);
      EXPECT_EQ(CL_SUCCESS, err) << "Failed to read volume";
      bool correct = true;
      int i;
      for (i = 0; i < dimx * dimy * dimz * 4; i++)
        if (test[i] != 1.0f) {
          correct = false;
          break;
        }
      EXPECT_EQ(true, correct)
          << "read in " << test[i] << " instead of 1.0f at index " << i;

      clReleaseKernel(kernel);
      clReleaseProgram(program);
      clReleaseMemObject(devvol);
    }
    clReleaseContext(devcontext);
    clReleaseCommandQueue(clq);
  }

  /***Same thing for CPU only device***/
  err = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_CPU, maxdevices, devices,
                       &numdevices);
  ASSERT_EQ(err, CL_SUCCESS) << "Error getting CPU ONLY DEVICE\n";
  ASSERT_GT(numdevices, (unsigned int)0) << "Error No Devices returned";

  if (err == CL_SUCCESS && numdevices > 0) {
    // create device and queue from first device
    cl_context devcontext =
        clCreateContext(ctxProps, 1, devices, NULL, NULL, &err);
    ASSERT_EQ(CL_SUCCESS, err) << "Error creating cpu device context";
    cl_command_queue clq =
        clCreateCommandQueue(devcontext, devices[0], 0, &err);
    ASSERT_EQ(CL_SUCCESS, err) << "Error creating cpu command queue";
    cl_program program;
    program = clCreateProgramWithSource(devcontext, 1, (const char **)&clsrc,
                                        NULL, &err);
    EXPECT_EQ(CL_SUCCESS, err) << "Failed to create program from source";
    err = clBuildProgram(program, 1, devices, "", NULL, NULL);
    EXPECT_EQ(CL_SUCCESS, err)
        << "Failed to built program with write_only image3D on CPU";
    if (err != CL_SUCCESS) {
      //  prints build fail log
      cl_int logStatus;
      char *buildLog = NULL;
      size_t buildLogSize = 0;
      logStatus =
          clGetProgramBuildInfo(program, devices[0], CL_PROGRAM_BUILD_LOG,
                                buildLogSize, buildLog, &buildLogSize);

      buildLog = (char *)malloc(buildLogSize);
      memset(buildLog, 0, buildLogSize);

      logStatus =
          clGetProgramBuildInfo(program, devices[0], CL_PROGRAM_BUILD_LOG,
                                buildLogSize, buildLog, NULL);

      std::cout << " \n\t\t\tBUILD LOG\n";
      std::cout << " ************************************************\n";
      std::cout << buildLog << std::endl;
      std::cout << " ************************************************\n";
      free(buildLog);
    } else {
      // create kernel
      cl_kernel kernel = clCreateKernel(program, "volume_kernel", &err);
      ASSERT_EQ(CL_SUCCESS, err)
          << "Failed to create kernel for \"volume_kernel\"";
      // create 3d image
      cl_image_format volform;
      volform.image_channel_order = CL_RGBA;
      volform.image_channel_data_type = CL_FLOAT;
      cl_mem devvol =
          clCreateImage3D(devcontext, CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR,
                          &volform, dimx, dimy, dimz, 0, 0, vol, &err);
      ASSERT_EQ(CL_SUCCESS, err) << "Failed to create 3D Image";
      // set args
      err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &devvol);
      err |= clSetKernelArg(kernel, 1, sizeof(int), &dimx);
      err |= clSetKernelArg(kernel, 2, sizeof(int), &dimy);
      err |= clSetKernelArg(kernel, 3, sizeof(int), &dimz);
      ASSERT_EQ(CL_SUCCESS, err) << "Failed to set kernel args\n";
      err = clEnqueueNDRangeKernel(clq, kernel, 3, NULL, globalWorkSize, NULL,
                                   0, 0, 0);
      EXPECT_EQ(CL_SUCCESS, err) << "Failed to enqueue kernel";
      EXPECT_EQ(CL_SUCCESS, clFinish(clq)) << "Failed to finish kernel";
      // read in image and check values
      size_t origin[] = {0, 0, 0};
      size_t region[] = {dimx, dimy, dimz};
      for (int i = 0; i < dimx * dimy * dimz * 4; i++)
        test[i] = -1.0f;
      err = clEnqueueReadImage(clq, devvol, true, origin, region, 0, 0, test, 0,
                               0, 0);
      EXPECT_EQ(CL_SUCCESS, err) << "Failed to read volume";
      bool correct = true;
      int i;
      for (i = 0; i < dimx * dimy * dimz * 4; i++)
        if (test[i] != 1.0f) {
          correct = false;
          break;
        }
      EXPECT_EQ(true, correct)
          << "read in " << test[i] << " instead of 1.0f at index " << i;

      clReleaseKernel(kernel);
      clReleaseProgram(program);
      clReleaseMemObject(devvol);
    }
    clReleaseContext(devcontext);
    clReleaseCommandQueue(clq);
  }

  delete[] vol;
  delete[] test;
}
