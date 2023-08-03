; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced, intel_feature_cpu_ryl
; RUN: opt < %s -passes='hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec' \
; RUN:     -mtriple=x86_64-unknown-unknown -mcpu=royal \
; RUN:     -disable-output -vplan-cost-model-print-analysis-for-vf=8 \
; RUN:     -vplan-force-vf=8 \
; RUN:     | FileCheck %s

; Checks that the -march setting enables VPlan unrolling heuristics and cost modelling
; by checking for cost adjustments due to the unroll heuristics.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define float @single_redn(ptr nocapture noundef readonly %A, ptr nocapture noundef readonly %B, i32 noundef %n) local_unnamed_addr #0 {
;
; CHECK-LABEL:  Cost Model for VPlan single_redn:HIR.#{{[0-9]+}} with VF = 8:
; CHECK:  Cost decrease due to Unroll heuristic is
entry:
  %cmp.not7 = icmp eq i32 %n, 0
  br i1 %cmp.not7, label %for.cond.cleanup, label %for.body.preheader

for.body.preheader:
  %0 = zext i32 %n to i64
  br label %for.body

for.cond.cleanup.loopexit:
  %add.lcssa = phi float [ %add, %for.body ]
  br label %for.cond.cleanup

for.cond.cleanup:
  %acc.0.lcssa = phi float [ 0.000000e+00, %entry ], [ %add.lcssa, %for.cond.cleanup.loopexit ]
  ret float %acc.0.lcssa

for.body:
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %acc.08 = phi float [ 0.000000e+00, %for.body.preheader ], [ %add, %for.body ]
  %arrayidx = getelementptr inbounds float, ptr %A, i64 %indvars.iv
  %1 = load float, ptr %arrayidx, align 4
  %arrayidx2 = getelementptr inbounds float, ptr %B, i64 %indvars.iv
  %2 = load float, ptr %arrayidx2, align 4
  %mul = fmul fast float %2, %1
  %add = fadd fast float %mul, %acc.08
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %cmp.not = icmp eq i64 %indvars.iv.next, %0
  br i1 %cmp.not, label %for.cond.cleanup.loopexit, label %for.body
}
; end INTEL_FEATURE_SW_ADVANCED
