; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

; RUN: opt  < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; This test checks that the integer-to-pointer instruction matches the zero
; element loaded from the GEP.

; Check Bad casting
; CHECK-LABEL: LLVMType: %class.OuterClass
; CHECK-NOT: Safety data:{{.*}}Bad casting{{.*}}

%class.OuterClass = type {%class.TestClass, %"class.std::vector.12"}
%"class.std::vector.12" = type { %"struct.std::_Vector_base.13" }
%"struct.std::_Vector_base.13" = type { %"struct.std::_Vector_base<TestClass, std::allocator<TestClass>>::_Vector_impl.67" }
%"struct.std::_Vector_base<TestClass, std::allocator<TestClass>>::_Vector_impl.67" = type { %class.OuterClass*, %class.OuterClass*, %class.OuterClass* }
%class.TestClass = type {i64, i64, i64}

define internal %class.OuterClass* @loadData(%class.OuterClass* nocapture %arg) {
bb:
  %tmp = getelementptr inbounds %class.OuterClass, %class.OuterClass* %arg, i64 0, i32 1
  %tmp2 = bitcast %"class.std::vector.12"* %tmp to i64*
  %tmp3 = load i64, i64* %tmp2
  %tmp4 = inttoptr i64 %tmp3 to %class.OuterClass*
  ret %class.OuterClass* %tmp4
}
