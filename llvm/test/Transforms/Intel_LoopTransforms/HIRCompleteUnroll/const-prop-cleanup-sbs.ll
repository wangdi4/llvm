;REQUIRES: asserts

; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-pre-vec-complete-unroll,print<hir>" -disable-output -debug-only=hir-transform-utils 2>&1 < %s | FileCheck %s

; Check that loop is deleted after constant propagation removes self assignment of
; mul reduction, which leaves the loop without any liveouts, and thus dead.
; Constprop cannot detect the leftover casted load to %conv, which was causing degradation.


;<0>          BEGIN REGION { }
;<26>               + DO i1 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 4294967295>
;<27>               |   + DO i2 = 0, 2, 1   <DO_LOOP>
;<9>                |   |   %conv = sitofp.i32.double((%A)[i2]);
;<10>               |   |   %mul = (@_ZL4zero)[0][i2]  *  %conv;
;<11>               |   |   %sum.020 = %mul  +  %sum.020;
;<27>               |   + END LOOP
;<26>               + END LOOP
;<0>          END REGION
;

;CHECK: NumPropagated: 3
;CHECK: NumFolded: 6
;CHECK: NumConstGlobalLoads: 3
;CHECK: NumInstsRemoved: 6

;*** IR Dump After HIR PreVec Complete Unroll ***
;CHECK: Function:

;CHECK:      BEGIN REGION { modified }
;CHECK-NEXT: END REGION


;Module Before HIR
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@_ZL4zero = internal unnamed_addr constant [3 x double] zeroinitializer, align 16

; Function Attrs: norecurse nounwind readonly uwtable
define dso_local double @_Z3fooiPi(i32 %n, ptr nocapture readonly %A) local_unnamed_addr #0 {
entry:
  %cmp19 = icmp sgt i32 %n, 0
  br i1 %cmp19, label %for.cond1.preheader.preheader, label %for.cond.cleanup

for.cond1.preheader.preheader:                    ; preds = %entry
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond1.preheader.preheader, %for.cond.cleanup3
  %ii.021 = phi i32 [ %inc7, %for.cond.cleanup3 ], [ 0, %for.cond1.preheader.preheader ]
  %sum.020 = phi double [ %add.lcssa, %for.cond.cleanup3 ], [ 0.000000e+00, %for.cond1.preheader.preheader ]
  br label %for.body4

for.cond.cleanup.loopexit:                        ; preds = %for.cond.cleanup3
  %add.lcssa.lcssa = phi double [ %add.lcssa, %for.cond.cleanup3 ]
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  %sum.0.lcssa = phi double [ 0.000000e+00, %entry ], [ %add.lcssa.lcssa, %for.cond.cleanup.loopexit ]
  ret double %sum.0.lcssa

for.cond.cleanup3:                                ; preds = %for.body4
  %add.lcssa = phi double [ %add, %for.body4 ]
  %inc7 = add nuw nsw i32 %ii.021, 1
  %exitcond22 = icmp eq i32 %inc7, %n
  br i1 %exitcond22, label %for.cond.cleanup.loopexit, label %for.cond1.preheader

for.body4:                                        ; preds = %for.cond1.preheader, %for.body4
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.body4 ]
  %sum.117 = phi double [ %sum.020, %for.cond1.preheader ], [ %add, %for.body4 ]
  %arrayidx = getelementptr inbounds [3 x double], ptr @_ZL4zero, i64 0, i64 %indvars.iv, !intel-tbaa !2
  %0 = load double, ptr %arrayidx, align 8, !tbaa !2
  %ptridx = getelementptr inbounds i32, ptr %A, i64 %indvars.iv
  %1 = load i32, ptr %ptridx, align 4, !tbaa !7
  %conv = sitofp i32 %1 to double
  %mul = fmul fast double %0, %conv
  %add = fadd fast double %mul, %sum.117
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 3
  br i1 %exitcond, label %for.cond.cleanup3, label %for.body4
}

attributes #0 = { norecurse nounwind readonly uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA3_d", !4, i64 0}
!4 = !{!"double", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C++ TBAA"}
!7 = !{!8, !8, i64 0}
!8 = !{!"int", !5, i64 0}
