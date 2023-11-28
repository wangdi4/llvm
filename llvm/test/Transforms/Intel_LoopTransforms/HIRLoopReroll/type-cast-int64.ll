; RUN: opt -disable-output -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-loop-reroll,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s

; #define SIZE 10
; #include <stdint.h>
; int64_t A[SIZE];
; int64_t B[SIZE];
; int64_t C[SIZE];

; void foo(int n) {
;   int D = n*n;
;   int q = 0;
;   for (int i=0;  i<n; i=i+4) {
;
;     B[i]   = n + i*i;
;
;     B[i+1] = n + (i+1)*(i+1);
;
;     B[i+2] = n + (i+2)*(i+2);
;
;     B[i+3] = n + (i+3)*(i+3);
;   }
;
; }

; CHECK: Function: foo
;
; CHECK:      BEGIN REGION { }
; CHECK:            + DO i1 = 0, (sext.i32.i64(%n) + -1)/u4, 1   <DO_LOOP>  <MAX_TC_EST = 2>
; CHECK:            |   %1 = 4 * i1  *  4 * i1;
; CHECK:            |   %2 = %1  +  %n;
; CHECK:            |   (@B)[0][4 * i1] = %2;
; CHECK:            |   %4 = 4 * i1 + 1  *  4 * i1 + 1;
; CHECK:            |   %5 = %4  +  %n;
; CHECK:            |   (@B)[0][4 * i1 + 1] = %5;
; CHECK:            |   %7 = 4 * i1 + 2  *  4 * i1 + 2;
; CHECK:            |   %8 = %7  +  %n;
; CHECK:            |   (@B)[0][4 * i1 + 2] = %8;
; CHECK:            |   %10 = 4 * i1 + 3  *  4 * i1 + 3;
; CHECK:            |   %11 = %10  +  %n;
; CHECK:            |   (@B)[0][4 * i1 + 3] = %11;
; CHECK:            + END LOOP
; CHECK:      END REGION

; Reroll happens using CE compare with relaxed mode.
; CHECK: Function: foo
;
; CHECK:      BEGIN REGION { }
; CHECK:            + DO i1 = 0, 4 * ((3 + sext.i32.i64(%n)) /u 4) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 8>
; CHECK:            |   %1 = i1  *  i1;
; CHECK:            |   %2 = %1  +  %n;
; CHECK:            |   (@B)[0][i1] = %2;
; CHECK:            + END LOOP
; CHECK:      END REGION

; <27>            + DO i64 i1 = 0, (sext.i32.i64(%n) + -1)/u4, 1   <DO_LOOP>  <MAX_TC_EST = 2>
; <27>            | <RVAL-REG> LINEAR i64 (sext.i32.i64(%n) + -1)/u4 {sb:2}
; <27>            |    <BLOB> LINEAR i32 %n {sb:4}
; <27>            |
; <2>             |   %1 = 4 * i1  *  4 * i1;
; <2>             |   <LVAL-REG> NON-LINEAR i64 %1 {sb:5}
; <2>             |   <RVAL-REG> LINEAR i64 4 * i1 {sb:2}
; <2>             |   <RVAL-REG> LINEAR i64 4 * i1 {sb:2}
; <2>             |
; <3>             |   %2 = %1  +  %n;
; <3>             |   <LVAL-REG> NON-LINEAR i64 %2 {sb:6}
; <3>             |   <RVAL-REG> NON-LINEAR i64 %1 {sb:5}
; <3>             |   <RVAL-REG> LINEAR sext.i32.i64(%n) {sb:2}
; <3>             |      <BLOB> LINEAR i32 %n {sb:4}
; <3>             |
; <5>             |   (@B)[0][4 * i1] = %2;
; <5>             |   <LVAL-REG> {al:16}(LINEAR [10 x i64]* @B)[i64 0][LINEAR i64 4 * i1] inbounds  !tbaa !3 {sb:24}
; <5>             |      <BLOB> LINEAR [10 x i64]* @B {sb:8}
; <5>             |   <RVAL-REG> NON-LINEAR i64 %2 {sb:6}
; <5>             |
; <7>             |   %4 = 4 * i1 + 1  *  4 * i1 + 1;
; <7>             |   <LVAL-REG> NON-LINEAR i64 %4 {sb:10}
; <7>             |   <RVAL-REG> LINEAR i64 4 * i1 + 1 {sb:2}
; <7>             |   <RVAL-REG> LINEAR i64 4 * i1 + 1 {sb:2}
; <7>             |
; <8>             |   %5 = %4  +  %n;
; <8>             |   <LVAL-REG> NON-LINEAR i64 %5 {sb:11}
; <8>             |   <RVAL-REG> NON-LINEAR i64 %4 {sb:10}
; <8>             |   <RVAL-REG> LINEAR sext.i32.i64(%n) {sb:2}
; <8>             |      <BLOB> LINEAR i32 %n {sb:4}
; <8>             |
; <10>            |   (@B)[0][4 * i1 + 1] = %5;
; <10>            |   <LVAL-REG> {al:8}(LINEAR [10 x i64]* @B)[i64 0][LINEAR i64 4 * i1 + 1] inbounds  !tbaa !3 {sb:24}
;
; <10>            |      <BLOB> LINEAR [10 x i64]* @B {sb:8}
; <10>            |   <RVAL-REG> NON-LINEAR i64 %5 {sb:11}


;Module Before HIR; ModuleID = 'nn.c'
source_filename = "nn.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = common dso_local local_unnamed_addr global [10 x i64] zeroinitializer, align 16
@A = common dso_local local_unnamed_addr global [10 x i64] zeroinitializer, align 16
@C = common dso_local local_unnamed_addr global [10 x i64] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable writeonly
define dso_local void @foo(i32 %n) local_unnamed_addr #0 {
entry:
  %cmp51 = icmp sgt i32 %n, 0
  br i1 %cmp51, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %0 = sext i32 %n to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.body
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %1 = mul nsw i64 %indvars.iv, %indvars.iv
  %2 = add nsw i64 %1, %0
  %arrayidx = getelementptr inbounds [10 x i64], ptr @B, i64 0, i64 %indvars.iv
  store i64 %2, ptr %arrayidx, align 16
  %3 = or i64 %indvars.iv, 1
  %4 = mul nsw i64 %3, %3
  %5 = add nsw i64 %4, %0
  %arrayidx9 = getelementptr inbounds [10 x i64], ptr @B, i64 0, i64 %3
  store i64 %5, ptr %arrayidx9, align 8
  %6 = or i64 %indvars.iv, 2
  %7 = mul nsw i64 %6, %6
  %8 = add nsw i64 %7, %0
  %arrayidx17 = getelementptr inbounds [10 x i64], ptr @B, i64 0, i64 %6
  store i64 %8, ptr %arrayidx17, align 16
  %9 = or i64 %indvars.iv, 3
  %10 = mul nsw i64 %9, %9
  %11 = add nsw i64 %10, %0
  %arrayidx25 = getelementptr inbounds [10 x i64], ptr @B, i64 0, i64 %9
  store i64 %11, ptr %arrayidx25, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 4
  %cmp = icmp slt i64 %indvars.iv.next, %0
  br i1 %cmp, label %for.body, label %for.cond.cleanup.loopexit
}



