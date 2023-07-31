; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-vec-dir-insert,hir-vplan-vec" -disable-output < %s

; This test used to crash in VPlanVLSAnalysisHIR.

target triple = "x86_64-unknown-linux-gnu"

define void @foo(ptr noalias %p, ptr noalias %q) {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret void

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds i1, ptr %q, i64 %indvars.iv
  %0 = load i1, ptr %arrayidx, align 4
  %xor = xor i1 %0, 1
  %arrayidx2 = getelementptr inbounds i1, ptr %p, i64 %indvars.iv
  store i1 %xor, ptr %arrayidx2, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 64
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

