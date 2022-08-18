// RUN: %ocltblgen %s --gen-ocl-bi --handle-impl-target-inheritance --emit-target-name=shared | FileCheck %s

include "GENERIC/opencl-builtin.td"
include "source/core/targets.td"

let Types = [v64i32] in
def bi : OclBuiltin<"foo", (outs), (ins gentype:$x)>;

// Checks that the "$LoSuffix" macro is replaced with [VecLength/2]

// CHECK: VecLength = 64
// CHECK: LoSuffix = 32
defvar num_arg = !size(bi.Ins);
defvar args = RangeStr<"$Arg#VarName", Range<0, num_arg>.Tout>.Tout;
def : OclBuiltinImpl<bi, bi.Types, false, [{
  // VecLength = $VecLength
  // LoSuffix = $LoSuffix
  return;
}]>, SHARED;
