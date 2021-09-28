// INTEL_COPYRIGHT_BEGIN
// Copyright (c) 2020, Intel Corporation. All rightrs reserved.
// INTEL_COPYRIGHT_END
#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <vector>

namespace __zert__ {

inline void my_strcpy_s(char *dst, size_t max_len, char const *src) {
  for (size_t i = 0; i < max_len; ++i) {
    dst[i] = src[i];
    if (src[i] == 0) {
      break;
    }
  }
}

namespace gsf {

enum MEMORY_OBJECT_CONTROL {
  MEMORY_OBJECT_CONTROL_PVC_DEFAULT = 0, // L3 writeback, L1 uncacheable
                                         //(L1 default cacheability by grits)
  MEMORY_OBJECT_CONTROL_PVC_UNCACHEABLE, // both L3 and L1 uncacheable
  MEMORY_OBJECT_CONTROL_PVC_L3WB_L1_UNCACHEABLE, // L3 writeback, L1
                                                 // uncacheable
  MEMORY_OBJECT_CONTROL_PVC_L3WB_L1_WRITEBYPASS,
  MEMORY_OBJECT_CONTROL_PVC_L3WB_L1_WRITEBACK,
  MEMORY_OBJECT_CONTROL_PVC_L3WB_L1_WRITETHROUGH,
  MEMORY_OBJECT_CONTROL_PVC_L3WB_L1_WRITESTREAMING,
  MEMORY_OBJECT_CONTROL_PVC_COUNT,

  MEMORY_OBJECT_CONTROL_UNKNOWN = 0xff
};
} // namespace gsf
} // namespace __zert__
