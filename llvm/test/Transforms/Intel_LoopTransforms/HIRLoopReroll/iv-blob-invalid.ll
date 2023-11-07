; RUN: opt -disable-output -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-loop-reroll,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s
; RUN: opt -disable-output -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-reroll" -print-changed -disable-output < %s 2>&1 | FileCheck %s --check-prefix=CHECK-CHANGED

; Out of the capability of current reroller. Without changing 1 and 2 into expressions with IV,
; reroll without % operation is not possible.
; ICC does not.

;#define SIZE 10
;#include <stdint.h>
;int64_t A[SIZE];
;int64_t B[SIZE];
;int64_t C[SIZE];
;
;void foo(int n) {
;  int D = n*n;
;  int q = 0;
;  for (int i=0;  i<n; i=i+2) {
;    B[i]   =(i + 1)*(2*i + 3) + 1;
;    B[i+1] =(i + 2)*(2*i + 5) + 2;
;
;    // B[i]   =(i + 1)*(2*i + 3) + i;
;    // B[i+1] =(i + 2)*(2*i + 5) + i+1;
;  }
;}

; CHECK:Function: foo

; CHECK:        BEGIN REGION { }
; CHECK:              + DO i1 = 0, (sext.i32.i64(%n) + -1)/u2, 1   <DO_LOOP>  <MAX_TC_EST = 5>
; CHECK:              |   %mul3 = 4 * i1 + 3  *  2 * i1 + 1;
; CHECK:              |   %add4 = %mul3  +  1;
; CHECK:              |   (@B)[0][2 * i1] = %add4;
; CHECK:              |   %mul8 = 4 * i1 + 5  *  2 * i1 + 2;
; CHECK:              |   %add9 = %mul8  +  2;
; CHECK:              |   (@B)[0][2 * i1 + 1] = %add9;
; CHECK:              + END LOOP
; CHECK:        END REGION

; CHECK:Function: foo

; CHECK:        BEGIN REGION { }
; CHECK:              + DO i1 = 0, (sext.i32.i64(%n) + -1)/u2, 1   <DO_LOOP>  <MAX_TC_EST = 5>
; CHECK:              |   %mul3 = 4 * i1 + 3  *  2 * i1 + 1;
; CHECK:              |   %add4 = %mul3  +  1;
; CHECK:              |   (@B)[0][2 * i1] = %add4;
; CHECK:              |   %mul8 = 4 * i1 + 5  *  2 * i1 + 2;
; CHECK:              |   %add9 = %mul8  +  2;
; CHECK:              |   (@B)[0][2 * i1 + 1] = %add9;
; CHECK:              + END LOOP
; CHECK:        END REGION

; Verify that pass is not dumped with print-changed if it bails out.


; CHECK-CHANGED: Dump Before HIRTempCleanup
; CHECK-CHANGED-NOT: Dump After HIRLoopReroll

;Module Before HIR; ModuleID = 'new-2.c'
source_filename = "new-2.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = common dso_local local_unnamed_addr global [10 x i64] zeroinitializer, align 16
@A = common dso_local local_unnamed_addr global [10 x i64] zeroinitializer, align 16
@C = common dso_local local_unnamed_addr global [10 x i64] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable writeonly
define dso_local void @foo(i32 %n) local_unnamed_addr #0 {
entry:
  %cmp27 = icmp sgt i32 %n, 0
  br i1 %cmp27, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %0 = sext i32 %n to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.body
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %i.028 = phi i32 [ 0, %for.body.preheader ], [ %add5, %for.body ]
  %1 = or i64 %indvars.iv, 1
  %add = or i32 %i.028, 1
  %2 = shl nuw nsw i64 %indvars.iv, 1
  %3 = trunc i64 %2 to i32
  %4 = or i32 %3, 3
  %mul3 = mul nsw i32 %4, %add
  %add4 = add nuw nsw i32 %mul3, 1
  %5 = zext i32 %add4 to i64
  %arrayidx = getelementptr inbounds [10 x i64], ptr @B, i64 0, i64 %indvars.iv
  store i64 %5, ptr %arrayidx, align 16
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 2
  %add5 = add nuw nsw i32 %i.028, 2
  %6 = trunc i64 %2 to i32
  %7 = add i32 %6, 5
  %mul8 = mul nsw i32 %7, %add5
  %add9 = add nuw nsw i32 %mul8, 2
  %8 = zext i32 %add9 to i64
  %arrayidx13 = getelementptr inbounds [10 x i64], ptr @B, i64 0, i64 %1
  store i64 %8, ptr %arrayidx13, align 8
  %cmp = icmp slt i64 %indvars.iv.next, %0
  br i1 %cmp, label %for.body, label %for.cond.cleanup.loopexit
}



