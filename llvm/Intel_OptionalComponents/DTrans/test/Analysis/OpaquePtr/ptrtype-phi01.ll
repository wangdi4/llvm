; REQUIRES: asserts

; RUN: opt -disable-output -whole-program-assume -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-CUR
; RUN: opt -disable-output -whole-program-assume -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-CUR

; Test pointer type recovery for "phi" instructions.

; Lines marked with CHECK-CUR are tests for the current form of IR.
; Lines marked with CHECK-FUT are placeholders for check lines that will
;   changed when the future opaque pointer form of IR is used.
; Lines marked with CHECK should remain the same when changing to use opaque
;   pointers.

; Test simple phi instructions without circular dependencies.
%struct.test01 = type { i64, i64, i64 }
define internal void @test01(i64 %in, %struct.test01* %struct) !dtrans_type !2 {
entry:
  %cmp = icmp eq i64 %in, 0
  br i1 %cmp, label %zlabel, label %nzlabel

zlabel:
  %zf0 = getelementptr %struct.test01, %struct.test01* %struct, i64 0, i32 0
  br label %merge

nzlabel:
  %nzf0 = getelementptr %struct.test01, %struct.test01* %struct, i64 0, i32 0
  %nzf1 = getelementptr %struct.test01, %struct.test01* %struct, i64 0, i32 1
  br label %merge

merge:
  ; Test phi that has same aliases for each input
  %ptr1 = phi i64* [ %zf0, %zlabel ], [ %nzf0, %nzlabel ]

  ; Test phi with different element pointees for each input
  %ptr2 = phi i64* [ %zf0, %zlabel ], [ %nzf1, %nzlabel ]

  ; Test phi with compiler constant
  %ptr3 = phi i64* [ null, %zlabel ], [ %nzf0, %nzlabel ]

  %v0 = load i64, i64* %ptr1
  %v1 = load i64, i64* %ptr2
  %v2 = load i64, i64* %ptr3

  ret void
}
; CHECK-LABEL: void @test01
; CHECK-CUR:  %ptr1 = phi i64* [ %zf0, %zlabel ], [ %nzf0, %nzlabel ]
; CHECK-FUT:  %ptr1 = phi p0 [ %zf0, %zlabel ], [ %nzf0, %nzlabel ]
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   i64*{{ *$}}
; CHECK-NEXT: Element pointees:
; CHECK-NEXT:   %struct.test01 @ 0
; CHECK-NEXT: No Dominant Type

; CHECK-CUR:  %ptr2 = phi i64* [ %zf0, %zlabel ], [ %nzf1, %nzlabel ]
; CHECK-FUT:  %ptr2 = phi p0 [ %zf0, %zlabel ], [ %nzf1, %nzlabel ]
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   i64*{{ *$}}
; CHECK-NEXT: Element pointees:
; CHECK-NEXT:   %struct.test01 @ 0
; CHECK-NEXT:   %struct.test01 @ 1
; CHECK-NEXT: No Dominant Type

; CHECK-CUR:  %ptr3 = phi i64* [ null, %zlabel ], [ %nzf0, %nzlabel ]
; CHECK-FUT:  %ptr3 = phi p0 [ null, %zlabel ], [ %nzf0, %nzlabel ]
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   i64*{{ *$}}
; CHECK-NEXT: Element pointees:
; CHECK-NEXT:   %struct.test01 @ 0
; CHECK-NEXT: No Dominant Type


; Test phi with self-reference.
%struct.test02 = type { i64, i64, i64 }
define internal void @test02(i64 %in, %struct.test02* %struct) !dtrans_type !6 {
entry:
  br label %loop
loop:
  %cur_ptr = phi %struct.test02* [%struct, %entry], [%cur_ptr, %loop]
  br i1 undef, label %loop, label %exit
exit:
  ret void
}
; CHECK-LABEL: void @test02
; CHECK-CUR: %cur_ptr = phi %struct.test02* [ %struct, %entry ], [ %cur_ptr, %loop ]
; CHECK-FUT: %cur_ptr = phi p0 [ %struct, %entry ], [ %cur_ptr, %loop ]
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:    %struct.test02*{{ *$}}
; CHECK-NEXT: No element pointees.
; CHECK-NEXT: DomTy: %struct.test02*


; Test phi with circular dependencies.
%struct.test03.a = type { i32, i32 }
%struct.test03.b = type { i32, i32 }
%struct.test03.c = type { i32, i32 }
define void @test03(%struct.test03.a* %a_in, %struct.test03.b* %b_in, %struct.test03.c* %c_in) !dtrans_type !10 {
entry:
  %tmpA = ptrtoint %struct.test03.a* %a_in to i64
  %tmpB = ptrtoint %struct.test03.b* %b_in to i64
  %tmpC = ptrtoint %struct.test03.c* %c_in to i64
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
  %badA = inttoptr i64 %a to %struct.test03.a*
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
!2 = !{!"F", i1 false, i32 2, !3, !1, !4}  ; void (i64, %struct.test01*)
!3 = !{!"void", i32 0}  ; void
!4 = !{!5, i32 1}  ; %struct.test01*
!5 = !{!"R", %struct.test01 zeroinitializer, i32 0}  ; %struct.test01
!6 = !{!"F", i1 false, i32 2, !3, !1, !7}  ; void (i64, %struct.test02*)
!7 = !{!8, i32 1}  ; %struct.test02*
!8 = !{!"R", %struct.test02 zeroinitializer, i32 0}  ; %struct.test02
!9 = !{i32 0, i32 0}  ; i32
!10 = !{!"F", i1 false, i32 3, !3, !11, !13, !15}  ; void (%struct.test03.a*, %struct.test03.b*, %struct.test03.c*)
!11 = !{!12, i32 1}  ; %struct.test03.a*
!12 = !{!"R", %struct.test03.a zeroinitializer, i32 0}  ; %struct.test03.a
!13 = !{!14, i32 1}  ; %struct.test03.b*
!14 = !{!"R", %struct.test03.b zeroinitializer, i32 0}  ; %struct.test03.b
!15 = !{!16, i32 1}  ; %struct.test03.c*
!16 = !{!"R", %struct.test03.c zeroinitializer, i32 0}  ; %struct.test03.c
!17 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !1, !1} ; { i64, i64, i64 }
!18 = !{!"S", %struct.test02 zeroinitializer, i32 3, !1, !1, !1} ; { i64, i64, i64 }
!19 = !{!"S", %struct.test03.a zeroinitializer, i32 2, !9, !9} ; { i32, i32 }
!20 = !{!"S", %struct.test03.b zeroinitializer, i32 2, !9, !9} ; { i32, i32 }
!21 = !{!"S", %struct.test03.c zeroinitializer, i32 2, !9, !9} ; { i32, i32 }

!dtrans_types = !{!17, !18, !19, !20, !21}
