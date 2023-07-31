; INTEL_FEATURE_SW_ADVANCED
; TODO: Enable a way to run with LLVM-IR VPlanDriver
; REQUIRES: intel_feature_sw_advanced
; RUN: opt < %s -passes='hir-ssa-deconstruction,hir-temp-cleanup,hir-vec-dir-insert,hir-vplan-vec' -xmain-opt-level=3 -vplan-cost-model-print-analysis-for-vf=4 -disable-output | FileCheck %s
; RUN: opt < %s -passes='hir-ssa-deconstruction,hir-temp-cleanup,hir-vec-dir-insert,hir-vplan-vec' -xmain-opt-level=3 -vplan-force-vf=4 -vplan-cost-model-print-analysis-for-vf=4 -disable-output | FileCheck %s --check-prefix=CHECK-FORCED

; CHECK: Extra cost due to SLP breaking heuristic
; CHECK-FORCED-NOT: Extra cost due to SLP breaking heuristic

target triple = "x86_64-unknown-linux-gnu"

@a = dso_local local_unnamed_addr global [1024 x i32] zeroinitializer, align 16
@b = dso_local local_unnamed_addr global [1024 x i32] zeroinitializer, align 16

; Function Attrs: nofree norecurse nounwind uwtable
define dso_local void @foo() local_unnamed_addr {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds [1024 x i32], ptr @a, i64 0, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx, align 16
  %1 = or i64 %indvars.iv, 1
  %arrayidx2 = getelementptr inbounds [1024 x i32], ptr @a, i64 0, i64 %1
  %2 = load i32, ptr %arrayidx2, align 4
  %add3 = add nsw i32 %2, %0
  %3 = or i64 %indvars.iv, 2
  %arrayidx6 = getelementptr inbounds [1024 x i32], ptr @a, i64 0, i64 %3
  %4 = load i32, ptr %arrayidx6, align 8
  %mul = shl nsw i32 %4, 1
  %add7 = add nsw i32 %add3, %mul
  %5 = or i64 %indvars.iv, 3
  %arrayidx10 = getelementptr inbounds [1024 x i32], ptr @a, i64 0, i64 %5
  %6 = load i32, ptr %arrayidx10, align 4
  %mul11 = mul nsw i32 %6, 3
  %add12 = add nsw i32 %add7, %mul11
  %arrayidx14 = getelementptr inbounds [1024 x i32], ptr @b, i64 0, i64 %indvars.iv
  store i32 %add12, ptr %arrayidx14, align 16
  %sub = sub i32 %0, %2
  %add24 = add nsw i32 %sub, %mul
  %7 = mul i32 %6, -3
  %sub29 = add i32 %add24, %7
  %arrayidx32 = getelementptr inbounds [1024 x i32], ptr @b, i64 0, i64 %1
  store i32 %sub29, ptr %arrayidx32, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 4
  %cmp = icmp ult i64 %indvars.iv, 1020
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.body
  ret void
}
; end INTEL_FEATURE_SW_ADVANCED
