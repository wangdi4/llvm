; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; RUN: opt < %s -disable-output -passes='hir-ssa-deconstruction,hir-temp-cleanup,hir-vec-dir-insert,hir-vplan-vec' -mattr=+sse4.2 -enable-intel-advanced-opts -vplan-cost-model-print-analysis-for-vf=4 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; The test is to kick in OVLS Cost Modelling code during re-vectorization.
; It seg.faulted before the fix for cmplrllvm32438.

; We check for presence of *OVLS* marker in cost model debug dumps, which
; means that VLS group is built and CM evaluates its cost vs emitting plain
; loads/stores. We do not require the test to be vectorized, all we need is
; CM to evaluate VLS groups built on top of vector types.

; CHECK: *OVLS*

define void @foo(ptr noalias %a, ptr noalias %c, i64 %length) {
entry:
  br label %for.body.preheader

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body, %for.body.preheader
  %i.053 = phi i64 [ %add28, %for.body ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr inbounds float, ptr %a, i64 %i.053
  %0 = load <4 x float>, ptr %arrayidx, align 16
  %add11 = or i64 %i.053, 4
  %arrayidx12 = getelementptr inbounds float, ptr %a, i64 %add11
  %1 = load <4 x float>, ptr %arrayidx12, align 16
  %add14 = or i64 %i.053, 8
  %arrayidx15 = getelementptr inbounds float, ptr %a, i64 %add14
  %2 = load <4 x float>, ptr %arrayidx15, align 16
  %add17 = or i64 %i.053, 12
  %arrayidx18 = getelementptr inbounds float, ptr %a, i64 %add17
  %3 = load <4 x float>, ptr %arrayidx18, align 16
  %arrayidx21 = getelementptr inbounds float, ptr %c, i64 %i.053
  store <4 x float> %0, ptr %arrayidx21, align 16
  %arrayidx23 = getelementptr inbounds float, ptr %c, i64 %add11
  store <4 x float> %1, ptr %arrayidx23, align 16
  %arrayidx25 = getelementptr inbounds float, ptr %c, i64 %add14
  store <4 x float> %2, ptr %arrayidx25, align 16
  %arrayidx27 = getelementptr inbounds float, ptr %c, i64 %add17
  store <4 x float> %3, ptr %arrayidx27, align 16
  %add28 = add nuw i64 %i.053, 16
  %cmp10 = icmp ult i64 %add28, %length
  br i1 %cmp10, label %for.body, label %for.exit

for.exit:                                 ; preds = %for.body
  ret void
}
; end INTEL_FEATURE_SW_ADVANCED
