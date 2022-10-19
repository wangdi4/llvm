; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test that storing a pointer to a structure into a member of an array of
; pointers does not set the "Field address taken" safety bit on either the
; structure or the array.

%struct.test01arc = type { i64, i64 }
%struct.test01basket = type { %struct.test01arc*, i64 }
@var01 = internal global %struct.test01basket*** null, !intel_dtrans_type !3
define void @test01() {
  %basket_ptrs = alloca [4061 x %struct.test01basket*], !intel_dtrans_type !4
  %first = getelementptr inbounds [4061 x %struct.test01basket*], [4061 x %struct.test01basket*]* %basket_ptrs, i64 0, i64 0
  %second = getelementptr inbounds %struct.test01basket*, %struct.test01basket** %first, i64 1
  %glob = load %struct.test01basket***, %struct.test01basket**** @var01
  store %struct.test01basket** %second, %struct.test01basket*** %glob
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01arc
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test01arc

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01basket
; CHECK: Safety data: Global pointer | Local pointer{{ *$}}
; CHECK: End LLVMType: %struct.test01basket

; CHECK-LABEL: DTRANS_ArrayInfo:
; CHECK: Number of elements: 4061
; CHECK: Element DTrans Type: %struct.test01basket*
; CHECK: Safety data: No issues found


!1 = !{i64 0, i32 0}  ; i64
!2 = !{%struct.test01arc zeroinitializer, i32 1}  ; %struct.test01arc*
!3 = !{%struct.test01basket zeroinitializer, i32 3}  ; %struct.test01basket***
!4 = !{!"A", i32 4061, !5}  ; [4061 x %struct.test01basket*]
!5 = !{%struct.test01basket zeroinitializer, i32 1}  ; %struct.test01basket*
!6 = !{!"S", %struct.test01arc zeroinitializer, i32 2, !1, !1} ; { i64, i64 }
!7 = !{!"S", %struct.test01basket zeroinitializer, i32 2, !2, !1} ; { %struct.test01arc*, i64 }

!intel.dtrans.types = !{!6, !7}
