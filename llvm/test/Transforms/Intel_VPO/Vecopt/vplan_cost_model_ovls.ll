; RUN: opt < %s -hir-ssa-deconstruction -hir-temp-cleanup -xmain-opt-level=3 \
; RUN:     -hir-vec-dir-insert -VPlanDriverHIR -disable-output \
; RUN:     -vplan-cost-model-print-analysis-for-vf=4 | FileCheck %s

; The test checks that OVLS group is detected by CM ("OVLS" string in
; the dump) and verifies that the first Load of VLS group receive whole Cost
; of the group, while consequent group members receive Cost 0.

; CHECK: Cost 18 for i64 {{.*}} = load i64* {{.*}} ( GS OVLS )
; CHECK: Cost 0 for i64 {{.*}} = load i64* {{.*}} ( GS OVLS )

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
