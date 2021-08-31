// Copyright (c) 2020, Intel Corporation. All rightrs reserved.
// INTEL_COPYRIGHT_END
#pragma once

#include "ze_module.hpp"
#include <map>
#include <set>

struct _ze_kernel_handle_t {};

namespace __zert__ {

// Index of Binding Table State Surfaces: 00h - 0EFh
// https://gfxspecs.intel.com/Predator/Home/Index/12338
const unsigned int max_bti = 239;

struct ZeKernel final : _ze_kernel_handle_t {
public:
  friend struct GenISAdevice;
  friend struct GRITSdevice;

  ZeKernel(ZeModule *);
  ~ZeKernel();

  ze_result_t destroy();
  ze_result_t initialize(const ze_kernel_desc_t *desc);
  ze_result_t setArgumentValue(uint32_t argIndex, size_t argSize,
                               const void *pArgVaule);
  ze_result_t suggestGroupSize(uint32_t globalSizeX, uint32_t globalSizeY,
                               uint32_t globalSizeZ, uint32_t *groupSizeX,
                               uint32_t *groupSizeY, uint32_t *groupSizeZ);
  ze_result_t setGroupSize(uint32_t groupSizeX, uint32_t groupSizeY,
                           uint32_t groupSizeZ);
  ZeModule *module() { return module_; }
  uint32_t paramCount() { return param_count_; }
  uint32_t argCount() { return (uint32_t)arguments_set_.size(); }

  std::vector<std::vector<uint8_t>> arguments_;
  std::set<uint32_t> arguments_set_;
  ze_result_t launch(ze_group_count_t);
  uint32_t paramSize(uint32_t pos) { return param_sizes_[pos]; }
  std::vector<uint32_t> const &getGroupSize() const { return groupSize; }
  std::string const &getGenBinaryFileName() const { return gen_binary_name_; }
  std::vector<uint8_t> const &getGenBinary() const { return gen_binary_; }
  bool isLargeGRFMode() { return large_grf_mode_; }
  ze_result_t setGroupCount(uint32_t groupCountX, uint32_t groupCountY,
                            uint32_t groupCountZ);
  std::vector<uint32_t> const &getGroupCount() const { return groupCount; }

private:
  ZeModule *module_ = nullptr;
  ze_kernel_desc_t descriptor_;
  std::vector<uint32_t> groupSize{0, 0, 0};
  uint32_t param_count_ = 0;
  std::vector<uint32_t> param_sizes_;
  std::string gen_binary_name_;
  std::vector<uint8_t> gen_binary_;
  bool large_grf_mode_ = true;
  std::vector<uint32_t> groupCount{0, 0, 0};
};

} // namespace __zert__
