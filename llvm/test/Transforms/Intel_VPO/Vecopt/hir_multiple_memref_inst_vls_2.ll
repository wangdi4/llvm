; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -VPlanDriverHIR -vplan-force-vf=4 -disable-output -print-after=VPlanDriverHIR < %s 2>&1 | FileCheck %s
;
; Test to check that correct vector code is generated for input HIR with
; multiple memrefs in the same HLInst. Incoming HIR into the vectorizer
; looks like the following:
;     DO i1 = 0, 99, 1   <DO_LOOP> <ivdep>
;       %add.r = (%src2)[i1].0  +  (%src1)[i1].0;
;       %add.i = (%src2)[i1].1  +  (%src1)[i1].1;
;       (%dest)[i1].0 = %add.r;
;       (%dest)[i1].1 = %add.i;
;     END LOOP
;
; The test verifies that we generate VLS loads that are 8-wide with
; appropriate shuffles. TODO - look into why we still generate scatters
; for the two stores.
;
; CHECK:       DO i1 = 0, 99, 4   <DO_LOOP> <auto-vectorized> <novectorize> <ivdep>
; CHECK-NEXT:    %.vls.load = (<8 x float>*)(%src2)[i1].0;
; CHECK-NEXT:    %vls.extract = shufflevector %.vls.load,  %.vls.load,  <i32 0, i32 2, i32 4, i32 6>;
; CHECK-NEXT:    %vls.extract1 = shufflevector %.vls.load,  %.vls.load,  <i32 1, i32 3, i32 5, i32 7>;
; CHECK-NEXT:    %.vls.load2 = (<8 x float>*)(%src1)[i1].0;
; CHECK-NEXT:    %vls.extract3 = shufflevector %.vls.load2,  %.vls.load2,  <i32 0, i32 2, i32 4, i32 6>;
; CHECK-NEXT:    %vls.extract4 = shufflevector %.vls.load2,  %.vls.load2,  <i32 1, i32 3, i32 5, i32 7>;
; CHECK-NEXT:    %.vec = %vls.extract  +  %vls.extract3;
; CHECK-NEXT:    %.vec5 = %vls.extract1  +  %vls.extract4;
; CHECK-NEXT:    (<4 x float>*)(%dest)[i1 + <i64 0, i64 1, i64 2, i64 3>].0 = %.vec;
; CHECK-NEXT:    (<4 x float>*)(%dest)[i1 + <i64 0, i64 1, i64 2, i64 3>].1 = %.vec5;
; CHECK:       END LOOP
;
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nounwind uwtable
define dso_local void @foo({ float, float }* nocapture %dest, { float, float }* nocapture readonly %src1, { float, float }* nocapture readonly %src2) local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %l1.08 = phi i64 [ 0, %entry ], [ %inc, %for.body ]
  %ptridx.realp = getelementptr inbounds { float, float }, { float, float }* %src1, i64 %l1.08, i32 0
  %ptridx.real = load float, float* %ptridx.realp, align 4
  %ptridx.imagp = getelementptr inbounds { float, float }, { float, float }* %src1, i64 %l1.08, i32 1
  %ptridx.imag = load float, float* %ptridx.imagp, align 4
  %ptridx1.realp = getelementptr inbounds { float, float }, { float, float }* %src2, i64 %l1.08, i32 0
  %ptridx1.real = load float, float* %ptridx1.realp, align 4
  %ptridx1.imagp = getelementptr inbounds { float, float }, { float, float }* %src2, i64 %l1.08, i32 1
  %ptridx1.imag = load float, float* %ptridx1.imagp, align 4
  %add.r = fadd fast float %ptridx1.real, %ptridx.real
  %add.i = fadd fast float %ptridx1.imag, %ptridx.imag
  %ptridx2.realp = getelementptr inbounds { float, float }, { float, float }* %dest, i64 %l1.08, i32 0
  %ptridx2.imagp = getelementptr inbounds { float, float }, { float, float }* %dest, i64 %l1.08, i32 1
  store float %add.r, float* %ptridx2.realp, align 4
  store float %add.i, float* %ptridx2.imagp, align 4
  %inc = add nuw nsw i64 %l1.08, 1
  %exitcond.not = icmp eq i64 %inc, 100
  br i1 %exitcond.not, label %for.end, label %for.body, !llvm.loop !0

for.end:                                          ; preds = %for.body
  ret void
}

!0 = distinct !{!0, !1}
!1 = !{!"llvm.loop.vectorize.ivdep_back"}
