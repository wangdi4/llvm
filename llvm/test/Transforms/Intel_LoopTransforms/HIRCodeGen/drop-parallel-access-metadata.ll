; RUN: opt < %s -hir-ssa-deconstruction -analyze -enable-new-pm=0 -hir-framework -hir-cg -simplifycfg -print-after=simplifycfg -force-hir-cg 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,print<hir>,hir-cg,simplifycfg,print" < %s -force-hir-cg 2>&1 | FileCheck %s

; Ensure that we identify regions with llvm.loop.parallel_accesses metadata and
; trivially maintain correctness by dropping the metadata. This metadata may be
; emitted by VPOParopt.

;     void test1(float *P) {
;     #pragma omp for
;       for (int I = 0; I < 100; I++)
;         P[I] = I;
;     }

; Verify that we do accept the metadata and identify an HIR region:
; CHECK: BEGIN REGION {
; Verify that the metadata is dropped by HIR codegen:
; CHECK-LABEL: @test1
; CHECK-NOT: !llvm.loop

define void @test1(float* %P) {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret void

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %0 = trunc i64 %indvars.iv to i32
  %conv = sitofp i32 %0 to float
  %ptridx = getelementptr inbounds float, float* %P, i64 %indvars.iv
  store float %conv, float* %ptridx, align 4, !llvm.access.group !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.cond.cleanup, label %for.body, !llvm.loop !0
}

!0 = distinct !{!0, !1}
!1 = !{!"llvm.loop.parallel_accesses", !2}
!2 = distinct !{}
