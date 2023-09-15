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

include "avx512f/avx512f.td"

// INTEL_CUSTOMIZATION
defset list<OclBuiltinImpl> amx_impls = {
  include "amx/intel-matrix_impl.td"
}
// end INTEL_CUSTOMIZATION

// includes AVX-512 F
class AMX : AVX512F {
    let Impls = amx_impls; // INTEL
}
