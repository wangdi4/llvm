// INTEL_COPYRIGHT_BEGIN
// Copyright (c) 2020, Intel Corporation. All rightrs reserved.
// INTEL_COPYRIGHT_END
#pragma once

#include "ze_context.hpp"
#include "ze_device.hpp"
#include "ze_event.hpp"
#include "ze_kernel.hpp"
#include <atomic>
#include <functional>
#include <future>
#include <list>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <unordered_set>

struct _ze_command_list_handle_t {};

namespace __zert__ {

struct ZeCmdList final : _ze_command_list_handle_t {
  using cmd_t = std::function<ze_result_t()>;
  ZeCmdList(ZeContext *, ZeDevice *, ze_command_list_desc_t);
  ZeCmdList(ZeContext *, ZeDevice *, ze_command_queue_desc_t);
  ~ZeCmdList();
  ZeContext *context() { return context_; }
  ZeDevice *device() { return device_; }
  ze_result_t append(ZeEvent *, std::vector<ZeEvent *>, cmd_t);
  ze_result_t close();
  ze_result_t reset();
  ze_result_t barrier(ZeEvent *new_event,
                      std::vector<ZeEvent *> wait_for_events);
  ze_result_t append_signal_event(ZeEvent *);
  ze_result_t append_reset_event(ZeEvent *);
  bool is_closed() const { return closed_; }
  std::unordered_set<ZeEvent *> submit();
  ze_command_list_desc_t const &list_descriptor() { return list_descriptor_; }
  ze_command_queue_desc_t const &queue_descriptor() {
    return queue_descriptor_;
  }
  ZeEvent *assignEvent(ZeEvent *);
  ze_result_t asyncExec(ZeEvent *, std::vector<ZeEvent *>, cmd_t);
  static std::condition_variable cv_;
  uint32_t get_pending_events(); // return number of remaining events;
  std::vector<ZeKernel *> &kernels() { return kernels_; }
  ze_result_t append(ZeEvent *, std::vector<ZeEvent *>, ZeKernel *, cmd_t);
  bool isSyncRequired(ZeKernel *kernel);

private:
  struct thread_safe_event_set : protected std::unordered_set<ZeEvent *> {
  private:
    using base_t = std::unordered_set<ZeEvent *>;
    std::mutex mutex_;

  public:
    thread_safe_event_set() : base_t() {}
    void insert(ZeEvent *event) {
      std::unique_lock<std::mutex> lk(mutex_);
      base_t::insert(event);
    }
    void erase(ZeEvent *event) {
      std::unique_lock<std::mutex> lk(mutex_);
      base_t::erase(event);
    }
    uint32_t size() {
      std::unique_lock<std::mutex> lk(mutex_);
      return uint32_t(base_t::size());
    }
    void clear() {
      std::unique_lock<std::mutex> lk(mutex_);
      base_t::clear();
    }
    bool empty() {
      std::unique_lock<std::mutex> lk(mutex_);
      return base_t::empty();
    }
    base_t copy() {
      std::unique_lock<std::mutex> lk(mutex_);
      return *this;
    }
  };
  ZeContext *context_ = nullptr;
  ZeDevice *device_ = nullptr;
  ze_command_list_desc_t list_descriptor_;
  ze_command_queue_desc_t queue_descriptor_;
  bool immediate_ = false;
  bool closed_ = false;
  struct deferred_cmd_t {
    ZeEvent *event_;
    std::vector<ZeEvent *> wait_events_;
    cmd_t cmd_;
  };
  std::mutex mutex_;
  std::unique_ptr<ZeEventPool> event_pool_;
  static uint32_t constexpr MAX_EVENTS = 8192;
  void createEventPool();
  thread_safe_event_set scheduled_events_;
  std::list<deferred_cmd_t> deferred_commands_;
  std::atomic<ZeEvent *> last_barrier_event_ = nullptr;
  ZeEvent *last_signal_reset_event_ = nullptr;
  std::vector<ZeKernel *> kernels_;
  std::unordered_map<ZeKernel *, bool> kernel_sync_map;
};

} // namespace __zert__
