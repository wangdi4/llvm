; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced

; RUN: opt < %s -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-vec-dir-insert,hir-vplan-vec" -xmain-opt-level=3 -disable-output -vplan-cost-model-print-analysis-for-vf=4 -mattr=+sse4.2 -vplan-force-vf=4 -vplan-force-uf=8 -enable-intel-advanced-opts -vplan-cm-unroll=true -vplan-enable-partial-sums=true -vplan-cm-unroll-ilp-score=1 | FileCheck %s

; Regression test for CMPLRLLVM-49253. Check that in a loop with partial sum candidate
; reductions that total loop costs do not become negative. This condition happened in the
; original case when the reduction(s) use loads in OVLS groups. 
; The checks confirm that the OVLS groups are recognized, the unroll heuristic reduces
; the cost, and the result is non-negative.

; CHECK: Cost {{[0-9]*}} for i32 {{.*}} = load ptr {{.*}} *OVLS*({{.*}}) AdjCost: {{[1-9][0-9]*}}
; CHECK: Cost {{[0-9]*}} for i32 {{.*}} = load ptr {{.*}} *OVLS*({{.*}}) AdjCost: 0
; CHECK: Cost {{[0-9]*}} for i32 {{.*}} = load ptr {{.*}} *OVLS*({{.*}}) AdjCost: 0
; CHECK: Cost {{[0-9]*}} for i32 {{.*}} = load ptr {{.*}} *OVLS*({{.*}}) AdjCost: 0
; CHECK: Cost decrease due to Unroll heuristic
; CHECK: Total Cost: {{[^-]}}

target triple = "x86_64-unknown-linux-gnu"

@a = dso_local local_unnamed_addr global [4096 x i32] zeroinitializer, align 64

define dso_local i32 @foo() local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %acc.08 = phi i32 [ 0, %entry ], [ %add, %for.body ]
  %arrayidx = getelementptr inbounds [4096 x i32], ptr @a, i32 0, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx, align 16
  %1 = or i64 %indvars.iv, 1
  %arrayidx2 = getelementptr inbounds [4096 x i32], ptr @a, i32 0, i64 %1
  %2 = load i32, ptr %arrayidx2, align 8
  %add1 = add nsw i32 %0, %2
  %3 = or i64 %indvars.iv, 2
  %arrayidx4 = getelementptr inbounds [4096 x i32], ptr @a, i32 0, i64 %3
  %4 = load i32, ptr %arrayidx4, align 8
  %5 = or i64 %indvars.iv, 3
  %arrayidx6 = getelementptr inbounds [4096 x i32], ptr @a, i32 0, i64 %5
  %6 = load i32, ptr %arrayidx6, align 8
  %add2 = add nsw i32 %4, %6
  %add3 = add nsw i32 %add1, %acc.08
  %add = add nsw i32 %add2, %add3
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 4
  %cmp = icmp ult i64 %indvars.iv, 4000
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.body
  ret i32 %add
}
; end INTEL_FEATURE_SW_ADVANCED
