; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

; RUN: opt  < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; This test checks that bad casting for related types is set for the types
; when the data is being moved to a destination with the same type as the
; source.

; CHECK: Safety data: {{.*}}Bad casting (related types){{.*}}

%"class.std::vector.12" = type { %"struct.std::_Vector_base.13" }
%"struct.std::_Vector_base.13" = type { %"struct.std::_Vector_base<TestClass, std::allocator<TestClass>>::_Vector_impl.67" }
%"struct.std::_Vector_base<TestClass, std::allocator<TestClass>>::_Vector_impl.67" = type { %class.TestClass*, %class.TestClass*, %class.TestClass* }
%class.TestClass = type {i64, i64, i64}

define internal void @copy(%"class.std::vector.12"* nocapture %arg, %"class.std::vector.12"* nocapture %arg1) {
bb:
  %tmp = getelementptr inbounds %"class.std::vector.12", %"class.std::vector.12"* %arg, i64 0, i32 0
  %tmp1 = bitcast %"struct.std::_Vector_base.13"* %tmp to i64*

  %tmp2 = getelementptr inbounds %"class.std::vector.12", %"class.std::vector.12"* %arg1, i64 0, i32 0
  %tmp3 = bitcast %"struct.std::_Vector_base.13"* %tmp2 to i64*
  %tmp4 = load i64, i64* %tmp3

  store i64 %tmp4, i64* %tmp1
  ret void
}
