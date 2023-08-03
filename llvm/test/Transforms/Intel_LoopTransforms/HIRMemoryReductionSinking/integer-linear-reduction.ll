; RUN: opt -aa-pipeline="basic-aa" -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir-framework>,hir-memory-reduction-sinking,print<hir-framework>" 2>&1 < %s | FileCheck %s

; Verify that we are able to sink invariant reductions in the presence of dependencies to linear reduction.

; Dump Before-

; CHECK: + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK: |   %0 = (@A)[0][i1];
; CHECK: |   (@A)[0][i1] = %0 + 2;
; CHECK: |   %1 = (@A)[0][5];
; CHECK: |   (@A)[0][5] = %1 + 3;
; CHECK: |   %2 = (@A)[0][8];
; CHECK: |   (@A)[0][8] = %2 + 9;
; CHECK: + END LOOP

; Dump After

; CHECK: modified

; CHECK:   %tmp = 0;
; CHECK:   %tmp4 = 0;
; CHECK: + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK: |   %0 = (@A)[0][i1];
; CHECK: |   (@A)[0][i1] = %0 + 2;
; CHECK: |   %tmp4 = %tmp4  +  3;
; CHECK: |   %tmp = %tmp  +  9;
; CHECK: + END LOOP
; CHECK:   %1 = (@A)[0][5];
; CHECK:   (@A)[0][5] = %1 + %tmp4;
; CHECK:   %2 = (@A)[0][8];
; CHECK:   (@A)[0][8] = %2 + %tmp;


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16

; Function Attrs: nofree norecurse nounwind uwtable
define dso_local void @foo() local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds [100 x i32], ptr @A, i64 0, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx, align 4
  %add = add nsw i32 %0, 2
  store i32 %add, ptr %arrayidx, align 4
  %1 = load i32, ptr getelementptr inbounds ([100 x i32], ptr @A, i64 0, i64 5), align 4
  %add3 = add nsw i32 %1, 3
  store i32 %add3, ptr getelementptr inbounds ([100 x i32], ptr @A, i64 0, i64 5), align 4
  %2 = load i32, ptr getelementptr inbounds ([100 x i32], ptr @A, i64 0, i64 8), align 4
  %add4 = add nsw i32 %2, 9
  store i32 %add4, ptr getelementptr inbounds ([100 x i32], ptr @A, i64 0, i64 8), align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

