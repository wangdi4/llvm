; Use tbaa to disambiguate between stores of %i and %f even when they have the same symbases and eliminate the first store to %i.
;
; RUN: opt -aa-pipeline="tbaa" -passes="hir-ssa-deconstruction,print<hir>,hir-dead-store-elimination,print<hir>" -hir-create-function-level-region -hir-details 2>&1 < %s | FileCheck %s
;
;*** IR Dump Before HIR Dead Store Elimination ***
; Function: foo
;
; CHECK:          BEGIN REGION { }
; CHECK:           (%c)[0] = 0;
; CHECK:           <LVAL-REG> {al:1}(LINEAR ptr %c)[i64 0] inbounds  !tbaa !2 {sb:[[SB1:.*]]}
;
; CHECK:           (%i)[0] = 1;
; CHECK:           <LVAL-REG> {al:4}(LINEAR ptr %i)[i64 0] inbounds  !tbaa !5 {sb:[[SB1]]}
;
; CHECK:           (%f)[0] = 2.000000e+00;
; CHECK:           <LVAL-REG> {al:4}(LINEAR ptr %f)[i64 0] inbounds  !tbaa !7 {sb:[[SB1]]}
;
; CHECK:           (%i)[0] = 3;
; CHECK:           <LVAL-REG> {al:4}(LINEAR ptr %i)[i64 0] inbounds  !tbaa !5 {sb:[[SB1]]}
;
; CHECK:           ret ;
; CHECK:     END REGION
;
; *** IR Dump After HIR Dead Store Elimination ***
;Function: foo
;
; CHECK-NOT:           (%i)[0] = 1;
;
define void @foo(ptr %c, ptr %i, ptr %f) {
entry:
  br label %bb

bb:
  store i8 0, ptr %c, align 1, !tbaa !2
  store i32 1, ptr %i, align 4, !tbaa !5
  store float 2.000000e+00, ptr %f, align 4, !tbaa !7
  store i32 3, ptr %i, align 4, !tbaa !5
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
