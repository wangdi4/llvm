// INTEL_COPYRIGHT_BEGIN
// Copyright (c) 2020, Intel Corporation. All rightrs reserved.
// INTEL_COPYRIGHT_END
#pragma once

//#include "genISAi_api.h"
#include "ze_device.hpp"
#include <list>
#include <memory>
#include <string>

struct _ze_module_handle_t {};

namespace __zert__ {

struct ZeKernel;

struct ZeModule final : _ze_module_handle_t {
  ZeModule(ZeContext *, ZeDevice *);
  ZeModule(ZeModule &&) = default;
  ~ZeModule();

  ze_result_t initialize(const ze_module_desc_t *desc);
  ZeContext *context() { return context_; }
  ZeDevice *device() { return device_; }
  ze_result_t createKernel(ze_kernel_desc_t const *desc,
                           ze_kernel_handle_t *handle);
  ze_result_t destroyKernel(ze_kernel_handle_t);

private:
  ZeContext *context_ = nullptr;
  ZeDevice *device_ = nullptr;
  ze_module_desc_t descriptor_;

  std::list<std::unique_ptr<ZeKernel>> kernels_;
};

} // namespace __zert__
