; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

; RUN: opt  < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s --check-prefix=CHECKSTORE
; RUN: opt  < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s --check-prefix=CHECKZERO

; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s --check-prefix=CHECKSTORE
; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s --check-prefix=CHECKZERO

; This test checks that casting from outer structure, loading the zero
; element and storing the data into a pointer of the same type produces
; an unsafe pointer store for related types.

; Check zero element access
; CHECKZERO-LABEL: LLVMType: %"class.std::vector.12" = type { %"struct.std::_Vector_base.13" }
; CHECKZERO: Safety data:{{.*}}Mismatched element access (related types){{.*}}

; CHECKZERO-LABEL: LLVMType: %"struct.std::_Vector_base.13"
; CHECKZERO: Safety data:{{.*}}Mismatched element access (related types){{.*}}

; CHECKZERO-LABEL: LLVMType: %"struct.std::_Vector_base<TestClass, std::allocator<TestClass>>::_Vector_impl.67"
; CHECKZERO: Safety data:{{.*}}Mismatched element access (related types){{.*}}

; Check unsafe pointer store
; CHECKSTORE-LABEL: LLVMType: %"class.std::vector.12" = type { %"struct.std::_Vector_base.13" }
; CHECKSTORE: Safety data:{{.*}}Unsafe pointer store (related types){{.*}}

; CHECKSTORE-LABEL: LLVMType: %"struct.std::_Vector_base.13"
; CHECKSTORE: Safety data:{{.*}}Unsafe pointer store (related types){{.*}}

; CHECKSTORE-LABEL: LLVMType: %"struct.std::_Vector_base<TestClass, std::allocator<TestClass>>::_Vector_impl.67"
; CHECKSTORE: Safety data:{{.*}}Unsafe pointer store (related types){{.*}}

; CHECKSTORE-LABEL: LLVMType: %class.TestClass = type { i64, i64, i64 }
; CHECKSTORE: Safety data:{{.*}}Unsafe pointer store (related types){{.*}}

%"class.std::vector.12" = type { %"struct.std::_Vector_base.13" }
%"struct.std::_Vector_base.13" = type { %"struct.std::_Vector_base<TestClass, std::allocator<TestClass>>::_Vector_impl.67" }
%"struct.std::_Vector_base<TestClass, std::allocator<TestClass>>::_Vector_impl.67" = type { %class.TestClass*, %class.TestClass*, %class.TestClass* }
%class.TestClass = type {i64, i64, i64}

define internal void @storeData(%"class.std::vector.12"* nocapture %arg, %"class.std::vector.12"* nocapture %arg2) {
bb:
  %tmp = getelementptr inbounds %"class.std::vector.12", %"class.std::vector.12"* %arg, i64 0, i32 0
  %tmp2 = bitcast %"struct.std::_Vector_base.13"* %tmp to i64*

  %tmp3 = getelementptr inbounds %"class.std::vector.12", %"class.std::vector.12"* %arg2, i64 0, i32 0
  %tmp4 = bitcast %"struct.std::_Vector_base.13"* %tmp3 to i64*
  %tmp5 = load i64, i64* %tmp4

  store i64 %tmp5, i64* %tmp2
  ret void
}