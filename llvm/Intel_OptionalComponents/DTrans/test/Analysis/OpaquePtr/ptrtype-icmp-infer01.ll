; REQUIRES: asserts

; RUN: opt -disable-output -whole-program-assume -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s  --check-prefix=CHECK --check-prefix=CHECK-CUR
; RUN: opt -disable-output -whole-program-assume -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s  --check-prefix=CHECK --check-prefix=CHECK-CUR

; Test pointer type inference based on use of pointers in the 'icmp' instruction.

; Lines marked with CHECK-CUR are tests for the current form of IR.
; Lines marked with CHECK-FUT are placeholders for check lines that will
;   changed when the future opaque pointer form of IR is used.
; Lines marked with CHECK should remain the same when changing to use opaque
;   pointers.

; In this case, types must be inferred for multiple values simultaneously
; where they join together at an icmp instruction involving pointers,
; and with multiple levels of indirection from the load instructions.
;
; Note: Even though there are bitcasts of the pointer types to i8*, these
; types will not show up in the pointer analyzer results in this case
; because the value is never used in a way that would reveal it as being i8*,
; such as via a memory operation/function call.
%struct.test01impl = type { %struct.test01 }
%struct.test01 = type { i32 (...)** }
define void @test01(%struct.test01* %arg1001) !dtrans_type !7 {
bb:
  br i1 undef, label %bb1020, label %bb1005

bb1005:
  %tmp1006 = icmp eq %struct.test01* %arg1001, null
  br i1 %tmp1006, label %bb1020, label %bb1007

bb1007:
  %tmp1008 = bitcast %struct.test01* %arg1001 to void (%struct.test01*, i8*)***
  %tmp1009 = load void (%struct.test01*, i8*)**, void (%struct.test01*, i8*)*** %tmp1008
  %tmp1010 = getelementptr inbounds void (%struct.test01*, i8*)*, void (%struct.test01*, i8*)** %tmp1009, i64 3
  %tmp1011 = load void (%struct.test01*, i8*)*, void (%struct.test01*, i8*)** %tmp1010
  %tmp1012 = bitcast void (%struct.test01*, i8*)* %tmp1011 to i8*
  %tmp1013 = bitcast void (%struct.test01impl*, i8*)* @helper_test01 to i8*
  %tmp1014 = icmp eq i8* %tmp1012, %tmp1013
  br label %bb1020

bb1020:
  ret void
}
define void @helper_test01(%struct.test01impl* %in1, i8* %in2) !dtrans_type !1 {
  ret void
}
; CHECK-LABEL:  Input Parameters: test01
; CHECK-CUR:    Arg 0: %struct.test01* %arg1001
; CHECK-FUT:    Arg 0: p0 %arg1001
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   %struct.test01*{{ *$}}
; CHECK-NEXT: No element pointees.

; CHECK-CUR:  %tmp1008 = bitcast %struct.test01* %arg1001 to void (%struct.test01*, i8*)***
; CHECK-FUT:  %tmp1008 = bitcast p0 %arg1001 to p0
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   %struct.test01*{{ *$}}
; CHECK-NEXT:   void (%struct.test01impl*, i8*)***{{ *$}}
; CHECK-NEXT: No element pointees.

; CHECK-CUR:  %tmp1009 = load void (%struct.test01*, i8*)**, void (%struct.test01*, i8*)*** %tmp1008
; CHECK-FUT:  %tmp1009 = load p0, p0 %tmp1008
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   void (%struct.test01impl*, i8*)**{{ *$}}
; CHECK-NEXT: No element pointees.

; CHECK-CUR:  %tmp1010 = getelementptr inbounds void (%struct.test01*, i8*)*, void (%struct.test01*, i8*)** %tmp1009, i64 3
; CHECK-FUT:  %tmp1010 = getelementptr inbounds p0, p0 %tmp1009, i64 3
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   void (%struct.test01impl*, i8*)**{{ *$}}
; CHECK-NEXT: No element pointees.

; CHECK-CUR:  %tmp1011 = load void (%struct.test01*, i8*)*, void (%struct.test01*, i8*)** %tmp1010
; CHECK-FUT:  %tmp1011 = load p0, p0 %tmp1010
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   void (%struct.test01impl*, i8*)*{{ *$}}
; CHECK-NEXT: No element pointees.

; CHECK-CUR:  %tmp1012 = bitcast void (%struct.test01*, i8*)* %tmp1011 to i8*
; CHECK-FUT:  %tmp1012 = bitcast p0 %tmp1011 to p0
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   void (%struct.test01impl*, i8*)*{{ *$}}
; CHECK-NEXT: No element pointees.

; CHECK-CUR:  %tmp1013 = bitcast void (%struct.test01impl*, i8*)* @helper_test01 to i8*
; CHECK-FUT:  %tmp1013 = bitcast p0 @helper_test01 to p0
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   void (%struct.test01impl*, i8*)*{{ *$}}
; CHECK-NEXT: No element pointees.


!1 = !{!"F", i1 false, i32 2, !2, !3, !5}  ; void (%struct.test01impl*, i8*)
!2 = !{!"void", i32 0}  ; void
!3 = !{!4, i32 1}  ; %struct.test01impl*
!4 = !{!"R", %struct.test01impl zeroinitializer, i32 0}  ; %struct.test01impl
!5 = !{i8 0, i32 1}  ; i8*
!6 = !{i8 0, i32 0}  ; i8
!7 = !{!"F", i1 false, i32 1, !2, !8}  ; void (%struct.test01*)
!8 = !{!9, i32 1}  ; %struct.test01*
!9 = !{!"R", %struct.test01 zeroinitializer, i32 0}  ; %struct.test01
!10 = !{!"F", i1 true, i32 0, !11}  ; i32 (...)
!11 = !{i32 0, i32 0}  ; i32
!12 = !{!10, i32 2}  ; i32 (...)**
!13 = !{!"S", %struct.test01impl zeroinitializer, i32 1, !9} ; { %struct.test01 }
!14 = !{!"S", %struct.test01 zeroinitializer, i32 1, !12} ; { i32 (...)** }

!dtrans_types = !{!13, !14}
