; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-pre-vec-complete-unroll,print<hir>" -disable-output 2>&1 < %s | FileCheck %s

; Check that FP selfblobs are constant propagated and folded properly. The loopnest contains
; conditional code that allow partial propagation for %tmp.039.


;*** IR Dump Before HIR PreVec Complete Unroll
;
;  BEGIN REGION { }
;       + DO i1 = 0, %n + -1, 1   <DO_LOOP>
;       |   %tmp.039 = 4.000000e+00;
;       |
;       |   + DO i2 = 0, 1, 1   <DO_LOOP>
;       |   |   + DO i3 = 0, 1, 1   <DO_LOOP>
;       |   |   |   if (%sum.042 < %tmp.039)
;       |   |   |   {
;       |   |   |      %tmp.039 = %tmp.039  -  %sum.042;
;       |   |   |      %mul = (@_ZL4glob)[0][i2][i3]  *  (%A)[i3];
;       |   |   |      %sum.042 = %mul  +  %sum.042;
;       |   |   |   }
;       |   |   + END LOOP
;       |   + END LOOP
;       + END LOOP
;  END REGION
;
;*** IR Dump After HIR PreVec Complete Unroll ***
;
;CHECK:  BEGIN REGION { modified }
;CHECK:   + DO i1 = 0, %n + -1, 1   <DO_LOOP>
;CHECK:   |   %tmp.039 = 4.000000e+00;
;CHECK:   |   if (%sum.042 < 4.000000e+00)
;CHECK:   |   {
;CHECK:   |      %tmp.039 = 4.000000e+00  -  %sum.042;
;CHECK:   |      %mul = (%A)[0];
;CHECK:   |      %sum.042 = %mul  +  %sum.042;
;CHECK:   |   }
;CHECK:   |   if (%sum.042 < %tmp.039)
;CHECK:   |   {
;CHECK:   |      %tmp.039 = %tmp.039  -  %sum.042;
;CHECK:   |      %mul = 2.000000e+00  *  (%A)[1];
;CHECK:   |      %sum.042 = %mul  +  %sum.042;
;CHECK:   |   }
;CHECK:   |   if (%sum.042 < %tmp.039)
;CHECK:   |   {
;CHECK:   |      %tmp.039 = %tmp.039  -  %sum.042;
;CHECK:   |      %mul = 3.000000e+00  *  (%A)[0];
;CHECK:   |      %sum.042 = %mul  +  %sum.042;
;CHECK:   |   }
;CHECK:   |   if (%sum.042 < %tmp.039)
;CHECK:   |   {
;CHECK:   |      %tmp.039 = %tmp.039  -  %sum.042;
;CHECK:   |   }
;CHECK:   + END LOOP
;CHECK: END REGION


;Module Before HIR
; ModuleID = '/nfs/sc/home/liuchen3/test/floatblob2.cpp'
source_filename = "/nfs/sc/home/liuchen3/test/floatblob2.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@_ZL4glob = internal unnamed_addr constant [2 x [2 x double]] [[2 x double] [double 1.000000e+00, double 2.000000e+00], [2 x double] [double 3.000000e+00, double 0.000000e+00]], align 16

; Function Attrs: norecurse nounwind readonly uwtable
define dso_local double @_Z3fooPdi(ptr nocapture readonly %A, i32 %n) local_unnamed_addr #0 {
entry:
  %cmp41 = icmp sgt i32 %n, 0
  br i1 %cmp41, label %for.cond1.preheader.preheader, label %for.cond.cleanup

for.cond1.preheader.preheader:                    ; preds = %entry
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond1.preheader.preheader, %for.cond.cleanup3
  %ii.043 = phi i32 [ %inc17, %for.cond.cleanup3 ], [ 0, %for.cond1.preheader.preheader ]
  %sum.042 = phi double [ %sum.3.lcssa.lcssa, %for.cond.cleanup3 ], [ 0.000000e+00, %for.cond1.preheader.preheader ]
  br label %for.cond5.preheader

for.cond.cleanup.loopexit:                        ; preds = %for.cond.cleanup3
  %sum.3.lcssa.lcssa.lcssa = phi double [ %sum.3.lcssa.lcssa, %for.cond.cleanup3 ]
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  %sum.0.lcssa = phi double [ 0.000000e+00, %entry ], [ %sum.3.lcssa.lcssa.lcssa, %for.cond.cleanup.loopexit ]
  ret double %sum.0.lcssa

for.cond5.preheader:                              ; preds = %for.cond1.preheader, %for.cond.cleanup7
  %cmp2 = phi i1 [ true, %for.cond1.preheader ], [ false, %for.cond.cleanup7 ]
  %indvars.iv44 = phi i64 [ 0, %for.cond1.preheader ], [ 1, %for.cond.cleanup7 ]
  %tmp.039 = phi double [ 4.000000e+00, %for.cond1.preheader ], [ %tmp.2.lcssa, %for.cond.cleanup7 ]
  %sum.138 = phi double [ %sum.042, %for.cond1.preheader ], [ %sum.3.lcssa, %for.cond.cleanup7 ]
  br label %for.body8

for.cond.cleanup3:                                ; preds = %for.cond.cleanup7
  %sum.3.lcssa.lcssa = phi double [ %sum.3.lcssa, %for.cond.cleanup7 ]
  %inc17 = add nuw nsw i32 %ii.043, 1
  %exitcond = icmp eq i32 %inc17, %n
  br i1 %exitcond, label %for.cond.cleanup.loopexit, label %for.cond1.preheader

for.cond.cleanup7:                                ; preds = %for.inc
  %sum.3.lcssa = phi double [ %sum.3, %for.inc ]
  %tmp.2.lcssa = phi double [ %tmp.2, %for.inc ]
  br i1 %cmp2, label %for.cond5.preheader, label %for.cond.cleanup3

for.body8:                                        ; preds = %for.cond5.preheader, %for.inc
  %cmp6 = phi i1 [ true, %for.cond5.preheader ], [ false, %for.inc ]
  %indvars.iv = phi i64 [ 0, %for.cond5.preheader ], [ 1, %for.inc ]
  %tmp.135 = phi double [ %tmp.039, %for.cond5.preheader ], [ %tmp.2, %for.inc ]
  %sum.234 = phi double [ %sum.138, %for.cond5.preheader ], [ %sum.3, %for.inc ]
  %cmp9 = fcmp fast olt double %sum.234, %tmp.135
  br i1 %cmp9, label %if.then, label %for.inc

if.then:                                          ; preds = %for.body8
  %sub = fsub fast double %tmp.135, %sum.234
  %ptridx = getelementptr inbounds double, ptr %A, i64 %indvars.iv
  %0 = load double, ptr %ptridx, align 8, !tbaa !2
  %arrayidx12 = getelementptr inbounds [2 x [2 x double]], ptr @_ZL4glob, i64 0, i64 %indvars.iv44, i64 %indvars.iv, !intel-tbaa !6
  %1 = load double, ptr %arrayidx12, align 8, !tbaa !6
  %mul = fmul fast double %1, %0
  %add = fadd fast double %mul, %sum.234
  br label %for.inc

for.inc:                                          ; preds = %for.body8, %if.then
  %sum.3 = phi double [ %add, %if.then ], [ %sum.234, %for.body8 ]
  %tmp.2 = phi double [ %sub, %if.then ], [ %tmp.135, %for.body8 ]
  br i1 %cmp6, label %for.body8, label %for.cond.cleanup7
}

attributes #0 = { norecurse nounwind readonly uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++ Compiler Pro 2021.1 (YYYY.x.0.MMDD)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"double", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C++ TBAA"}
!6 = !{!7, !3, i64 0}
!7 = !{!"array@_ZTSA2_A2_d", !8, i64 0}
!8 = !{!"array@_ZTSA2_d", !3, i64 0}
