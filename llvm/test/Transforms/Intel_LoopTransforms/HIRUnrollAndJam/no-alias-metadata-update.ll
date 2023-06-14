; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-unroll-and-jam" -print-before=hir-unroll-and-jam -print-after=hir-unroll-and-jam -hir-details < %s 2>&1 | FileCheck %s

; Verify that the noalias scope metadata specified via calls to 
; @llvm.experimental.noalias.scope.decl() is updated at all loop levels
; starting from the i1 loop which is being unrolled and jammed.

; CHECK: Dump Before

; CHECK: + NoAlias scope lists: [[SCOPE1:!.*]]
; CHECK: + DO i32 i1 = 0, 99, 1   <DO_LOOP> <unroll and jam = 2>
; CHECK: |   + NoAlias scope lists: [[SCOPE2:!.*]]
; CHECK: |   + DO i64 i2 = 0, 99, 1   <DO_LOOP>
; CHECK: |   |   + NoAlias scope lists: [[SCOPE3:!.*]]
; CHECK: |   |   + DO i64 i3 = 0, 99, 1   <DO_LOOP>
; CHECK: |   |   |   %add = (%B)[i2]  +  (%C)[i3]; <fast>
; CHECK: |   |   |   <RVAL-REG> {al:4}(LINEAR ptr %B)[LINEAR i64 i2] inbounds  !alias.scope [[SCOPE1]]
; CHECK: |   |   |   <RVAL-REG> {al:4}(LINEAR ptr %C)[LINEAR i64 i3] inbounds  !alias.scope [[SCOPE2]]
; CHECK: |   |   |
; CHECK: |   |   |   (@A)[0][i2][i3] = %add;
; CHECK: |   |   |   <LVAL-REG> {al:4}(LINEAR ptr @A)[i64 0][LINEAR i64 i2][LINEAR i64 i3] inbounds  !alias.scope [[SCOPE3]]
; CHECK: |   |   |
; CHECK: |   |   + END LOOP
; CHECK: |   + END LOOP
; CHECK: + END LOOP


; CHECK: Dump After

; CHECK: + NoAlias scope lists: [[NEW_SCOPE1:.*]], [[NEW_SCOPE2:.*]]
; CHECK: + DO i32 i1 = 0, 49, 1   <DO_LOOP> <nounroll and jam>
; CHECK: |   + NoAlias scope lists: [[NEW_SCOPE3:.*]], [[NEW_SCOPE4:.*]]
; CHECK: |   + DO i64 i2 = 0, 99, 1   <DO_LOOP>
; CHECK: |   |   + NoAlias scope lists: [[NEW_SCOPE5:.*]], [[NEW_SCOPE6:.*]]
; CHECK: |   |   + DO i64 i3 = 0, 99, 1   <DO_LOOP>
; CHECK: |   |   |   %add = (%B)[i2]  +  (%C)[i3]; <fast>
; CHECK: |   |   |   <RVAL-REG> {al:4}(LINEAR ptr %B)[LINEAR i64 i2] inbounds  !alias.scope [[NEW_SCOPE1]]
; CHECK: |   |   |   <RVAL-REG> {al:4}(LINEAR ptr %C)[LINEAR i64 i3] inbounds  !alias.scope [[NEW_SCOPE3]]
; CHECK: |   |   |
; CHECK: |   |   |   (@A)[0][i2][i3] = %add;
; CHECK: |   |   |   <LVAL-REG> {al:4}(LINEAR ptr @A)[i64 0][LINEAR i64 i2][LINEAR i64 i3] inbounds  !alias.scope [[NEW_SCOPE5]]
; CHECK: |   |   |
; CHECK: |   |   |   %add = (%B)[i2]  +  (%C)[i3]; <fast>
; CHECK: |   |   |   <RVAL-REG> {al:4}(LINEAR ptr %B)[LINEAR i64 i2] inbounds  !alias.scope [[NEW_SCOPE2]]
; CHECK: |   |   |   <RVAL-REG> {al:4}(LINEAR ptr %C)[LINEAR i64 i3] inbounds  !alias.scope [[NEW_SCOPE4]]
; CHECK: |   |   |
; CHECK: |   |   |   (@A)[0][i2][i3] = %add;
; CHECK: |   |   |   <LVAL-REG> {al:4}(LINEAR ptr @A)[i64 0][LINEAR i64 i2][LINEAR i64 i3] inbounds  !alias.scope [[NEW_SCOPE6]]
; CHECK: |   |   + END LOOP
; CHECK: |   + END LOOP
; CHECK: + END LOOP

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = dso_local local_unnamed_addr global [100 x [100 x float]] zeroinitializer, align 16

define dso_local void @foo(ptr nocapture noundef readonly %B, ptr nocapture noundef readonly %C) {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.inc16
  %i.028 = phi i32 [ 0, %entry ], [ %inc17, %for.inc16 ]
  tail call void @llvm.experimental.noalias.scope.decl(metadata !3)
  br label %for.cond4.preheader

for.cond4.preheader:                              ; preds = %for.cond1.preheader, %for.inc13
  %indvars.iv29 = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next30, %for.inc13 ]
  tail call void @llvm.experimental.noalias.scope.decl(metadata !5)
  %arrayidx = getelementptr inbounds float, ptr %B, i64 %indvars.iv29
  br label %for.body6

for.body6:                                        ; preds = %for.cond4.preheader, %for.body6
  %indvars.iv = phi i64 [ 0, %for.cond4.preheader ], [ %indvars.iv.next, %for.body6 ]
  tail call void @llvm.experimental.noalias.scope.decl(metadata !7)
  %0 = load float, ptr %arrayidx, align 4, !alias.scope !3
  %arrayidx8 = getelementptr inbounds float, ptr %C, i64 %indvars.iv
  %1 = load float, ptr %arrayidx8, align 4, !alias.scope !5
  %add = fadd fast float %0, %1
  %arrayidx12 = getelementptr inbounds [100 x [100 x float]], ptr @A, i64 0, i64 %indvars.iv29, i64 %indvars.iv
  store float %add, ptr %arrayidx12, align 4, !alias.scope !7
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond.not, label %for.inc13, label %for.body6

for.inc13:                                        ; preds = %for.body6
  %indvars.iv.next30 = add nuw nsw i64 %indvars.iv29, 1
  %exitcond31.not = icmp eq i64 %indvars.iv.next30, 100
  br i1 %exitcond31.not, label %for.inc16, label %for.cond4.preheader

for.inc16:                                        ; preds = %for.inc13
  %inc17 = add nuw nsw i32 %i.028, 1
  %exitcond32.not = icmp eq i32 %inc17, 100
  br i1 %exitcond32.not, label %for.end18, label %for.cond1.preheader, !llvm.loop !0

for.end18:                                        ; preds = %for.inc16
  ret void
}

declare void @llvm.experimental.noalias.scope.decl(metadata) #1

attributes #1 = { nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: readwrite) }

!0 = distinct !{!0, !1}
!1 = !{!"llvm.loop.unroll_and_jam.count", i32 2}
!2 = distinct !{!2, !"domain"}
!3 = !{!4}
!4 = distinct !{!4, !2, !"scope1"}
!5 = !{!6}
!6 = distinct !{!6, !2, !"scope2"}
!7 = !{!8}
!8 = distinct !{!8, !2, !"scope3"}



