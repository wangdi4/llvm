; RUN: llc -O1 -fp-contract=fast -csa-opt-df-pass=1 -mtriple=csa -csa-hw-reducer-experiment < %s | FileCheck %s --check-prefix=CSA_CHECK

; ModuleID = 'loop_kernel.cpp'
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

; This test should generate a FMA reduction, stride, and sequence. 

; Function Attrs: nounwind readonly
define double @dot_kernel(i32 %n, double* nocapture readonly %x, double* nocapture readonly %y) #0 {

; CSA_CHECK-DAG: redaddf64

entry:
  %cmp.9 = icmp sgt i32 %n, 0
  br i1 %cmp.9, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.body, %entry
  %result.0.lcssa = phi double [ 0.000000e+00, %entry ], [ %add, %for.body ]
  ret double %result.0.lcssa

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %entry ]
  %result.010 = phi double [ %add, %for.body ], [ 0.000000e+00, %entry ]
  %arrayidx = getelementptr inbounds double, double* %x, i64 %indvars.iv
  %0 = load double, double* %arrayidx, align 8, !tbaa !1
  %arrayidx2 = getelementptr inbounds double, double* %y, i64 %indvars.iv
  %1 = load double, double* %arrayidx2, align 8, !tbaa !1
  %mul = fmul double %0, %1
  %add = fadd double %result.010, %mul
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %n
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

attributes #0 = { nounwind readonly "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 "}
!1 = !{!2, !2, i64 0}
!2 = !{!"double", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
