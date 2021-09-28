// INTEL_COPYRIGHT_BEGIN
// Copyright (c) 2020, Intel Corporation. All rightrs reserved.
// INTEL_COPYRIGHT_END
#pragma once

#include "ze_context.hpp"
#include "ze_device.hpp"
#include "ze_event.hpp"
#include <unordered_set>

struct _ze_command_queue_handle_t {};

namespace __zert__ {

struct ZeCmdList;
struct ZeCmdQueue final : _ze_command_queue_handle_t {
  ZeCmdQueue(ZeContext *, ZeDevice *, ze_command_queue_desc_t);
  ~ZeCmdQueue();
  ze_result_t queue_execute(std::vector<ZeCmdList *>);
  ze_result_t sync(uint64_t);
  ZeContext *context() { return context_; }
  ZeDevice *device() { return device_; }
  ze_command_queue_desc_t const &descriptor() { return descriptor_; }

private:
  ZeContext *context_ = nullptr;
  ZeDevice *device_ = nullptr;
  ze_command_queue_desc_t descriptor_;
  ze_result_t execute_status_;
  std::vector<ZeCmdList *> cmdlists_;
  std::unordered_set<ZeEvent *> events_from_cmdlists_;
};

} // namespace __zert__
