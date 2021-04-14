; Test verifies aliases collection for privates in loop preheader.
; RUN: opt -vplan-print-legality -S -VPlanDriver -vplan-force-vf=4 -disable-output < %s 2>&1 | FileCheck %s
; UNSUPPORTED: windows
; TO-DO : The test case fails on Windows. Analyze and fix.

; CHECK: VPOLegality Descriptor Lists
; CHECK-EMPTY:
; CHECK-EMPTY:
; CHECK-NEXT: VPOLegality PrivateList:
; CHECK-NEXT: Ref:   %d.lpriv = alloca double, align 8
; CHECK-NEXT:  UpdateInstruction:
; CHECK-NEXT:  AliasRef:   %d.lpriv.promoted = load double, double* %d.lpriv, align 8
; CHECK-NEXT:    UpdateInstruction:
; CHECK-NEXT:  AliasRef:   store double %conv735.lcssa, double* %d.lpriv, align 8
; CHECK-NEXT:    UpdateInstruction:
; CHECK-NEXT: PrivDescr: {IsCond: 1, IsLast: 1}
;
; TODO: Re-enable the test when added support for conditional privates
; XFAIL: *
;

; Function Attrs: nounwind uwtable
; outer loop
define dso_local double @test_preheader_aliases() local_unnamed_addr #3 {
entry:
  %d.lpriv = alloca double, align 8
  store double 0.000000e+00, double* %d.lpriv, align 8
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %DIR.OMP.SIMD.2
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LASTPRIVATE:CONDITIONAL"(double* %d.lpriv), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null) ]
  br label %DIR.OMP.SIMD.237

DIR.OMP.SIMD.237:                                 ; preds = %DIR.OMP.SIMD.1
  %d.lpriv.promoted = load double, double* %d.lpriv, align 8
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %if.end, %DIR.OMP.SIMD.237
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.237 ], [ %indvars.iv.next, %if.end ]
  %conv734 = phi double [ %d.lpriv.promoted, %DIR.OMP.SIMD.237 ], [ %conv735, %if.end ]
  %tobool = icmp eq i64 %indvars.iv, 25
  br i1 %tobool, label %if.end, label %if.then

if.then:                                          ; preds = %omp.inner.for.body
  %add6 = fdiv double %conv734, 2.000000e+00
  br label %if.end

if.end:                                           ; preds = %omp.inner.for.body, %if.then
  %conv735 = phi double [ %conv734, %omp.inner.for.body ], [ %add6, %if.then ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 171
  br i1 %exitcond, label %omp.inner.for.cond.omp.loop.exit.split_crit_edge, label %omp.inner.for.body

omp.inner.for.cond.omp.loop.exit.split_crit_edge: ; preds = %if.end
  %conv735.lcssa = phi double [ %conv735, %if.end ]
  store double %conv735.lcssa, double* %d.lpriv, align 8
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:                               ; preds = %omp.inner.for.cond.omp.loop.exit.split_crit_edge
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %DIR.OMP.END.SIMD.3, %entry
  ret double %conv735.lcssa
}
; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)
