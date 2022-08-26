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

include "avx2/AVX2.td"

defset list<OclBuiltinImpl> avx512f_impls = {
  include "avx512f/math_impl.td"
}

class AVX512F : AVX2 {
  include "avx512f/math.td"
  include "avx512f/common.td"
  include "avx512f/conversion.td"
  include "avx512f/vloadstore.td"
  include "avx512f/integers.td"
  include "avx512f/masked_load_store.td"
  include "avx512f/transpose.td"

  let Impls = avx512f_impls;
}

