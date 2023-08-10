//==--- base_fixture.h - tests for both FPGA HW and Emu     -*- C++ -*------==//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#ifndef __BASE_FIXTURE_H__
#define __BASE_FIXTURE_H__

#include "CL/cl_ext.h"
#include "gtest_wrapper.h"
#include "pretty_printers.h"

#include <string>
#include <type_traits>
#include <vector>

/**
 *  Provides basic API for tests development:
 *   * automatically selects platform and devices
 *   * contains helpers for creating contexts, programs, buffers, pipes, kernels
 *   * takes care about OpenCL objects: automaticallly releases all created
 *     stuff
 */
class OCLFPGABaseFixture : public ::testing::Test {
protected:
  /**
   *  Selects first available platform and all available devices of type
   *  CL_DEVICE_TYPE_ACCELERATOR on that platform
   */
  void SetUp() override {
    cl_int error = CL_SUCCESS;

    cl_uint num_platforms;
    error = clGetPlatformIDs(0, NULL, &num_platforms);
    ASSERT_EQ(CL_SUCCESS, error)
        << "clGetPlatformIDs failed with error " << ErrToStr(error);
    ASSERT_GE(num_platforms, 1u) << "no available platforms found";

    error = clGetPlatformIDs(1, &m_platform, NULL);
    ASSERT_EQ(CL_SUCCESS, error)
        << "clGetPlatformIDs failed with error " << ErrToStr(error);

    cl_uint num_devices;
    error = clGetDeviceIDs(m_platform, CL_DEVICE_TYPE_ACCELERATOR, 0, NULL,
                           &num_devices);
    ASSERT_EQ(CL_SUCCESS, error)
        << "clGetDeviceIDs failed with error " << ErrToStr(error);
    ASSERT_GE(num_devices, 1u) << "no available devices found";
    m_devices.resize(num_devices);

    error = clGetDeviceIDs(m_platform, CL_DEVICE_TYPE_ACCELERATOR, num_devices,
                           &m_devices.front(), NULL);
    ASSERT_EQ(CL_SUCCESS, error)
        << "clGetDeviceIDs failed with error " << ErrToStr(error);
  }

  /**
   *  \returns selected platform
   */
  cl_platform_id platform() const {
    assert(nullptr != m_platform && "No platform available");
    return m_platform;
  }

  /**
   *  \returns first device in the selected platform
   */
  cl_device_id device() const {
    assert(m_devices.size() > 0 && "No devices available");
    return m_devices[0];
  }

  /**
   *  \returns list of all devices of type CL_DEVICE_TYPE_ACCELERATOR available
   *  int the selected platform
   */
  const std::vector<cl_device_id> &devices() const { return m_devices; }

  /**
   *  \returns num of devices of type CL_DEVICE_TYPE_ACCELERATOR available in
   *  the selected platform
   */
  size_t num_devices() const { return m_devices.size(); }

  /**
   *  Automatically releases all created memory objects (buffers, pipes),
   *  kernels, programs, command queues
   */
  void TearDown() override {
    for (const auto mem_obj : m_mem_objects) {
      clReleaseMemObject(mem_obj);
    }

    for (const auto kernel : m_kernels) {
      clReleaseKernel(kernel);
    }

    for (const auto program : m_programs) {
      clReleaseProgram(program);
    }

    for (const auto queue : m_command_queues) {
      clReleaseCommandQueue(queue);
    }

    for (const auto context : m_contexts) {
      clReleaseContext(context);
    }
  }

  /**
   *  \brief Creates OpenCL context for the specified device
   *
   *  \note Created context will be released automatically, \see TearDown
   *
   *  \param [in] device
   *  \returns context if created successfully, nullptr otherwise
   */
  cl_context createContext(const cl_device_id device) {
    return createContext(1, &device);
  }

  /**
   *  \brief Creates OpenCL context with the specified properties for the
   *  specified list of devices
   *
   *  \note Created context will be released automatically, \see TearDown
   *
   *  \param [in] properties
   *  \param [in] device_list
   *  \returns context if created successfully, nullptr otherwise
   */
  cl_context createContext(const std::vector<cl_device_id> &device_list) {
    return createContext(device_list.size(), device_list.data());
  }

  /**
   *  \brief Creates OpenCL context with the specified properties for the
   *  specified list of devices
   *
   *  \note Created context will be released automatically, \see TearDown
   *
   *  \param [in] properties
   *  \param [in] num_devices
   *  \param [in] device_list
   *  \returns context if created successfully, nullptr otherwise
   */
  cl_context createContext(cl_uint num_devices,
                           const cl_device_id *devices_list) {
    cl_int error = CL_SUCCESS;
    cl_context context = clCreateContext(nullptr, num_devices, devices_list,
                                         nullptr, nullptr, &error);
    EXPECT_EQ(CL_SUCCESS, error)
        << "clCreateContext failed with error " << ErrToStr(error);
    if (CL_SUCCESS == error) {
      m_contexts.push_back(context);
      return context;
    }

    return nullptr;
  }

  /**
   *  \brief Creates OpenCL buffer in the specified context with the specified
   *  size and memory flags
   *
   *  \note Created buffer will be released automatically, \see TearDown
   *
   *  \param [in] context
   *  \param [in] size Size of the requested buffer in bytes
   *  \param [in] flags
   *  \param [in] hostptr
   *  \returns pipe if created successfully, nullptr otherwise
   */
  cl_mem createBuffer(cl_context context, size_t size,
                      cl_mem_flags flags = CL_MEM_READ_WRITE,
                      void *hostptr = nullptr) {
    cl_int error = CL_SUCCESS;
    cl_mem buffer = clCreateBuffer(context, flags, size, hostptr, &error);
    EXPECT_EQ(CL_SUCCESS, error)
        << "clCreateBuffer failed with error " << ErrToStr(error);
    if (CL_SUCCESS == error) {
      m_mem_objects.push_back(buffer);
      return buffer;
    }

    return nullptr;
  }

  /**
   *  \brief Creates OpenCL pipe in the specified context with the specified
   *  packet size, max packets and memory flags
   *
   *  \note Created pipe will be released automatically, \see TearDown
   *
   *  \param [in] context
   *  \param [in] pipe_packet_size Size of the packet in bytes
   *  \param [in] pipe_max_packets Maximum number of packets in pipe
   *  \param [in] flags
   *  \returns buffer if created successfully, nullptr otherwise
   */
  cl_mem createPipe(cl_context context, cl_uint pipe_packet_size,
                    cl_uint pipe_max_packets, cl_mem_flags flags = 0) {
    cl_int error = CL_SUCCESS;
    cl_mem pipe = clCreatePipe(context, flags, pipe_packet_size,
                               pipe_max_packets, nullptr, &error);
    EXPECT_EQ(CL_SUCCESS, error)
        << "clCreatePipe failed with error " << ErrToStr(error);
    if (CL_SUCCESS == error) {
      m_mem_objects.push_back(pipe);
      return pipe;
    }

    return nullptr;
  }

  /**
   *  \brief Creates and builds OpenCL program in the specified context (for all
   *  devices in the context) using specified program sources and build options
   *
   *  \note Created program will be released automatically, \see TearDown
   *
   *  This function emulates offline compiler. It is done by the following
   *  actions:
   *    1. OpenCL program is created from sources: let's call it
   *       \c program_with_source
   *    2. \c program_with_source is built for all devices in the context
   *    3. OpenCL program is re-created from binaries extracted from
   *       \c program_with_source. Let's call produced progam
   *       \c program_with_binary
   *    4. \c program_with_binary is build for all devices in the context
   *
   *  \param [in] context
   *  \param [in] sources
   *  \param [in] build_options
   *  \returns OpenCL program created from binary if successfull, nullptr
   *  otherwise
   */
  cl_program createAndBuildProgram(cl_context context,
                                   const std::string &sources,
                                   const std::string &build_options = "") {
    cl_int error = CL_SUCCESS;

    const char *sources_str = sources.c_str();
    cl_program program_with_source =
        clCreateProgramWithSource(context, 1, &sources_str, nullptr, &error);
    EXPECT_EQ(CL_SUCCESS, error)
        << "clCreateProgramWithSource failed with error " << ErrToStr(error);
    if (CL_SUCCESS != error) {
      return nullptr;
    }

    // We need this info later to extract build logs and program binaries
    cl_uint num_devices = 0;
    error = clGetProgramInfo(program_with_source, CL_PROGRAM_NUM_DEVICES,
                             sizeof(cl_uint), &num_devices, nullptr);
    EXPECT_EQ(CL_SUCCESS, error)
        << "clGetProgramInfo failed with error " << ErrToStr(error);
    std::vector<cl_device_id> devices_list(num_devices, nullptr);
    error = clGetProgramInfo(program_with_source, CL_PROGRAM_DEVICES,
                             sizeof(cl_device_id) * num_devices,
                             &devices_list.front(), nullptr);
    EXPECT_EQ(CL_SUCCESS, error)
        << "clGetProgramInfo failed with error " << ErrToStr(error);

    error = clBuildProgram(program_with_source, 0, nullptr,
                           build_options.c_str(), nullptr, nullptr);
    EXPECT_EQ(CL_SUCCESS, error)
        << "clBuildProgram failed with error " << ErrToStr(error);
    if (CL_SUCCESS != error) {
      // Extract build logs
      extractAndPrintBuildLog(program_with_source, devices_list.front());
      return nullptr;
    }

    // Mimic for offline compiler
    cl_program program_with_binary =
        emulateOfflineCompiler(context, program_with_source, devices_list);
    clReleaseProgram(program_with_source);
    if (nullptr != program_with_binary) {
      m_programs.push_back(program_with_binary);
    }
    return program_with_binary;
  }

  /**
   *  \brief Creates OpenCL kernel in the specified program
   *
   *  \note Created kernel will be released automatically, \see TearDown
   *
   *  \param [in] program
   *  \param [in] kernel_name
   *  \returns kernel if created successfully, nullptr otherwise
   */
  cl_kernel createKernel(cl_program program, const std::string &kernel_name) {
    cl_int error = CL_SUCCESS;
    cl_kernel kernel = clCreateKernel(program, kernel_name.c_str(), &error);
    EXPECT_EQ(CL_SUCCESS, error)
        << "clCreateKernel failed with error " << ErrToStr(error);
    if (CL_SUCCESS == error) {
      m_kernels.push_back(kernel);
      return kernel;
    }

    return nullptr;
  }

  /**
   *  \brief Creates OpenCL command queue in the specified context for the
   *  specified device
   *
   *  \note Created command queue will be released automatically, \see TearDown
   *
   *  \param [in] context
   *  \param [in] device
   *  \returns command queue if created successfully, nullptr otherwise
   */
  cl_command_queue createCommandQueue(cl_context context, cl_device_id device,
                                      bool use_out_of_order = false) {
    cl_int error = CL_SUCCESS;
    cl_queue_properties queue_type[] = {
        CL_QUEUE_PROPERTIES, CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, 0};
    cl_command_queue queue = (use_out_of_order)
                                 ? clCreateCommandQueueWithProperties(
                                       context, device, queue_type, &error)
                                 : clCreateCommandQueueWithProperties(
                                       context, device, nullptr, &error);

    EXPECT_EQ(CL_SUCCESS, error)
        << "clCreateCommandQueueWithProperties failed with error "
        << ErrToStr(error);
    if (CL_SUCCESS == error) {
      m_command_queues.push_back(queue);
      return queue;
    }

    return nullptr;
  }

private:
  cl_platform_id m_platform = nullptr;
  std::vector<cl_device_id> m_devices = std::vector<cl_device_id>();

  std::vector<cl_context> m_contexts;

  std::vector<cl_mem> m_mem_objects;

  std::vector<cl_program> m_programs;

  std::vector<cl_kernel> m_kernels;

  std::vector<cl_command_queue> m_command_queues;

  void extractAndPrintBuildLog(cl_program program, cl_device_id device) {
    cl_int error = CL_SUCCESS;
    size_t log_size = 0;
    error = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0,
                                  nullptr, &log_size);
    ASSERT_EQ(CL_SUCCESS, error)
        << " clGetProgramBuildInfo failed with error " << ErrToStr(error);

    std::string log("", log_size);
    error = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG,
                                  log_size, &log[0], nullptr);
    ASSERT_EQ(CL_SUCCESS, error)
        << " clGetProgramBuildInfo failed with error " << ErrToStr(error);
    FAIL() << log << "\n";
  }

  cl_program
  emulateOfflineCompiler(cl_context context, cl_program program_with_source,
                         const std::vector<cl_device_id> devices_list) {
    cl_int error = CL_SUCCESS;
    size_t num_devices = devices_list.size();
    std::vector<size_t> binary_sizes(num_devices, 0);
    error = clGetProgramInfo(program_with_source, CL_PROGRAM_BINARY_SIZES,
                             sizeof(size_t) * num_devices,
                             &binary_sizes.front(), nullptr);
    EXPECT_EQ(CL_SUCCESS, error)
        << "clGetProgramInfo failed with error " << ErrToStr(error);
    if (CL_SUCCESS != error) {
      return nullptr;
    }

    size_t total_binary_size = 0;
    for (size_t i = 0; i < num_devices; ++i) {
      total_binary_size += binary_sizes[i];
    }
    std::vector<unsigned char> binaries(total_binary_size, '\0');

    std::vector<unsigned char *> binary_ptrs(num_devices);
    size_t offset = 0;
    for (size_t i = 0; i < num_devices; ++i) {
      binary_ptrs[i] = binaries.data() + offset;
      offset += binary_sizes[i];
    }
    error = clGetProgramInfo(program_with_source, CL_PROGRAM_BINARIES,
                             sizeof(unsigned char *) * num_devices,
                             &binary_ptrs.front(), nullptr);
    EXPECT_EQ(CL_SUCCESS, error)
        << "clGetProgramInfo failed with error " << ErrToStr(error);
    if (CL_SUCCESS != error) {
      return nullptr;
    }

    cl_program program_with_binary = clCreateProgramWithBinary(
        context, num_devices, devices_list.data(), binary_sizes.data(),
        const_cast<const unsigned char **>(binary_ptrs.data()), nullptr,
        &error);
    EXPECT_EQ(CL_SUCCESS, error)
        << "clCreateProgramWithBinary failed with error " << ErrToStr(error);
    if (CL_SUCCESS != error) {
      return nullptr;
    }

    error = clBuildProgram(program_with_binary, 0, nullptr, nullptr, nullptr,
                           nullptr);
    EXPECT_EQ(CL_SUCCESS, error)
        << "clBuildProgram failed with error " << ErrToStr(error);
    if (CL_SUCCESS == error) {
      return program_with_binary;
    }

    extractAndPrintBuildLog(program_with_binary, devices_list.front());
    return nullptr;
  }
};

#endif // __BASE_FIXTURE_H__
