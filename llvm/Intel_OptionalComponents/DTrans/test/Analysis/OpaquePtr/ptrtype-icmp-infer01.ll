; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s  --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s  --check-prefix=CHECK --check-prefix=CHECK-OPAQUE

; Test pointer type inference based on use of pointers in the 'icmp' instruction.

; Lines marked with CHECK-NONOPAQUE are tests for the current form of IR.
; Lines marked with CHECK-OPAQUE are placeholders for check lines that will
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
define void @test01(%struct.test01* "intel_dtrans_func_index"="1" %arg1001) !intel.dtrans.func.type !6 {
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
define void @helper_test01(%struct.test01impl* "intel_dtrans_func_index"="1" %in1, i8* "intel_dtrans_func_index"="2" %in2) !intel.dtrans.func.type !9 {
  ret void
}
; CHECK-LABEL:  Input Parameters: test01
; CHECK-NONOPAQUE:    Arg 0: %struct.test01* %arg1001
; CHECK-OPAQUE:    Arg 0: ptr %arg1001
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   %struct.test01*{{ *$}}
; CHECK-NEXT: No element pointees.

; CHECK-NONOPAQUE:  %tmp1008 = bitcast %struct.test01* %arg1001 to void (%struct.test01*, i8*)***
; CHECK-OPAQUE:  %tmp1008 = bitcast ptr %arg1001 to ptr
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   %struct.test01*{{ *$}}
; CHECK-NEXT:   i32 (...)***{{ *$}}
; CHECK-NEXT:   void (%struct.test01impl*, i8*)***{{ *$}}
; CHECK-NEXT: Element pointees:
; CHECK-NEXT:   %struct.test01 @ 0

; CHECK-NONOPAQUE:  %tmp1009 = load void (%struct.test01*, i8*)**, void (%struct.test01*, i8*)*** %tmp1008
; CHECK-OPAQUE:  %tmp1009 = load ptr, ptr %tmp1008
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   i32 (...)**{{ *$}}
; CHECK-NEXT:   void (%struct.test01impl*, i8*)**{{ *$}}
; CHECK-NEXT: No element pointees.

; CHECK-NONOPAQUE:  %tmp1010 = getelementptr inbounds void (%struct.test01*, i8*)*, void (%struct.test01*, i8*)** %tmp1009, i64 3
; CHECK-OPAQUE:  %tmp1010 = getelementptr inbounds ptr, ptr %tmp1009, i64 3
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   i32 (...)**{{ *$}}
; CHECK-NEXT:   void (%struct.test01impl*, i8*)**{{ *$}}
; CHECK-NEXT: No element pointees.

; CHECK-NONOPAQUE:  %tmp1011 = load void (%struct.test01*, i8*)*, void (%struct.test01*, i8*)** %tmp1010
; CHECK-OPAQUE:  %tmp1011 = load ptr, ptr %tmp1010
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   i32 (...)*{{ *$}}
; CHECK-NEXT:   void (%struct.test01impl*, i8*)*{{ *$}}
; CHECK-NEXT: No element pointees.

; CHECK-NONOPAQUE:  %tmp1012 = bitcast void (%struct.test01*, i8*)* %tmp1011 to i8*
; CHECK-OPAQUE:  %tmp1012 = bitcast ptr %tmp1011 to ptr
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   i32 (...)*{{ *$}}
; CHECK-NEXT:   void (%struct.test01impl*, i8*)*{{ *$}}
; CHECK-NEXT: No element pointees.

; CHECK-NONOPAQUE:  %tmp1013 = bitcast void (%struct.test01impl*, i8*)* @helper_test01 to i8*
; CHECK-OPAQUE:  %tmp1013 = bitcast ptr @helper_test01 to ptr
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   i32 (...)*{{ *$}}
; CHECK-NEXT:   void (%struct.test01impl*, i8*)*{{ *$}}
; CHECK-NEXT: No element pointees.


!1 = !{%struct.test01 zeroinitializer, i32 0}  ; %struct.test01
!2 = !{!"F", i1 true, i32 0, !3}  ; i32 (...)
!3 = !{i32 0, i32 0}  ; i32
!4 = !{!2, i32 2}  ; i32 (...)**
!5 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!6 = distinct !{!5}
!7 = !{%struct.test01impl zeroinitializer, i32 1}  ; %struct.test01impl*
!8 = !{i8 0, i32 1}  ; i8*
!9 = distinct !{!7, !8}
!10 = !{!"S", %struct.test01impl zeroinitializer, i32 1, !1} ; { %struct.test01 }
!11 = !{!"S", %struct.test01 zeroinitializer, i32 1, !4} ; { i32 (...)** }

!intel.dtrans.types = !{!10, !11}
