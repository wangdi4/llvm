; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; RUN: opt < %s -hir-ssa-deconstruction -hir-temp-cleanup -xmain-opt-level=3 \
; RUN:     -hir-vec-dir-insert -hir-vplan-vec -disable-output \
; RUN:     -vplan-cost-model-print-analysis-for-vf=4 -mattr=+sse4.2 \
; RUN:     -enable-intel-advanced-opts | FileCheck %s

; RUN: opt < %s -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-vec-dir-insert,hir-vplan-vec" -xmain-opt-level=3 -disable-output -vplan-cost-model-print-analysis-for-vf=4 -mattr=+sse4.2 -enable-intel-advanced-opts | FileCheck %s

; RUN: opt < %s -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-vec-dir-insert,hir-vplan-vec<LightWeight>" -xmain-opt-level=3 -disable-output -vplan-cost-model-print-analysis-for-vf=4 -mattr=+sse4.2 -enable-intel-advanced-opts  | FileCheck %s --check-prefix=LIGHT

; The test checks that OVLS group is detected by CM "OVLS" string in
; the dump) and verifies that the first Load of VLS group receive whole Cost
; of the group, while consequent group members receive Cost 0.

; CHECK: Cost {{[0-9]*}} for i64 {{.*}} = load i64* {{.*}} *OVLS*({{.*}}) AdjCost: 18
; CHECK: Cost {{[0-9]*}} for i64 {{.*}} = load i64* {{.*}} *OVLS*({{.*}}) AdjCost: 0

; OVLS is not a part of lightweight pipeline.
; LIGHT-NOT: Cost {{[0-9]*}} for i64 {{.*}} = load i64* {{.*}} *OVLS*({{.*}})

target triple = "x86_64-unknown-linux-gnu"

@a = dso_local local_unnamed_addr global [1024 x i64] zeroinitializer, align 64
@b = dso_local local_unnamed_addr global [1024 x i64] zeroinitializer, align 64

define dso_local void @foo() local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds [1024 x i64], [1024 x i64]* @a, i64 0, i64 %indvars.iv
  %0 = load i64, i64* %arrayidx, align 16
  %1 = or i64 %indvars.iv, 1
  %arrayidx2 = getelementptr inbounds [1024 x i64], [1024 x i64]* @a, i64 0, i64 %1
  %2 = load i64, i64* %arrayidx2, align 8
  %add3 = add nsw i64 %2, %0
  %arrayidx6 = getelementptr inbounds [1024 x i64], [1024 x i64]* @b, i64 0, i64 %indvars.iv
  store i64 %add3, i64* %arrayidx6, align 16
  %sub = sub nsw i64 %0, %2
  %arrayidx14 = getelementptr inbounds [1024 x i64], [1024 x i64]* @b, i64 0, i64 %1
  store i64 %sub, i64* %arrayidx14, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 2
  %cmp = icmp ult i64 %indvars.iv, 1022
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.body
  ret void
}
; end INTEL_FEATURE_SW_ADVANCED
