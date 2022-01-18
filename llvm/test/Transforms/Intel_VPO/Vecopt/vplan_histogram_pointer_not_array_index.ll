target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; REQUIRES: asserts
; RUN: opt -S -mattr=+avx512vl,+avx512cd -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -hir-vplan-vec -debug-only=parvec-analysis -disable-vplan-codegen -disable-output < %s 2>&1 | FileCheck %s

; <13>         + DO i1 = 0, zext.i32.i64(%TC) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
; <3>          |   %0 = (%B)[i1];
; <4>          |   %1 = (%0)[0];
; <6>          |   (%0)[0] = %1 + 100;
; <13>         + END LOOP

; CHECK: [VConflict Idiom] Skipped: Non-linear base address is not supported.

; Function Attrs: nofree norecurse nosync nounwind uwtable mustprogress
define dso_local void @foo(i32** noalias nocapture readonly %B, i32 %TC) local_unnamed_addr #0 {
entry:
  %cmp6 = icmp sgt i32 %TC, 0
  br i1 %cmp6, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %wide.trip.count8 = zext i32 %TC to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.body
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds i32*, i32** %B, i64 %indvars.iv
  %0 = load i32*, i32** %arrayidx, align 8
  %1 = load i32, i32* %0, align 4
  %add = add nsw i32 %1, 100
  store i32 %add, i32* %0, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count8
  br i1 %exitcond.not, label %for.cond.cleanup.loopexit, label %for.body
}

attributes #0 = { nofree norecurse nosync nounwind uwtable mustprogress "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" }
