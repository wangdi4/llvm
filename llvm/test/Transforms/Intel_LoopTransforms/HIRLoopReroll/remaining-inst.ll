; RUN: opt -disable-output -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-loop-reroll,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s

; The loop body contains HLInsts not fitting into rerolling patterns.

;  CHECK: Function: foo
;
;  CHECK:      BEGIN REGION { }
;  CHECK:            + DO i1 = 0, (sext.i32.i64(%n) + -1)/u4, 1   <DO_LOOP>  <MAX_TC_EST = 2>
;  CHECK:            |   %add = 4 * %n * i1  +  %q.080;
;  CHECK:            |   %2 = (@C)[0][4 * i1];
;  CHECK:            |   (@B)[0][4 * i1] = %2;
;  CHECK:            |   (@A)[0][4 * i1] = (%n * %n * %2);
;  CHECK:            |   %4 = (@C)[0][4 * i1 + 1];
;  CHECK:            |   (@B)[0][4 * i1 + 1] = %4;
;  CHECK:            |   (@A)[0][4 * i1 + 1] = (%n * %n * %4);
;  CHECK:            |   %6 = (@C)[0][4 * i1 + 2];
;  CHECK:            |   (@B)[0][4 * i1 + 2] = %6;
;  CHECK:            |   (@A)[0][4 * i1 + 2] = (%n * %n * %6);
;  CHECK:            |   %8 = (@C)[0][4 * i1 + 3];
;  CHECK:            |   (@B)[0][4 * i1 + 3] = %8;
;  CHECK:            |   (@A)[0][4 * i1 + 3] = (%n * %n * %8);
;  CHECK:            |   %q.080 = %add;
;  CHECK:            + END LOOP
;  CHECK:      END REGION

;  CHECK: Function: foo
;
;  CHECK:      BEGIN REGION { }
;  CHECK:            + DO i1 = 0, (sext.i32.i64(%n) + -1)/u4, 1   <DO_LOOP>  <MAX_TC_EST = 2>
;  CHECK:            |   %add = 4 * %n * i1  +  %q.080;
;  CHECK:            |   %2 = (@C)[0][4 * i1];
;  CHECK:            |   (@B)[0][4 * i1] = %2;
;  CHECK:            |   (@A)[0][4 * i1] = (%n * %n * %2);
;  CHECK:            |   %4 = (@C)[0][4 * i1 + 1];
;  CHECK:            |   (@B)[0][4 * i1 + 1] = %4;
;  CHECK:            |   (@A)[0][4 * i1 + 1] = (%n * %n * %4);
;  CHECK:            |   %6 = (@C)[0][4 * i1 + 2];
;  CHECK:            |   (@B)[0][4 * i1 + 2] = %6;
;  CHECK:            |   (@A)[0][4 * i1 + 2] = (%n * %n * %6);
;  CHECK:            |   %8 = (@C)[0][4 * i1 + 3];
;  CHECK:            |   (@B)[0][4 * i1 + 3] = %8;
;  CHECK:            |   (@A)[0][4 * i1 + 3] = (%n * %n * %8);
;  CHECK:            |   %q.080 = %add;
;  CHECK:            + END LOOP
;  CHECK:      END REGION

;Module Before HIR; ModuleID = 'negative-remaining-inst.c'
source_filename = "negative-remaining-inst.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@C = common dso_local local_unnamed_addr global [10 x i32] zeroinitializer, align 16
@B = common dso_local local_unnamed_addr global [10 x i32] zeroinitializer, align 16
@A = common dso_local local_unnamed_addr global [10 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define dso_local void @foo(i32 %n) local_unnamed_addr #0 {
entry:
  %mul = mul nsw i32 %n, %n
  %cmp79 = icmp sgt i32 %n, 0
  br i1 %cmp79, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %0 = sext i32 %n to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.body
  %add.lcssa = phi i32 [ %add, %for.body ]
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  %q.0.lcssa = phi i32 [ 0, %entry ], [ %add.lcssa, %for.cond.cleanup.loopexit ]
  store i32 %q.0.lcssa, ptr getelementptr inbounds ([10 x i32], ptr @C, i64 0, i64 3), align 4
  ret void

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %q.080 = phi i32 [ 0, %for.body.preheader ], [ %add, %for.body ]
  %1 = trunc i64 %indvars.iv to i32
  %mul1 = mul nsw i32 %1, %n
  %add = add nsw i32 %mul1, %q.080
  %arrayidx = getelementptr inbounds [10 x i32], ptr @C, i64 0, i64 %indvars.iv
  %2 = load i32, ptr %arrayidx, align 16
  %arrayidx3 = getelementptr inbounds [10 x i32], ptr @B, i64 0, i64 %indvars.iv
  store i32 %2, ptr %arrayidx3, align 16
  %mul6 = mul nsw i32 %2, %mul
  %arrayidx8 = getelementptr inbounds [10 x i32], ptr @A, i64 0, i64 %indvars.iv
  store i32 %mul6, ptr %arrayidx8, align 16
  %3 = or i64 %indvars.iv, 1
  %arrayidx11 = getelementptr inbounds [10 x i32], ptr @C, i64 0, i64 %3
  %4 = load i32, ptr %arrayidx11, align 4
  %arrayidx14 = getelementptr inbounds [10 x i32], ptr @B, i64 0, i64 %3
  store i32 %4, ptr %arrayidx14, align 4
  %mul18 = mul nsw i32 %4, %mul
  %arrayidx21 = getelementptr inbounds [10 x i32], ptr @A, i64 0, i64 %3
  store i32 %mul18, ptr %arrayidx21, align 4
  %5 = or i64 %indvars.iv, 2
  %arrayidx24 = getelementptr inbounds [10 x i32], ptr @C, i64 0, i64 %5
  %6 = load i32, ptr %arrayidx24, align 8
  %arrayidx27 = getelementptr inbounds [10 x i32], ptr @B, i64 0, i64 %5
  store i32 %6, ptr %arrayidx27, align 8
  %mul31 = mul nsw i32 %6, %mul
  %arrayidx34 = getelementptr inbounds [10 x i32], ptr @A, i64 0, i64 %5
  store i32 %mul31, ptr %arrayidx34, align 8
  %7 = or i64 %indvars.iv, 3
  %arrayidx37 = getelementptr inbounds [10 x i32], ptr @C, i64 0, i64 %7
  %8 = load i32, ptr %arrayidx37, align 4
  %arrayidx40 = getelementptr inbounds [10 x i32], ptr @B, i64 0, i64 %7
  store i32 %8, ptr %arrayidx40, align 4
  %mul44 = mul nsw i32 %8, %mul
  %arrayidx47 = getelementptr inbounds [10 x i32], ptr @A, i64 0, i64 %7
  store i32 %mul44, ptr %arrayidx47, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 4
  %cmp = icmp slt i64 %indvars.iv.next, %0
  br i1 %cmp, label %for.body, label %for.cond.cleanup.loopexit
}



