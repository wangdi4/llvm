; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-pre-vec-complete-unroll -disable-output -print-after=hir-pre-vec-complete-unroll 2>&1 < %s | FileCheck %s

; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-pre-vec-complete-unroll,print<hir>" -disable-output 2>&1 < %s | FileCheck %s

; Check that multiplies by zero are only folded if both nnan and nsz are set.

; BEGIN REGION { }
;       + DO i1 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;       |   %Ai = (%A)[i1];
;       |   %sum1.018 = 0.000000e+00;
;       |
;       |   + DO i2 = 0, 0, 1   <DO_LOOP> <unroll>
;       |   |   %mul.lhs = 0.000000e+00  *  %Ai;
;       |   |   %mul.rhs = %Ai  *  0.000000e+00;
;       |   |   %mul.lhs.nnan = 0.000000e+00  *  %Ai;
;       |   |   %mul.rhs.nnan = %Ai  *  0.000000e+00;
;       |   |   %mul.lhs.nsz = 0.000000e+00  *  %Ai;
;       |   |   %mul.rhs.nsz = %Ai  *  0.000000e+00;
;       |   |   %mul.lhs.nnan.nsz = 0.000000e+00  *  %Ai;
;       |   |   %mul.rhs.nnan.nsz = %Ai  *  0.000000e+00;
;       |   |   %add0 = %mul.lhs  +  %mul.rhs;
;       |   |   %add1 = %add0  +  %mul.lhs.nnan;
;       |   |   %add2 = %add1  +  %mul.rhs.nnan;
;       |   |   %add3 = %add2  +  %mul.lhs.nsz;
;       |   |   %add4 = %add3  +  %mul.rhs.nsz;
;       |   |   %add5 = %add4  +  %mul.lhs.nnan.nsz;
;       |   |   %add6 = %add5  +  %mul.rhs.nnan.nsz;
;       |   |   %sum1.018 = %add6  +  %sum1.018;
;       |   + END LOOP
;       |
;       |   %add7 = %sum1.018  +  %sum.021;
;       |   %sum.021 = %add5;
;       + END LOOP
; END REGION

;CHECK:    BEGIN REGION { modified }
;CHECK:          + DO i1 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;CHECK:          |   %Ai = (%A)[i1];
;CHECK:          |   %mul.lhs = 0.000000e+00  *  %Ai;
;CHECK:          |   %mul.rhs = %Ai  *  0.000000e+00;
;CHECK:          |   %mul.lhs.nnan = 0.000000e+00  *  %Ai;
;CHECK:          |   %mul.rhs.nnan = %Ai  *  0.000000e+00;
;CHECK:          |   %mul.lhs.nsz = 0.000000e+00  *  %Ai;
;CHECK:          |   %mul.rhs.nsz = %Ai  *  0.000000e+00;
;CHECK:          |   %add0 = %mul.lhs  +  %mul.rhs;
;CHECK:          |   %add1 = %add0  +  %mul.lhs.nnan;
;CHECK:          |   %add2 = %add1  +  %mul.rhs.nnan;
;CHECK:          |   %add3 = %add2  +  %mul.lhs.nsz;
;CHECK:          |   %add4 = %add3  +  %mul.rhs.nsz;
;CHECK:          |   %add5 = %add4;
;CHECK:          |   %add6 = %add5;
;CHECK:          |   %sum1.018 = %add6;
;CHECK:          |   %sum.021 = %sum1.018  +  %sum.021;
;CHECK:          + END LOOP
;CHECK:    END REGION

;Module Before HIR
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind readnone uwtable
define dso_local double @_Z3fooi(i32 %n, double* %A) local_unnamed_addr #0 {
entry:
  %cmp20 = icmp sgt i32 %n, 0
  br i1 %cmp20, label %for.cond1.preheader.preheader, label %for.cond.cleanup

for.cond1.preheader.preheader:                    ; preds = %entry
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond1.preheader.preheader, %for.cond.cleanup3
  %ii.022 = phi i32 [ %inc7, %for.cond.cleanup3 ], [ 0, %for.cond1.preheader.preheader ]
  %sum.021 = phi double [ %add7, %for.cond.cleanup3 ], [ 0.000000e+00, %for.cond1.preheader.preheader ]
  %Aip = getelementptr inbounds double, double* %A, i32 %ii.022
  %Ai = load double, double* %Aip, align 8
  br label %for.body4

for.cond.cleanup.loopexit:                        ; preds = %for.cond.cleanup3
  %add7.lcssa = phi double [ %add7, %for.cond.cleanup3 ]
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  %sum.0.lcssa = phi double [ 0.000000e+00, %entry ], [ %add7.lcssa, %for.cond.cleanup.loopexit ]
  ret double %sum.0.lcssa

for.cond.cleanup3:                                ; preds = %for.body4
  %add.lcssa = phi double [ %add, %for.body4 ]
  %add7 = fadd double %add.lcssa, %sum.021
  %inc7 = add nuw nsw i32 %ii.022, 1
  %exitcond23 = icmp eq i32 %inc7, %n
  br i1 %exitcond23, label %for.cond.cleanup.loopexit, label %for.cond1.preheader

for.body4:                                        ; preds = %for.cond1.preheader, %for.body4
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.body4 ]
  %sum1.018 = phi double [ 0.000000e+00, %for.cond1.preheader ], [ %add, %for.body4 ]
  %mul.lhs = fmul double 0.0, %Ai
  %mul.rhs = fmul double %Ai, 0.0
  %mul.lhs.nnan = fmul nnan double 0.0, %Ai
  %mul.rhs.nnan = fmul nnan double %Ai, 0.0
  %mul.lhs.nsz = fmul nsz double 0.0, %Ai
  %mul.rhs.nsz = fmul nsz double %Ai, 0.0
  %mul.lhs.nnan.nsz = fmul nnan nsz double 0.0, %Ai
  %mul.rhs.nnan.nsz = fmul nnan nsz double %Ai, 0.0
  %add0 = fadd double %mul.lhs, %mul.rhs
  %add1 = fadd double %add0, %mul.lhs.nnan
  %add2 = fadd double %add1, %mul.rhs.nnan
  %add3 = fadd double %add2, %mul.lhs.nsz
  %add4 = fadd double %add3, %mul.rhs.nsz
  %add5 = fadd double %add4, %mul.lhs.nnan.nsz
  %add6 = fadd double %add5, %mul.rhs.nnan.nsz
  %add = fadd double %add6, %sum1.018
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1
  br i1 %exitcond, label %for.cond.cleanup3, label %for.body4, !llvm.loop !2
}

attributes #0 = { norecurse nounwind readnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!2 = distinct !{!2, !3}
!3 = !{!"llvm.loop.unroll.full"}
