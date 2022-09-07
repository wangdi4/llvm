; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

; RUN: opt  < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; This test checks that casting from outer structure, loading and then casting
; to a structure that is not in the nest produces a bad casting.

; Check Bad casting
; CHECK-LABEL:  LLVMType: %class.TestClass = type { i64, i64, i64 }
; CHECK: Safety data:{{.*}}Bad casting{{.*}}

%"class.std::vector.12" = type { %"struct.std::_Vector_base.13" }
%"struct.std::_Vector_base.13" = type { %"struct.std::_Vector_base<TestClass, std::allocator<TestClass>>::_Vector_impl.67" }
%"struct.std::_Vector_base<TestClass, std::allocator<TestClass>>::_Vector_impl.67" = type { %class.TestClass*, %class.TestClass*, %class.TestClass* }
%class.TestClass = type {i64, i64, i64}
%class.TestClass2 = type {i64, i64, i64}

define internal %class.TestClass2* @loadData(%"class.std::vector.12"* nocapture %arg) {
bb:
  %tmp = getelementptr inbounds %"class.std::vector.12", %"class.std::vector.12"* %arg, i64 0, i32 0
  %tmp2 = bitcast %"struct.std::_Vector_base.13"* %tmp to i64*
  %tmp3 = load i64, i64* %tmp2
  %tmp4 = inttoptr i64 %tmp3 to %class.TestClass2*
  ret %class.TestClass2* %tmp4
}