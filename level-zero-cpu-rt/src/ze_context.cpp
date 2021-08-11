// INTEL_COPYRIGHT_BEGIN
// Copyright (c) 2020, Intel Corporation. All rightrs reserved.
// INTEL_COPYRIGHT_END

#include "ze_context.hpp"
#include <cassert>

namespace __zert__ {

ZeContext::ZeContext(ZeDriver *driver, ze_context_desc_t desc)
    : driver_(driver), desc_(desc) {}

ZeContext::~ZeContext() {}

ze_result_t ZeContext::getMemAllocProperties(
    const void *ptr, ze_memory_allocation_properties_t *pMemAllocProperties,
    ze_device_handle_t *phDevice) {
  assert(nullptr != this->driver_ && "driver can't be null");
  return this->driver()->getMemAllocProperties(ptr, pMemAllocProperties,
                                               phDevice);
}

} // namespace __zert__
