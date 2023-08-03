; RUN: opt < %s -passes="hir-ssa-deconstruction,hir-temp-cleanup" -print-before=hir-temp-cleanup -print-after=hir-temp-cleanup 2>&1 | FileCheck %s

; Verify that we are able to forward substitute loads in the presence of assume
; call which has inaccessiblememonly attribute.

; CHECK: Dump Before

; CHECK: + DO i1 = 0, sext.i32.i64((-1 + %n)), 1   <DO_LOOP>
; CHECK: |   %0 = (%B)[i1];
; CHECK: |   %1 = (%A)[i1];
; CHECK: |   @llvm.assume(-1);
; CHECK: |   %add = %0  +  %1;
; CHECK: |   (%A)[i1] = %add;
; CHECK: + END LOOP

; CHECK: Dump After

; CHECK: + DO i1 = 0, sext.i32.i64((-1 + %n)), 1   <DO_LOOP>
; CHECK: |   @llvm.assume(-1);
; CHECK: |   %add = (%B)[i1]  +  (%A)[i1];
; CHECK: |   (%A)[i1] = %add;
; CHECK: + END LOOP


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(ptr nocapture %A, ptr nocapture readonly %B, i32 %n) {
entry:
  %cmp.7 = icmp sgt i32 %n, 0
  br i1 %cmp.7, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr inbounds float, ptr %B, i64 %indvars.iv
  %0 = load float, ptr %arrayidx, align 4
  %arrayidx2 = getelementptr inbounds float, ptr %A, i64 %indvars.iv
  %1 = load float, ptr %arrayidx2, align 4
  call void @llvm.assume(i1 true)
  %add = fadd float %0, %1
  store float %add, ptr %arrayidx2, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %n
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

declare void @llvm.assume(i1 noundef) #1

attributes #1 = { inaccessiblememonly }

