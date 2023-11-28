; RUN: opt -disable-output -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-loop-reroll,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s
; RUN: opt -disable-output -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-reroll" -print-changed -disable-output < %s 2>&1 | FileCheck %s --check-prefix=CHECK-CHANGED

; Rerolls with IVs and Blobs in the right pattern

; CHECK: Function: foo

; CHECK:     BEGIN REGION { }
; CHECK:           + DO i1 = 0, (sext.i32.i64(%n) + -1)/u4, 1   <DO_LOOP>  <MAX_TC_EST = 2>
; CHECK:           |   (@B)[0][4 * i1] = 4 * i1 + ((1 + %n) * %n);
; CHECK:           |   (@B)[0][4 * i1 + 1] = 4 * i1 + ((2 + %n) * %n) + 1;
; CHECK:           |   (@B)[0][4 * i1 + 2] = 4 * i1 + ((1 + %n) * %n) + 2;
; CHECK:           |   (@B)[0][4 * i1 + 3] = 4 * i1 + ((2 + %n) * %n) + 3;
; CHECK:           + END LOOP
; CHECK:     END REGION

; CHECK: Function: foo

; CHECK:     BEGIN REGION { }
; CHECK:           + DO i1 = 0, 2 * ((3 + sext.i32.i64(%n)) /u 4) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 4>
; CHECK:           |   (@B)[0][2 * i1] = 2 * i1 + ((1 + %n) * %n);
; CHECK:           |   (@B)[0][2 * i1 + 1] = 2 * i1 + ((2 + %n) * %n) + 1;
; CHECK:           + END LOOP
; CHECK:     END REGION

; Further check that reroll can be suppressed using a compiler flag

; RUN: opt -disable-output -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-reroll,print<hir>" -hir-loop-reroll-size-threshold=3 -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s --check-prefix=NOREROLL

; NOREROLL: Function: foo

; NOREROLL:     BEGIN REGION { }
; NOREROLL:           + DO i1 = 0, (sext.i32.i64(%n) + -1)/u4, 1   <DO_LOOP>  <MAX_TC_EST = 2>
; NOREROLL:           |   (@B)[0][4 * i1] = 4 * i1 + ((1 + %n) * %n);
; NOREROLL:           |   (@B)[0][4 * i1 + 1] = 4 * i1 + ((2 + %n) * %n) + 1;
; NOREROLL:           |   (@B)[0][4 * i1 + 2] = 4 * i1 + ((1 + %n) * %n) + 2;
; NOREROLL:           |   (@B)[0][4 * i1 + 3] = 4 * i1 + ((2 + %n) * %n) + 3;
; NOREROLL:           + END LOOP
; NOREROLL:     END REGION

; Verify that pass is dumped with print-changed when it triggers.


; CHECK-CHANGED: Dump Before HIRTempCleanup
; CHECK-CHANGED: Dump After HIRLoopReroll

;Module Before HIR; ModuleID = 'blob-no-pattern.c'
source_filename = "blob-no-pattern.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = common dso_local local_unnamed_addr global [10 x i32] zeroinitializer, align 16
@A = common dso_local local_unnamed_addr global [10 x i32] zeroinitializer, align 16
@C = common dso_local local_unnamed_addr global [10 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable writeonly
define dso_local void @foo(i32 %n) local_unnamed_addr #0 {
entry:
  %mul = mul nsw i32 %n, %n
  %add = add nsw i32 %n, 1
  %mul1 = mul nsw i32 %add, %n
  %cmp45 = icmp sgt i32 %n, 0
  br i1 %cmp45, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %0 = sext i32 %n to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.body
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %1 = trunc i64 %indvars.iv to i32
  %add2 = add nsw i32 %1, %n
  %add3 = add i32 %add2, %mul
  %arrayidx = getelementptr inbounds [10 x i32], ptr @B, i64 0, i64 %indvars.iv
  store i32 %add3, ptr %arrayidx, align 16
  %add5 = add i32 %add2, %mul1
  %add6 = add i32 %add5, 1
  %2 = or i64 %indvars.iv, 1
  %arrayidx9 = getelementptr inbounds [10 x i32], ptr @B, i64 0, i64 %2
  store i32 %add6, ptr %arrayidx9, align 4
  %add12 = add i32 %add3, 2
  %3 = or i64 %indvars.iv, 2
  %arrayidx15 = getelementptr inbounds [10 x i32], ptr @B, i64 0, i64 %3
  store i32 %add12, ptr %arrayidx15, align 8
  %add18 = add i32 %add5, 3
  %4 = or i64 %indvars.iv, 3
  %arrayidx21 = getelementptr inbounds [10 x i32], ptr @B, i64 0, i64 %4
  store i32 %add18, ptr %arrayidx21, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 4
  %cmp = icmp slt i64 %indvars.iv.next, %0
  br i1 %cmp, label %for.body, label %for.cond.cleanup.loopexit
}



