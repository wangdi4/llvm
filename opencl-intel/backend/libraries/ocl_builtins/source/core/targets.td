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

// Used for extending OclBuiltinImpl fields
// Usage:
//   def : OclBuiltinImpl<...>, SSE;
// so that the field `Target` is merged into the OclBuiltinImpl record.

class OclImplTarget<string name> {
  string Name = name;
  code Prolog = [{
    #ifdef __x86_64__
      #include "svml_ia32e.h"
    #else
      #include "svml_ia32.h"
    #endif
  }];
}

defvar shared_prolog = [{
  #include "svml_shared.h"
  #include "generic-builtin-defines.h"
}];
let Prolog = shared_prolog in
def _SHARED_TARGET : OclImplTarget<"shared">;

def _SSE_TARGET : OclImplTarget<"sse">;
def _AVX_TARGET : OclImplTarget<"avx">;
def _AVX2_TARGET : OclImplTarget<"avx2">;
def _AVX512_TARGET : OclImplTarget<"avx512">;
def _AMX_TARGET : OclImplTarget<"amx">;

class SHARED {
  OclImplTarget Target = _SHARED_TARGET;
}

class SSE {
  OclImplTarget Target = _SSE_TARGET;
}

class AVX {
  OclImplTarget Target = _AVX_TARGET;
}

class AVX2 {
  OclImplTarget Target = _AVX2_TARGET;
}

class AVX512 {
  OclImplTarget Target = _AVX512_TARGET;
}

class AMX {
  OclImplTarget Target = _AMX_TARGET;
}
