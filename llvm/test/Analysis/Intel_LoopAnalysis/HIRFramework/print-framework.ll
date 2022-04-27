; Check that -analyze -enable-new-pm=0 prints framework info and -hir-framework-details turns on/off framework info in regular print.

; RUN: opt -hir-ssa-deconstruction -analyze -hir-framework -enable-new-pm=0 < %s 2>&1 | FileCheck %s
; RUN: opt -hir-ssa-deconstruction -hir-post-vec-complete-unroll -print-after=hir-post-vec-complete-unroll -hir-framework-details -S < %s 2>&1 | FileCheck %s

; RUN: opt -passes="hir-ssa-deconstruction,print<hir-framework>" -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-post-vec-complete-unroll,print<hir>" -hir-framework-details -disable-output < %s 2>&1 | FileCheck %s

; CHECK: BEGIN REGION
; CHECK: EntryBB
; CHECK: DO i1 = 0

; RUN: opt < %s -hir-ssa-deconstruction -hir-post-vec-complete-unroll -print-after=hir-post-vec-complete-unroll 2>&1 -S | FileCheck --check-prefix=REG-CHECK %s

; REG-CHECK: BEGIN REGION
; REG-CHECK-NOT: EntryBB
; REG-CHECK: DO i1 = 0

; ModuleID = 'print-framework.ll'
source_filename = "print.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define i32 @foo(i32* nocapture %p, i32 %n) local_unnamed_addr #0 {
entry:
  %cmp7 = icmp sgt i32 %n, 0
  br i1 %cmp7, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr inbounds i32, i32* %p, i64 %indvars.iv
  %0 = trunc i64 %indvars.iv to i32
  store i32 %0, i32* %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %n
  %indvars.iv.in = bitcast i64 %indvars.iv.next to i64
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  %1 = load i32, i32* %p, align 4
  ret i32 %1
}

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

