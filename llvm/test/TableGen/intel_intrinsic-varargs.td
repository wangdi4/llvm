// INTEL_FEATURE_ISA_FP16
// REQUIRES: intel_feature_isa_fp16
// RUN: llvm-tblgen -gen-intrinsic-impl -DINTEL_FEATURE_ISA_FP16 %s | FileCheck %s
// XFAIL: vg_leak

class IntrinsicProperty;
class SDNodeProperty;

class ValueType<int size, int value> {
  string Namespace = "MVT";
  int Size = size;
  int Value = value;
}

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

// isVoid needs to match the definition in ValueTypes.td
def isVoid : ValueType<0, 130>;   // Produces no value
def llvm_vararg_ty : LLVMType<isVoid>;   // this means vararg here

// CHECK: /* 0 */ 0, 29, 0,
def int_foo : Intrinsic<"llvm.foo", [llvm_vararg_ty]>;
// end INTEL_FEATURE_ISA_FP16
