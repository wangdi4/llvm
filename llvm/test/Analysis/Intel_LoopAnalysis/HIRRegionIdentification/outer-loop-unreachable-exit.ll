; RUN: opt < %s -passes='print<hir-region-identification>' -disable-output 2>&1 | FileCheck %s
; Check that outer multi-exit loops can form HIR region if all early exits are "unreachable".

; CHECK: EntryBB: %for.outer.us

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(ptr nocapture noundef %a, i32 noundef %n, ptr nocapture noundef %p) {
entry:
  %cmp28 = icmp sgt i32 %n, 0
  br i1 %cmp28, label %for.outer.us.preheader, label %for.outer.cleanup

for.outer.us.preheader:
  br label %for.outer.us

for.outer.us:
  %indvars.iv30 = phi i32 [ 0, %for.outer.us.preheader ], [ %indvars.iv.next31, %for.outer.inc.us ]
  %arrayidx.us = getelementptr inbounds i32, ptr %a, i32 %indvars.iv30
  %0 = load i32, ptr %arrayidx.us, align 4
  switch i32 %0, label %sw.default [
    i32 1, label %sw.bb.us
    i32 2, label %for.inner.us.preheader
  ]

sw.bb.us:
  store i32 2, ptr %arrayidx.us, align 4
  br label %for.outer.inc.us

for.inner.us.preheader:
  br label %for.inner.us

for.inner.us:
  %indvars.iv = phi i32 [ %indvars.iv.next, %for.inner.us ], [ 0, %for.inner.us.preheader ]
  %arrayidx9.us = getelementptr inbounds float, ptr %p, i32 %indvars.iv
  %1 = load float, ptr %arrayidx9.us, align 4
  %add10.us = fadd fast float %1, 1.000000e+00
  store float %add10.us, ptr %arrayidx9.us, align 4
  %indvars.iv.next = add nuw nsw i32 %indvars.iv, 1
  %exitcond.not = icmp eq i32 %indvars.iv.next, %n
  br i1 %exitcond.not, label %for.inner.us.loopexit, label %for.inner.us

for.inner.us.loopexit:
  br label %for.outer.inc.us

for.outer.inc.us:
  %indvars.iv.next31 = add nuw nsw i32 %indvars.iv30, 1
  %exitcond33.not = icmp eq i32 %indvars.iv.next31, %n
  br i1 %exitcond33.not, label %for.outer.cleanup.loopexit, label %for.outer.us

for.outer.cleanup.loopexit:
  br label %for.outer.cleanup

for.outer.cleanup:
  ret void

sw.default:
  unreachable
}
