; RUN: opt -passes="hir-ssa-deconstruction,print<hir>,hir-cg" -hir-details -force-hir-cg -S 2>&1 %s | FileCheck %s

; Verify that we populate loop's NoAlias scope lists using the calls to 
; @llvm.experimental.noalias.scope.decl() and then remove them from HIR.

; Also check that they are generated at the beginning of the loop by hir-cg.

; CHECK:      + NoAlias scope lists: !1, !4
; CHECK:      + DO i32 i1 = 0, 3, 1   <DO_LOOP>
; CHECK-NEXT: |   %0 = (%a)[-1 * i1];
; CHECK:      |   (%b)[-1 * i1] = %0;
; CHECK:      + END LOOP

; CHECK: loop.{{.*}}:
; CHECK-NEXT:  call void @llvm.experimental.noalias.scope.decl(metadata !1)
; CHECK-NEXT:  call void @llvm.experimental.noalias.scope.decl(metadata !4)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @test(ptr nocapture noundef readonly %a, ptr nocapture noundef writeonly %b) {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret void

for.body:                                         ; preds = %for.body, %entry
  %i.07 = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  %and = and i32 %i.07, 1
  %idxprom = zext i32 %and to i64
  %arrayidx = getelementptr inbounds i32, ptr %b, i64 %idxprom
  %arrayidx3 = getelementptr inbounds i32, ptr %a, i64 %idxprom
  tail call void @llvm.experimental.noalias.scope.decl(metadata !1)
  tail call void @llvm.experimental.noalias.scope.decl(metadata !4)
  %0 = load i32, ptr %arrayidx3, align 4, !alias.scope !4, !noalias !1
  store i32 %0, ptr %arrayidx, align 4, !alias.scope !1, !noalias !4
  %inc = add nuw nsw i32 %i.07, 1
  %exitcond.not = icmp eq i32 %inc, 4
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body
}

declare void @llvm.experimental.noalias.scope.decl(metadata) #1

!1 = !{!2}
!2 = distinct !{!2, !3, !"copy: %to"}
!3 = distinct !{!3, !"copy"}
!4 = !{!5}
!5 = distinct !{!5, !3, !"copy: %from"}
