; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; Check parsing output for the loop verifying that the index of the load is merged properly.

; CHECK: DO i1 = 0, %i2lc + -1
; CHECK-NEXT: %0 = (%in)[undef * i1 + sext.i32.i64(undef)]
; CHECK-NEXT: END LOOP

; ModuleID = 'bugpoint-reduced-simplified.bc'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @sum_power_2u_evenx(i32 %i2lc, float* nocapture readonly %in) {
entry:
  br i1 undef, label %for.body.lr.ph, label %for.cond.161.preheader

for.body.lr.ph:                                   ; preds = %entry
  %idxprom55 = sext i32 undef to i64
  br label %for.body

for.cond.161.preheader.loopexit:                  ; preds = %for.body
  br label %for.cond.161.preheader

for.cond.161.preheader:                           ; preds = %for.cond.161.preheader.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %i2.0750 = phi i32 [ 0, %for.body.lr.ph ], [ %inc159, %for.body ]
  %in.addr.0746 = phi float* [ %in, %for.body.lr.ph ], [ %add.ptr, %for.body ]
  %arrayidx56 = getelementptr inbounds float, float* %in.addr.0746, i64 %idxprom55
  %0 = load float, float* %arrayidx56, align 4
  %add.ptr = getelementptr inbounds float, float* %in.addr.0746, i64 undef
  %inc159 = add nuw nsw i32 %i2.0750, 1
  %exitcond788 = icmp eq i32 %inc159, %i2lc
  br i1 %exitcond788, label %for.cond.161.preheader.loopexit, label %for.body
}

