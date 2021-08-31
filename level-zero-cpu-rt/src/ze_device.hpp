// INTEL_COPYRIGHT_BEGIN
// Copyright (c) 2020, Intel Corporation. All rightrs reserved.
// INTEL_COPYRIGHT_END
#pragma once

#include "ze_buffer.hpp"
#include "ze_driver.hpp"
//#include <filesystem>
#include <cassert>
#include <condition_variable>
#include <list>
#include <memory>
#include <mutex>
#include <random>

struct _ze_device_handle_t {};

namespace __zert__ {

struct ZeModule;
struct ZeKernel;
struct ZeCmdQueue;
struct ZeCmdList;

enum cpu_device_kind_t {
  CPU_DEVICE_ARCH_SSE42,
  CPU_DEVICE_ARCH_AVX,
  CPU_DEVICE_ARCH_AVX2,
  CPU_DEVICE_ARCH_AVX512
};

struct ZeDevice : _ze_device_handle_t {
  // device kind, Currently we only support CPU
  enum Kind {
    kCPU, //
  };

  ZeDevice(ZeDriver *, Kind kind, int32_t tile_count);
  virtual ~ZeDevice();

  Kind kind() const { return kind_; }

  ze_device_properties_t const &properties() const { return properties_; }

  ZeDriver *driver() { return driver_; }

  ze_result_t getPlatform(cpu_device_kind_t &device);

  virtual ze_result_t initialize();

  ze_result_t createModule(ZeContext *context, ze_module_desc_t const *desc,
                           ze_module_handle_t *module);
  ze_result_t destroyModule(ze_module_handle_t);

  ze_result_t createCmdQueue(ZeContext *context,
                             ze_command_queue_desc_t const *desc,
                             ze_command_queue_handle_t *phCommandQueue);
  ze_result_t destroyCmdQueue(ze_command_queue_handle_t);

  ze_result_t createCmdList(ZeContext *context,
                            ze_command_list_desc_t const *desc,
                            ze_command_list_handle_t *phCommandList);
  ze_result_t createCmdList(ZeContext *context,
                            ze_command_queue_desc_t const *desc,
                            ze_command_list_handle_t *phCommandList);
  ze_result_t destroyCmdList(ze_command_list_handle_t);
  virtual ze_result_t launch(ZeKernel *, ze_group_count_t config) = 0;
  ZeDevice *subdevice(int i) const { return subdevices_[i]; }
  size_t subdeviceCount() const { return subdevices_.size(); }

  ze_result_t
  getComputeProperties(ze_device_compute_properties_t *pComputeProperties);

protected:
  virtual std::unique_ptr<ZeDevice> newSubdevice() = 0;

protected:
  ZeDriver *driver_;
  Kind kind_;
  int32_t tile_count_;
  std::list<std::unique_ptr<ZeModule>> modules_;
  std::list<std::unique_ptr<ZeCmdQueue>> cmd_queues_;
  std::list<std::unique_ptr<ZeCmdList>> cmd_lists_;

  std::list<std::unique_ptr<ZeDevice>> subdevices_excluding_this_device_;
  std::vector<ZeDevice *> subdevices_;

protected:
  ze_device_properties_t properties_;
};

struct Semaphore {
public:
  Semaphore(int count_ = 0) : count(count_) {}

  inline void notify() {
    std::unique_lock<std::mutex> lock(mtx);
    count++;
    // notify the waiting thread
    cv.notify_one();
  };

  inline void wait() {
    std::unique_lock<std::mutex> lock(mtx);
    while (count == 0) {
      // wait on the mutex until notify is called
      cv.wait(lock);
    }
    count--;
  }

private:
  std::mutex mtx;
  std::condition_variable cv;
  int count;
};

struct CPUdevice final : ZeDevice {
private:
  using Base = ZeDevice;

public:
  CPUdevice(ZeDriver *, int32_t tile_count);
  virtual ~CPUdevice();
  static Kind this_kind() { return kCPU; }
  static bool classof(ZeDevice const *d) { return d->kind() == this_kind(); }
  ze_result_t launch(ZeKernel *, ze_group_count_t config) override;
  ze_result_t initialize() override;

protected:
  std::unique_ptr<ZeDevice> newSubdevice() override;
  int maxParallelKernelsLaunchPerDevice_ = 4;
  std::unique_ptr<Semaphore> semaphore_;
};

} // namespace __zert__
