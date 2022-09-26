// RUN: %ocltblgen %s --gen-ocl-bi --handle-impl-target-inheritance --emit-target-name=shared | FileCheck %s --check-prefix=CHECK-SHARED
// RUN: %ocltblgen %s --gen-ocl-bi --handle-impl-target-inheritance --emit-target-name=sse | FileCheck %s --check-prefix=CHECK-SSE
// RUN: %ocltblgen %s --gen-ocl-bi --handle-impl-target-inheritance --emit-target-name=avx | FileCheck %s --check-prefix=CHECK-AVX
// RUN: %ocltblgen %s --gen-ocl-bi --handle-impl-target-inheritance --emit-target-name=avx2 | FileCheck %s --check-prefix=CHECK-AVX2
// RUN: %ocltblgen %s --gen-ocl-bi --handle-impl-target-inheritance --emit-target-name=avx512 | FileCheck %s --check-prefix=CHECK-AVX512
// RUN: %ocltblgen %s --gen-ocl-bi --handle-impl-target-inheritance --emit-target-name=amx | FileCheck %s --check-prefix=CHECK-AMX

include "GENERIC/opencl-builtin.td"
include "source/core/targets.td"

let Types = [v1i32] in
def bi : OclBuiltin<"foo", (outs), (ins gentype:$x)>;

// CHECK-SHARED: Target short name: shared
def : OclBuiltinImpl<bi, bi.Types, false, [{
  // Target short name: $TargetShortName
  return;
}]>, SHARED;

// CHECK-SSE: Target short name: {{h|n}}8
// CHECK-AVX: Target short name: {{e|g}}9
// CHECK-AVX2: Target short name: {{l|s}}9
// CHECK-AVX512: Target short name: {{z|x}}0
// CHECK-AMX: Target short name: {{z|x}}1
def : OclBuiltinImpl<bi, bi.Types, false, [{
  // Target short name: $TargetShortName
  return;
}]>, SSE;
