; RUN: opt -hir-ssa-deconstruction -hir-prefetching -hir-prefetching-num-cachelines-threshold=1 -hir-prefetching-skip-non-modified-regions=false -hir-prefetching-num-memory-streams-threshold=1 -hir-prefetching-skip-AVX2-check=true -hir-details -print-after=hir-prefetching 2>&1 < %s | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-prefetching" -hir-prefetching-num-cachelines-threshold=1 -hir-prefetching-skip-non-modified-regions=false -hir-prefetching-num-memory-streams-threshold=1 -hir-prefetching-skip-AVX2-check=true -hir-details -print-after=hir-prefetching 2>&1 < %s | FileCheck %s

; Verify that we are able to conclude that (%A)[i1] with the IV truncation from
; i64 to i32 is linear due to LEGAL_MAX_TC of the loop which fits in i32.


; CHECK: + DO i64 i1 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>   <LEGAL_MAX_TC = 2147483647>
; CHECK: |   (%A)[i1] = i1;
; CHECK: |   <LVAL-REG> {al:4}(LINEAR i32* %A)[LINEAR trunc.i64.i32(i1)]
; CHECK: |   @llvm.prefetch.p0i8(&((i8*)(%A)[i1 + 140]),  0,  3,  1);
; CHECK: + END LOOP


target datalayout = "e-m:e-p:32:32-i64:64-f80:128-n8:16:32:64-S128"

define dso_local void @foo(i32 %n, i32* %A) {
entry:
  %wide.n = sext i32 %n to i64
  %cmp = icmp sgt i32 %n, 0
  br i1 %cmp, label %for.body.pre, label %exit

for.body.pre:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %for.body.pre ], [ %indvars.iv.next, %for.body ]
  %0 = trunc i64 %indvars.iv to i32
  %arrayidx = getelementptr inbounds i32, i32* %A, i32 %0
  store i32 %0, i32* %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp sge i64 %indvars.iv.next, %wide.n
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  br label %exit

exit:
  ret void
}

