// INTEL_COPYRIGHT_BEGIN
// Copyright (c) 2020, Intel Corporation. All rightrs reserved.
// INTEL_COPYRIGHT_END

#include "ze_cmdqueue.hpp"
#include "ze_cmdlist.hpp"
#include "ze_utils.hpp"
#include <chrono>

namespace __zert__ {

//-----------------------------------------------------------------------------

ZeCmdQueue::ZeCmdQueue(ZeContext *context, ZeDevice *device,
                       ze_command_queue_desc_t desc)
    : context_(context), device_(device), descriptor_(desc) {}

//-----------------------------------------------------------------------------

ZeCmdQueue::~ZeCmdQueue() = default;

//-----------------------------------------------------------------------------

ze_result_t ZeCmdQueue::queue_execute(std::vector<ZeCmdList *> cmdlists) {
  this->cmdlists_ = cmdlists;

  // for kCPU device
  {
    for (auto &l : cmdlists) {
      auto events = l->submit();
      events_from_cmdlists_.insert(events.begin(), events.end());
    }
  }
  this->execute_status_ = ZE_RESULT_SUCCESS;
  return this->execute_status_;
}

//-----------------------------------------------------------------------------

ze_result_t ZeCmdQueue::sync(uint64_t timeout_in_msec) {
  auto start = std::chrono::steady_clock::now();
  auto end = std::chrono::steady_clock::now();
  std::chrono::duration<double> elapsed_seconds = end - start;
  double timeout_in_sec = double(timeout_in_msec) / 1e3;
  do {
    bool ready = true;
    for (auto e : events_from_cmdlists_) {
      ready = ready && e->is_ready();
    }
    for (auto lst : cmdlists_) {
      lst->cv_.notify_one();
    }
    ze_result_t ret = ZE_RESULT_SUCCESS;
    if (ready) {
      for (auto e : events_from_cmdlists_) {
        if (e->result() != ZE_RESULT_SUCCESS) {
          ZESIMERR << "Command failed in a cmdlist submitted to a "
                      "queue, error"
                   << e->result();
          ret = ZE_RESULT_ERROR_UNKNOWN;
        }
      }
      // if all commond lists are done execution
      // clear events lsit, and return execution status
      events_from_cmdlists_.clear();
      return ret;
    }
    end = std::chrono::steady_clock::now();
    elapsed_seconds = end - start;
  } while (elapsed_seconds.count() < timeout_in_sec);
  return ZE_RESULT_NOT_READY;
}

//-----------------------------------------------------------------------------
} // namespace __zert__
