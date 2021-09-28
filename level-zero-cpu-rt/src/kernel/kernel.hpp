// Copyright (c) 2020, Intel Corporation. All rightrs reserved.
// INTEL_COPYRIGHT_END
#pragma once

#include <map>
#include <set>

struct _ze_kernel_handle_t
{
};

namespace cpulevel0 {


class Kernel {
  public:

    Kernel(ZeModule *module);
    ~Kernel();

    ze_result_t destroy();
    ze_result_t initialize(const ze_kernel_desc_t *desc);
    ze_result_t
    setArgumentValue(uint32_t argIndex, size_t argSize, const void *pArgVaule);
    ze_result_t suggestGroupSize(uint32_t globalSizeX,
                                 uint32_t globalSizeY,
                                 uint32_t globalSizeZ,
                                 uint32_t *groupSizeX,
                                 uint32_t *groupSizeY,
                                 uint32_t *groupSizeZ);
    ze_result_t
    setGroupSize(uint32_t groupSizeX, uint32_t groupSizeY, uint32_t groupSizeZ);
    ZeModule *module() { return module_; }
    uint32_t paramCount() { return param_count_; }
    uint32_t argCount() { return (uint32_t)arguments_set_.size(); }

    std::vector<std::vector<uint8_t>> arguments_;
    std::set<uint32_t> arguments_set_;
    ze_result_t launch(ze_group_count_t);
    uint32_t paramSize(uint32_t pos) { return param_sizes_[pos]; }
    std::vector<uint32_t> const &getGroupSize() const { return groupSize; }
    std::string const &getGenBinaryFileName() const { return binary_name_; }
    
    ze_result_t setGroupCount(uint32_t groupCountX,
                              uint32_t groupCountY,
                              uint32_t groupCountZ);
    std::vector<uint32_t> const &getGroupCount() const { return groupCount; }

  private:

    ZeModule *module_ = nullptr;
    ze_kernel_desc_t descriptor_;
    std::vector<uint32_t> groupSize{0, 0, 0};
    uint32_t param_count_ = 0;
    std::vector<uint32_t> param_sizes_;
    std::string binary_name_;
    std::vector<uint32_t> groupCount{0, 0, 0};
};

}    // namespace __zert__
