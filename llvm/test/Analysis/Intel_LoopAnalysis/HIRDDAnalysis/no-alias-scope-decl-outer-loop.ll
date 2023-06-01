; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,print<hir-dd-analysis>" -hir-dd-analysis-verify=Region -hir-details -disable-output 2>&1 %s | FileCheck %s

; Verify that we assign same symbase to (%b)[i2] and (%a)[i2] despite them
; having appropriate noalias metadata for disambiguation because the scopes are
; specified at a particular loop level (i1 in this case) in HIR using calls to 
; @llvm.experimental.noalias.scope.decl(). This means that they are only
; independant at levels deeper than where the intrinsic exists. For this test,
; they are independant in i2 loop but not in i1 loop.


; CHECK: + NoAlias scope lists: !1, !4
; CHECK: + DO i64 i1 = 0, 39, 1   <DO_LOOP>
; CHECK: |   + DO i64 i2 = 0, 39, 1   <DO_LOOP>
; CHECK: |   |   (%b)[i2] = (%a)[i2];
; CHECK: |   |   <LVAL-REG> {al:4}(LINEAR ptr %b)[LINEAR i64 i2] inbounds  !alias.scope !1 !noalias !4 {sb:[[SB:.*]]}
; CHECK: |   |   <RVAL-REG> {al:4}(LINEAR ptr %a)[LINEAR i64 i2] inbounds  !alias.scope !4 !noalias !1 {sb:[[SB]]}
; CHECK: |   + END LOOP
; CHECK: + END LOOP


; Verify that the DV between (%a)[i2] and (%b)[i2] is formed as (* 0) with 0
; indicating that the ref is independant at i2 level.

; CHECK-DAG: (%b)[i2] --> (%a)[i2] FLOW (* 0) 
; CHECK-DAG: (%a)[i2] --> (%b)[i2] ANTI (* 0) 



target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @test(ptr nocapture noundef readonly %a, ptr nocapture noundef writeonly %b) {
entry:
  br label %for.body.outer

for.body.outer:
  %iv.outer = phi i64 [ 0, %entry ], [ %inc.outer, %outer.latch ]
  tail call void @llvm.experimental.noalias.scope.decl(metadata !1)
  tail call void @llvm.experimental.noalias.scope.decl(metadata !4)
  br label %for.body

for.body:                                         ; preds = %for.body, %for.body.outer
  %i.07 = phi i64 [ 0, %for.body.outer ], [ %inc, %for.body ]
  %arrayidx = getelementptr inbounds i32, ptr %b, i64 %i.07
  %arrayidx3 = getelementptr inbounds i32, ptr %a, i64 %i.07
  %0 = load i32, ptr %arrayidx3, align 4, !alias.scope !4, !noalias !1
  store i32 %0, ptr %arrayidx, align 4, !alias.scope !1, !noalias !4
  %inc = add nuw nsw i64 %i.07, 1
  %exitcond.not = icmp eq i64 %inc, 40
  br i1 %exitcond.not, label %outer.latch, label %for.body

outer.latch:
  %inc.outer = add nuw nsw i64 %iv.outer, 1
  %exitcond = icmp eq i64 %inc.outer, 40
  br i1 %exitcond, label %for.cond.cleanup, label %for.body.outer

for.cond.cleanup:                                 ; preds = %for.body.outer
  ret void
}

declare void @llvm.experimental.noalias.scope.decl(metadata) #1

!1 = !{!2}
!2 = distinct !{!2, !3, !"copy: %to"}
!3 = distinct !{!3, !"copy"}
!4 = !{!5}
!5 = distinct !{!5, !3, !"copy: %from"}
