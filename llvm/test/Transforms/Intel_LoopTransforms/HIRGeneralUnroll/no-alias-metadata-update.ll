; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-general-unroll" -hir-details -print-before=hir-general-unroll -print-after=hir-general-unroll -disable-output 2>&1 < %s | FileCheck %s

; Verify that 'alias.scope' and 'nolias' metadata on the load and store is
; updated for each unrolled iteration since they can alias across iterations
; as specified by @llvm.experimental.noalias.scope.decl() calls in the 
; incoming IR. The remainder loop with a trip count of 1 is completely unrolled
; so we generate explicit calls to @llvm.experimental.noalias.scope.decl() for
; it.


; CHECK: Dump Before

; CHECK: + NoAlias scope lists: [[SCOPE1:!.*]], [[SCOPE2:!.*]]
; CHECK: + DO i64 i1 = 0, 10, 1   <DO_LOOP> <unroll = 2>
; CHECK: |   (%b)[i1] = (%a)[i1];
; CHECK: |   <LVAL-REG> {al:4}(LINEAR ptr %b)[LINEAR i64 i1] inbounds  !alias.scope [[SCOPE1]] !noalias [[SCOPE2]]
; CHECK: |   <RVAL-REG> {al:4}(LINEAR ptr %a)[LINEAR i64 i1] inbounds  !alias.scope [[SCOPE2]] !noalias [[SCOPE1]]
; CHECK: |
; CHECK: + END LOOP


; CHECK: Dump After

; CHECK: + NoAlias scope lists: [[NEW_SCOPE1:.*]], [[NEW_SCOPE2:.*]], [[NEW_SCOPE3:.*]], [[NEW_SCOPE4:.*]]
; CHECK: + DO i64 i1 = 0, 4, 1   <DO_LOOP> <nounroll>
; CHECK: |   (%b)[2 * i1] = (%a)[2 * i1];
; CHECK: |   <LVAL-REG> {al:4}(LINEAR ptr %b)[LINEAR i64 2 * i1] inbounds  !alias.scope [[NEW_SCOPE1]] !noalias [[NEW_SCOPE2]]
; CHECK: |   <RVAL-REG> {al:4}(LINEAR ptr %a)[LINEAR i64 2 * i1] inbounds  !alias.scope [[NEW_SCOPE2]] !noalias [[NEW_SCOPE1]]
; CHECK: |
; CHECK: |   (%b)[2 * i1 + 1] = (%a)[2 * i1 + 1];
; CHECK: |   <LVAL-REG> {al:4}(LINEAR ptr %b)[LINEAR i64 2 * i1 + 1] inbounds  !alias.scope [[NEW_SCOPE3]] !noalias [[NEW_SCOPE4]]
; CHECK: |   <RVAL-REG> {al:4}(LINEAR ptr %a)[LINEAR i64 2 * i1 + 1] inbounds  !alias.scope [[NEW_SCOPE4]] !noalias [[NEW_SCOPE3]]
; CHECK: + END LOOP


; CHECK: @llvm.experimental.noalias.scope.decl([[NEW_SCOPE5:.*]]);
; CHECK: @llvm.experimental.noalias.scope.decl([[NEW_SCOPE6:.*]]);

; CHECK: (%b)[10] = (%a)[10];
; CHECK: <LVAL-REG> {al:4}(LINEAR ptr %b)[i64 10] inbounds  !alias.scope [[NEW_SCOPE5]] !noalias [[NEW_SCOPE6]]
; CHECK: <RVAL-REG> {al:4}(LINEAR ptr %a)[i64 10] inbounds  !alias.scope [[NEW_SCOPE6]] !noalias [[NEW_SCOPE5]]


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @test(ptr nocapture noundef readonly %a, ptr nocapture noundef writeonly %b) {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret void

for.body:                                         ; preds = %for.body, %entry
  %i.07 = phi i64 [ 0, %entry ], [ %inc, %for.body ]
  %arrayidx = getelementptr inbounds i32, ptr %b, i64 %i.07
  %arrayidx3 = getelementptr inbounds i32, ptr %a, i64 %i.07
  tail call void @llvm.experimental.noalias.scope.decl(metadata !0)
  tail call void @llvm.experimental.noalias.scope.decl(metadata !3)
  %0 = load i32, ptr %arrayidx3, align 4, !alias.scope !3, !noalias !0
  store i32 %0, ptr %arrayidx, align 4, !alias.scope !0, !noalias !3
  %inc = add nuw nsw i64 %i.07, 1
  %exitcond.not = icmp eq i64 %inc, 11
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body, !llvm.loop !5
}

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: readwrite)
declare void @llvm.experimental.noalias.scope.decl(metadata) #1

attributes #1 = { nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: readwrite) }

!0 = !{!1}
!1 = distinct !{!1, !2, !"copy: %to"}
!2 = distinct !{!2, !"copy"}
!3 = !{!4}
!4 = distinct !{!4, !2, !"copy: %from"}
!5 = distinct !{!5, !6}
!6 = !{!"llvm.loop.unroll.count", i32 2}
