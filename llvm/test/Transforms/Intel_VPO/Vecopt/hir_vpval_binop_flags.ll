; LIT test to check that we propagate fast math flags during VPValue code
; generation. We currently revert to using the flags from the underlying binary
; operator while the proper fix of preserving the flags during VPlan
; construction and propagating the same during vector code generation is being
; worked on (CMPLRLLVM-11656). Preserving the fast math flags is important for
; achieving performance parity with mixed code generation mode.
;
; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -VPlanDriverHIR -vplan-force-vf=4 -print-after=VPlanDriverHIR -enable-vp-value-codegen-hir -hir-details -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,vplan-driver-hir,print<hir>" -aa-pipeline="basic-aa" -vplan-force-vf=4 -enable-vp-value-codegen-hir -hir-details -disable-output < %s 2>&1 | FileCheck %s

define double @foo(double* nocapture readonly %darr) #0 {
; CHECK:      [[RED_VAR0:%.*]] = 0.000000e+00
; CHECK:    DO i64 i1 = 0, 99, 4   <DO_LOOP> <novectorize>
; CHECK:      [[DOTVEC0:%.*]] = (<4 x double>*)([[DARR0:%.*]])[i1]
; CHECK:      [[DOTVEC10:%.*]] = [[DOTVEC0]]  *  2.000000e+00; <fast>
; CHECK:      [[RED_VAR0]] = [[DOTVEC10]]  +  [[RED_VAR0]]; <fast>
; CHECK:    END LOOP
; CHECK:      [[SUM_060:%.*]] = @llvm.experimental.vector.reduce.v2.fadd.f64.v4f64([[SUM_060]],  [[RED_VAR0]])
;
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %l1.07 = phi i64 [ 0, %entry ], [ %inc, %for.body ]
  %sum.06 = phi double [ 0.000000e+00, %entry ], [ %add, %for.body ]
  %arrayidx = getelementptr inbounds double, double* %darr, i64 %l1.07
  %0 = load double, double* %arrayidx, align 8
  %mul = fmul fast double %0, 2.000000e+00
  %add = fadd fast double %mul, %sum.06
  %inc = add nuw nsw i64 %l1.07, 1
  %exitcond = icmp eq i64 %inc, 100
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  %add.lcssa = phi double [ %add, %for.body ]
  ret double %add.lcssa
}
