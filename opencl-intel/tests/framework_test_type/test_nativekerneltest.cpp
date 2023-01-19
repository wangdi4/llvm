//| TESTSUITE: NativeKernelSuite
//|
//| Test the implementation of native kernels + their interaction and
//| memory object sharing with OpenCL
//|
#include "CL/cl.h"
#include "FrameworkTest.h"
#include "cl_types.h"
#include "gtest_wrapper.h"
#include <cstdlib>
#include <cstring>
#include <iostream>

using namespace std;

extern cl_device_type gDeviceType;

// The copying native kernel function simply copies an input buffer into the
// output buffer.
//
struct copying_native_kernel_arg_type {
  cl_uchar *input_buf;
  cl_uchar *output_buf;
  size_t bufsize;
};

static void CL_CALLBACK copying_native_kernel_func(void *arg_) {
  copying_native_kernel_arg_type *arg = (copying_native_kernel_arg_type *)arg_;

  for (size_t i = 0; i < arg->bufsize; ++i)
    arg->output_buf[i] = arg->input_buf[i];
}

// The adding native kernel function adds two source buffers element-wise into
// a destination buffer.
//
struct adding_native_kernel_arg_type {
  cl_uchar *input1_buf;
  cl_uchar *input2_buf;
  cl_uchar *output_buf;
  size_t bufsize;
};

static void CL_CALLBACK adding_native_kernel_func(void *arg_) {
  adding_native_kernel_arg_type *arg = (adding_native_kernel_arg_type *)arg_;

  for (size_t i = 0; i < arg->bufsize; ++i)
    arg->output_buf[i] = arg->input1_buf[i] + arg->input2_buf[i];
}

class NativeKernelSuite : public ::testing::Test {
protected:
  // Common setup code for tests in this suite - creates the device, context
  // and a command queue.
  //
  virtual void SetUp() {
    cl_platform_id platform_id;
    cl_int rc = clGetPlatformIDs(1, &platform_id, 0);
    ASSERT_EQ(CL_SUCCESS, rc);

    cl_uint num_devices;
    rc = clGetDeviceIDs(platform_id, gDeviceType, 0, 0, &num_devices);
    ASSERT_EQ(CL_SUCCESS, rc);
    m_devices = (cl_device_id *)malloc(num_devices * sizeof(*m_devices));
    ASSERT_TRUE(m_devices != 0);
    rc = clGetDeviceIDs(platform_id, gDeviceType, num_devices, m_devices, 0);
    ASSERT_EQ(CL_SUCCESS, rc);

    cl_context_properties props[3] = {CL_CONTEXT_PLATFORM,
                                      (cl_context_properties)platform_id, 0};

    m_context = clCreateContext(props, 1, &m_devices[0], 0, 0, &rc);
    ASSERT_EQ(CL_SUCCESS, rc);

    m_command_queue =
        clCreateCommandQueueWithProperties(m_context, m_devices[0], 0, &rc);
    ASSERT_EQ(CL_SUCCESS, rc);
  }

  virtual void TearDown() {
    cl_int rc = clReleaseCommandQueue(m_command_queue);
    ASSERT_EQ(CL_SUCCESS, rc);
    rc = clReleaseContext(m_context);
    ASSERT_EQ(CL_SUCCESS, rc);
    free(m_devices);
  }

  // Common code for running a copying native kernel with the given buffers
  // and arguments.
  //
  virtual void execute_copying_native_kernel(cl_mem mem_buffer_src,
                                             cl_mem mem_buffer_dst,
                                             size_t bufsize) {
    cl_mem mem_list[] = {mem_buffer_src, mem_buffer_dst};
    void *buf_loc_ptrs[2];
    copying_native_kernel_arg_type kernel_arg;
    memset(&kernel_arg, 0, sizeof(kernel_arg));
    kernel_arg.bufsize = bufsize;
    buf_loc_ptrs[0] = (void *)(&(kernel_arg.input_buf));
    buf_loc_ptrs[1] = (void *)(&(kernel_arg.output_buf));

    cl_int rc =
        clEnqueueNativeKernel(m_command_queue, copying_native_kernel_func,
                              &kernel_arg, sizeof(kernel_arg), 2, mem_list,
                              const_cast<const void **>(buf_loc_ptrs), 0, 0, 0);
    ASSERT_EQ(CL_SUCCESS, rc);
  }

protected:
  cl_command_queue m_command_queue;
  cl_context m_context;
  cl_device_id *m_devices;
};

//| TEST: NativeKernelSuite.basic_write_read
//|
//| Purpose
//| -------
//|
//| Test basic usage of a native kernel with buffers via read/write.
//|
//| Method
//| ------
//|
//| 1. Create an input buffer and an output buffer
//| 2. Enqueue a write into the input buffer
//| 3. Enqueue execution of a native kernel that copies the input buffer into
//|    the output buffer.
//| 4. Enqueue a read from the output buffer
//|
//| Pass criteria
//| -------------
//|
//| The data was correctly copied into the output buffer.
//|
TEST_F(NativeKernelSuite, basic_write_read) {
  size_t bufsize = 512;
  size_t total_buf_size = bufsize * sizeof(cl_uchar);

  cl_uchar *src_data = new cl_uchar[bufsize];
  for (size_t i = 0; i < bufsize; ++i)
    src_data[i] = (cl_uchar)i * 8;
  cl_uchar *dst_data = new cl_uchar[bufsize];

  cl_int rc;
  cl_mem mem_buffer_src =
      clCreateBuffer(m_context, CL_MEM_READ_ONLY, total_buf_size, 0, &rc);
  ASSERT_EQ(CL_SUCCESS, rc);
  cl_mem mem_buffer_dst =
      clCreateBuffer(m_context, CL_MEM_READ_WRITE, total_buf_size, 0, &rc);
  ASSERT_EQ(CL_SUCCESS, rc);

  rc = clEnqueueWriteBuffer(m_command_queue, mem_buffer_src, CL_FALSE, 0,
                            total_buf_size, src_data, 0, 0, 0);
  ASSERT_EQ(CL_SUCCESS, rc);

  execute_copying_native_kernel(mem_buffer_src, mem_buffer_dst, bufsize);

  rc = clEnqueueReadBuffer(m_command_queue, mem_buffer_dst, CL_TRUE, 0,
                           total_buf_size, dst_data, 0, 0, 0);
  ASSERT_EQ(CL_SUCCESS, rc);

  // Verify
  EXPECT_EQ(0, memcmp(src_data, dst_data, total_buf_size));

  delete[] src_data;
  delete[] dst_data;
  rc = clReleaseMemObject(mem_buffer_src);
  ASSERT_EQ(CL_SUCCESS, rc);
  rc = clReleaseMemObject(mem_buffer_dst);
  ASSERT_EQ(CL_SUCCESS, rc);
  rc = clFinish(m_command_queue);
  ASSERT_EQ(CL_SUCCESS, rc);
}

//| TEST: NativeKernelSuite.hostptr_read
//|
//| Purpose
//| -------
//|
//| Test basic usage of native kernel with input buffer created from host
//| pointer.
//|
//| Method
//| ------
//|
//| 1. Create an input buffer with CL_MEM_USE_HOST_PTR to take data from a host
//|    pointer, and an output buffer
//| 2. Enqueue execution of a native kernel that copies the input buffer into
//|    the output buffer
//| 3. Enqueue a read from the output buffer
//|
//| Pass criteria
//| -------------
//|
//| The data was correctly copied into the output buffer.
//|
TEST_F(NativeKernelSuite, hostptr_read) {
  size_t bufsize = 512;
  size_t total_buf_size = bufsize * sizeof(cl_uchar);

  cl_uchar *src_data = new cl_uchar[bufsize];
  for (size_t i = 0; i < bufsize; ++i)
    src_data[i] = (cl_uchar)i * 8;
  cl_uchar *dst_data = new cl_uchar[bufsize];

  cl_int rc;
  cl_mem mem_buffer_src =
      clCreateBuffer(m_context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                     total_buf_size, src_data, &rc);
  ASSERT_EQ(CL_SUCCESS, rc);
  cl_mem mem_buffer_dst =
      clCreateBuffer(m_context, CL_MEM_READ_WRITE, total_buf_size, 0, &rc);
  ASSERT_EQ(CL_SUCCESS, rc);

  execute_copying_native_kernel(mem_buffer_src, mem_buffer_dst, bufsize);

  rc = clEnqueueReadBuffer(m_command_queue, mem_buffer_dst, CL_TRUE, 0,
                           total_buf_size, dst_data, 0, 0, 0);
  ASSERT_EQ(CL_SUCCESS, rc);

  // Verify
  EXPECT_EQ(0, memcmp(src_data, dst_data, total_buf_size));

  delete[] src_data;
  delete[] dst_data;
  rc = clReleaseMemObject(mem_buffer_src);
  ASSERT_EQ(CL_SUCCESS, rc);
  rc = clReleaseMemObject(mem_buffer_dst);
  ASSERT_EQ(CL_SUCCESS, rc);
  rc = clFinish(m_command_queue);
  ASSERT_EQ(CL_SUCCESS, rc);
}

//| TEST: NativeKernelSuite.native_to_ocl
//|
//| Purpose
//| -------
//|
//| Test passing of data in a buffer from a native kernel to an OpenCL kernel.
//|
//| Method
//| ------
//|
//| 1. Create three buffers of cl_uchar:
//|
//|   a. buf_src - read only
//|   b. buf_temp - r/w
//|   c. buf_dst - write only
//|
//| 2. Enqueue a write of data into buf_src
//| 3. Enqueue a native kernel and an OpenCL kernel as follows:
//|
//|   a. Native kernel - copies data from input to output buffer. Its input is
//|      buf_src and output is buf_temp
//|   b. OpenCL kernel - inverts (logical NOT) the data in input into output
//|      buffer. Its input is buf_temp and output is buf_dst
//|
//| 4. Enqueue a read from buf_dst
//|
//| Pass criteria
//| -------------
//|
//| The data in buf_src is correctly copied and inverted into buf_dst.
//|
TEST_F(NativeKernelSuite, native_to_ocl) {
  // Prepare a kernel object for the OCL kernel.
  //
  const char *ocl_invert_kernel[] = {"__kernel void ocl_invert_kernel(__global "
                                     "uchar* src, __global uchar* dst)"
                                     "{"
                                     "    size_t tid = get_global_id(0);"
                                     "    dst[tid] = ~src[tid];"
                                     "}"};

  cl_int rc;
  cl_program ocl_program =
      clCreateProgramWithSource(m_context, 1, ocl_invert_kernel, NULL, &rc);
  ASSERT_EQ(CL_SUCCESS, rc);
  ASSERT_TRUE(ocl_program != NULL);

  rc = clBuildProgram(ocl_program, 0, NULL, NULL, NULL, NULL);
  ASSERT_EQ(CL_SUCCESS, rc);
  cl_kernel ocl_kernel = clCreateKernel(ocl_program, "ocl_invert_kernel", &rc);
  ASSERT_EQ(CL_SUCCESS, rc);
  ASSERT_TRUE(ocl_kernel != NULL);

  // Arrays for input and final output of the computation.
  //
  size_t bufsize = 512;
  size_t total_buf_size = bufsize * sizeof(cl_uchar);

  cl_uchar *src_data = new cl_uchar[bufsize];
  for (size_t i = 0; i < bufsize; ++i)
    src_data[i] = (cl_uchar)i * 8;
  cl_uchar *dst_data = new cl_uchar[bufsize];

  // Buffer objects for moving data between kernels and the host
  //
  cl_mem mem_buf_src =
      clCreateBuffer(m_context, CL_MEM_READ_ONLY, total_buf_size, 0, &rc);
  ASSERT_EQ(CL_SUCCESS, rc);
  cl_mem mem_buf_temp =
      clCreateBuffer(m_context, CL_MEM_READ_WRITE, total_buf_size, 0, &rc);
  ASSERT_EQ(CL_SUCCESS, rc);
  cl_mem mem_buf_dst =
      clCreateBuffer(m_context, CL_MEM_WRITE_ONLY, total_buf_size, 0, &rc);
  ASSERT_EQ(CL_SUCCESS, rc);

  // Fill command queue with computation steps
  //
  rc = clEnqueueWriteBuffer(m_command_queue, mem_buf_src, CL_FALSE, 0,
                            total_buf_size, src_data, 0, 0, 0);
  ASSERT_EQ(CL_SUCCESS, rc);
  execute_copying_native_kernel(mem_buf_src, mem_buf_temp, bufsize);

  rc = clSetKernelArg(ocl_kernel, 0, sizeof(mem_buf_temp),
                      (void *)&mem_buf_temp);
  ASSERT_EQ(CL_SUCCESS, rc);
  rc = clSetKernelArg(ocl_kernel, 1, sizeof(mem_buf_dst), (void *)&mem_buf_dst);
  ASSERT_EQ(CL_SUCCESS, rc);

  size_t global_work_size[1] = {bufsize};
  rc = clEnqueueNDRangeKernel(m_command_queue, ocl_kernel, 1, NULL,
                              global_work_size, NULL, 0, NULL, NULL);
  ASSERT_EQ(CL_SUCCESS, rc);

  rc = clEnqueueReadBuffer(m_command_queue, mem_buf_dst, CL_TRUE, 0,
                           total_buf_size, dst_data, 0, NULL, NULL);
  ASSERT_EQ(CL_SUCCESS, rc);

  // Verify
  //
  for (size_t i = 0; i < bufsize; ++i) {
    cl_uchar expected = ~src_data[i];
    EXPECT_EQ(expected, dst_data[i]);
  }

  delete[] src_data;
  delete[] dst_data;
  rc = clReleaseMemObject(mem_buf_src);
  ASSERT_EQ(CL_SUCCESS, rc);
  rc = clReleaseMemObject(mem_buf_temp);
  ASSERT_EQ(CL_SUCCESS, rc);
  rc = clReleaseMemObject(mem_buf_dst);
  ASSERT_EQ(CL_SUCCESS, rc);
  rc = clFinish(m_command_queue);
  ASSERT_EQ(CL_SUCCESS, rc);
  rc = clReleaseKernel(ocl_kernel);
  ASSERT_EQ(CL_SUCCESS, rc);
  rc = clReleaseProgram(ocl_program);
  ASSERT_EQ(CL_SUCCESS, rc);
}

//| TEST: NativeKernelSuite.ocl_ooo_to_native
//|
//| Purpose
//| -------
//|
//| Test passing data from OCL kernels to a native kernel running in an OOO
//| queue.
//|
//| Method
//| ------
//|
//| 1. Create 4 buffers of cl_uchar:
//|
//|   a. buf_src - read only
//|   b. buf_out_invert - r/w
//|   c. buf_out_mask - r/w
//|   c. buf_dst - write only
//|
//| 2. Enqueue a write of data into buf_src
//| 3. Enqueue two OCL kernels with buf_src as input:
//|
//|   a. Inverting kernel - inverts (logical NOT) the data in the input, writing
//|      into buf_out_invert.
//|   b. Masking kernel - masks the data in the input by logical-AND with 0x3F,
//|      writing into buf_out_mask.
//|
//| 4. Enqueue a native kernel that takes buf_out_invert and buf_out_mask as
//|    inputs, adds them up element-wise and writes the result into buf_dst.
//|
//| 5. Enqueue a read from buf_dst
//|
//| Pass criteria
//| -------------
//|
//| The data in buf_src is correctly transformed into buf_dst.
//|
TEST_F(NativeKernelSuite, ocl_ooo_to_native) {
  cl_int rc;

  // Create the command queue (not using m_command_queue here, since we need
  // an out-of-order queue.
  //
  cl_queue_properties props[] = {CL_QUEUE_PROPERTIES,
                                 CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, 0};
  cl_command_queue ooo_cmd_queue =
      clCreateCommandQueueWithProperties(m_context, m_devices[0], props, &rc);
  ASSERT_EQ(CL_SUCCESS, rc);

  // Prepare kernel objects for the OCL kernels.
  //
  const char *ocl_kernels[] = {
      "__kernel void ocl_invert_kernel(__global uchar* src, __global uchar* "
      "dst)"
      "{"
      "    size_t tid = get_global_id(0);"
      "    dst[tid] = ~src[tid];"
      "}",

      "__kernel void ocl_mask_kernel(__global uchar* src, __global uchar* dst)"
      "{"
      "    size_t tid = get_global_id(0);"
      "    dst[tid] = src[tid] & 0x3F;"
      "}",
  };

  cl_program ocl_program =
      clCreateProgramWithSource(m_context, 2, ocl_kernels, NULL, &rc);
  ASSERT_EQ(CL_SUCCESS, rc);
  ASSERT_TRUE(ocl_program != NULL);
  rc = clBuildProgram(ocl_program, 0, NULL, NULL, NULL, NULL);
  ASSERT_EQ(CL_SUCCESS, rc);

  cl_kernel ocl_invert_kernel =
      clCreateKernel(ocl_program, "ocl_invert_kernel", &rc);
  ASSERT_EQ(CL_SUCCESS, rc);
  ASSERT_TRUE(ocl_invert_kernel != NULL);
  cl_kernel ocl_mask_kernel =
      clCreateKernel(ocl_program, "ocl_mask_kernel", &rc);
  ASSERT_EQ(CL_SUCCESS, rc);
  ASSERT_TRUE(ocl_mask_kernel != NULL);

  // Arrays for input and final output of the computation.
  //
  size_t bufsize = 512 * 1024;
  size_t total_buf_size = bufsize * sizeof(cl_uchar);

  cl_uchar *src_data = new cl_uchar[bufsize];
  for (size_t i = 0; i < bufsize; ++i)
    src_data[i] = (cl_uchar)i * 7;
  cl_uchar *dst_data = new cl_uchar[bufsize];

  cl_uchar *zeros = new cl_uchar[bufsize];
  memset(zeros, 0, total_buf_size);

  // Buffer objects for moving data between kernels and the host.
  // Note: the source buffer is deliberately initialized to zeros to make sure
  // it was actually read correctly prior to being processed by the kernels.
  //
  cl_mem mem_buf_src =
      clCreateBuffer(m_context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                     total_buf_size, zeros, &rc);

  ASSERT_EQ(CL_SUCCESS, rc);
  cl_mem mem_buf_out_invert =
      clCreateBuffer(m_context, CL_MEM_READ_WRITE, total_buf_size, 0, &rc);
  ASSERT_EQ(CL_SUCCESS, rc);
  cl_mem mem_buf_out_mask =
      clCreateBuffer(m_context, CL_MEM_READ_WRITE, total_buf_size, 0, &rc);
  ASSERT_EQ(CL_SUCCESS, rc);
  cl_mem mem_buf_dst =
      clCreateBuffer(m_context, CL_MEM_WRITE_ONLY, total_buf_size, 0, &rc);
  ASSERT_EQ(CL_SUCCESS, rc);

  // Fill command queue with computation steps...
  //

  // Write input buffer
  cl_event ev_writebuf;
  rc = clEnqueueWriteBuffer(ooo_cmd_queue, mem_buf_src, CL_FALSE, 0,
                            total_buf_size, src_data, 0, 0, &ev_writebuf);
  ASSERT_EQ(CL_SUCCESS, rc);

  // Enqueue inverting OCL kernel
  rc = clSetKernelArg(ocl_invert_kernel, 0, sizeof(mem_buf_src),
                      (void *)&mem_buf_src);
  ASSERT_EQ(CL_SUCCESS, rc);
  rc = clSetKernelArg(ocl_invert_kernel, 1, sizeof(mem_buf_out_invert),
                      (void *)&mem_buf_out_invert);
  ASSERT_EQ(CL_SUCCESS, rc);

  size_t global_work_size[1] = {bufsize};

  cl_event ev_invert;
  rc = clEnqueueNDRangeKernel(ooo_cmd_queue, ocl_invert_kernel, 1, NULL,
                              global_work_size, NULL, 1, &ev_writebuf,
                              &ev_invert);
  ASSERT_EQ(CL_SUCCESS, rc);

  // Enqueue masking OCL kernel
  rc = clSetKernelArg(ocl_mask_kernel, 0, sizeof(mem_buf_src),
                      (void *)&mem_buf_src);
  ASSERT_EQ(CL_SUCCESS, rc);
  rc = clSetKernelArg(ocl_mask_kernel, 1, sizeof(mem_buf_out_mask),
                      (void *)&mem_buf_out_mask);
  ASSERT_EQ(CL_SUCCESS, rc);

  cl_event ev_mask;
  rc =
      clEnqueueNDRangeKernel(ooo_cmd_queue, ocl_mask_kernel, 1, NULL,
                             global_work_size, NULL, 1, &ev_writebuf, &ev_mask);
  ASSERT_EQ(CL_SUCCESS, rc);

  // Enqueue native kernel
  cl_mem mem_list[] = {mem_buf_out_invert, mem_buf_out_mask, mem_buf_dst};
  void *buf_loc_ptrs[3];
  adding_native_kernel_arg_type kernel_arg;
  memset(&kernel_arg, 0, sizeof(kernel_arg));
  kernel_arg.bufsize = bufsize;
  buf_loc_ptrs[0] = (void *)(&(kernel_arg.input1_buf));
  buf_loc_ptrs[1] = (void *)(&(kernel_arg.input2_buf));
  buf_loc_ptrs[2] = (void *)(&(kernel_arg.output_buf));

  cl_event ev_native;
  cl_event native_wait_list[] = {ev_invert, ev_mask};
  rc = clEnqueueNativeKernel(ooo_cmd_queue, adding_native_kernel_func,
                             &kernel_arg, sizeof(kernel_arg), 3, mem_list,
                             const_cast<const void **>(buf_loc_ptrs), 2,
                             native_wait_list, &ev_native);
  ASSERT_EQ(CL_SUCCESS, rc);

  // Enqueue blocking read to force computation
  rc = clEnqueueReadBuffer(ooo_cmd_queue, mem_buf_dst, CL_TRUE, 0,
                           total_buf_size, dst_data, 1, &ev_native, NULL);

  // Verify
  //
  for (size_t i = 0; i < bufsize; ++i) {
    cl_uchar expected = ~src_data[i] + (0x3F & src_data[i]);
    EXPECT_EQ(expected, dst_data[i]);
  }

  // Cleanup
  //
  delete[] zeros;
  delete[] src_data;
  delete[] dst_data;
  rc = clFinish(ooo_cmd_queue);
  ASSERT_EQ(CL_SUCCESS, rc);
  rc = clReleaseEvent(ev_native);
  ASSERT_EQ(CL_SUCCESS, rc);
  rc = clReleaseEvent(ev_writebuf);
  ASSERT_EQ(CL_SUCCESS, rc);
  rc = clReleaseEvent(ev_invert);
  ASSERT_EQ(CL_SUCCESS, rc);
  rc = clReleaseEvent(ev_mask);
  ASSERT_EQ(CL_SUCCESS, rc);
  rc = clReleaseCommandQueue(ooo_cmd_queue);
  ASSERT_EQ(CL_SUCCESS, rc);
  rc = clReleaseMemObject(mem_buf_src);
  ASSERT_EQ(CL_SUCCESS, rc);
  rc = clReleaseMemObject(mem_buf_out_invert);
  ASSERT_EQ(CL_SUCCESS, rc);
  rc = clReleaseMemObject(mem_buf_out_mask);
  ASSERT_EQ(CL_SUCCESS, rc);
  rc = clReleaseMemObject(mem_buf_dst);
  ASSERT_EQ(CL_SUCCESS, rc);
  rc = clReleaseKernel(ocl_invert_kernel);
  ASSERT_EQ(CL_SUCCESS, rc);
  rc = clReleaseKernel(ocl_mask_kernel);
  ASSERT_EQ(CL_SUCCESS, rc);
  rc = clReleaseProgram(ocl_program);
  ASSERT_EQ(CL_SUCCESS, rc);
}
