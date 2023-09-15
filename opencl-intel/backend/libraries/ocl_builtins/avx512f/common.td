// Copyright (C) 2016 Intel Corporation
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

// step
OclBuiltinImpl step_v16f32 = OclBuiltinImpl<step, [v16f32] , 0, [{
    // According to the specification this built-in should return
    // 0.0 if $Arg1VarName < $Arg0VarName, else 1.0

    __mmask16 cmpMask = _mm512_cmp_ps_mask($Arg1VarName, $Arg0VarName, _CMP_GE_OS);
    return _mm512_maskz_mov_ps(cmpMask, _mm512_set1_ps(1.0f));
  }]>;

OclBuiltinImpl step_v8f64 = OclBuiltinImpl<step, [v8f64] , 0, [{
    // According to the specification this built-in should return
    // 0.0 if $Arg1VarName < $Arg0VarName, else 1.0,

    __mmask8 cmpMask = _mm512_cmp_pd_mask($Arg1VarName, $Arg0VarName, _CMP_GE_OS);
    return _mm512_maskz_mov_pd(cmpMask, _mm512_set1_pd(1.0));
  }]>;
