// INTEL_COPYRIGHT_BEGIN
// Copyright (c) 2020, Intel Corporation. All rightrs reserved.
// INTEL_COPYRIGHT_END
#pragma once

#include "ze_driver.hpp"

struct _ze_context_handle_t {};

namespace __zert__ {

struct ZeContext final : _ze_context_handle_t {
public:
  ZeContext(ZeDriver *, ze_context_desc_t desc);
  ~ZeContext();
  ZeDriver *driver() { return driver_; }
  ze_context_desc_t const &desc() const { return desc_; }
  ze_result_t
  getMemAllocProperties(const void *ptr,
                        ze_memory_allocation_properties_t *pMemAllocProperties,
                        ze_device_handle_t *phDevice);

private:
  ZeDriver *driver_;
  ze_context_desc_t desc_;
};

} // namespace __zert__
