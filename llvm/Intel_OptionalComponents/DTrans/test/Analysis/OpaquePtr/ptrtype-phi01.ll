; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s

; Test pointer type recovery for "phi" instructions.


; Test simple phi instructions without circular dependencies.
%struct.test01 = type { i64, i64, i64 }
define internal void @test01(i64 %in, ptr "intel_dtrans_func_index"="1" %struct) !intel.dtrans.func.type !3 {
entry:
  %cmp = icmp eq i64 %in, 0
  br i1 %cmp, label %zlabel, label %nzlabel

zlabel:
  %zf0 = getelementptr %struct.test01, ptr %struct, i64 0, i32 0
  br label %merge

nzlabel:
  %nzf0 = getelementptr %struct.test01, ptr %struct, i64 0, i32 0
  %nzf1 = getelementptr %struct.test01, ptr %struct, i64 0, i32 1
  br label %merge

merge:
  ; Test phi that has same aliases for each input
  %ptr1 = phi ptr [ %zf0, %zlabel ], [ %nzf0, %nzlabel ]

  ; Test phi with different element pointees for each input
  %ptr2 = phi ptr [ %zf0, %zlabel ], [ %nzf1, %nzlabel ]

  ; Test phi with compiler constant
  %ptr3 = phi ptr [ null, %zlabel ], [ %nzf0, %nzlabel ]

  %v0 = load i64, ptr %ptr1
  %v1 = load i64, ptr %ptr2
  %v2 = load i64, ptr %ptr3

  ret void
}
; CHECK-LABEL: void @test01
; CHECK:  %ptr1 = phi ptr [ %zf0, %zlabel ], [ %nzf0, %nzlabel ]
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   i64*{{ *$}}
; CHECK-NEXT: Element pointees:
; CHECK-NEXT:   %struct.test01 @ 0
; CHECK-NEXT: DomTy: i64*{{ *}}

; CHECK:  %ptr2 = phi ptr [ %zf0, %zlabel ], [ %nzf1, %nzlabel ]
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   i64*{{ *$}}
; CHECK-NEXT: Element pointees:
; CHECK-NEXT:   %struct.test01 @ 0
; CHECK-NEXT:   %struct.test01 @ 1
; CHECK-NEXT: DomTy: i64*{{ *}}

; CHECK:  %ptr3 = phi ptr [ null, %zlabel ], [ %nzf0, %nzlabel ]
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   i64*{{ *$}}
; CHECK-NEXT: Element pointees:
; CHECK-NEXT:   %struct.test01 @ 0
; CHECK-NEXT: DomTy: i64*{{ *}}


; Test phi with self-reference.
%struct.test02 = type { i64, i64, i64 }
define internal void @test02(i64 %in, ptr "intel_dtrans_func_index"="1" %struct) !intel.dtrans.func.type !5 {
entry:
  br label %loop
loop:
  %cur_ptr = phi ptr [%struct, %entry], [%cur_ptr, %loop]
  br i1 undef, label %loop, label %exit
exit:
  ret void
}
; CHECK-LABEL: void @test02
; CHECK: %cur_ptr = phi ptr [ %struct, %entry ], [ %cur_ptr, %loop ]
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:    %struct.test02*{{ *$}}
; CHECK-NEXT: No element pointees.
; CHECK-NEXT: DomTy: %struct.test02*


; Test phi with circular dependencies.
%struct.test03.a = type { i32, i32 }
%struct.test03.b = type { i32, i32 }
%struct.test03.c = type { i32, i32 }
define void @test03(ptr "intel_dtrans_func_index"="1" %a_in, ptr "intel_dtrans_func_index"="2" %b_in, ptr "intel_dtrans_func_index"="3" %c_in) !intel.dtrans.func.type !10 {
entry:
  %tmpA = ptrtoint ptr %a_in to i64
  %tmpB = ptrtoint ptr %b_in to i64
  %tmpC = ptrtoint ptr %c_in to i64
  br i1 undef, label %block_A, label %block_BorC

block_BorC:
  br i1 undef, label %block_B, label %block_C

block_C:
  %c = phi i64 [%a, %merge_AorC], [%b, %merge_BorC], [%tmpC, %block_BorC]
  br i1 undef, label %merge, label %block_AorB

block_AorB:
  br i1 undef, label %block_A, label %block_B

block_A:
  %a = phi i64 [%d, %merge], [%c, %block_AorB], [%tmpA, %entry]
  br i1 undef, label %merge_AorC, label %exit_A

merge_AorC:
  br i1 undef, label %merge, label %block_C

block_B:
  %b = phi i64 [%d, %merge], [%c, %block_AorB], [%tmpB, %block_BorC]
  br i1 undef, label %merge_BorC, label %exit_B

merge_BorC:
  br i1 undef, label %merge, label %block_C

merge:
  %d = phi i64 [%a, %merge_AorC], [%b, %merge_BorC], [%c, %block_C]
  br i1 undef, label %block_A, label %block_B

exit_A:
  %badA = inttoptr i64 %a to ptr
  br label %exit

exit_B:
  br label %exit

exit:
  ret void
}
; CHECK-LABEL: void @test03
; CHECK:  %c = phi i64 [ %a, %merge_AorC ], [ %b, %merge_BorC ], [ %tmpC, %block_BorC ]
; CHECK:    LocalPointerInfo:
; CHECK:      Aliased types:
; CHECK-NEXT:   %struct.test03.a*{{ *$}}
; CHECK-NEXT:   %struct.test03.b*{{ *$}}
; CHECK-NEXT:   %struct.test03.c*{{ *$}}
; CHECK-NEXT: No element pointees.
; CHECK-NEXT: Ambiguous Dominant Type

; CHECK:  %a = phi i64 [ %d, %merge ], [ %c, %block_AorB ], [ %tmpA, %entry ]
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   %struct.test03.a*{{ *$}}
; CHECK-NEXT:   %struct.test03.b*{{ *$}}
; CHECK-NEXT:   %struct.test03.c*{{ *$}}
; CHECK-NEXT: No element pointees.
; CHECK-NEXT: Ambiguous Dominant Type

; CHECK:  %b = phi i64 [ %d, %merge ], [ %c, %block_AorB ], [ %tmpB, %block_BorC ]
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   %struct.test03.a*{{ *$}}
; CHECK-NEXT:   %struct.test03.b*{{ *$}}
; CHECK-NEXT:   %struct.test03.c*{{ *$}}
; CHECK-NEXT: No element pointees.
; CHECK-NEXT: Ambiguous Dominant Type

; CHECK:  %d = phi i64 [ %a, %merge_AorC ], [ %b, %merge_BorC ], [ %c, %block_C ]
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   %struct.test03.a*{{ *$}}
; CHECK-NEXT:   %struct.test03.b*{{ *$}}
; CHECK-NEXT:   %struct.test03.c*{{ *$}}
; CHECK-NEXT: No element pointees.
; CHECK-NEXT: Ambiguous Dominant Type


!1 = !{i64 0, i32 0}  ; i64
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = distinct !{!2}
!4 = !{%struct.test02 zeroinitializer, i32 1}  ; %struct.test02*
!5 = distinct !{!4}
!6 = !{i32 0, i32 0}  ; i32
!7 = !{%struct.test03.a zeroinitializer, i32 1}  ; %struct.test03.a*
!8 = !{%struct.test03.b zeroinitializer, i32 1}  ; %struct.test03.b*
!9 = !{%struct.test03.c zeroinitializer, i32 1}  ; %struct.test03.c*
!10 = distinct !{!7, !8, !9}
!11 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !1, !1} ; { i64, i64, i64 }
!12 = !{!"S", %struct.test02 zeroinitializer, i32 3, !1, !1, !1} ; { i64, i64, i64 }
!13 = !{!"S", %struct.test03.a zeroinitializer, i32 2, !6, !6} ; { i32, i32 }
!14 = !{!"S", %struct.test03.b zeroinitializer, i32 2, !6, !6} ; { i32, i32 }
!15 = !{!"S", %struct.test03.c zeroinitializer, i32 2, !6, !6} ; { i32, i32 }

!intel.dtrans.types = !{!11, !12, !13, !14, !15}
