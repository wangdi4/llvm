; REQUIRES: asserts

; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-post-vec-complete-unroll" -debug-only=hir-complete-unroll -disable-hir-create-fusion-regions 2>&1 < %s | FileCheck %s

; Verify that load of (@A)[0][i1] in second loop is considered to be optimized
; away after unrolling due to identical store in previous loop. Also verify
; that stores are not considered to be optimized away.


; CHECK: Number of memrefs which can be eliminated: 0

; CHECK: + DO i1 = 0, 9, 1   <DO_LOOP>
; CHECK: |   (@A)[0][i1] = i1;
; CHECK: + END LOOP


; CHECK: Number of memrefs which can be eliminated: 10

; CHECK: + DO i1 = 0, 9, 1   <DO_LOOP>
; CHECK: |   (%dst)[0][i1] = (@A)[0][i1];
; CHECK: + END LOOP


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common local_unnamed_addr global [100 x i32] zeroinitializer, align 16

; Function Attrs: nounwind readonly uwtable
define i32 @foo(ptr %dst) local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv30 = phi i64 [ 0, %entry ], [ %indvars.iv.next31, %for.body ]
  %arrayidx = getelementptr inbounds [100 x i32], ptr @A, i64 0, i64 %indvars.iv30
  %trunc = trunc i64 %indvars.iv30 to i32
  store i32 %trunc, ptr %arrayidx, align 4
  %indvars.iv.next31 = add nuw nsw i64 %indvars.iv30, 1
  %exitcond32 = icmp eq i64 %indvars.iv.next31, 10
  br i1 %exitcond32, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  br label %for.body5

for.body5:                                        ; preds = %for.body5, %for.end
  %indvars.iv = phi i64 [ 0, %for.end ], [ %indvars.iv.next, %for.body5 ]
  %arrayidx1 = getelementptr inbounds [100 x i32], ptr @A, i64 0, i64 %indvars.iv
  %ld = load i32, ptr %arrayidx1, align 4
  %arrayidx2 = getelementptr inbounds [100 x i32], ptr %dst, i64 0, i64 %indvars.iv
  store i32 %ld, ptr %arrayidx2, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 10
  br i1 %exitcond, label %for.end15, label %for.body5

for.end15:                                        ; preds = %for.body5
  ret i32 0
}

