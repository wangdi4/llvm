; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

; RUN: opt  < %s -whole-program-assume -dtransanalysis -debug-only=dtrans-lpa -disable-output 2>&1 | FileCheck %s

; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -debug-only=dtrans-lpa -disable-output 2>&1 | FileCheck %s

; This test checks that the local pointer analysis sets the aliases and
; memory accesses for the 0 element correctly. This is the case when the
; source is a GEP.

; CHECK-LABEL: analyzeValue   %tmp4 = bitcast %"struct.std::_Vector_base.13"* %tmp to i64*
; CHECK:  Aliased types:
; CHECK:    %"struct.std::_Vector_base.13"*
; CHECK:  Element pointees:
; CHECK:    %struct.std::_Vector_base<TestClass, std::allocator<TestClass>>::_Vector_impl.67 @ 0

; CHECK-LABEL: analyzeValue   %tmp5 = load i64, i64* %tmp4
; CHECK:  Aliased types:
; CHECK:    %class.TestClass*
; CHECK:  No element pointees.

%"class.std::vector.12" = type { %"struct.std::_Vector_base.13" }
%"struct.std::_Vector_base.13" = type { %"struct.std::_Vector_base<TestClass, std::allocator<TestClass>>::_Vector_impl.67" }
%"struct.std::_Vector_base<TestClass, std::allocator<TestClass>>::_Vector_impl.67" = type { %class.TestClass*, %class.TestClass*, %class.TestClass* }
%class.TestClass = type {i64, i64, i64}

define internal i64 @isSize24(%"class.std::vector.12"* nocapture %arg) {
bb:
  %tmp = getelementptr inbounds %"class.std::vector.12", %"class.std::vector.12"* %arg, i64 0, i32 0
  %tmp1 = getelementptr inbounds %"class.std::vector.12", %"class.std::vector.12"* %arg, i64 0, i32 0, i32 0, i32 1
  %tmp2 = bitcast %class.TestClass** %tmp1 to i64*
  %tmp3 = load i64, i64* %tmp2
  %tmp4 = bitcast %"struct.std::_Vector_base.13"* %tmp to i64*
  %tmp5 = load i64, i64* %tmp4
  %tmp6 = sub i64 %tmp3, %tmp5
  %tmp7 = sdiv exact i64 %tmp6, 24
  ret i64 %tmp7
}