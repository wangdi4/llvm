; Test verifies aliases collection for privates in loop post exit.
; RUN: opt -vplan-print-legality -S -VPlanDriver -vplan-force-vf=4 -disable-output < %s 2>&1 | FileCheck %s

; CHECK: VPOLegality Descriptor Lists
; CHECK-EMPTY:
; CHECK-EMPTY:
; CHECK-NEXT: VPOLegality PrivateList:
; CHECK-NEXT: Ref:   %f.lpriv = alloca float, align 4
; CHECK-NEXT:  UpdateInstruction:
; CHECK-NEXT:  AliasRef:   store float %add7.lcssa, float* %f.lpriv, align 4
; CHECK-NEXT:    UpdateInstruction:
; CHECK-NEXT: PrivDescr: {IsCond: 0, IsLast: 1}

; Function Attrs: nounwind uwtable
define dso_local float @test_post_exit_aliases() local_unnamed_addr {
entry:
  %f.lpriv = alloca float, align 4
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %DIR.OMP.SIMD.2
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LASTPRIVATE"(float* %f.lpriv)]
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %DIR.OMP.SIMD.236
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.1 ], [ %indvars.iv.next, %omp.inner.for.body ]
  %add7 = sitofp i64 %indvars.iv to float
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 123
  br i1 %exitcond, label %DIR.OMP.END.SIMD.3, label %omp.inner.for.body

DIR.OMP.END.SIMD.3:                               ; preds = %omp.inner.for.body
  %add7.lcssa = phi float [ %add7, %omp.inner.for.body ]
  store float %add7.lcssa, float* %f.lpriv
  br label %DIR.OMP.END.SIMD.337

DIR.OMP.END.SIMD.337:                             ; preds = %DIR.OMP.END.SIMD.3
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %DIR.OMP.END.SIMD.337, %entry
  ret float %add7.lcssa
}
; Function Attrs: argmemonly nounwind
declare token @llvm.directive.region.entry()
; Function Attrs: argmemonly nounwind
declare void @llvm.directive.region.exit(token)
