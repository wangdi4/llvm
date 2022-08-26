; RUN: opt -enable-new-pm=0 -hir-ssa-deconstruction -hir-vec-dir-insert --vector-library=SVML -hir-vplan-vec -print-after=hir-vplan-vec -hir-details -vplan-force-vf=4 -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>" --vector-library=SVML -hir-details -vplan-force-vf=4 -disable-output < %s 2>&1 | FileCheck %s

; Test that the memory reference created for the masked uniform store is created
; correctly. We were failing to set the canon expr src type correctly which made
; it appear that the canon expr had an invalid bitcast. HIR CG was however still
; generating valid IR due to implicit broadcasts that it generates and this issue
; went unnoticed till loopopt team's recent use of TTI which caused an assertion
; fail.
;
; The scalar HIR has the following masked uniform store
;    if (%0 >= 0.000000e+00)
;    {
;       (@r)[0] = %0;
;    }
; The generated vector HIR is expected to look like the following
;    (<4 x float>*)(@r)[0] = %.vec, Mask = @{%.vec1};
;    <LVAL-REG> {al:4}(<4 x float>*)(LINEAR float* @r)[<4 x i64> 0]
;
; CHECK:  DO i64 i1 = 0, 99, 4   <DO_LOOP> <auto-vectorized> <novectorize>
; CHECK:    (<4 x float>*)(@r)[0] = %.vec, Mask = @{%.vec1};
; CHECK:     <LVAL-REG> {al:4}(<4 x float>*)(LINEAR float* @r)[<4 x i64> 0]
; CHECK:  END LOOP
;
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@S = common dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 16
@r = common dso_local local_unnamed_addr global float 0.000000e+00, align 4

; Function Attrs: nofree norecurse nounwind uwtable
define dso_local void @test() local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.inc, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc ]
  %arrayidx = getelementptr inbounds [100 x float], [100 x float]* @S, i64 0, i64 %indvars.iv
  %0 = load float, float* %arrayidx, align 4
  %cmp1 = fcmp ult float %0, 0.000000e+00
  br i1 %cmp1, label %for.inc, label %if.then

if.then:                                          ; preds = %for.body
  store float %0, float* @r, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.inc
  ret void
}

; Test to check that the canon expression src/dest types are set consistently
; when replacing scalar math library calls in the remainder loop with the svml
; version to maintain precision consistency. We failed to set the src type
; of the arg-copy canon expression, which made it appear as if we had
; an invalid bitcast. The loop has a call to llvm.exp.f64. When replacing this
; call by a call to __svml_exp4 in the remainder loop, we create a copy of the
; arg, and the call argument is replaced with a canon expr whose type is set to
; vector so that we do a broadcast.
; The call in the remainder loop is expected to look like the following:
;    %__svml_exp41 = @__svml_exp4(%copy); <fast>
;    <RVAL-REG> NON-LINEAR <4 x double> %copy
;
; CHECK:  DO i64 i1 = {{.*}}, 97, 1   <DO_LOOP>
; CHECK:    %{{__svml_exp[0-9]+}} = @__svml_exp4(%copy); <fast>
; CHECK:    <RVAL-REG> NON-LINEAR <4 x double> %copy
; CHECK:  END LOOP

define dso_local void @foo(double* nocapture %arr) local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %l1.08 = phi i64 [ %inc, %for.body ], [ 0, %entry ]
  %ptridx = getelementptr inbounds double, double* %arr, i64 %l1.08
  %0 = load double, double* %ptridx, align 8
  %1 = tail call fast double @llvm.exp.f64(double %0)
  store double %1, double* %ptridx, align 8
  %inc = add nuw nsw i64 %l1.08, 1
  %exitcond = icmp eq i64 %inc, 98
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

declare double @llvm.exp.f64(double) #1
