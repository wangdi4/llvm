; RUN: opt -hir-ssa-deconstruction -hir-create-function-level-region -hir-dead-store-elimination -print-before=hir-dead-store-elimination -print-after=hir-dead-store-elimination < %s 2>&1 | FileCheck %s
; RUN: opt -aa-pipeline="basic-aa" -passes="hir-ssa-deconstruction,print<hir>,hir-dead-store-elimination,print<hir>" -hir-create-function-level-region 2>&1 < %s | FileCheck %s

; Check that the second store to (%i)[0] is eliminated but the first one is not
; due to aliasing with (%c)[0].

;*** IR Dump Before HIR Dead Store Elimination ***
;Function: foo
;
; CHECK:      BEGIN REGION { }
; CHECK:            (%i)[0] = 1;
; CHECK:            %t = (%c)[0];
; CHECK:            (%i)[0] = 3;
; CHECK:            (%i)[0] = 4;
; CHECK:            ret ;
; CHECK:      END REGION
;
;*** IR Dump After HIR Dead Store Elimination ***
;Function: foo
;
; CHECK-NOT:       (%i)[0] = 3;
;
define void @foo(i8* %c, i32* %i) {
entry:
  br label %bb

bb:
  store i32 1, i32* %i, align 4, !tbaa !5
  %t = load i8, i8*  %c, align 1, !tbaa !2
  store i32 3, i32* %i, align 4, !tbaa !5
  store i32 4, i32* %i, align 4, !tbaa !5
  ret void
}


!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
!5 = !{!6, !6, i64 0}
!6 = !{!"int", !3, i64 0}
!7 = !{!8, !8, i64 0}
!8 = !{!"float", !3, i64 0}
