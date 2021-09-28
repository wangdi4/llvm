// INTEL_COPYRIGHT_BEGIN
// Copyright (c) 2020, Intel Corporation. All rightrs reserved.
// INTEL_COPYRIGHT_END

#include "ze_cmdlist.hpp"
#include "ze_cmdqueue.hpp"
#include "ze_utils.hpp"
#include <algorithm>
#include <cassert>
#include <thread>

namespace __zert__ {

//-----------------------------------------------------------------------------
std::condition_variable ZeCmdList::cv_;

ZeCmdList::ZeCmdList(ZeContext *context, ZeDevice *device,
                     ze_command_list_desc_t list_desc)
    : context_(context), device_(device), list_descriptor_(list_desc),
      immediate_(false) {
  this->createEventPool();
}
ZeCmdList::ZeCmdList(ZeContext *context, ZeDevice *device,
                     ze_command_queue_desc_t queue_desc)
    : context_(context), device_(device), queue_descriptor_(queue_desc),
      immediate_(true) {
  this->createEventPool();
}

void ZeCmdList::createEventPool() {
  ze_event_pool_desc_t desc;
  desc.count = MAX_EVENTS;
  ze_result_t result;
  event_pool_ = ZeEventPool::create(desc, this->device()->driver(),
                                    {this->device()}, result);
  if (!event_pool_) {
    ZESIMERR << "Failed to create event pool, error=" << result;
    std::abort();
  }
}

ZeCmdList::~ZeCmdList() {
  uint32_t pending_events = this->get_pending_events();
  if (pending_events != 0) {
    ZESIMERR << "~ZeCmdList: Not all submitted commands are completed, count= "
             << pending_events;
    std::abort();
  }
}

//-----------------------------------------------------------------------------

ze_result_t ZeCmdList::close() {
  this->closed_ = true;
  this->scheduled_events_.clear();
  return ZE_RESULT_SUCCESS;
}

//-----------------------------------------------------------------------------

ze_result_t ZeCmdList::reset() {

  if (this->immediate_ && (this->get_pending_events() > 0)) {
    ZESIMERR << "Trying to reset immediate list with pending work. May "
                "result in undefined behavior.";
    return ZE_RESULT_ERROR_UNKNOWN;
  }
  this->scheduled_events_.clear();
  this->deferred_commands_.clear();
  this->closed_ = false;
  this->kernels_.clear();
  kernel_sync_map.clear();
  return ZE_RESULT_SUCCESS;
}

//-----------------------------------------------------------------------------

ZeEvent *ZeCmdList::assignEvent(ZeEvent *event) {
  if (nullptr == event) {
    ze_event_desc_t desc;
    desc.index = event_pool_->uniqueEventIndex();
    ze_event_handle_t handle;
    auto ret = event_pool_->createEvent(desc, &handle);
    if (ZE_RESULT_SUCCESS != ret) {
      ZESIMERR << "Failed to created event in asyncExec, error=" << ret;
      std::abort();
    }
    event = static_cast<ZeEvent *>(handle);
  }
  return event;
}

//-----------------------------------------------------------------------------

ze_result_t ZeCmdList::asyncExec(ZeEvent *event,
                                 std::vector<ZeEvent *> wait_events,
                                 cmd_t cmd) {
  assert(event && "event must not null");
  ZeEvent::reset(event);
#ifndef NDEBUG
  for (auto e : wait_events) {
    assert(e && "Event cannot be null in wait_events list");
  }
#endif
  auto launch_cmd = [event, this, cmd, wait_events]() mutable -> ze_result_t {
    std::mutex mutex;
    std::unique_lock<std::mutex> lk(mutex);
    cv_.wait(lk, [wait_events] {
      bool ready = true;
      for (auto e : wait_events) {
        ready = ready && e->is_ready();
      }
      return ready;
    });
    auto dependent_ret = ZE_RESULT_SUCCESS;
    for (auto e : wait_events) {
      if (e->result() != ZE_RESULT_SUCCESS) {
        dependent_ret = ZE_RESULT_ERROR_UNKNOWN;
      }
    }
    if (dependent_ret != ZE_RESULT_SUCCESS) {
      ZESIMERR << "Dependent commands failed, skipping execution";
    }
    auto cmd_ret = dependent_ret == ZE_RESULT_SUCCESS ? cmd() : dependent_ret;
    scheduled_events_.erase(event);

    {
      ZeEvent *nullevent = nullptr;
      ZeEvent *possible_barrier_event = event;
      std::atomic_compare_exchange_strong<ZeEvent *>(
          &this->last_barrier_event_, &possible_barrier_event, nullevent);
      // nullptr "this->last_barrier_event_" once this barrier is cleared
      // inclusion of this signaled event in the subsequent commands may
      // cause hang if this event is included into wait_events list
      // Hang example:
      //   cmd1
      //   e1 <- barrier(..)
      //   hostsync(e1)  // wait for event complition on the host
      //   hostreset(e1)  // reset event
      //   cmd2
      // cmd2 shall not include e1 into its wait_event list, since e1
      // is completed (see "runtime/events.cpp::device_barrier" for use
      // case that would hang otherwise)
      //
      // The following case manifests race, and has UB
      //   cmd1
      //   e1 <- barrier(..)
      //   e2 <- execKernel(..., e1); // async launc kernel, after e1
      //                              // signals
      //   hostsync(e1)  // wait on the host to sync e1
      //   hostreset(e1) // race wrt to execKernel, we don't know who will
      //                 // be first, execKernel or hostreset upon
      //                 // e1 signaling.
      //
    }

    if (cmd_ret != ZE_RESULT_SUCCESS) {
      ZESIMERR << "Command in a list failed";
    }
    cv_.notify_one(); // notify cv_ that we are about to be done..
    ZeEvent::signal(event, cmd_ret);

    // can't call cv_.notify_one() after signaling, why? because, the master
    // host thread could be waiting for this last signal after which it may
    // destroy the commandlist to which this cv_ belongs. Such race would
    // leave to UB when executing cv_.notify_one() after event is signalled.
    return cmd_ret;
  };

  if (this->immediate_ &&
      this->queue_descriptor_.mode == ZE_COMMAND_QUEUE_MODE_SYNCHRONOUS) {
    return launch_cmd();
  }
#if 0
    auto t = std::thread(launch_cmd);
    std::string seq_flag = util::get_envvar("L0SIM_SEQUENCE_COMMANDS");
    if (seq_flag.empty() || seq_flag == "0")
    {
        t.detach();
        return ZE_RESULT_SUCCESS;
    }
    else
    {
        t.join();
        return event->result();
    }
#endif // cdai2
  ZESIMERR << "asyncExec: unreachable";
  return ZE_RESULT_ERROR_UNKNOWN;
}

//-----------------------------------------------------------------------------

ze_result_t ZeCmdList::append(ZeEvent *event,
                              std::vector<ZeEvent *> wait_events,
                              std::function<ze_result_t()> cmd) {
  // barrier orders commands. All work that barrier
  // waits on need to be completed before scheduling any further work beyond
  // barrier. For example:
  //    e1 <- cmd1
  //    e2 <- cmd2
  //    e3 <- cmd3
  //    barrier(e1,e2)
  //    e4 <- cmd4
  // Here,cmd4 will not start until cmd1 & cmd2 is completed, but it will wait
  // for cmd3 completiotion. It is possible that cmd3 & cmd4 will run
  // concurrently.
  auto barrier_event = last_barrier_event_.load();
  if (event != barrier_event && barrier_event != nullptr) {
    wait_events.push_back(barrier_event);
  }
  event = this->assignEvent(event);
  ze_result_t ret = ZE_RESULT_SUCCESS;
  scheduled_events_.insert(event);

  if (this->immediate_) {
    ret = asyncExec(event, wait_events, cmd);
  } else {
    this->deferred_commands_.push_back(deferred_cmd_t{event, wait_events, cmd});
  }
  cv_.notify_one(); // opportunistically notify any in-flight threads to
                    // wake-up and do some work.
  return ret;
}

//-----------------------------------------------------------------------------

std::unordered_set<ZeEvent *> ZeCmdList::submit() {
  assert(scheduled_events_.empty());
  assert(!this->immediate_);
  assert(this->closed_);
  std::unordered_set<ZeEvent *> events;
  for (auto &ec : this->deferred_commands_) {
    asyncExec(ec.event_, ec.wait_events_, ec.cmd_);
    events.insert(ec.event_);
  }
  return events;
}

//-----------------------------------------------------------------------------

ze_result_t ZeCmdList::barrier(ZeEvent *new_event,
                               std::vector<ZeEvent *> wait_events) {
  if (wait_events.empty()) {
    for (auto e : scheduled_events_.copy()) {
      wait_events.push_back(e);
    }
    scheduled_events_.clear();
  }

  // traverse wait events list to set the depended kernels' sync value
  for (auto const e : wait_events) {
    ZeKernel *k = e->getKernel();
    if (k && k == kernels().back()) {
      kernel_sync_map[k] = true;
    }
  }

  new_event = assignEvent(new_event);
  // store last barrier event, since we need to order all other commands after
  // the latest barrer
  last_barrier_event_ = new_event;
  this->append(new_event, wait_events, []() { return ZE_RESULT_SUCCESS; });
  return ZE_RESULT_SUCCESS;
}

//-----------------------------------------------------------------------------

ze_result_t ZeCmdList::append_signal_event(ZeEvent *event) {
  std::vector<ZeEvent *> wait_events;
  std::string seq_flag = util::get_envvar("L0SIM_SEQUENCE_SIGNAL_RESET_EVENT");
  if (!seq_flag.empty() && seq_flag != "0") {
    if (last_signal_reset_event_ != nullptr) {
      wait_events.push_back(last_signal_reset_event_);
    }
  }
  auto signal_event = assignEvent(nullptr);
  assert(signal_event);
  // store last signal_reset event, in case we want to debug event sequencing
  // in general since signal & reset are not ordered in the
  // command list. For example:
  //  Assume e1 and e2 are not signaled, and e3 is in signaled state:
  //    signal(e1)
  //    signal(e2)
  //    reset(e3)
  // When e2 is signaled, it is NOT guaranteed that e1 is also signaled.
  // it is not guarnateed that e3 is in non-signaled state either way.
  // Same true when e3 is in non-signaled state, it is NOT guaranteed that e2
  // or e1is signaled.
  // With L0SIM_SEQUENCE_SIGNAL_RESET_EVENT=1 we can enforce that when e2 is
  // signaled, it is guaranteed that e1 is signalled, and when e3 is in
  // non-signalled state, it is guaranteed that e2 and e1 is signalled.
  last_signal_reset_event_ = signal_event;
  auto ret = this->append(signal_event, wait_events, [event]() {
    return ZeEvent::signal(event, ZE_RESULT_SUCCESS);
  });
  return ret;
}

//-----------------------------------------------------------------------------

ze_result_t ZeCmdList::append_reset_event(ZeEvent *event) {
  std::vector<ZeEvent *> wait_events;
  std::string seq_flag = util::get_envvar("L0SIM_SEQUENCE_SIGNAL_RESET_EVENT");
  if (!seq_flag.empty() && seq_flag != "0") {
    if (last_signal_reset_event_ != nullptr) {
      wait_events.push_back(last_signal_reset_event_);
    }
  }
  auto reset_event = assignEvent(nullptr);
  assert(reset_event);
  last_signal_reset_event_ = reset_event;
  auto ret = this->append(reset_event, wait_events,
                          [event]() { return ZeEvent::reset(event); });
  return ret;
}

//-----------------------------------------------------------------------------

uint32_t ZeCmdList::get_pending_events() {
  return this->scheduled_events_.size();
}

//-----------------------------------------------------------------------------

ze_result_t ZeCmdList::append(ZeEvent *event,
                              std::vector<ZeEvent *> wait_events,
                              ZeKernel *kernel,
                              std::function<ze_result_t()> cmd) {
  auto barrier_event = last_barrier_event_.load();
  if (event != barrier_event && barrier_event != nullptr) {
    wait_events.push_back(barrier_event);
  }
  event = this->assignEvent(event);
  ze_result_t ret = ZE_RESULT_SUCCESS;
  scheduled_events_.insert(event);

  if (kernel) {
    kernels_.push_back(kernel);
    event->setKernel(kernel);
    // initialize cerrent kernel's sync value as false
    kernel_sync_map.insert(std::make_pair(kernel, false));
  }

  if (this->immediate_) {
    // if device_type == kCPU
    ret = asyncExec(event, wait_events, cmd);
  } else {
    this->deferred_commands_.push_back(deferred_cmd_t{event, wait_events, cmd});
  }
  cv_.notify_one();

  return ret;
}

//-----------------------------------------------------------------------------

bool ZeCmdList::isSyncRequired(ZeKernel *kernel) {
  auto search = kernel_sync_map.find(kernel);
  if (search != kernel_sync_map.end()) {
    return search->second;
  }
  return false;
}

} // namespace __zert__
