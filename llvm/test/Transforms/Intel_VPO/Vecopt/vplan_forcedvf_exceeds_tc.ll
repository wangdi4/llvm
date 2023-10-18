; RUN: opt -passes='vplan-vec,print' -disable-output -debug-only=LoopVectorizationPlanner -vplan-enable-masked-main-loop=0 < %s 2>&1 | FileCheck %s
; RUN: opt -passes='hir-ssa-deconstruction,hir-vplan-vec' -disable-output -debug-only=LoopVectorizationPlanner -vplan-enable-masked-main-loop=0 < %s 2>&1 | FileCheck %s
; RUN: opt -passes='vpo-directive-cleanup,print,hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec' -disable-output -debug-only=LoopVectorizationPlanner -vplan-force-vf=16 -vplan-enable-masked-main-loop=0 < %s 2>&1 | FileCheck %s --check-prefixes=NODIRCHECK,CHECK
; RUN: opt -passes=vplan-vec,intel-ir-optreport-emitter -disable-output -intel-opt-report=high -vplan-enable-masked-main-loop=0 < %s 2>&1 | FileCheck %s --check-prefix=OPTRPTHI
; RUN: opt -passes=hir-ssa-deconstruction,hir-vplan-vec,hir-cg,simplifycfg,intel-ir-optreport-emitter -disable-output -intel-opt-report=high -vplan-enable-masked-main-loop=0 < %s 2>&1 | FileCheck %s --check-prefix=OPTRPTHI-HIR
;
; RUN: opt -passes='vplan-vec,print' -disable-output -debug-only=LoopVectorizationPlanner -vplan-enable-masked-main-loop=1 < %s 2>&1 | FileCheck %s --check-prefixes=CHECK_MME
; RUN: opt -passes='hir-ssa-deconstruction,hir-vplan-vec' -disable-output -debug-only=LoopVectorizationPlanner -vplan-enable-masked-main-loop=1 < %s 2>&1 | FileCheck %s --check-prefixes=CHECK_MME
; RUN: opt -passes='vpo-directive-cleanup,print,hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec' -disable-output -debug-only=LoopVectorizationPlanner -vplan-force-vf=16 -vplan-enable-masked-main-loop=1 < %s 2>&1 | FileCheck %s --check-prefixes=NODIRCHECK_MME,CHECK_MME
;
; REQUIRES: asserts
;
; LIT test to check that we bail out early when force vf, either from simdlen
; or command line option exceeds loop's known trip count.
;
; NODIRCHECK-NOT:   @llvm.directive.region
; CHECK:            Enforced or only valid vectorization factor exceeds the known trip count for this loop.
; OPTRPTHI: remark #15436: loop was not vectorized: Enforced or only valid vectorization factor exceeds the known trip count for this loop.
; OPTRPTHI-HIR: remark #15436: loop was not vectorized: HIR: Enforced or only valid vectorization factor exceeds the known trip count for this loop.
;
; NODIRCHECK_MME-NOT:   @llvm.directive.region
; CHECK_MME:  Using masked VPlan, VF=16 TC=8
; CHECK_MME:  Selecting VPlan with VF=16
;
define void @foo(ptr %lp) {
entry:
  %entry.region1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i64 16) ]
  br label %loop

loop:
  %index = phi i64 [ 0, %entry ], [ %indvar, %loop ]
  %arrayidx = getelementptr inbounds i64, ptr %lp,  i64 %index
  store i64 %index, ptr %arrayidx
  %indvar = add nuw i64 %index, 1
  %vl.cond = icmp ult i64 %indvar, 8
  br i1 %vl.cond, label %loop, label %end.region

end.region:
  call void @llvm.directive.region.exit(token %entry.region1) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
