; RUN: opt -passes="loop-simplify,hir-ssa-deconstruction,print<hir>,hir-post-vec-complete-unroll,print<hir>" 2>&1 < %s | FileCheck %s

; Check that we can completely unroll loops with call statements.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; CHECK: Function

; CHECK: + DO i1 = 0, 499, 1   <DO_LOOP>
; CHECK: |   + DO i2 = 0, 4, 1   <DO_LOOP>
; CHECK: |   |   %1 = (@A)[0][i2][i1];
; CHECK: |   |   %add = %1  +  i1;
; CHECK: |   |   %call = @foo1(i1 + %1);
; CHECK: |   + END LOOP
; CHECK: + END LOOP

; CHECK: Function

; CHECK: + DO i1 = 0, 499, 1   <DO_LOOP>
; CHECK: |   %1 = (@A)[0][0][i1];
; CHECK: |   %add = %1  +  i1;
; CHECK: |   %call = @foo1(i1 + %1);
; CHECK: |   %1 = (@A)[0][1][i1];
; CHECK: |   %add = %1  +  i1;
; CHECK: |   %call = @foo1(i1 + %1);
; CHECK: |   %1 = (@A)[0][2][i1];
; CHECK: |   %add = %1  +  i1;
; CHECK: |   %call = @foo1(i1 + %1);
; CHECK: |   %1 = (@A)[0][3][i1];
; CHECK: |   %add = %1  +  i1;
; CHECK: |   %call = @foo1(i1 + %1);
; CHECK: |   %1 = (@A)[0][4][i1];
; CHECK: |   %add = %1  +  i1;
; CHECK: |   %call = @foo1(i1 + %1);
; CHECK: + END LOOP


@A = common global [1000 x [1000 x i32]] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define void @foo() {
entry:
  br label %for.cond.1.preheader

for.cond.1.preheader:                             ; preds = %for.inc.6, %entry
  %indvars.iv20 = phi i64 [ 0, %entry ], [ %indvars.iv.next21, %for.inc.6 ]
  %0 = trunc i64 %indvars.iv20 to i32
  br label %for.body.3

for.body.3:                                       ; preds = %for.body.3, %for.cond.1.preheader
  %indvars.iv = phi i64 [ 0, %for.cond.1.preheader ], [ %indvars.iv.next, %for.body.3 ]
  %arrayidx5 = getelementptr inbounds [1000 x [1000 x i32]], ptr @A, i64 0, i64 %indvars.iv, i64 %indvars.iv20
  %1 = load i32, ptr %arrayidx5, align 4
  %add = add nsw i32 %1, %0
  %call = tail call i32 @foo1(i32 %add)
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 5
  br i1 %exitcond, label %for.inc.6, label %for.body.3

for.inc.6:                                        ; preds = %for.body.3
  %indvars.iv.next21 = add nuw nsw i64 %indvars.iv20, 1
  %exitcond22 = icmp eq i64 %indvars.iv.next21, 500
  br i1 %exitcond22, label %for.end.8, label %for.cond.1.preheader

for.end.8:                                        ; preds = %for.inc.6
  %add9 = add nsw i32 %add, 5
  store i32 %add9, ptr getelementptr inbounds ([1000 x [1000 x i32]], ptr @A, i64 0, i64 5, i64 5), align 4
  ret void
}

; Function Attrs: nounwind
declare void @llvm.lifetime.start(i64, ptr nocapture)

declare i32 @foo1(i32)

; Function Attrs: nounwind
declare void @llvm.lifetime.end(i64, ptr nocapture)
