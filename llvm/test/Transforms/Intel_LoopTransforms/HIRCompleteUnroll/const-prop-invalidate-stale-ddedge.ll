; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-pre-vec-complete-unroll -print-before=hir-pre-vec-complete-unroll -print-after=hir-pre-vec-complete-unroll -analyze -enable-new-pm=0 -hir-dd-analysis -hir-dd-analysis-verify=Innermost 2>&1 < %s | FileCheck %s

; RUN: opt < %s -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-pre-vec-complete-unroll,print<hir>,print<hir-dd-analysis>" -hir-dd-analysis-verify=Innermost 2>&1 | FileCheck %s

; Check that any folded instructions from constant propagation are invalidated and do
; not become stale edges in DDG. In this case, we remove the instruction
; %t1.035 = %t1.035  +  0.000000e+00;
; and verify that the DDedge is deleted

; *** IR Dump Before HIR PreVec Complete Unroll ***
; Function: foo
;    BEGIN REGION { }
;         + DO i1 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 4294967295>
;         |   + DO i2 = 0, 3, 1   <DO_LOOP>
;         |   |   (%Arr)[0][i2] = i2;
;         |   + END LOOP
;         |
;         |
;         |   + DO i2 = 0, 7, 1   <DO_LOOP>
;         |   |   if (%n < 4)
;         |   |   {
;         |   |      %t1.035 = %t1.035  +  (%A)[i2 + -1];
;         |   |   }
;         |   |   else
;         |   |   {
;         |   |      %t1.035 = %t1.035  +  0.000000e+00;
;         |   |   }
;         |   + END LOOP
;         + END LOOP
;    END REGION
;
; *** IR Dump After HIR PreVec Complete Unroll ***
; Function: foo
;
;    BEGIN REGION { modified }
;         + DO i1 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 4294967295>
;         |   (%Arr)[0][0] = 0;
;         |   (%Arr)[0][1] = 1;
;         |   (%Arr)[0][2] = 2;
;         |   (%Arr)[0][3] = 3;
;         |
;         |   + DO i2 = 0, 7, 1   <DO_LOOP>
;         |   |   if (%n < 4)
;         |   |   {
;         |   |      %t1.035 = %t1.035  +  (%A)[i2 + -1];
;         |   |   }
;         |   + END LOOP
;         + END LOOP
;    END REGION

; CHECK: <[[LINE:[0-9]+]]>   |   |      %t1.035 = %t1.035  +  0.000000e+00;
; CHECK:  BEGIN REGION { modified }
; CHECK-NOT: <[[LINE]]>   |   |      %t1.035 = %t1.035  +  0.000000e+00;

; CHECK: DD graph for function foo:
; CHECK-NOT: :[[LINE]]
; CHECK-NOT: [[LINE]]:


;Module Before HIR
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind readonly uwtable
define dso_local float @foo(float* nocapture readonly %A, float* nocapture readonly %B, i32 %n) local_unnamed_addr #0 {
entry:
  %Arr = alloca [4 x i32], align 16
  %cmp34 = icmp sgt i32 %n, 0
  br i1 %cmp34, label %for.cond1.preheader.lr.ph, label %for.end18

for.cond1.preheader.lr.ph:                        ; preds = %entry
  %cmp7 = icmp slt i32 %n, 4
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond1.preheader.lr.ph, %for.inc16
  %i.036 = phi i32 [ 0, %for.cond1.preheader.lr.ph ], [ %inc17, %for.inc16 ]
  %t1.035 = phi float [ 0.000000e+00, %for.cond1.preheader.lr.ph ], [ %t1.2.lcssa, %for.inc16 ]
  br label %for.body3

for.body3:                                        ; preds = %for.cond1.preheader, %for.body3
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.body3 ]
  %arrayidx = getelementptr inbounds [4 x i32], [4 x i32]* %Arr, i64 0, i64 %indvars.iv, !intel-tbaa !2
  %0 = trunc i64 %indvars.iv to i32
  store i32 %0, i32* %arrayidx, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 4
  br i1 %exitcond, label %for.body6.preheader, label %for.body3

for.body6.preheader:                              ; preds = %for.body3
  br label %for.body6

for.body6:                                        ; preds = %for.body6.preheader, %for.inc13
  %indvars.iv37 = phi i64 [ %indvars.iv.next38.pre-phi, %for.inc13 ], [ 0, %for.body6.preheader ]
  %t1.133 = phi float [ %t1.2, %for.inc13 ], [ %t1.035, %for.body6.preheader ]
  br i1 %cmp7, label %if.then, label %if.else

if.then:                                          ; preds = %for.body6
  %1 = add nsw i64 %indvars.iv37, -1
  %ptridx = getelementptr inbounds float, float* %A, i64 %1
  %2 = load float, float* %ptridx, align 4, !tbaa !7
  %add = fadd float %t1.133, %2
  br label %for.inc13

if.else:                                          ; preds = %for.body6
  %sub12 = fadd float %t1.133, 0.000000e+00
  br label %for.inc13

for.inc13:                                        ; preds = %if.then, %if.else
  %t1.2 = phi float [ %add, %if.then ], [ %sub12, %if.else ]
  %indvars.iv.next38.pre-phi = add nuw nsw i64 %indvars.iv37, 1
  %exitcond41 = icmp eq i64 %indvars.iv.next38.pre-phi, 8
  br i1 %exitcond41, label %for.inc16, label %for.body6

for.inc16:                                        ; preds = %for.inc13
  %t1.2.lcssa = phi float [ %t1.2, %for.inc13 ]
  %inc17 = add nuw nsw i32 %i.036, 1
  %exitcond42 = icmp eq i32 %inc17, %n
  br i1 %exitcond42, label %for.end18.loopexit, label %for.cond1.preheader

for.end18.loopexit:                               ; preds = %for.inc16
  %t1.2.lcssa.lcssa = phi float [ %t1.2.lcssa, %for.inc16 ]
  %arrayidx19.phi.trans.insert = getelementptr inbounds [4 x i32], [4 x i32]* %Arr, i64 0, i64 1
  %.pre = load i32, i32* %arrayidx19.phi.trans.insert, align 4, !tbaa !2
  %phi.cast = sitofp i32 %.pre to float
  br label %for.end18

for.end18:                                        ; preds = %for.end18.loopexit, %entry
  %t5 = phi float [ 0.000000e+00, %entry ], [ %phi.cast, %for.end18.loopexit ]
  %t1.0.lcssa = phi float [ 0.000000e+00, %entry ], [ %t1.2.lcssa.lcssa, %for.end18.loopexit ]
  %add20 = fadd float %t1.0.lcssa, %t5
  ret float %add20
}

attributes #0 = { norecurse nounwind readonly uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="broadwell" "target-features"="+adx,+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++ Compiler Pro 2021.1 (YYYY.x.0.MMDD)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA4_i", !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = !{!8, !8, i64 0}
!8 = !{!"float", !5, i64 0}
