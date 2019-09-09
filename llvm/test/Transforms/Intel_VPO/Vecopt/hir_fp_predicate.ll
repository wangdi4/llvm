; Check that VPlan HIR decomposer is able to handle HLIf nodes with single predicate (of Float type).
; The test also checks the correctness of code generation for this scenario.
; Input LLVM-IR generated with command: icx -O2 -mllvm -print-module-before-loopopt

; float  foo(float *arr, int n1)
; {
;     int index;
;
; #pragma vector always
; #pragma ivdep
;     for (index = 0; index < 1024; index++) {
;         if (arr[index] > 0) {
;             arr[index + n1] = index + n1 * n1 + 3;
;         }
;     }
;     return arr[0];
; }

; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -VPlanDriverHIR -vplan-print-after-simplify-cfg < %s 2>&1 | FileCheck %s --check-prefix=HCFG

; Check plain HCFG
; HCFG: i1 [[Cmp:%.*]] = fcmp float [[Arr:%.*]] float 0.000000e+00
; HCFG: Condition({{BB.*}}): [DA: Divergent] i1 [[Cmp]] = fcmp float [[Arr]] float 0.000000e+00

; NOTE: We force VF=4 here to test correctness of code generation
; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -VPlanDriverHIR -vplan-force-vf=4 -print-after=VPlanDriverHIR -S < %s 2>&1 | FileCheck %s

; Check HIR
; CHECK: DO i1 = 0, 1023, 4   <DO_LOOP>
; CHECK: [[WideCmp:%.*]] = {{%.*}} > 0.000000e+00;
; CHECK: Mask = @{[[WideCmp]]}


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define dso_local float @foo(float* nocapture %arr, i32 %n1) local_unnamed_addr #0 {
entry:
  %mul = mul nsw i32 %n1, %n1
  %add = add nuw i32 %mul, 3
  %0 = sext i32 %n1 to i64
  br label %for.body

for.body:                                         ; preds = %for.inc, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc ]
  %arrayidx = getelementptr inbounds float, float* %arr, i64 %indvars.iv
  %1 = load float, float* %arrayidx, align 4, !tbaa !2
  %cmp1 = fcmp ogt float %1, 0.000000e+00
  br i1 %cmp1, label %if.then, label %for.inc

if.then:                                          ; preds = %for.body
  %2 = trunc i64 %indvars.iv to i32
  %add2 = add i32 %add, %2
  %conv = sitofp i32 %add2 to float
  %3 = add nsw i64 %indvars.iv, %0
  %arrayidx5 = getelementptr inbounds float, float* %arr, i64 %3
  store float %conv, float* %arrayidx5, align 4, !tbaa !2
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.end, label %for.body, !llvm.loop !6

for.end:                                          ; preds = %for.inc
  %4 = load float, float* %arr, align 4, !tbaa !2
  ret float %4
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }


!2 = !{!3, !3, i64 0}
!3 = !{!"float", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = distinct !{!6, !7, !8, !9}
!7 = !{!"llvm.loop.vectorize.ivdep_back"}
!8 = !{!"llvm.loop.vectorize.ignore_profitability"}
!9 = !{!"llvm.loop.vectorize.enable", i1 true}
