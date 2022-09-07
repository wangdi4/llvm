; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

; RUN: opt  < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; This test checks that casting from outer structure, loading and then casting
; to the inner-most structure won't produce a bad casting. This is the same
; test case as zero-element07.ll but with an argument.

; Check Bad casting
; CHECK-LABEL:  LLVMType: %class.TestClass = type { i64, i64, i64 }
; CHECK-NOT: Safety data:{{.*}}Bad casting{{.*}}

%"class.std::vector.12" = type { %"struct.std::_Vector_base.13" }
%"struct.std::_Vector_base.13" = type { %"struct.std::_Vector_base<TestClass, std::allocator<TestClass>>::_Vector_impl.67" }
%"struct.std::_Vector_base<TestClass, std::allocator<TestClass>>::_Vector_impl.67" = type { %class.TestClass*, %class.TestClass*, %class.TestClass* }
%class.TestClass = type {i64, i64, i64}

define internal %class.TestClass* @loadData(%"class.std::vector.12"* nocapture %arg) {
bb:
  %tmp = bitcast %"class.std::vector.12"* %arg to i64*
  %tmp2 = load i64, i64* %tmp
  %tmp3 = inttoptr i64 %tmp2 to %class.TestClass*
  ret %class.TestClass* %tmp3
}