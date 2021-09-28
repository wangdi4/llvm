// INTEL_COPYRIGHT_BEGIN
// Copyright (c) 2020, Intel Corporation. All rightrs reserved.
// INTEL_COPYRIGHT_END

#include "ze_event.hpp"
#include "ze_cmdlist.hpp"
#include "ze_utils.hpp"
#include <chrono>
#include <cstring>
#include <unordered_set>

namespace __zert__ {

//-----------------------------------------------------------------------------

ZeEventPool::ZeEventPool() = default;
ZeEventPool::~ZeEventPool() = default;

//-----------------------------------------------------------------------------

std::unique_ptr<ZeEventPool>
ZeEventPool::create(ze_event_pool_desc_t const &desc, ZeDriver *driver,
                    std::vector<ZeDevice *> const &devices,
                    ze_result_t &result) {
  auto pool = std::unique_ptr<ZeEventPool>(new ZeEventPool());
  pool->desc_ = desc;
  pool->driver_ = driver;
  pool->devices_ = devices;
  result = ZE_RESULT_SUCCESS;
  return pool;
}

//-----------------------------------------------------------------------------

ze_result_t ZeEventPool::destroy(ZeEventPool *pool) {
  return pool->driver()->destroyEventPool(pool);
}

//-----------------------------------------------------------------------------

uint32_t ZeEventPool::uniqueEventIndex() {
  std::unordered_set<uint32_t> events_idx_set;
  for (auto &e : events_) {
    events_idx_set.insert(e->desc().index);
  }
  for (uint32_t i = 0; i < desc_.count; ++i) {
    if (events_idx_set.count(i) == 0) {
      return i;
    }
  }
  return desc_.count;
}

//-----------------------------------------------------------------------------

ze_result_t ZeEventPool::createEvent(ze_event_desc_t const &desc,
                                     ze_event_handle_t *eventp) {
  if (events_.size() == desc_.count) {
    ZESIMERR << "Unable to add more events, max size " << desc_.count
             << " is reached";
    return ZE_RESULT_ERROR_UNKNOWN;
  }
  if (desc.index >= desc_.count) {
    ZESIMERR << "Event index " << desc.index << " is out of range [0,"
             << desc_.count << "]";
    return ZE_RESULT_ERROR_UNKNOWN;
  }
  for (auto &e : events_) {
    if (e->desc().index == desc.index) {
      *eventp = e.get();
      return ZE_RESULT_SUCCESS;
    }
  }
  auto event = std::unique_ptr<ZeEvent>(new ZeEvent(this, desc));
  *eventp = event.get();
  events_.push_back(std::move(event));
  return ZE_RESULT_SUCCESS;
}

//-----------------------------------------------------------------------------

bool ZeEventPool::owns(ZeEvent *event) {
  auto it = std::find_if(events_.begin(), events_.end(),
                         [event](auto &e) { return e.get() == event; });
  return it != events_.end();
}

//-----------------------------------------------------------------------------

ze_result_t ZeEventPool::destroyEvent(ZeEvent *event) {
  auto it = std::find_if(events_.begin(), events_.end(),
                         [event](auto &e) { return e.get() == event; });
  if (it != events_.end()) {
    events_.erase(it);
    return ZE_RESULT_SUCCESS;
  }
  ZESIMERR << "Event is not found in the event pool";
  return ZE_RESULT_ERROR_UNKNOWN;
}

//-----------------------------------------------------------------------------

ZeEvent::ZeEvent(ZeEventPool *event_pool, ze_event_desc_t const &desc)
    : event_pool_(event_pool), desc_(desc) {}

//-----------------------------------------------------------------------------

ZeEvent::~ZeEvent() = default;

//-----------------------------------------------------------------------------

ze_result_t ZeEvent::signal(ZeEvent *event, ze_result_t ret) {
  // do not allow reordering of the following stores:
  // this is to ensure that ret value is setwhenever other threads observed
  // ready_ to be true
  event->result_.store(ret, std::memory_order_release);
  event->ready_.store(true, std::memory_order_release);
  return ZE_RESULT_SUCCESS;
}

//-----------------------------------------------------------------------------

ze_result_t ZeEvent::reset(ZeEvent *event) {
  event->result_.store(ZE_RESULT_ERROR_UNKNOWN, std::memory_order_release);
  event->ready_.store(false, std::memory_order_release);
  return ZE_RESULT_SUCCESS;
}

//-----------------------------------------------------------------------------

ze_result_t ZeEvent::queryStatus() {
  if (is_ready()) {
    return ZE_RESULT_SUCCESS;
  }
  return ZE_RESULT_NOT_READY;
}

//----------------------------------------------------------------------------

ze_result_t ZeEvent::hostSync(uint64_t timeout_in_ms) {
  auto start = std::chrono::steady_clock::now();
  auto end = std::chrono::steady_clock::now();
  std::chrono::duration<double> elapsed_seconds = end - start;
  double timeout_in_sec = double(timeout_in_ms) / 1e3;

  do {
    if (is_ready()) {
      return ZE_RESULT_SUCCESS;
    }
    ZeCmdList::cv_.notify_one();
    end = std::chrono::steady_clock::now();
    elapsed_seconds = end - start;
  } while (elapsed_seconds.count() < timeout_in_sec);
  return ZE_RESULT_NOT_READY;
}

//-----------------------------------------------------------------------------

ze_result_t
ZeEvent::queryKernelTimestamp(ze_kernel_timestamp_result_t *dstptr) {
  if (queryStatus() != ZE_RESULT_SUCCESS) {
    return ZE_RESULT_NOT_READY;
  }

  dstptr->context.kernelStart = this->contextStartTS;
  dstptr->context.kernelEnd = this->contextEndTS;
  dstptr->global.kernelStart = this->globalStartTS;
  dstptr->global.kernelEnd = this->globalEndTS;

  return ZE_RESULT_SUCCESS;
}
} // namespace __zert__
