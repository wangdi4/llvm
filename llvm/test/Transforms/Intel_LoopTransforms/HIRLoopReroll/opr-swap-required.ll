; RUN: opt -disable-output -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-loop-reroll,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s

; Considering swaping of two addends in the RHS is required to get the following code unrolled.
; Even after sorting in the order of increasing Blob indices,
; blob indices B(%0) < B(%n) < B(%3) requires swaping of operands in comparing (%0 + %n) and (%n + %3)

; #define SIZE 1000
; long long A[SIZE];
; long long B[SIZE];
; long long C[SIZE];
; long long D[SIZE];
;
; void foo(long long n) {
;
;   for (int i=0;  i<SIZE; i = i + 2) {
;     A[i] = B[i] + n;
;     C[i] = D[i] + n;
;     A[i+1] = B[i+1] + n;
;     C[i+1] = D[i+1] + n;
;   }
; }

; CHECK: Function: foo
;
; CHECK:       BEGIN REGION { }
; CHECK:             + DO i1 = 0, 499, 1   <DO_LOOP>
; CHECK:             |   %0 = (@B)[0][2 * i1];
; CHECK:             |   (@A)[0][2 * i1] = %0 + %n;
; CHECK:             |   %1 = (@D)[0][2 * i1];
; CHECK:             |   (@C)[0][2 * i1] = %n + %1;
; CHECK:             |   %3 = (@B)[0][2 * i1 + 1];
; CHECK:             |   (@A)[0][2 * i1 + 1] = %n + %3;
; CHECK:             |   %4 = (@D)[0][2 * i1 + 1];
; CHECK:             |   (@C)[0][2 * i1 + 1] = %n + %4;
; CHECK:             + END LOOP
; CHECK:       END REGION

; Rerolled

; CHECK: Function: foo
;
; CHECK:       BEGIN REGION { }
; CHECK:             + DO i1 = 0, 999, 1   <DO_LOOP>
; CHECK:             |   %0 = (@B)[0][i1];
; CHECK:             |   (@A)[0][i1] = %0 + %n;
; CHECK:             |   %1 = (@D)[0][i1];
; CHECK:             |   (@C)[0][i1] = %n + %1;
; CHECK:             + END LOOP
; CHECK:       END REGION


;Module Before HIR; ModuleID = 'two-chains-ver2.c'
source_filename = "two-chains-ver2.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = common dso_local local_unnamed_addr global [1000 x i64] zeroinitializer, align 16
@A = common dso_local local_unnamed_addr global [1000 x i64] zeroinitializer, align 16
@D = common dso_local local_unnamed_addr global [1000 x i64] zeroinitializer, align 16
@C = common dso_local local_unnamed_addr global [1000 x i64] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define dso_local void @foo(i64 %n) local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret void

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds [1000 x i64], ptr @B, i64 0, i64 %indvars.iv
  %0 = load i64, ptr %arrayidx, align 16
  %add = add nsw i64 %0, %n
  %arrayidx2 = getelementptr inbounds [1000 x i64], ptr @A, i64 0, i64 %indvars.iv
  store i64 %add, ptr %arrayidx2, align 16
  %arrayidx4 = getelementptr inbounds [1000 x i64], ptr @D, i64 0, i64 %indvars.iv
  %1 = load i64, ptr %arrayidx4, align 16
  %add5 = add nsw i64 %1, %n
  %arrayidx7 = getelementptr inbounds [1000 x i64], ptr @C, i64 0, i64 %indvars.iv
  store i64 %add5, ptr %arrayidx7, align 16
  %2 = or i64 %indvars.iv, 1
  %arrayidx10 = getelementptr inbounds [1000 x i64], ptr @B, i64 0, i64 %2
  %3 = load i64, ptr %arrayidx10, align 8
  %add11 = add nsw i64 %3, %n
  %arrayidx14 = getelementptr inbounds [1000 x i64], ptr @A, i64 0, i64 %2
  store i64 %add11, ptr %arrayidx14, align 8
  %arrayidx17 = getelementptr inbounds [1000 x i64], ptr @D, i64 0, i64 %2
  %4 = load i64, ptr %arrayidx17, align 8
  %add18 = add nsw i64 %4, %n
  %arrayidx21 = getelementptr inbounds [1000 x i64], ptr @C, i64 0, i64 %2
  store i64 %add18, ptr %arrayidx21, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 2
  %cmp = icmp ult i64 %indvars.iv.next, 1000
  br i1 %cmp, label %for.body, label %for.cond.cleanup
}



