; RUN: opt -disable-output -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-loop-reroll,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s

; Reroll happend. ICC does not reroll.

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
;    B[A[i]]   =1 ;
;    B[A[i+1]] =1 ;
;  }
;}

; CHECK:Function: foo

; CHECK:        BEGIN REGION { }
; CHECK:              + DO i1 = 0, (sext.i32.i64(%n) + -1)/u2, 1   <DO_LOOP>  <MAX_TC_EST = 5>
; CHECK:              |   %1 = (@A)[0][2 * i1];
; CHECK:              |   (@B)[0][%1] = 1;
; CHECK:              |   %3 = (@A)[0][2 * i1 + 1];
; CHECK:              |   (@B)[0][%3] = 1;
; CHECK:              + END LOOP
; CHECK:        END REGION

; CHECK:Function: foo

; CHECK:        BEGIN REGION { }
; CHECK:              + DO i1 = 0, 2 * ((1 + sext.i32.i64(%n)) /u 2) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 10>
; CHECK:              |   %1 = (@A)[0][i1];
; CHECK:              |   (@B)[0][%1] = 1;
; CHECK:              + END LOOP
; CHECK:        END REGION

;Module Before HIR; ModuleID = 'indirect-1.c'
source_filename = "indirect-1.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = common dso_local local_unnamed_addr global [10 x i64] zeroinitializer, align 16
@A = common dso_local local_unnamed_addr global [10 x i64] zeroinitializer, align 16
@C = common dso_local local_unnamed_addr global [10 x i64] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define dso_local void @foo(i32 %n) local_unnamed_addr #0 {
entry:
  %cmp14 = icmp sgt i32 %n, 0
  br i1 %cmp14, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %0 = sext i32 %n to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.body
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds [10 x i64], ptr @A, i64 0, i64 %indvars.iv
  %1 = load i64, ptr %arrayidx, align 16
  %arrayidx1 = getelementptr inbounds [10 x i64], ptr @B, i64 0, i64 %1
  store i64 1, ptr %arrayidx1, align 8
  %2 = or i64 %indvars.iv, 1
  %arrayidx3 = getelementptr inbounds [10 x i64], ptr @A, i64 0, i64 %2
  %3 = load i64, ptr %arrayidx3, align 8
  %arrayidx4 = getelementptr inbounds [10 x i64], ptr @B, i64 0, i64 %3
  store i64 1, ptr %arrayidx4, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 2
  %cmp = icmp slt i64 %indvars.iv.next, %0
  br i1 %cmp, label %for.body, label %for.cond.cleanup.loopexit
}



