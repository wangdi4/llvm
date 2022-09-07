; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

; RUN: opt  < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s --check-prefix=CHECKSTORE
; RUN: opt  < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s --check-prefix=CHECKZERO

; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s --check-prefix=CHECKSTORE
; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s --check-prefix=CHECKZERO

; This test checks that storing 0 into a pointer that is cast to a zero element
; won't produces an unsafe pointer store. Also, it checks if mismatched
; element access for related types is set.

; Check zero element access
; CHECKZERO-LABEL: LLVMType: %"class.std::vector.12" = type { %"struct.std::_Vector_base.13" }
; CHECKZERO: Safety data:{{.*}}Mismatched element access (related types){{.*}}

; CHECKZERO-LABEL: LLVMType: %"struct.std::_Vector_base.13"
; CHECKZERO: Safety data:{{.*}}Mismatched element access (related types){{.*}}

; CHECKZERO-LABEL: LLVMType: %"struct.std::_Vector_base<TestClass, std::allocator<TestClass>>::_Vector_impl.67"
; CHECKZERO: Safety data:{{.*}}Mismatched element access (related types){{.*}}

; Check unsafe pointer store
; CHECKSTORE-LABEL: LLVMType: %"class.std::vector.12" = type { %"struct.std::_Vector_base.13" }
; CHECKSTORE-NOT: Safety data:{{.*}}Unsafe pointer store{{.*}}

; CHECKSTORE-LABEL: LLVMType: %"struct.std::_Vector_base.13"
; CHECKSTORE-NOT: Safety data:{{.*}}Unsafe pointer store{{.*}}

; CHECKSTORE-LABEL: LLVMType: %"struct.std::_Vector_base<TestClass, std::allocator<TestClass>>::_Vector_impl.67"
; CHECKSTORE-NOT: Safety data:{{.*}}Unsafe pointer store{{.*}}

%"class.std::vector.12" = type { %"struct.std::_Vector_base.13" }
%"struct.std::_Vector_base.13" = type { %"struct.std::_Vector_base<TestClass, std::allocator<TestClass>>::_Vector_impl.67" }
%"struct.std::_Vector_base<TestClass, std::allocator<TestClass>>::_Vector_impl.67" = type { %class.TestClass*, %class.TestClass*, %class.TestClass* }
%class.TestClass = type {i64, i64, i64}

define internal void @clearData(%"class.std::vector.12"* nocapture %arg) {
bb:
  %tmp = getelementptr inbounds %"class.std::vector.12", %"class.std::vector.12"* %arg, i64 0, i32 0
  %tmp2 = bitcast %"struct.std::_Vector_base.13"* %tmp to i64*

  store i64 0, i64* %tmp2
  ret void
}