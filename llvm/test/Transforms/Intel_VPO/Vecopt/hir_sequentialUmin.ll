; RUN: opt < %s -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -disable-output -vplan-print-after-plain-cfg -vplan-print-after-vpentity-instrs 2>&1 -vplan-enable-new-cfg-merge-hir=0 | FileCheck %s
; RUN: opt < %s -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -disable-output -vplan-print-after-plain-cfg -vplan-print-after-vpentity-instrs 2>&1 -vplan-enable-new-cfg-merge-hir=1 | FileCheck %s
; RUN: opt < %s -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec" -disable-output -vplan-print-after-plain-cfg -vplan-print-after-vpentity-instrs 2>&1 -vplan-enable-new-cfg-merge-hir=0 | FileCheck %s
; RUN: opt < %s -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec" -disable-output -vplan-print-after-plain-cfg -vplan-print-after-vpentity-instrs 2>&1 -vplan-enable-new-cfg-merge-hir=1 | FileCheck %s

; As of now VPlan just support umin_seq SCEV which is recognized as an idiom in
; the input code. VPlan doesn't support its vectorization though.

; CHECK: umin-seq i1 [[VP1:%.*]] i1 [[VP2:%.*]]
; CHECK-NOT: VPlan after insertion of VPEntities instructions

target triple = "x86_64-unknown-linux-gnu"

define void @foo(i8* %p1, i8* %p2) {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc ]
  %arrayidx1 = getelementptr inbounds i8, i8* %p1, i64 %indvars.iv
  %0 = load i8, i8* %arrayidx1, align 1
  %arrayidx2 = getelementptr inbounds i8, i8* %p2, i64 %indvars.iv
  %1 = load i8, i8* %arrayidx2, align 1
  %cmp1 = icmp eq i8 %0, 0
  %cmp2 = icmp eq i8 %1, 0
  %or.cond = select i1 %cmp1, i1 true, i1 %cmp2
  br i1 %or.cond, label %for.end.split.loop.exit, label %for.inc

for.inc:                                          ; preds = %for.body
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond.not, label %for.end.loopexit, label %for.body

for.end.split.loop.exit:                          ; preds = %for.body
  br label %for.end

for.end.loopexit:                                 ; preds = %for.inc
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %for.end.split.loop.exit
  ret void
}
