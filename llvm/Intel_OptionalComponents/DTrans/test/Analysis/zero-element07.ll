; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

; RUN: opt  < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s --check-prefix=CHECKBC
; RUN: opt  < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s --check-prefix=CHECKZERO

; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s --check-prefix=CHECKBC
; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s --check-prefix=CHECKZERO

; This test checks that casting from outer structure, loading and then casting
; to the inner-most structure won't produce a bad casting. Also, this test checks
; that the safety data for mismatched element access for related types is added.

; Check Bad casting
; CHECKBC-LABEL:  LLVMType: %class.TestClass = type { i64, i64, i64 }
; CHECKBC-NOT: Safety data:{{.*}}Bad casting{{.*}}

; Check zero element access
; CHECKZERO-LABEL: LLVMType: %"class.std::vector.12" = type { %"struct.std::_Vector_base.13" }
; CHECKZERO: Safety data:{{.*}}Mismatched element access (related types){{.*}}

; CHECKZERO-LABEL: LLVMType: %"struct.std::_Vector_base.13"
; CHECKZERO: Safety data:{{.*}}Mismatched element access (related types){{.*}}

; CHECKZERO-LABEL: LLVMType: %"struct.std::_Vector_base<TestClass, std::allocator<TestClass>>::_Vector_impl.67"
; CHECKZERO: Safety data:{{.*}}Mismatched element access (related types){{.*}}


%"class.std::vector.12" = type { %"struct.std::_Vector_base.13" }
%"struct.std::_Vector_base.13" = type { %"struct.std::_Vector_base<TestClass, std::allocator<TestClass>>::_Vector_impl.67" }
%"struct.std::_Vector_base<TestClass, std::allocator<TestClass>>::_Vector_impl.67" = type { %class.TestClass*, %class.TestClass*, %class.TestClass* }
%class.TestClass = type {i64, i64, i64}

define internal %class.TestClass* @loadData(%"class.std::vector.12"* nocapture %arg) {
bb:
  %tmp = getelementptr inbounds %"class.std::vector.12", %"class.std::vector.12"* %arg, i64 0, i32 0
  %tmp2 = bitcast %"struct.std::_Vector_base.13"* %tmp to i64*
  %tmp3 = load i64, i64* %tmp2
  %tmp4 = inttoptr i64 %tmp3 to %class.TestClass*
  ret %class.TestClass* %tmp4
}