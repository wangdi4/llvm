; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

; RUN: opt  < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; This test checks that mismatched element access is in the types since
; %"struct.std::_Vector_base.13", which is loaded from %"class.std::vector.12",
; is cast to %"struct.std::_Vector_base.13.2", and then is casted to i64*
; to collect the zero element.

; Check mismatched element access
; CHECK-LABEL: LLVMType: %"class.std::vector.12" = type { %"struct.std::_Vector_base.13" }
; CHECK: Safety data:{{.*}}Mismatched element access{{.*}}

; CHECK-LABEL: LLVMType: %"struct.std::_Vector_base.13"
; CHECK: Safety data:{{.*}}Mismatched element access{{.*}}

; CHECK-LABEL: LLVMType: %"struct.std::_Vector_base<TestClass, std::allocator<TestClass>>::_Vector_impl.67"
; CHECK: Safety data:{{.*}}Mismatched element access{{.*}}

%"class.std::vector.12" = type { %"struct.std::_Vector_base.13" }
%"struct.std::_Vector_base.13" = type { %"struct.std::_Vector_base<TestClass, std::allocator<TestClass>>::_Vector_impl.67" }
%"struct.std::_Vector_base<TestClass, std::allocator<TestClass>>::_Vector_impl.67" = type { %class.TestClass*, %class.TestClass*, %class.TestClass* }
%class.TestClass = type {i64, i64, i64}
%"struct.std::_Vector_base.13.2" = type { %"struct.std::_Vector_base<TestClass, std::allocator<TestClass>>::_Vector_impl.67" }

define internal %class.TestClass* @loadData(%"class.std::vector.12"* nocapture %arg) {
bb:
  %tmp = getelementptr inbounds %"class.std::vector.12", %"class.std::vector.12"* %arg, i64 0, i32 0
  %tmp2 = bitcast %"struct.std::_Vector_base.13"* %tmp to %"struct.std::_Vector_base.13.2"*
  %tmp3 = bitcast %"struct.std::_Vector_base.13.2"* %tmp2 to i64*
  %tmp4 = load i64, i64* %tmp3
  %tmp5 = inttoptr i64 %tmp4 to %class.TestClass*
  ret %class.TestClass* %tmp5
}