; RUN: opt -disable-output -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-loop-reroll,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s

; int A[SIZE];
; int B[SIZE];
; int C[SIZE];
;
; void foo(int n) {
;   int D = n*n;
;   int q = 0;
;   for (int i=0;  i<n; i=i+4) {
;
;     B[i] = n + 1;
;
;     B[i+1] = n + 1;
;
;     B[i+2] = n + 1;
;
;     B[i+3] = n + 1;
;   }
;
; }

; CHECK: Function: foo
;
; CHECK:       BEGIN REGION { }
; CHECK:             + DO i1 = 0, (sext.i32.i64(%n) + -1)/u4, 1   <DO_LOOP>  <MAX_TC_EST = 2>
; CHECK:             |   (@B)[0][4 * i1] = %n + 1;
; CHECK:             |   (@B)[0][4 * i1 + 1] = %n + 1;
; CHECK:             |   (@B)[0][4 * i1 + 2] = %n + 1;
; CHECK:             |   (@B)[0][4 * i1 + 3] = %n + 1;
; CHECK:             + END LOOP
; CHECK:       END REGION

; CHECK: Function: foo
;
; CHECK:       BEGIN REGION { }
; CHECK:             + DO i1 = 0, 4 * ((3 + sext.i32.i64(%n)) /u 4) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 8>
; CHECK:             |   (@B)[0][i1] = %n + 1;
; CHECK:             + END LOOP
; CHECK:       END REGION

;Module Before HIR; ModuleID = 'blob-const.c'
source_filename = "blob-const.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = common dso_local local_unnamed_addr global [10 x i32] zeroinitializer, align 16
@A = common dso_local local_unnamed_addr global [10 x i32] zeroinitializer, align 16
@C = common dso_local local_unnamed_addr global [10 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable writeonly
define dso_local void @foo(i32 %n) local_unnamed_addr #0 {
entry:
  %cmp28 = icmp sgt i32 %n, 0
  br i1 %cmp28, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:                                   ; preds = %entry
  %add = add nsw i32 %n, 1
  %0 = sext i32 %n to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.body
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body.lr.ph, %for.body
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds [10 x i32], ptr @B, i64 0, i64 %indvars.iv
  store i32 %add, ptr %arrayidx, align 16
  %1 = or i64 %indvars.iv, 1
  %arrayidx4 = getelementptr inbounds [10 x i32], ptr @B, i64 0, i64 %1
  store i32 %add, ptr %arrayidx4, align 4
  %2 = or i64 %indvars.iv, 2
  %arrayidx8 = getelementptr inbounds [10 x i32], ptr @B, i64 0, i64 %2
  store i32 %add, ptr %arrayidx8, align 8
  %3 = or i64 %indvars.iv, 3
  %arrayidx12 = getelementptr inbounds [10 x i32], ptr @B, i64 0, i64 %3
  store i32 %add, ptr %arrayidx12, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 4
  %cmp = icmp slt i64 %indvars.iv.next, %0
  br i1 %cmp, label %for.body, label %for.cond.cleanup.loopexit
}

