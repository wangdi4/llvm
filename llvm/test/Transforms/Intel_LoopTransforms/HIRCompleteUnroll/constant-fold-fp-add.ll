;REQUIRES: asserts

; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-pre-vec-complete-unroll -disable-output -print-after=hir-pre-vec-complete-unroll -debug-only=hir-transform-utils 2>&1 < %s | FileCheck %s

; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-pre-vec-complete-unroll,print<hir>" -disable-output -debug-only=hir-transform-utils 2>&1 < %s | FileCheck %s

; Check that constants are substituted from global array and folded
; completely in unrolled loop.

;          BEGIN REGION { }
;               + DO i1 = 0, %n + -1, 1
;               |   %sum1.018 = 0.000000e+00;
;               |
;               |   + DO i2 = 0, 2, 1
;               |   |   %sum1.018 = (@_ZL4glob)[0][i2]  +  %sum1.018;
;               |   + END LOOP
;               |
;               |   %sum.021 = %sum1.018  +  %sum.021;
;               + END LOOP
;          END REGION

;CHECK: NumPropagated: 4
;CHECK: NumFolded: 3
;CHECK: NumConstGlobalLoads: 3
;CHECK: NumInstsRemoved: 4


;CHECK:    BEGIN REGION { modified }
;CHECK:         + DO i1 = 0, %n + -1, 1
;CHECK:         |   %sum.021 = 6.000000e+00  +  %sum.021;
;CHECK:         + END LOOP
;CHECK:    END REGION


;Module Before HIR
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@_ZL4glob = internal unnamed_addr constant [3 x double] [double 0.000000e+00, double 2.000000e+00, double 4.000000e+00], align 16

; Function Attrs: norecurse nounwind readnone uwtable
define dso_local double @_Z3fooi(i32 %n) local_unnamed_addr #0 {
entry:
  %cmp20 = icmp sgt i32 %n, 0
  br i1 %cmp20, label %for.cond1.preheader.preheader, label %for.cond.cleanup

for.cond1.preheader.preheader:                    ; preds = %entry
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond1.preheader.preheader, %for.cond.cleanup3
  %ii.022 = phi i32 [ %inc7, %for.cond.cleanup3 ], [ 0, %for.cond1.preheader.preheader ]
  %sum.021 = phi double [ %add5, %for.cond.cleanup3 ], [ 0.000000e+00, %for.cond1.preheader.preheader ]
  br label %for.body4

for.cond.cleanup.loopexit:                        ; preds = %for.cond.cleanup3
  %add5.lcssa = phi double [ %add5, %for.cond.cleanup3 ]
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  %sum.0.lcssa = phi double [ 0.000000e+00, %entry ], [ %add5.lcssa, %for.cond.cleanup.loopexit ]
  ret double %sum.0.lcssa

for.cond.cleanup3:                                ; preds = %for.body4
  %add.lcssa = phi double [ %add, %for.body4 ]
  %add5 = fadd double %add.lcssa, %sum.021
  %inc7 = add nuw nsw i32 %ii.022, 1
  %exitcond23 = icmp eq i32 %inc7, %n
  br i1 %exitcond23, label %for.cond.cleanup.loopexit, label %for.cond1.preheader

for.body4:                                        ; preds = %for.cond1.preheader, %for.body4
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.body4 ]
  %sum1.018 = phi double [ 0.000000e+00, %for.cond1.preheader ], [ %add, %for.body4 ]
  %arrayidx = getelementptr inbounds [3 x double], [3 x double]* @_ZL4glob, i64 0, i64 %indvars.iv, !intel-tbaa !2
  %0 = load double, double* %arrayidx, align 8, !tbaa !2
  %add = fadd double %0, %sum1.018
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 3
  br i1 %exitcond, label %for.cond.cleanup3, label %for.body4
}

attributes #0 = { norecurse nounwind readnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA3_d", !4, i64 0}
!4 = !{!"double", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C++ TBAA"}
