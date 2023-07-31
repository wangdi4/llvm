target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; REQUIRES: asserts
; RUN: opt %s -S -mattr=+avx512vl,+avx512cd -passes='hir-ssa-deconstruction,hir-temp-cleanup,hir-vec-dir-insert,hir-vplan-vec' -debug-only=parvec-analysis,VPlanHCFGBuilder 2>&1 | FileCheck %s

; BEGIN REGION { }
;       + DO i1 = 0, 1023, 1   <DO_LOOP>
;       |   %0 = (%B)[i1];
;       |   %flt = sitofp.i64.float(i1);
;       |   %1 = (%A)[%0];
;       |   (%A)[%0] = %flt;
;       + END LOOP
; END REGION

; CHECK:      [VConflict Idiom] Looking at store candidate:<[[NUM1:[0-9]+]]>          (%A)[%0] = %flt;
; CHECK-NEXT: [VConflict Idiom] Depends(WAR) on:<[[NUM2:[0-9]+]]>          %1 = (%A)[%0];
; CHECK-NEXT: [VConflict Idiom] Detected, legality pending further dependence checking!
; CHECK-NEXT: Idiom List
; CHECK-NEXT: VConflictLikeStore: <[[NUM1]]>          (%A)[%0] = %flt;

; CHECK:      VConflict load's use-chain escapes the region.
; CHECK-NEXT: The current VConflict idiom is not supported.

; Function Attrs: nofree norecurse nounwind uwtable mustprogress
define dso_local float @_Z4foo1PfPi(ptr noalias nocapture %A, ptr noalias nocapture readonly %B) local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret float %1

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %ptridx = getelementptr inbounds i32, ptr %B, i64 %indvars.iv
  %0 = load i32, ptr %ptridx, align 4
  %flt = sitofp i64 %indvars.iv to float
  %idxprom1 = sext i32 %0 to i64
  %ptridx2 = getelementptr inbounds float, ptr %A, i64 %idxprom1
  %1 = load float, ptr %ptridx2, align 4
  store float %flt, ptr %ptridx2, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body
}

attributes #0 = { nofree norecurse nounwind uwtable mustprogress "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" "use-soft-float"="false" }

