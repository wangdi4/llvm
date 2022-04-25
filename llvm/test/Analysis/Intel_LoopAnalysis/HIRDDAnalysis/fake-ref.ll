; RUN: opt -hir-ssa-deconstruction -analyze -enable-new-pm=0 -hir-dd-analysis -hir-dd-analysis-verify=Region -hir-cost-model-throttling=0 < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,print<hir-dd-analysis>" -hir-dd-analysis-verify=Region -hir-cost-model-throttling=0 -disable-output < %s 2>&1 | FileCheck %s

; Test checks that DD analysis does not refine (*) edges between real and fake DDRefs.

; CHECK: (%A)[0] --> (%A)[5] FLOW (*)

; <11>               + DO i1 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 4294967295>
; <2>                |   @bar(&((%A)[0]));
; <4>                |   %ld = (%A)[5];
; <11>               + END LOOP
; <0>          END REGION


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(i32* %A, i32 %n) {
entry:
  %cmp.4 = icmp sgt i32 %n, 0
  br i1 %cmp.4, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %i.05 = phi i32 [ %inc, %for.body ], [ 0, %for.body.preheader ]
  call void @bar(i32* %A)
  %gep = getelementptr inbounds i32, i32* %A, i64 5
  %ld = load i32, i32* %gep, align 4
  %inc = add nuw nsw i32 %i.05, 1
  %exitcond = icmp eq i32 %inc, %n
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  %ld.lcssa = phi i32 [ %ld, %for.body ]
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

declare void @bar(i32*)

