//==--- simple_fixture.h - tests for both FPGA HW and Emu   -*- C++ -*------==//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#ifndef __SIMPLE_FIXTURE_H__
#define __SIMPLE_FIXTURE_H__

#include "CL/cl_ext.h"
#include "base_fixture.h"
#include "gtest_wrapper.h"

#include <map>
#include <string>
#include <type_traits>

/**
 *  Provides minimalistic API for tests development:
 *   * implicitly creates context on a first device in the platform
 *   * create context is used in all OpenCL API calls automatically
 */
class OCLFPGASimpleFixture : public OCLFPGABaseFixture {
protected:
  typedef OCLFPGABaseFixture parent_t;

  /**
   *  Creates a context on a first device in the platform
   */
  void SetUp() override {
    parent_t::SetUp();
    m_context = parent_t::createContext(parent_t::device());
    ASSERT_NE(nullptr, m_context) << "createContext failed";
  }

  void TearDown() override { parent_t::TearDown(); }

  /**
   *  \brief Creates and builds an OpenCL program
   *
   *  \note Created program will be used automatically in further calls like:
   *  enqueueNDRange, enqueueTask
   */
  bool createAndBuildProgram(const std::string &program_sources,
                             const std::string &build_options = "") {
    m_program = parent_t::createAndBuildProgram(m_context, program_sources,
                                                build_options);
    return nullptr != m_program;
  }

  /**
   *  \brief Enqueues NDRange OpenCL kernel
   *
   *  The following actions are performed in order to enqueue kernel:
   *  1. create kernel with name \c kernel_name in the program (created
   *     previously by \c createAndBuildProgram)
   *  2. if it is a first enqueue of the kernel create command queue and
   *     associate it with \c kernel_name identifier. Otherwise,
   *     existing command queue will be re-used.
   *  3. set kernel args
   *
   *  \param [in] kernel_name
   *  \param [in] num_dims
   *  \param [in] global_size
   *  \param [in] local_size
   *  \param [in] args Set of kernel arguments which will be automatically set
   *  for the kernel. To set memory object as kernel argument you need to pass
   *  cl_mem here. To set scalar value as kernel argument you need to pass it
   *  here by value. Arguments are set in order they are passed to this method
   *
   *  \returns true if kernel and queue create successfully, all kernel
   *  arguments were set successfully and kernel was enqueued successfully;
   *  false otherwise
   */
  template <typename... Args>
  bool enqueueNDRange(const std::string &kernel_name, size_t num_dims,
                      const size_t *global_size, const size_t *local_size,
                      cl_event *event, Args... args) {
    assert(nullptr != m_program && "no valid program object created!");
    cl_kernel kernel = parent_t::createKernel(m_program, kernel_name);
    EXPECT_TRUE(kernel != nullptr) << "createKernel failed";
    if (nullptr == kernel) {
      return false;
    }

    cl_command_queue queue = getOrCreateQueue(kernel_name);
    if (nullptr == queue) {
      return false;
    }

    return enqueueNDRange(0, num_dims, global_size, local_size, queue, kernel,
                          event, args...);
  }

  /**
   *  \brief Enqueues single work-item (task) OpenCL kernel
   *
   *  This function is a helper, \see enqueueNDRange for detailed description
   */
  template <typename... Args>
  bool enqueueTask(const std::string &kernel_name, Args... args) {
    return enqueueTaskImpl(/*OOO*/ false, kernel_name, /*event*/ nullptr,
                           args...);
  }

  /**
   *  \brief Enqueues single work-item (task) OpenCL kernel in out-of-order
   *  queue
   *
   *  This function is a helper, \see enqueueNDRange for detailed description
   */
  template <typename... Args>
  bool enqueueOOOTask(const std::string &kernel_name, cl_event *event,
                      Args... args) {
    return enqueueTaskImpl(/*OOO*/ true, kernel_name, event, args...);
  }

  /**
   *  \brief Finishes command queue associated with a \c kernel_name identifier
   *
   *  Use \c kernel_name passed previously to \c enqueueNDRange or
   *  \c enqueueTask to finish execution of that kernel.
   *
   *  \param [in] kernel_name Identifier of command queue. Correspond to kernel
   *  name passed to \c enqueueNDRange or \c enqueueTask
   *
   *  \returns true if command queue accociated with \c kernel_name identifier
   *  exists and false otherwise.
   */
  bool finish(const std::string &kernel_name) {
    if (m_queues.find(kernel_name) == m_queues.end()) {
      return false;
    }

    cl_command_queue queue = m_queues[kernel_name];
    clFinish(queue);
    return true;
  }

  /**
   *  \brief Creates OpenCL buffer
   *
   *  Wrapper around \c OCLFPGABaseFixture::createBuffer
   *
   *  \tparam T element type of the buffer to be created
   *  \param [in] num_elements Number of elements in buffer
   *  \param [in] flags
   *  \param [in,out] hostptr
   *  \returns buffer if created successfully, nullptr otherwise
   */
  template <typename T>
  cl_mem createBuffer(size_t num_elements,
                      cl_mem_flags flags = CL_MEM_READ_WRITE,
                      T *hostptr = nullptr) {
    return parent_t::createBuffer(m_context, num_elements * sizeof(T), flags,
                                  (void *)hostptr);
  }

  /**
   *  \brief Enqueues \b blocking command to read buffer contents
   *
   *  \tparam T element type of the buffer
   *  \param [in] kernel_name identifier of command queue which will be used to
   *  enqueue command. Must be an identifier that used previously in
   *  \c enqueueNDRange or \c enqueueTask methods
   *  \param [in] buffer
   *  \param [in] num_elements Number of elements to read
   *  \param [out] ptr Pointer to memory where data is to be read into
   *  \returns true if command queue with identifier \c kernel_name exists and
   *  read command enqueued successfully, false otherwise
   */
  template <typename T>
  bool readBuffer(const std::string &kernel_name, cl_mem buffer,
                  size_t num_elements, T *ptr) {
    cl_command_queue queue = getOrCreateQueue(kernel_name);
    EXPECT_TRUE(nullptr != queue) << "getOrCreateCommandQueue failed";
    if (nullptr == queue) {
      return false;
    }

    cl_int error =
        clEnqueueReadBuffer(queue, buffer, CL_TRUE, 0, num_elements * sizeof(T),
                            ptr, 0, nullptr, nullptr);
    EXPECT_EQ(CL_SUCCESS, error)
        << "clEnqueueReadBuffer failed with error " << ErrToStr(error);
    return CL_SUCCESS == error;
  }

  /**
   *  \brief Creates OpenCL pipe
   *
   *  Wrapper around \c OCLFPGABaseFixture::createPipe
   *
   *  \tparam T element type of pipe packets
   *  \param [in] pipe_max_packets
   *  \param [in] flags
   */
  template <typename T>
  cl_mem createPipe(cl_uint pipe_max_packets, cl_mem_flags flags = 0) {
    return parent_t::createPipe(m_context, sizeof(T), pipe_max_packets, flags);
  }

  cl_context getContext() const { return m_context; }
  cl_command_queue getOOOQueue() const { return m_out_of_order_queue; }
  cl_device_id getDevice() const { return parent_t::device(); }

private:
  cl_context m_context = nullptr;
  cl_program m_program = nullptr;
  cl_command_queue m_out_of_order_queue = nullptr;

  std::map<std::string, cl_command_queue> m_queues;

  /**
   *  \brief Enqueues single work-item (task) OpenCL kernel
   *
   *  This function is a helper, \see enqueueNDRange for detailed description
   */
  template <typename... Args>
  bool enqueueTaskImpl(bool OOO, const std::string &kernel_name,
                       cl_event *event, Args... args) {
    assert(nullptr != m_program && "no valid program object created!");
    cl_kernel kernel = parent_t::createKernel(m_program, kernel_name);
    EXPECT_TRUE(kernel != nullptr) << "createKernel failed";
    if (nullptr == kernel) {
      return false;
    }

    cl_command_queue queue =
        OOO ? getOrCreateOOOQueue() : getOrCreateQueue(kernel_name);
    if (nullptr == queue) {
      return false;
    }

    const size_t one = 1;

    return enqueueNDRange(0, 1, &one, &one, queue, kernel, event, args...);
  }

  template <typename T, typename... Args>
  bool enqueueNDRange(size_t index, size_t num_dims, const size_t *global_size,
                      const size_t *local_size, cl_command_queue queue,
                      cl_kernel kernel, cl_event *event, T arg, Args... args) {
    cl_int error = clSetKernelArg(kernel, index, sizeof(T), &arg);
    EXPECT_EQ(CL_SUCCESS, error)
        << "clSetKernelArg failed with error " << ErrToStr(error)
        << " while setting argument #" << index;
    // TODO: print name of type T here
    if (CL_SUCCESS != error) {
      return false;
    }

    return enqueueNDRange(index + 1, num_dims, global_size, local_size, queue,
                          kernel, event, args...);
  }

  bool enqueueNDRange(size_t, size_t num_dims, const size_t *global_size,
                      const size_t *local_size, cl_command_queue queue,
                      cl_kernel kernel, cl_event *event) {
    cl_int error =
        clEnqueueNDRangeKernel(queue, kernel, num_dims, nullptr, global_size,
                               local_size, 0, nullptr, event);
    EXPECT_EQ(CL_SUCCESS, error)
        << "clEnqueueNDRangeKernel failed with error " << ErrToStr(error);
    return CL_SUCCESS == error;
  }

  cl_command_queue getOrCreateQueue(const std::string &kernel_name) {
    if (m_queues.find(kernel_name) == m_queues.end()) {
      cl_command_queue queue =
          parent_t::createCommandQueue(m_context, parent_t::device());
      EXPECT_TRUE(nullptr != queue) << "createCommandQueue failed";
      if (nullptr == queue) {
        return nullptr;
      }

      m_queues[kernel_name] = queue;
    }

    return m_queues[kernel_name];
  }

  cl_command_queue getOrCreateOOOQueue() {
    if (!m_out_of_order_queue)
      m_out_of_order_queue =
          parent_t::createCommandQueue(m_context, parent_t::device(), true);

    return m_out_of_order_queue;
  }
};

#endif // __SIMPLE_FIXTURE_H__
