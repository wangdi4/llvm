// INTEL_COPYRIGHT_BEGIN
// Copyright (c) 2020, Intel Corporation. All rightrs reserved.
// INTEL_COPYRIGHT_END

#include "ze_kernel.hpp"
#include "ze_buffer.hpp"
#include "ze_utils.hpp"
#include <cassert>
#include <cstdio>

namespace __zert__ {

ZeKernel::ZeKernel(ZeModule *module) : module_(module) {}

ZeKernel::~ZeKernel() {}

ze_result_t ZeKernel::initialize(const ze_kernel_desc_t *desc) {
  assert(desc != nullptr);

  descriptor_ = *desc;

  return ZE_RESULT_SUCCESS;
}

ze_result_t ZeKernel::setArgumentValue(uint32_t argIndex, size_t argSize,
                                       const void *pArgValue) {
  if (argIndex >= param_count_ && argSize > 0) {
    ZESIMERR << "Incorrect arg index, expecting argIndex= " << argIndex << " < "
             << param_count_ << pArgValue;
    return ZE_RESULT_ERROR_INVALID_KERNEL_ARGUMENT_INDEX;
  }

  arguments_set_.insert(argIndex);
  return ZE_RESULT_SUCCESS;
}

ze_result_t
ZeKernel::suggestGroupSize(uint32_t globalSizeX, uint32_t globalSizeY,
                           uint32_t globalSizeZ, uint32_t *groupSizeX,
                           uint32_t *groupSizeY, uint32_t *groupSizeZ) {
  (void)globalSizeX;
  (void)globalSizeY;
  (void)globalSizeZ;
  *groupSizeX = 4;
  *groupSizeY = 1;
  *groupSizeZ = 1;
  return ZE_RESULT_SUCCESS;
}

ze_result_t ZeKernel::setGroupSize(uint32_t groupSizeX, uint32_t groupSizeY,
                                   uint32_t groupSizeZ) {
  if ((0 == groupSizeX) || (0 == groupSizeY) || (0 == groupSizeZ)) {
    ZESIMERR << "Cannot have groupSize zero, got [" << groupSizeX << ","
             << groupSizeY << "," << groupSizeZ << "]";
    return ZE_RESULT_ERROR_INVALID_ARGUMENT;
  }

  this->groupSize[0] = groupSizeX;
  this->groupSize[1] = groupSizeY;
  this->groupSize[2] = groupSizeZ;

  return ZE_RESULT_SUCCESS;
}

ze_result_t ZeKernel::launch(ze_group_count_t config) {
  return module()->device()->launch(this, config);
}

ze_result_t ZeKernel::setGroupCount(uint32_t groupCountX, uint32_t groupCountY,
                                    uint32_t groupCountZ) {
  if ((0 == groupCountX) || (0 == groupCountY) || (0 == groupCountZ)) {
    ZESIMERR << "Cannot have groupSize zero, got [" << groupCountX << ","
             << groupCountY << "," << groupCountZ << "]";
    return ZE_RESULT_ERROR_INVALID_ARGUMENT;
  }

  this->groupCount[0] = groupCountX;
  this->groupCount[1] = groupCountY;
  this->groupCount[2] = groupCountZ;

  return ZE_RESULT_SUCCESS;
}
} // namespace __zert__
