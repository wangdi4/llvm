; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test that storing a pointer to a structure into a member of an array of
; pointers does not set the "Field address taken" safety bit on either the
; structure or the array.

%struct.test01arc = type { i64, i64 }
%struct.test01basket = type { %struct.test01arc*, i64 }
@var01 = internal global %struct.test01basket*** null, !dtrans_type !4
define void @test01() {
  %basket_ptrs = alloca [4061 x %struct.test01basket*], !dtrans_type !6
  %first = getelementptr inbounds [4061 x %struct.test01basket*], [4061 x %struct.test01basket*]* %basket_ptrs, i64 0, i64 0
  %second = getelementptr inbounds %struct.test01basket*, %struct.test01basket** %first, i64 1
  %glob = load %struct.test01basket***, %struct.test01basket**** @var01
  store %struct.test01basket** %second, %struct.test01basket*** %glob
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01arc
; CHECK: Safety data: No issues found

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01basket
; CHECK: Safety data: Global pointer | Local pointer{{ *$}}

; CHECK-LABEL: DTRANS_ArrayInfo:
; CHECK: Number of elements: 4061
; CHECK: Element DTrans Type: %struct.test01basket*
; CHECK: Safety data: No issues found


!1 = !{i64 0, i32 0}  ; i64
!2 = !{!3, i32 1}  ; %struct.test01arc*
!3 = !{!"R", %struct.test01arc zeroinitializer, i32 0}  ; %struct.test01arc
!4 = !{!5, i32 3}  ; %struct.test01basket***
!5 = !{!"R", %struct.test01basket zeroinitializer, i32 0}  ; %struct.test01basket
!6 = !{!"A", i32 4061, !7}  ; [4061 x %struct.test01basket*]
!7 = !{!5, i32 1}  ; %struct.test01basket*
!8 = !{!"S", %struct.test01arc zeroinitializer, i32 2, !1, !1} ; { i64, i64 }
!9 = !{!"S", %struct.test01basket zeroinitializer, i32 2, !2, !1} ; { %struct.test01arc*, i64 }

!dtrans_types = !{!8, !9}
