; TODO; enable when const prop turned on
;  XFAIL: *
;
; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-pre-vec-complete-unroll -print-before=hir-pre-vec-complete-unroll -disable-output -print-after=hir-pre-vec-complete-unroll 2>&1 < %s | FileCheck %s

; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-pre-vec-complete-unroll,print<hir>" -disable-output 2>&1 < %s | FileCheck %s

; Verify constant array access is replaced after unrolling and downstream code is
; simplified and removed.

; Source Code:

;int const glob[2][2] = {{1,2},{3,0}};
;int foo (int sum) {
;  #pragma nounroll
;  for (int k = 0; k< 10 ; ++k)
;    for (int i = 0; i< 2 ; ++i)
;      for (int j = 0;j<2;j++)
;        if (glob[i][j] != 0)
;          sum += glob[j][i]*glob[i][j];
;}

; CHECK: Function:
; CHECK: BEGIN REGION { }
; CHECK:       + DO i1 = 0, 9, 1   <DO_LOOP> <nounroll>
; CHECK:       |   + DO i2 = 0, 1, 1   <DO_LOOP>
; CHECK:       |   |   + DO i3 = 0, 1, 1   <DO_LOOP>
; CHECK:       |   |   |   %0 = (@glob)[0][i2][i3];
; CHECK:       |   |   |   if (%0 != 0)
; CHECK:       |   |   |   {
; CHECK:       |   |   |      %1 = (@glob)[0][i3][i2];
; CHECK:       |   |   |      %sum.addr.043 = (%0 * %1)  +  %sum.addr.043;
; CHECK:       |   |   |   }
; CHECK:       |   |   + END LOOP
; CHECK:       |   + END LOOP
; CHECK:       + END LOOP
; CHECK: END REGION

; CHECK: Function:
; CHECK: BEGIN REGION { modified }
; CHECK: + DO i1 = 0, 9, 1
; CHECK: |   %0 = 1;
; CHECK-NOT: if
; CHECK: |   %1 = 1;
; CHECK: |   %sum.addr.043 = (%0 * %1)  +  %sum.addr.043;
; CHECK: |   %0 = 2;
; CHECK-NOT: if
; CHECK: |   %1 = 3;
; CHECK: |   %sum.addr.043 = (%0 * %1)  +  %sum.addr.043;
; CHECK: |   %0 = 3;
; CHECK-NOT: if
; CHECK: |   %1 = 2;
; CHECK: |   %sum.addr.043 = (%0 * %1)  +  %sum.addr.043;
; CHECK: |   %0 = 0;
; CHECK-NOT: if
; CHECK: + END LOOP
; CHECK: END REGION

;Module Before HIR
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@glob = dso_local local_unnamed_addr constant [2 x [2 x i32]] [[2 x i32] [i32 1, i32 2], [2 x i32] [i32 3, i32 0]], align 16

; Function Attrs: norecurse nounwind readnone uwtable
define dso_local i32 @foo(i32 %sum) local_unnamed_addr #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond.cleanup3, %entry
  %k.044 = phi i32 [ 0, %entry ], [ %inc24, %for.cond.cleanup3 ]
  %sum.addr.043 = phi i32 [ %sum, %entry ], [ %sum.addr.3.lcssa.lcssa, %for.cond.cleanup3 ]
  br label %for.cond5.preheader

for.cond.cleanup:                                 ; preds = %for.cond.cleanup3
  %sum.addr.3.lcssa.lcssa.lcssa = phi i32 [ %sum.addr.3.lcssa.lcssa, %for.cond.cleanup3 ]
  ret i32 %sum.addr.3.lcssa.lcssa.lcssa

for.cond5.preheader:                              ; preds = %for.cond1.preheader, %for.cond.cleanup7
  %cmp2 = phi i1 [ true, %for.cond1.preheader ], [ false, %for.cond.cleanup7 ]
  %indvars.iv45 = phi i64 [ 0, %for.cond1.preheader ], [ 1, %for.cond.cleanup7 ]
  %sum.addr.141 = phi i32 [ %sum.addr.043, %for.cond1.preheader ], [ %sum.addr.3.lcssa, %for.cond.cleanup7 ]
  br label %for.body8

for.cond.cleanup3:                                ; preds = %for.cond.cleanup7
  %sum.addr.3.lcssa.lcssa = phi i32 [ %sum.addr.3.lcssa, %for.cond.cleanup7 ]
  %inc24 = add nuw nsw i32 %k.044, 1
  %exitcond = icmp eq i32 %inc24, 10
  br i1 %exitcond, label %for.cond.cleanup, label %for.cond1.preheader, !llvm.loop !2

for.cond.cleanup7:                                ; preds = %for.inc
  %sum.addr.3.lcssa = phi i32 [ %sum.addr.3, %for.inc ]
  br i1 %cmp2, label %for.cond5.preheader, label %for.cond.cleanup3

for.body8:                                        ; preds = %for.cond5.preheader, %for.inc
  %cmp6 = phi i1 [ true, %for.cond5.preheader ], [ false, %for.inc ]
  %indvars.iv = phi i64 [ 0, %for.cond5.preheader ], [ 1, %for.inc ]
  %sum.addr.239 = phi i32 [ %sum.addr.141, %for.cond5.preheader ], [ %sum.addr.3, %for.inc ]
  %arrayidx10 = getelementptr inbounds [2 x [2 x i32]], [2 x [2 x i32]]* @glob, i64 0, i64 %indvars.iv45, i64 %indvars.iv, !intel-tbaa !4
  %0 = load i32, i32* %arrayidx10, align 4, !tbaa !4
  %cmp11 = icmp eq i32 %0, 0
  br i1 %cmp11, label %for.inc, label %if.then

if.then:                                          ; preds = %for.body8
  %arrayidx15 = getelementptr inbounds [2 x [2 x i32]], [2 x [2 x i32]]* @glob, i64 0, i64 %indvars.iv, i64 %indvars.iv45, !intel-tbaa !4
  %1 = load i32, i32* %arrayidx15, align 4, !tbaa !4
  %mul = mul nsw i32 %1, %0
  %add = add nsw i32 %mul, %sum.addr.239
  br label %for.inc

for.inc:                                          ; preds = %for.body8, %if.then
  %sum.addr.3 = phi i32 [ %add, %if.then ], [ %sum.addr.239, %for.body8 ]
  br i1 %cmp6, label %for.body8, label %for.cond.cleanup7
}

attributes #0 = { norecurse nounwind readnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!2 = distinct !{!2, !3}
!3 = !{!"llvm.loop.unroll.disable"}
!4 = !{!5, !7, i64 0}
!5 = !{!"array@_ZTSA2_A2_i", !6, i64 0}
!6 = !{!"array@_ZTSA2_i", !7, i64 0}
!7 = !{!"int", !8, i64 0}
!8 = !{!"omnipotent char", !9, i64 0}
!9 = !{!"Simple C/C++ TBAA"}
