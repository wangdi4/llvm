; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-pre-vec-complete-unroll" -hir-details -print-before=hir-pre-vec-complete-unroll -print-after=hir-pre-vec-complete-unroll -disable-output 2>&1 < %s | FileCheck %s

; Verify that we update noalias scope !0 for each iteration as it is specified
; in @llvm.experimental.noalias.scope.decl() but not !3.


; CHECK: Dump Before

; CHECK: + NoAlias scope lists: [[SCOPE1:!.*]]
; CHECK: + DO i32 i1 = 0, 1, 1   <DO_LOOP> <unroll>
; CHECK: |   (%b)[i1] = (%a)[i1];
; CHECK: |   <LVAL-REG> {al:4}(LINEAR ptr %b)[LINEAR zext.i32.i64(i1)] inbounds  !alias.scope [[SCOPE2:.*]] !noalias [[SCOPE1]]
; CHECK: |   <RVAL-REG> {al:4}(LINEAR ptr %a)[LINEAR zext.i32.i64(i1)] inbounds  !alias.scope [[SCOPE1]] !noalias [[SCOPE2]]
; CHECK: |
; CHECK: + END LOOP


; CHECK: Dump After

; CHECK: @llvm.experimental.noalias.scope.decl([[NEW_SCOPE1:.*]]);

; CHECK: (%b)[0] = (%a)[0];
; CHECK: <LVAL-REG> {al:4}(LINEAR ptr %b)[i64 0] inbounds  !alias.scope [[NEW_SCOPE2:.*]] !noalias [[NEW_SCOPE1]]
; CHECK: <RVAL-REG> {al:4}(LINEAR ptr %a)[i64 0] inbounds  !alias.scope [[NEW_SCOPE1]] !noalias [[NEW_SCOPE2]]

; CHECK: @llvm.experimental.noalias.scope.decl([[NEW_SCOPE3:.*]]);

; CHECK: (%b)[1] = (%a)[1];
; CHECK: <LVAL-REG> {al:4}(LINEAR ptr %b)[i64 1] inbounds  !alias.scope [[NEW_SCOPE2]] !noalias [[NEW_SCOPE3]]
; CHECK: <RVAL-REG> {al:4}(LINEAR ptr %a)[i64 1] inbounds  !alias.scope [[NEW_SCOPE3]] !noalias [[NEW_SCOPE2]]


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
  tail call void @llvm.experimental.noalias.scope.decl(metadata !0)
  %0 = load i32, ptr %arrayidx3, align 4, !alias.scope !0, !noalias !3
  store i32 %0, ptr %arrayidx, align 4, !alias.scope !3, !noalias !0
  %inc = add nuw nsw i32 %i.07, 1
  %exitcond.not = icmp eq i32 %inc, 2
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
!6 = !{!"llvm.loop.unroll.full"}
