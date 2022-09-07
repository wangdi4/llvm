; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

; RUN: opt  < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; This test checks that casting unsafe pointer store for related types won't
; be added into the types since a constant different than 0 is being stored
; in the pointer.

; CHECK: LLVMType: %"class.std::vector.12" = type { %"struct.std::_Vector_base.13" }
; CHECK-NOT: Safety data:{{.*}}Unsafe pointer store (related types){{.*}}

%"class.std::vector.12" = type { %"struct.std::_Vector_base.13" }
%"struct.std::_Vector_base.13" = type { %"struct.std::_Vector_base<TestClass, std::allocator<TestClass>>::_Vector_impl.67" }
%"struct.std::_Vector_base<TestClass, std::allocator<TestClass>>::_Vector_impl.67" = type { %class.TestClass*, %class.TestClass*, %class.TestClass* }
%class.TestClass = type {i64, i64, i64}

define internal void @badData(%"class.std::vector.12"* nocapture %arg) {
bb:
  %tmp = getelementptr inbounds %"class.std::vector.12", %"class.std::vector.12"* %arg, i64 0, i32 0
  %tmp2 = bitcast %"struct.std::_Vector_base.13"* %tmp to i64*

  store i64 1, i64* %tmp2
  ret void
}