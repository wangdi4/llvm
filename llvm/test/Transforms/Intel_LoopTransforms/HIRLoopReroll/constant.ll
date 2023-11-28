
; RUN: opt -disable-output -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-loop-reroll,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s

; Notice llvm will recognize memset pattern before HIR Loopopt if RHSs were zero.

; CHECK: Function: foo

; CHECK:    BEGIN REGION { }
; CHECK:          + DO i1 = 0, (sext.i32.i64(%n) + -1)/u4, 1   <DO_LOOP>  <MAX_TC_EST = 2>
; CHECK:          |   (@A)[0][4 * i1] = 3;
; CHECK:          |   (@A)[0][4 * i1 + 1] = 3;
; CHECK:          |   (@A)[0][4 * i1 + 2] = 3;
; CHECK:          |   (@A)[0][4 * i1 + 3] = 3;
; CHECK:          + END LOOP
; CHECK:    END REGION

; CHECK: Function: foo

; CHECK:     BEGIN REGION { }
; CHECK:           + DO i1 = 0, 4 * ((3 + sext.i32.i64(%n)) /u 4) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 8>
; CHECK:           |   (@A)[0][i1] = 3;
; CHECK:           + END LOOP
; CHECK:     END REGION

;Module Before HIR; ModuleID = 'new-constant.c'
source_filename = "new-constant.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common dso_local local_unnamed_addr global [10 x i32] zeroinitializer, align 16
@B = common dso_local local_unnamed_addr global [10 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable writeonly
define dso_local void @foo(i32 %n) local_unnamed_addr #0 {
entry:
  %cmp19 = icmp sgt i32 %n, 0
  br i1 %cmp19, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %0 = sext i32 %n to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.body
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds [10 x i32], ptr @A, i64 0, i64 %indvars.iv
  store i32 3, ptr %arrayidx, align 16
  %1 = or i64 %indvars.iv, 1
  %arrayidx2 = getelementptr inbounds [10 x i32], ptr @A, i64 0, i64 %1
  store i32 3, ptr %arrayidx2, align 4
  %2 = or i64 %indvars.iv, 2
  %arrayidx5 = getelementptr inbounds [10 x i32], ptr @A, i64 0, i64 %2
  store i32 3, ptr %arrayidx5, align 8
  %3 = or i64 %indvars.iv, 3
  %arrayidx8 = getelementptr inbounds [10 x i32], ptr @A, i64 0, i64 %3
  store i32 3, ptr %arrayidx8, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 4
  %cmp = icmp slt i64 %indvars.iv.next, %0
  br i1 %cmp, label %for.body, label %for.cond.cleanup.loopexit
}



