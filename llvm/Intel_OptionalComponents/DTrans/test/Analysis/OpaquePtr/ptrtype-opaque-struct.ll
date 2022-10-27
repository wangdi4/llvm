; REQUIRES: asserts
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; This test case checks that analyzing structure %struct.test01 (opaque
; structure) won't produce a segmentation fault.

; CHECK:   Input Parameters: foo
; CHECK:     Arg 0: ptr %arg
; CHECK:     LocalPointerInfo: CompletelyAnalyzed
; CHECK:       Aliased types:
; CHECK:         %struct.test01*
; CHECK:       No element pointees.
; CHECK:       DomTy: %struct.test01*
; CHECK: define internal void @foo(ptr "intel_dtrans_func_index"="1" %arg) !intel.dtrans.func.type !6 {

; CHECK:   %tmp0 = bitcast ptr %arg to ptr
; CHECK:     LocalPointerInfo: CompletelyAnalyzed
; CHECK:       Aliased types:
; CHECK:         %struct.test01*
; CHECK:         %struct.test02**
; CHECK:       No element pointees.
; CHECK:       Ambiguous Dominant Type

; CHECK:   %tmp1 = load ptr, ptr %tmp0, align 8
; CHECK:     LocalPointerInfo: CompletelyAnalyzed
; CHECK:       Aliased types:
; CHECK:         %struct.test01 = type opaque
; CHECK:         %struct.test02*
; CHECK:       No element pointees.
; CHECK:       Ambiguous Dominant Type

; CHECK:   %tmp2 = bitcast ptr %arg to ptr
; CHECK:     LocalPointerInfo: CompletelyAnalyzed
; CHECK:       Aliased types:
; CHECK:         %struct.test01*
; CHECK:         %struct.test03*
; CHECK:       No element pointees.
; CHECK:       Ambiguous Dominant Type

; CHECK:   %tmp4 = call ptr @bar(ptr %tmp2)
; CHECK:     LocalPointerInfo: CompletelyAnalyzed
; CHECK:       Aliased types:
; CHECK:         %struct.test02*
; CHECK:       No element pointees.
; CHECK:       DomTy: %struct.test02*

; CHECK:   %tmp5 = phi ptr [ %tmp1, %entry ], [ %tmp4, %bb1 ]
; CHECK:     LocalPointerInfo: CompletelyAnalyzed
; CHECK:       Aliased types:
; CHECK:         %struct.test01 = type opaque
; CHECK:         %struct.test02*
; CHECK:       No element pointees.
; CHECK:       Ambiguous Dominant Type

; CHECK:   %tmp6 = getelementptr inbounds %struct.test02, ptr %tmp5, i64 0, i32 1
; CHECK:     LocalPointerInfo: CompletelyAnalyzed
; CHECK:       Aliased types:
; CHECK:         %struct.test03*
; CHECK:       Element pointees:
; CHECK:         %struct.test02 @ 1
; CHECK:       DomTy: %struct.test03*

%struct.test01 = type opaque
%struct.test02 = type { %struct.test02*, %struct.test03 }
%struct.test03 = type { i32 }

define internal void @foo(%struct.test01* "intel_dtrans_func_index"="1" %arg) !intel.dtrans.func.type !8 {
entry:
  %tmp0 = bitcast %struct.test01* %arg to %struct.test02**
  %tmp1 = load %struct.test02*, %struct.test02** %tmp0
  %tmp2 = bitcast %struct.test01* %arg to %struct.test03*
  %tmp3 = icmp eq %struct.test02* %tmp1, null
  br i1 %tmp3, label %bb1, label %bb2

bb1:
  %tmp4 = call %struct.test02* @bar(%struct.test03* %tmp2)
  br label %bb2

bb2:
  %tmp5 = phi %struct.test02* [%tmp1, %entry], [%tmp4, %bb1]
  %tmp6 = getelementptr inbounds %struct.test02, %struct.test02* %tmp5, i64 0, i32 1

  ret void
}

declare !intel.dtrans.func.type !11 "intel_dtrans_func_index"="1"  %struct.test02* @bar(%struct.test03* "intel_dtrans_func_index"="2" %arg)

!intel.dtrans.types = !{!1, !2, !5}

!1 = !{!"S", %struct.test01 zeroinitializer, i32 -1} ; type opaque
!2 = !{!"S", %struct.test02 zeroinitializer, i32 2, !3, !4}
!3 = !{%struct.test02 zeroinitializer, i32 1}
!4 = !{%struct.test03 zeroinitializer, i32 0}
!5 = !{!"S", %struct.test03 zeroinitializer, i32 1, !6}
!6 = !{i32 0, i32 0}
!7 = !{%struct.test01 zeroinitializer, i32 1}
!8 = distinct !{!7}
!9 = !{%struct.test02 zeroinitializer, i32 1}
!10 = !{%struct.test03 zeroinitializer, i32 1}
!11 = distinct !{!9, !10}
