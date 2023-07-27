; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-pre-vec-complete-unroll,print<hir>" -disable-output 2>&1 < %s | FileCheck %s

; Check that constant assignments are not removed for Loop live-ins.
; %add2.041 in this test is live in to i2 but not live out. However, we can't remove that def due to the backwards use in i2.


;*** IR Dump Before HIR PreVec Complete Unroll ***

;         BEGIN REGION { }
;              + DO i1 = 0, %n + -1, 1   <DO_LOOP>
;              |   %add2.041 = 0;
;              |
;              |   + DO i2 = 0, 1, 1   <DO_LOOP>
;              |   |   %sum.047 = %add2.041  +  %sum.047;
;              |   |   if ((%A)[i2] != 0)
;              |   |   {
;              |   |      @_Z3barv();
;              |   |      %add2.041 = 2;
;              |   |   }
;              |   + END LOOP
;              |
;              |   %add.044 = 1;
;              |
;              |   + DO i2 = 0, 1, 1   <DO_LOOP>
;              |   |   %1 = (%A)[i2];
;              |   |   %sum.047 = (%1 * %add.044)  +  %sum.047;
;              |   |   if ((@_ZL4glob)[0][0][i2] !=u 0.000000e+00)
;              |   |   {
;              |   |      @_Z3barv();
;              |   |      %add.044 = 2;
;              |   |   }
;              |   + END LOOP
;              + END LOOP
;         END REGION

;*** IR Dump After HIR PreVec Complete Unroll ***

; CHECK: BEGIN REGION { modified }
; CHECK:      + DO i1 = 0, %n + -1, 1   <DO_LOOP>
; CHECK:      |   %add2.041 = 0;
; CHECK:      |
; CHECK:      |   + DO i2 = 0, 1, 1   <DO_LOOP>
; CHECK:      |   |   %sum.047 = %add2.041  +  %sum.047;
; CHECK:      |   |   if ((%A)[i2] != 0)
; CHECK:      |   |   {
; CHECK:      |   |      @_Z3barv();
; CHECK:      |   |      %add2.041 = 2;
; CHECK:      |   |   }
; CHECK:      |   + END LOOP
;             |
;             |   %add.044 = 1;
;             |   %1 = (%A)[0];
;             |   %sum.047 = %1  +  %sum.047;
;             |   if (1.000000e+00 !=u 0.000000e+00)
;             |   {
;             |      @_Z3barv();
;             |      %add.044 = 2;
;             |   }
;             |   %1 = (%A)[1];
;             |   %sum.047 = (%1 * %add.044)  +  %sum.047;
;             |   if (2.000000e+00 !=u 0.000000e+00)
;             |   {
;             |      @_Z3barv();
;             |   }
;             + END LOOP
;        END REGION

;Module Before HIR
source_filename = "t"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@_ZL4glob = internal unnamed_addr constant [2 x [2 x double]] [[2 x double] [double 1.000000e+00, double 2.000000e+00], [2 x double] [double 3.000000e+00, double 0.000000e+00]], align 16

; Function Attrs: uwtable
define dso_local i32 @_Z3fooPii(ptr nocapture readonly %A, i32 %n) local_unnamed_addr #0 {
entry:
  %cmp46 = icmp sgt i32 %n, 0
  br i1 %cmp46, label %for.cond1.preheader.preheader, label %for.cond.cleanup

for.cond1.preheader.preheader:                    ; preds = %entry
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond1.preheader.preheader, %for.cond.cleanup10
  %ii.048 = phi i32 [ %inc23, %for.cond.cleanup10 ], [ 0, %for.cond1.preheader.preheader ]
  %sum.047 = phi i32 [ %add14.lcssa, %for.cond.cleanup10 ], [ 0, %for.cond1.preheader.preheader ]
  br label %for.body4

for.cond.cleanup.loopexit:                        ; preds = %for.cond.cleanup10
  %add14.lcssa.lcssa = phi i32 [ %add14.lcssa, %for.cond.cleanup10 ]
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  %sum.0.lcssa = phi i32 [ 0, %entry ], [ %add14.lcssa.lcssa, %for.cond.cleanup.loopexit ]
  ret i32 %sum.0.lcssa

for.body4:                                        ; preds = %for.cond1.preheader, %for.inc
  %cmp2 = phi i1 [ true, %for.cond1.preheader ], [ false, %for.inc ]
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ 1, %for.inc ]
  %add2.041 = phi i32 [ 0, %for.cond1.preheader ], [ %add2.1, %for.inc ]
  %sum.140 = phi i32 [ %sum.047, %for.cond1.preheader ], [ %add5, %for.inc ]
  %add5 = add nsw i32 %add2.041, %sum.140
  %ptridx = getelementptr inbounds i32, ptr %A, i64 %indvars.iv
  %0 = load i32, ptr %ptridx, align 4, !tbaa !2
  %cmp6 = icmp eq i32 %0, 0
  br i1 %cmp6, label %for.inc, label %if.then

if.then:                                          ; preds = %for.body4
  tail call void @_Z3barv()
  br label %for.inc

for.inc:                                          ; preds = %for.body4, %if.then
  %add2.1 = phi i32 [ 2, %if.then ], [ %add2.041, %for.body4 ]
  br i1 %cmp2, label %for.body4, label %for.body11.preheader

for.body11.preheader:                             ; preds = %for.inc
  %add5.lcssa = phi i32 [ %add5, %for.inc ]
  br label %for.body11

for.cond.cleanup10:                               ; preds = %for.inc19
  %add14.lcssa = phi i32 [ %add14, %for.inc19 ]
  %inc23 = add nuw nsw i32 %ii.048, 1
  %exitcond = icmp eq i32 %inc23, %n
  br i1 %exitcond, label %for.cond.cleanup.loopexit, label %for.cond1.preheader

for.body11:                                       ; preds = %for.body11.preheader, %for.inc19
  %cmp9 = phi i1 [ false, %for.inc19 ], [ true, %for.body11.preheader ]
  %indvars.iv49 = phi i64 [ 1, %for.inc19 ], [ 0, %for.body11.preheader ]
  %add.044 = phi i32 [ %add.1, %for.inc19 ], [ 1, %for.body11.preheader ]
  %sum.243 = phi i32 [ %add14, %for.inc19 ], [ %add5.lcssa, %for.body11.preheader ]
  %ptridx13 = getelementptr inbounds i32, ptr %A, i64 %indvars.iv49
  %1 = load i32, ptr %ptridx13, align 4, !tbaa !2
  %mul = mul nsw i32 %1, %add.044
  %add14 = add nsw i32 %mul, %sum.243
  %arrayidx = getelementptr inbounds [2 x [2 x double]], ptr @_ZL4glob, i64 0, i64 0, i64 %indvars.iv49
  %2 = load double, ptr %arrayidx, align 8, !tbaa !6
  %cmp16 = fcmp fast une double %2, 0.000000e+00
  br i1 %cmp16, label %if.then17, label %for.inc19

if.then17:                                        ; preds = %for.body11
  tail call void @_Z3barv()
  br label %for.inc19

for.inc19:                                        ; preds = %for.body11, %if.then17
  %add.1 = phi i32 [ 2, %if.then17 ], [ %add.044, %for.body11 ]
  br i1 %cmp9, label %for.body11, label %for.cond.cleanup10
}

declare dso_local void @_Z3barv() local_unnamed_addr #1

attributes #0 = { uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++ Compiler Pro 2021.1 (YYYY.x.0.MMDD)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C++ TBAA"}
!6 = !{!7, !9, i64 0}
!7 = !{!"array@_ZTSA2_A2_d", !8, i64 0}
!8 = !{!"array@_ZTSA2_d", !9, i64 0}
!9 = !{!"double", !4, i64 0}
