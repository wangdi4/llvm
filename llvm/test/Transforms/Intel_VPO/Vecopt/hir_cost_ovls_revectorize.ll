; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; RUN: opt < %s -disable-output -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -hir-vplan-vec -mattr=+sse4.2 -enable-intel-advanced-opts -vplan-cost-model-print-analysis-for-vf=4 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; The test is to kick in OVLS Cost Modelling code during re-vectorization.
; It seg.faulted before the fix for cmplrllvm32438.

; We check for presence of *OVLS* marker in cost model debug dumps, which
; means that VLS group is built and CM evaluates its cost vs emitting plain
; loads/stores. We do not require the test to be vectorized, all we need is
; CM to evaluate VLS groups built on top of vector types.

; CHECK: *OVLS*

define void @foo(float* noalias %a, float* noalias %c, i64 %length) {
entry:
  br label %for.body.preheader

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body, %for.body.preheader
  %i.053 = phi i64 [ %add28, %for.body ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr inbounds float, float* %a, i64 %i.053
  %0 = bitcast float* %arrayidx to <4 x float>*
  %1 = load <4 x float>, <4 x float>* %0, align 16
  %add11 = or i64 %i.053, 4
  %arrayidx12 = getelementptr inbounds float, float* %a, i64 %add11
  %2 = bitcast float* %arrayidx12 to <4 x float>*
  %3 = load <4 x float>, <4 x float>* %2, align 16
  %add14 = or i64 %i.053, 8
  %arrayidx15 = getelementptr inbounds float, float* %a, i64 %add14
  %4 = bitcast float* %arrayidx15 to <4 x float>*
  %5 = load <4 x float>, <4 x float>* %4, align 16
  %add17 = or i64 %i.053, 12
  %arrayidx18 = getelementptr inbounds float, float* %a, i64 %add17
  %6 = bitcast float* %arrayidx18 to <4 x float>*
  %7 = load <4 x float>, <4 x float>* %6, align 16
  %arrayidx21 = getelementptr inbounds float, float* %c, i64 %i.053
  %8 = bitcast float* %arrayidx21 to <4 x float>*
  store <4 x float> %1, <4 x float>* %8, align 16
  %arrayidx23 = getelementptr inbounds float, float* %c, i64 %add11
  %9 = bitcast float* %arrayidx23 to <4 x float>*
  store <4 x float> %3, <4 x float>* %9, align 16
  %arrayidx25 = getelementptr inbounds float, float* %c, i64 %add14
  %10 = bitcast float* %arrayidx25 to <4 x float>*
  store <4 x float> %5, <4 x float>* %10, align 16
  %arrayidx27 = getelementptr inbounds float, float* %c, i64 %add17
  %11 = bitcast float* %arrayidx27 to <4 x float>*
  store <4 x float> %7, <4 x float>* %11, align 16
  %add28 = add nuw i64 %i.053, 16
  %cmp10 = icmp ult i64 %add28, %length
  br i1 %cmp10, label %for.body, label %for.exit

for.exit:                                 ; preds = %for.body
  ret void
}
; end INTEL_FEATURE_SW_ADVANCED
