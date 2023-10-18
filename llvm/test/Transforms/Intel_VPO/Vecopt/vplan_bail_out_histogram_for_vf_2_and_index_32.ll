; VConflict instruction is not available for all VFs. One of these cases is when
; VF =2 and the index is 32 bits.

target datalayout = "e-m:e-p:32:32-p270:32:32-p271:32:32-p272:64:64-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

; REQUIRES: asserts
; RUN: opt -S -mattr=+avx512vl,+avx512cd -passes='hir-ssa-deconstruction,hir-temp-cleanup,hir-vec-dir-insert,hir-vplan-vec' -vplan-force-vf=2 -debug-only=LoopVectorizationPlanner -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -S -mattr=+avx512vl,+avx512cd -passes=hir-ssa-deconstruction,hir-temp-cleanup,hir-vec-dir-insert,hir-vplan-vec,hir-cg,simplifycfg,intel-ir-optreport-emitter -vplan-force-vf=2 -disable-output -intel-opt-report=high < %s 2>&1 | FileCheck %s --check-prefix=OPTRPTHI

; Function Attrs: nofree norecurse nosync nounwind mustprogress
define dso_local void @_Z3fooPiS_i(ptr noalias nocapture %A, ptr noalias nocapture readonly %B, i32 %TC) local_unnamed_addr #0 {
entry:
; CHECK: No vectorization factor was found that can satisfy all VConflict idioms in the loop.
; OPTRPTHI: remark #15436: loop was not vectorized: No vectorization factor was found that can satisfy all VConflict idioms in the loop.
;
  %cmp9 = icmp sgt i32 %TC, 0
  br i1 %cmp9, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.body
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body.preheader, %for.body
  %i.010 = phi i32 [ %inc, %for.body ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr inbounds i32, ptr %B, i32 %i.010
  %0 = load i32, ptr %arrayidx, align 4
  %arrayidx1 = getelementptr inbounds i32, ptr %A, i32 %0
  %1 = load i32, ptr %arrayidx1, align 4
  %add = add nsw i32 %1, 100
  store i32 %add, ptr %arrayidx1, align 4
  %inc = add nuw nsw i32 %i.010, 1
  %exitcond.not = icmp eq i32 %inc, %TC
  br i1 %exitcond.not, label %for.cond.cleanup.loopexit, label %for.body
}

attributes #0 = { nofree norecurse nosync nounwind mustprogress "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" }
