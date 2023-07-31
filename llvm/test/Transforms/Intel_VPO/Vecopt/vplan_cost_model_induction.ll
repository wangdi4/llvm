; RUN: opt < %s -disable-output -passes=vplan-vec -mtriple=x86_64-unknown-unknown -mattr=+avx2 -vplan-force-vf=4 -vplan-cost-model-print-analysis-for-vf=4 | FileCheck %s --check-prefix=CHECK-IR

; RUN: opt < %s -disable-output -passes='hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec' -mtriple=x86_64-unknown-unknown -mattr=+avx2 -vplan-force-vf=4 -vplan-cost-model-print-analysis-for-vf=4 | FileCheck %s --check-prefix=CHECK-HIR

; The test verifies the cost of induction-init/step/final instructions
; for two corner cases: 0 known init value, 1 known step and unknown init
; value and step value.

; LoopOpt replaces %j induction with %i induction, so HIR checks verify only %i
; induction

@arr = external local_unnamed_addr global [128 x i32], align 16

define void @foo_ind_add(i32 %j.init, i32 %j.step) {
; CHECK-IR:    Cost 0 for i32 [[VP_I_MERGE_IND_INIT:%.*]] = induction-init{add} i32 live-in0 i32 1
; CHECK-IR-NEXT:    Cost 0 for i32 [[VP_I_MERGE_IND_INIT_STEP:%.*]] = induction-init-step{add} i32 1
; CHECK-IR-NEXT:    Cost 5 for i32 [[VP_J_MERGE_IND_INIT:%.*]] = induction-init{add} i32 live-in1 i32 [[J_STEP0:%.*]]
; CHECK-IR-NEXT:    Cost 2 for i32 [[VP_J_MERGE_IND_INIT_STEP:%.*]] = induction-init-step{add} i32 [[J_STEP0]]
; CHECK-IR:    Cost 0 for i32 [[VP_I_MERGE_IND_FINAL:%.*]] = induction-final{add} i32 0 i32 1
; CHECK-IR-NEXT:    Cost 2 for i32 [[VP_J_MERGE_IND_FINAL:%.*]] = induction-final{add} i32 [[J_INIT0:%.*]] i32 [[J_STEP0:%.*]]

; CHECK-HIR:    Cost 0 for i32 [[VP_I_MERGE_IND_INIT:%.*]] = induction-init{add} i32 live-in0 i32 1
; CHECK-HIR-NEXT:    Cost 0 for i32 [[VP_I_MERGE_IND_INIT_STEP:%.*]] = induction-init-step{add} i32 1
; CHECK-HIR:    Cost 0 for i32 [[VP_I_MERGE_IND_FINAL:%.*]] = induction-final{add} i32 0 i32 1

entry:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %for.body

for.body:
  %i.merge = phi i32 [ 0, %entry ], [ %i.next, %for.body ]
  %j.merge = phi i32 [ %j.init, %entry ], [ %j.next, %for.body ]
  %idx = getelementptr inbounds [128 x i32], ptr @arr, i32 0, i32 %i.merge
  store i32 %j.merge, ptr %idx, align 4
  %i.next = add nuw nsw i32 %i.merge, 1
  %j.next = add nuw nsw i32 %j.merge, %j.step
  %exitcond = icmp eq i32 %i.next, 128
  br i1 %exitcond, label %for.exit, label %for.body

for.exit:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"()]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
