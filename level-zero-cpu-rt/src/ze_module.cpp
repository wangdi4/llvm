// INTEL_COPYRIGHT_BEGIN
// Copyright (c) 2020, Intel Corporation. All rightrs reserved.
// INTEL_COPYRIGHT_END

#include "ze_module.hpp"
#include "ze_kernel.hpp"
#include "ze_utils.hpp"

#include <cassert>
#include <map>

namespace __zert__ {

ZeModule::ZeModule(ZeContext *context, ZeDevice *device)
    : context_(context), device_(device) {}

ZeModule::~ZeModule() { kernels_.clear(); }

ze_result_t ZeModule::initialize(const ze_module_desc_t *desc) {
  assert(desc != nullptr);
  assert(desc->pInputModule != nullptr);
  assert(desc->inputSize != 0);

  if (desc->format != ZE_MODULE_FORMAT_NATIVE) {
    ZESIMERR << "Only supporting ZE_MODULE_FORMAT_NATIVE";
    return ZE_RESULT_ERROR_INVALID_ARGUMENT;
  }

  descriptor_ = *desc;

  return ZE_RESULT_SUCCESS;
}

//-----------------------------------------------------------------------------

ze_result_t ZeModule::createKernel(ze_kernel_desc_t const *desc,
                                   ze_kernel_handle_t *kernel) {
  auto zekernel = std::make_unique<ZeKernel>(this);

  ze_result_t ret = zekernel->initialize(desc);
  if (ZE_RESULT_SUCCESS != ret) {
    ZESIMERR << "Failed to initialize kernel";
    return ret;
  }

  *kernel = zekernel.get();
  kernels_.push_back(std::move(zekernel));

  return ZE_RESULT_SUCCESS;
}

ze_result_t ZeModule ::destroyKernel(ze_kernel_handle_t kernel) {
  for (auto it = kernels_.begin(); it != kernels_.end(); ++it) {
    if (it->get() == kernel) {
      kernels_.erase(it);
      return ZE_RESULT_SUCCESS;
    }
  }
  ZESIMERR << "Unknown kernel, cannot destroy";
  return ZE_RESULT_ERROR_INVALID_ARGUMENT;
}

} // namespace __zert__
