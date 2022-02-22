; RUN: opt < %s -enable-new-pm=0 -analyze -hir-scc-formation | FileCheck %s
; RUN: opt %s -passes="print<hir-scc-formation>" -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -enable-new-pm=0 -opaque-pointers -analyze -hir-scc-formation | FileCheck %s
; RUN: opt %s -passes="print<hir-scc-formation>" -opaque-pointers -disable-output 2>&1 | FileCheck %s

; Verify that we trace through certain intrinsics like llvm.minnum to form SCCs.

; CHECK: SCC1: %1 -> %min.011

@a = dso_local local_unnamed_addr global [1024 x float] zeroinitializer, align 16

define dso_local float @foo() {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  %.lcssa = phi float [ %1, %for.body ]
  ret float %.lcssa

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %min.011 = phi float [ 2.550000e+02, %entry ], [ %1, %for.body ]
  %arrayidx = getelementptr inbounds [1024 x float], [1024 x float]* @a, i64 0, i64 %indvars.iv
  %0 = load float, float* %arrayidx, align 4
  %1 = tail call fast float @llvm.minnum.f32(float %0, float %min.011)
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %cmp = icmp eq i64 %indvars.iv.next, 1024
  br i1 %cmp, label %for.cond.cleanup, label %for.body
}

; Function Attrs: nounwind readnone speculatable willreturn
declare float @llvm.minnum.f32(float, float) #1

attributes #1 = { nounwind readnone speculatable willreturn }

