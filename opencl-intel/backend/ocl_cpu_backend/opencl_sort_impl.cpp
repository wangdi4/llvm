// Copyright (C) 2022 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#include "cl_dev_backend_api.h"
#include <algorithm>
#include <vector>

template <typename T> bool descendingComp(T I, T J) { return (I > J); }

template <typename PairT> bool descendingCompPair(PairT I, PairT J) {
  return (I.first > J.first);
}

template <typename T>
static void sortImpl(T *Data, uint32_t Size, bool Descending) {
  if (Descending == 0)
    std::stable_sort(Data, Data + Size);
  else
    std::stable_sort(Data, Data + Size, descendingComp<T>);
}

template <typename T, typename U>
static void sortImplKeyValue(T *Key, U *Value, uint32_t Size, bool Descending) {
  std::vector<std::pair<T, U>> Temp;
  for (uint32_t I = 0; I < Size; ++I) {
    Temp.push_back(std::make_pair(Key[I], Value[I]));
  }
  if (Descending == 0)
    std::stable_sort(Temp.begin(), Temp.end());
  else
    std::stable_sort(Temp.begin(), Temp.end(),
                     descendingCompPair<std::pair<T, U>>);

  for (uint32_t I = 0; I < Size; ++I) {
    Key[I] = Temp[I].first;
    Value[I] = Temp[I].second;
  }
}

#define SORT_IMPL_KEY_ONLY_BUILTIN(TYPE)                                       \
  extern "C" LLVM_BACKEND_API void __ocl_sort_##TYPE(                          \
      TYPE *data, uint32_t size, bool mode) {                                  \
    sortImpl(data, size, mode);                                                \
  }
#define SORT_KEY_VALUE_IMPL_BUILTIN(TTYPE, UTYPE)                              \
  extern "C" LLVM_BACKEND_API void __ocl_sort_##TTYPE##_##UTYPE(               \
      TTYPE *key, UTYPE *value, uint32_t size, bool mode) {                    \
    sortImplKeyValue(key, value, size, mode);                                  \
  }

#define SORT_KEY_VALUE_HELPER(TTYPE)                                           \
  SORT_KEY_VALUE_IMPL_BUILTIN(TTYPE, char)                                     \
  SORT_KEY_VALUE_IMPL_BUILTIN(TTYPE, uint8_t)                                  \
  SORT_KEY_VALUE_IMPL_BUILTIN(TTYPE, short)                                    \
  SORT_KEY_VALUE_IMPL_BUILTIN(TTYPE, uint16_t)                                 \
  SORT_KEY_VALUE_IMPL_BUILTIN(TTYPE, int)                                      \
  SORT_KEY_VALUE_IMPL_BUILTIN(TTYPE, uint32_t)                                 \
  SORT_KEY_VALUE_IMPL_BUILTIN(TTYPE, long)                                     \
  SORT_KEY_VALUE_IMPL_BUILTIN(TTYPE, uint64_t)                                 \
  SORT_KEY_VALUE_IMPL_BUILTIN(TTYPE, half)                                     \
  SORT_KEY_VALUE_IMPL_BUILTIN(TTYPE, float)                                    \
  SORT_KEY_VALUE_IMPL_BUILTIN(TTYPE, double)

#define SORT_IMPL(TYPE)                                                        \
  SORT_KEY_VALUE_HELPER(TYPE)                                                  \
  SORT_IMPL_KEY_ONLY_BUILTIN(TYPE)

SORT_IMPL(char)
SORT_IMPL(uint8_t)
SORT_IMPL(short)
SORT_IMPL(uint16_t)
SORT_IMPL(int)
SORT_IMPL(uint32_t)
SORT_IMPL(long)
SORT_IMPL(uint64_t)
SORT_IMPL(half)
SORT_IMPL(float)
SORT_IMPL(double)
