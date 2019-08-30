<<<<<<< HEAD
// INTEL_CUSTOMIZATION
// INTEL_FEATURE_ISA_FP16
// UNSUPPORTED: intel_feature_isa_fp16
// end INTEL_FEATURE_ISA_FP16
// end INTEL_CUSTOMIZATION
// RUN: llvm-tblgen -gen-intrinsic-impl %s | FileCheck %s
=======
// RUN: llvm-tblgen -gen-intrinsic-impl -I %p/../../include %s | FileCheck %s
>>>>>>> 8d5fbecf8ba7a2ef98d003e5d1f85c023b761ad2
// XFAIL: vg_leak

include "llvm/CodeGen/ValueTypes.td"

class IntrinsicProperty;
class SDNodeProperty;

class LLVMType<ValueType vt> {
  ValueType VT = vt;
}

class Intrinsic<string name, list<LLVMType> param_types = []> {
  string LLVMName = name;
  bit isTarget = 0;
  string TargetPrefix = "";
  list<LLVMType> RetTypes = [];
  list<LLVMType> ParamTypes = param_types;
  list<IntrinsicProperty> IntrProperties = [];
  list<SDNodeProperty> Properties = [];
}

def llvm_vararg_ty : LLVMType<isVoid>;   // this means vararg here

// CHECK: /* 0 */ 0, 29, 0,
def int_foo : Intrinsic<"llvm.foo", [llvm_vararg_ty]>;
