// INTEL_COPYRIGHT_BEGIN
// Copyright (c) 2020, Intel Corporation. All rightrs reserved.
// INTEL_COPYRIGHT_END
#pragma once

#include "ze_device.hpp"
#include "ze_driver.hpp"
#include "ze_kernel.hpp"
#include <functional>
#include <future>
#include <list>

struct _ze_event_handle_t {};
struct _ze_event_pool_handle_t {};

namespace __zert__ {

struct ZeEvent;
struct ZeEventPool final : _ze_event_pool_handle_t {
public:
  static std::unique_ptr<ZeEventPool>
  create(ze_event_pool_desc_t const &desc, ZeDriver *driver,
         std::vector<ZeDevice *> const &devices, ze_result_t &);
  static ze_result_t destroy(ZeEventPool *);
  ze_result_t createEvent(ze_event_desc_t const &desc,
                          ze_event_handle_t *event);
  ze_result_t destroyEvent(ZeEvent *event);
  ~ZeEventPool();
  ZeDriver *driver() { return driver_; }
  uint32_t uniqueEventIndex();
  bool owns(ZeEvent *);

private:
  ZeEventPool();
  ze_event_pool_desc_t desc_;
  ZeDriver *driver_;
  std::vector<ZeDevice *> devices_;
  std::list<std::unique_ptr<ZeEvent>> events_;
};

struct ZeEvent final : _ze_event_handle_t {
public:
  ZeEvent(ZeEventPool *, ze_event_desc_t const &);
  ~ZeEvent();

  ZeEventPool *event_pool() const { return event_pool_; }
  ze_event_desc_t desc() const { return desc_; }
  bool is_ready() const { return ready_; }
  static ze_result_t signal(ZeEvent *, ze_result_t);
  static ze_result_t reset(ZeEvent *);
  ze_result_t result() const { return result_; }
  ze_result_t hostSync(uint64_t);
  ze_result_t queryStatus();
  void setKernel(ZeKernel *kernel) { kernel_ = kernel; } // for grits-path only
  ZeKernel *getKernel() { return kernel_; }              // for grits-path only
  ze_result_t queryKernelTimestamp(ze_kernel_timestamp_result_t *dstprt);

private:
  ZeEventPool *event_pool_ = nullptr;
  ze_event_desc_t desc_;
  std::atomic<bool> ready_ = false;
  std::atomic<ze_result_t> result_ = ZE_RESULT_ERROR_UNKNOWN;
  ZeKernel *kernel_ = nullptr; // used by grits-path only to track which
                               // kernel this event bind to
  // Hard-code the timestamp values as sim does not care about them
  uint64_t globalStartTS = 0;
  uint64_t globalEndTS = 100;
  uint64_t contextStartTS = 0;
  uint64_t contextEndTS = 100;
};

} // namespace __zert__
