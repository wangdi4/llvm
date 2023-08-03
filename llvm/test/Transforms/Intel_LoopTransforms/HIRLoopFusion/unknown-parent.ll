; RUN: opt -passes="hir-ssa-deconstruction,hir-loop-fusion,print<hir>" -aa-pipeline="basic-aa" -disable-output -hir-create-function-level-region < %s 2>&1 | FileCheck %s

; Check that two i2 loops were fused even if their parent is unknown loop.

; BEGIN REGION { }
;       if (%call35 > 0)
;       {
;          + UNKNOWN LOOP i1
;          |   <i1 = 0>
;          |   for.cond1.preheader:
;          |
;          |   + DO i2 = 0, 1023, 1   <DO_LOOP>
;          |   |   (@A)[0][i2] = i1 + i2;
;          |   + END LOOP
;          |
;          |
;          |   + DO i2 = 0, 1023, 1   <DO_LOOP>
;          |   |   %3 = (@A)[0][i2];
;          |   |   (@B)[0][i2] = i2 + %3;
;          |   + END LOOP
;          |
;          |   %call = @bar();
;          |   if (i1 + 1 < %call)
;          |   {
;          |      <i1 = i1 + 1>
;          |      goto for.cond1.preheader;
;          |   }
;          + END LOOP
;       }
;       ret ;
; END REGION

; CHECK: + UNKNOWN LOOP i1
; CHECK: + DO i2 = 0, 1023, 1
; CHECK: |   (@A)[0][i2] = i1 + i2;
; CHECK-NOT: DO i2
; CHECK: |   %3 = (@A)[0][i2];
; CHECK: |   (@B)[0][i2] = i2 + %3;
; CHECK: + END LOOP
; CHECK-NOT: DO i2
; CHECK: + END LOOP

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16
@B = dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define dso_local void @foo() local_unnamed_addr #0 {
entry:
  %call35 = tail call i32 (...) @bar() #2
  %cmp36 = icmp sgt i32 %call35, 0
  br i1 %cmp36, label %for.cond1.preheader.preheader, label %for.cond.cleanup

for.cond1.preheader.preheader:                    ; preds = %entry
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond1.preheader.preheader, %for.cond.cleanup8
  %indvars.iv42 = phi i64 [ %indvars.iv.next43, %for.cond.cleanup8 ], [ 0, %for.cond1.preheader.preheader ]
  br label %for.body4

for.cond.cleanup.loopexit:                        ; preds = %for.cond.cleanup8
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body4:                                        ; preds = %for.cond1.preheader, %for.body4
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.body4 ]
  %0 = add nuw nsw i64 %indvars.iv, %indvars.iv42
  %arrayidx = getelementptr inbounds [100 x i32], ptr @A, i64 0, i64 %indvars.iv
  %1 = trunc i64 %0 to i32
  store i32 %1, ptr %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.body9.preheader, label %for.body4

for.body9.preheader:                              ; preds = %for.body4
  br label %for.body9

for.cond.cleanup8:                                ; preds = %for.body9
  %indvars.iv.next43 = add nuw nsw i64 %indvars.iv42, 1
  %call = tail call i32 (...) @bar() #2
  %2 = sext i32 %call to i64
  %cmp = icmp slt i64 %indvars.iv.next43, %2
  br i1 %cmp, label %for.cond1.preheader, label %for.cond.cleanup.loopexit

for.body9:                                        ; preds = %for.body9.preheader, %for.body9
  %indvars.iv39 = phi i64 [ %indvars.iv.next40, %for.body9 ], [ 0, %for.body9.preheader ]
  %arrayidx11 = getelementptr inbounds [100 x i32], ptr @A, i64 0, i64 %indvars.iv39
  %3 = load i32, ptr %arrayidx11, align 4
  %4 = trunc i64 %indvars.iv39 to i32
  %add12 = add nsw i32 %3, %4
  %arrayidx14 = getelementptr inbounds [100 x i32], ptr @B, i64 0, i64 %indvars.iv39
  store i32 %add12, ptr %arrayidx14, align 4
  %indvars.iv.next40 = add nuw nsw i64 %indvars.iv39, 1
  %exitcond41 = icmp eq i64 %indvars.iv.next40, 1024
  br i1 %exitcond41, label %for.cond.cleanup8, label %for.body9
}

declare dso_local i32 @bar(...) local_unnamed_addr #1

