; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

; RUN: opt  < %s -whole-program-assume -dtransanalysis -debug-only=dtrans-lpa -disable-output 2>&1 | FileCheck %s

; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -debug-only=dtrans-lpa -disable-output 2>&1 | FileCheck %s

; This test checks that the local pointer analysis sets the aliases and
; memory accesses for the 0 element correctly. This is the case when the
; source is an Argument.

; Check that the aliases and pointees are set correctly for the 0 element
; CHECK-LABEL: analyzeValue   %tmp6 = bitcast %"class.std::vector.12"* %arg to i64*
; CHECK: Aliased types:
; CHECK: %"class.std::vector.12"*
; CHECK: Element pointees:
; CHECK: %struct.std::_Vector_base<TestClass, std::allocator<TestClass>>::_Vector_impl.67 @ 0

; CHECK-LABEL: analyzeValue   %tmp7 = load i64, i64* %tmp6
; CHECK:   Aliased types:
; CHECK:    %class.TestClass*
; CHECK:  No element pointees.

%"class.std::vector.12" = type { %"struct.std::_Vector_base.13" }
%"struct.std::_Vector_base.13" = type { %"struct.std::_Vector_base<TestClass, std::allocator<TestClass>>::_Vector_impl.67" }
%"struct.std::_Vector_base<TestClass, std::allocator<TestClass>>::_Vector_impl.67" = type { %class.TestClass*, %class.TestClass*, %class.TestClass* }
%class.TestClass = type {i64, i64, i64}

define internal %class.TestClass* @make_new(%"class.std::vector.12"* nocapture %arg) {
bb:
  %tmp = getelementptr %"class.std::vector.12", %"class.std::vector.12"* %arg, i64 0, i32 0
  %tmp1 = getelementptr inbounds %"struct.std::_Vector_base.13", %"struct.std::_Vector_base.13"* %tmp, i64 0, i32 0
  %tmp2 = getelementptr inbounds %"struct.std::_Vector_base<TestClass, std::allocator<TestClass>>::_Vector_impl.67", %"struct.std::_Vector_base<TestClass, std::allocator<TestClass>>::_Vector_impl.67"* %tmp1, i64 0, i32 1
  %tmp3 = load %class.TestClass*, %class.TestClass** %tmp2
  br label %bb4

bb4:                                              ; preds = %bb
  %tmp5 = ptrtoint %class.TestClass* %tmp3 to i64
  %tmp6 = bitcast %"class.std::vector.12"* %arg to i64*
  %tmp7 = load i64, i64* %tmp6
  %tmp8 = sub i64 %tmp5, %tmp7
  %tmp9 = sdiv exact i64 %tmp8, 24
  br label %bb10

bb10:                                             ; preds = %bb4
  %tmp11 = tail call noalias nonnull i8* @_Znwm(i64 %tmp8)
  %tmp12 = bitcast i8* %tmp11 to %class.TestClass*
  ret %class.TestClass* %tmp12
}

declare noalias i8* @_Znwm(i64)

