; REQUIRES: asserts
; RUN: opt -enable-new-pm=0 -O2 -debug-only=hir-loop-interchange  -hir-ssa-deconstruction -hir-temp-cleanup -hir-loop-interchange -print-after=hir-loop-interchange  < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-sinking-for-perfect-loopnest,hir-loop-interchange,print<hir>" -aa-pipeline="basic-aa" -debug-only=hir-loop-interchange < %s 2>&1 | FileCheck %s
; CHECK:  Loopnest Interchanged: ( 1 2 3 ) --> ( 3 2 1 )
;
; Source code:
;
; int a[T][T];
; int b[T][T];
; int c[T][T];
;
;void f(int N) {
;int i, j, k;
;for(j=0;j<N; j++)
;for(i=0;i<N; i++)
;for(k=0;k<N; k++)
;c[i][j]= c[i][j] + a[k + 1][i] * b[k][j+1] + 1;
;}

;***IR Dump Before HIR Loop Interchange ***
;
;<40>         + DO i1 = 0, sext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 999>
;<41>         |   + DO i2 = 0, sext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1000>
;<42>         |   |   + DO i3 = 0, sext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 999>
;<7>          |   |   |   %1 = (@c)[0][i2][i1];
;<15>         |   |   |   %2 = (@a)[0][i3 + 1][i2];
;<17>         |   |   |   %3 = (@b)[0][i3][i1 + 1];
;<20>         |   |   |   %1 = %1 + 1  +  (%2 * %3);
;<27>         |   |   |   (@c)[0][i2][i1] = %1;
;<42>         |   |   + END LOOP
;<41>         |   + END LOOP
;<40>         + END LOOP
;
;***IR Dump After HIR Loop Interchange ***
;
;<0>BEGIN REGION { modified }
;<40>+ DO i1 = 0, sext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 999>
;<41>|   + DO i2 = 0, sext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1000>
;<42>|   |   + DO i3 = 0, sext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 999>
;<7> |   |   |   %1 = (@c)[0][i2][i3];
;<15>|   |   |   %2 = (@a)[0][i1 + 1][i2];
;<17>|   |   |   %3 = (@b)[0][i1][i3 + 1];
;<20>|   |   |   %1 = %1 + 1  +  (%2 * %3);
;<27>|   |   |   (@c)[0][i2][i3] = %1;
;<42>|   |   + END LOOP
;<41>|   + END LOOP
;<40>+ END LOOP
;<0>END REGION
;
;Module Before HIR; ModuleID = 'mt6.c'
source_filename = "mt6.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@c = common local_unnamed_addr global [1000 x [1000 x i32]] zeroinitializer, align 16
@a = common local_unnamed_addr global [1000 x [1000 x i32]] zeroinitializer, align 16
@b = common local_unnamed_addr global [1000 x [1000 x i32]] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define void @f(i32 %N) local_unnamed_addr #0 {
entry:
  %cmp49 = icmp sgt i32 %N, 0
  br i1 %cmp49, label %for.body.lr.ph, label %for.end29

for.body.lr.ph:                                   ; preds = %entry
  %wide.trip.count = sext i32 %N to i64
  br label %for.body3.lr.ph

for.body3.lr.ph:                                  ; preds = %for.body.lr.ph, %for.inc27
  %indvars.iv55 = phi i64 [ 0, %for.body.lr.ph ], [ %0, %for.inc27 ]
  %0 = add nuw nsw i64 %indvars.iv55, 1
  br label %for.body6.lr.ph

for.body6.lr.ph:                                  ; preds = %for.body3.lr.ph, %for.inc24
  %indvars.iv51 = phi i64 [ 0, %for.body3.lr.ph ], [ %indvars.iv.next52, %for.inc24 ]
  %arrayidx8 = getelementptr inbounds [1000 x [1000 x i32]], [1000 x [1000 x i32]]* @c, i64 0, i64 %indvars.iv51, i64 %indvars.iv55
  %arrayidx8.promoted = load i32, i32* %arrayidx8, align 4, !tbaa !2
  br label %for.body6

for.body6:                                        ; preds = %for.body6, %for.body6.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body6.lr.ph ], [ %indvars.iv.next, %for.body6 ]
  %1 = phi i32 [ %arrayidx8.promoted, %for.body6.lr.ph ], [ %add19, %for.body6 ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %arrayidx12 = getelementptr inbounds [1000 x [1000 x i32]], [1000 x [1000 x i32]]* @a, i64 0,  i64 %indvars.iv.next, i64 %indvars.iv51
  %2 = load i32, i32* %arrayidx12, align 4, !tbaa !2
  %arrayidx17 = getelementptr inbounds [1000 x [1000 x i32]], [1000 x [1000 x i32]]* @b, i64 0, i64 %indvars.iv, i64 %0
  %3 = load i32, i32* %arrayidx17, align 4, !tbaa !2
  %mul = mul nsw i32 %3, %2
  %add18 = add i32 %1, 1
  %add19 = add i32 %add18, %mul
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.inc24, label %for.body6

for.inc24:                                        ; preds = %for.body6
  %add19.lcssa = phi i32 [ %add19, %for.body6 ]
  store i32 %add19.lcssa, i32* %arrayidx8, align 4, !tbaa !2
  %indvars.iv.next52 = add nuw nsw i64 %indvars.iv51, 1
  %exitcond54 = icmp eq i64 %indvars.iv.next52, %wide.trip.count
  br i1 %exitcond54, label %for.inc27, label %for.body6.lr.ph

for.inc27:                                        ; preds = %for.inc24
  %exitcond59 = icmp eq i64 %0, %wide.trip.count
  br i1 %exitcond59, label %for.end29.loopexit, label %for.body3.lr.ph

for.end29.loopexit:                               ; preds = %for.inc27
  br label %for.end29

for.end29:                                        ; preds = %for.end29.loopexit, %entry
  ret void
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" "loopopt-pipeline"="full" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 6.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 57303327e688d928c77069562958db1ee842a174) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm ca1ea02a1798f131c12c0826a70e87ff2ab6cb8a)"}
!2 = !{!3, !5, i64 0}
!3 = !{!"array@_ZTSA1000_A1000_i", !4, i64 0}
!4 = !{!"array@_ZTSA1000_i", !5, i64 0}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
