// RUN: %ocltblgen %s --gen-ocl-bi --handle-impl-target-inheritance --emit-target-name=shared | FileCheck %s

include "GENERIC/opencl-builtin.td"
include "source/core/targets.td"

let Types = [v1i32] in
def bi : OclBuiltin<"foo", (outs), (ins gentype:$x, gentype:$y)>;

// Checks that the "$ARGS" macro is first replaced with "$Arg0VarName, $Arg1VarName"
// Then recursively replaced with "x, y"

// CHECK: I have 2 arguments: x, y
defvar num_arg = !size(bi.Ins);
defvar args = RangeStr<"$Arg#VarName", Range<0, num_arg>.Tout>.Tout;
def : OclBuiltinImpl<bi, bi.Types, false, [{
  // I have $NUMARG arguments: $ARGS
  return;
}], [["$NUMARG", !cast<string>(num_arg)], ["$ARGS", !interleave(args, ", ")]]>, SHARED;
