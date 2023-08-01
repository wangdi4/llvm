; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-loop-collapse,print<hir>" -aa-pipeline="basic-aa" -hir-details -disable-output < %s 2>&1 | FileCheck %s

; Verify blob consistency by checking that def@lvl for j loop is correctly set as level 1 after multiplication of blobs due to loop collapse

; *** Source Code ***
;int foo(unsigned X, unsigned Y) {
;  int i, j, k;
;  int sum = 0
;  for (i = 0; i < X; ++i) {
;    for (j = 0; j < Y; ++j) {
;      for (k = 0; k < 10; ++k) {
;        sum += A[i][j][k];
;      }
;    }
;    Y = Y / 2;
;  }
;  return A[0][0][0] + sum + 1;
;}

; CHECK: BEGIN REGION { }
; CHECK:   DO i64 i2 = 0, sext.i32.i64(%Y.addr.037) + -1, 1   <DO_LOOP>
; CHECK:     <RVAL-REG> LINEAR i64 sext.i32.i64(%Y.addr.037) + -1{def@1}
; CHECK:     <BLOB> LINEAR i32 %Y.addr.037{def@1}

; CHECK: BEGIN REGION { modified }
; CHECK:   DO i64 i2 = 0, 10 * sext.i32.i64(%Y.addr.037) + -1, 1   <DO_LOOP>
; CHECK:     <RVAL-REG> LINEAR i64 10 * sext.i32.i64(%Y.addr.037) + -1{def@1}
; CHECK:     <BLOB> LINEAR i32 %Y.addr.037{def@1}


;Module Before HIR
; ModuleID = 'l.c'
source_filename = "l.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = dso_local local_unnamed_addr global [10 x [10 x [10 x i32]]] zeroinitializer, align 16

; Function Attrs: norecurse nounwind readonly uwtable
define dso_local i32 @foo(i32 %X, i32 %Y) local_unnamed_addr #0 {
entry:
  %cmp36 = icmp eq i32 %X, 0
  br i1 %cmp36, label %for.end16, label %for.cond1.preheader.preheader

for.cond1.preheader.preheader:                    ; preds = %entry
  %wide.trip.count47 = sext i32 %X to i64
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.end13, %for.cond1.preheader.preheader
  %indvars.iv45 = phi i64 [ 0, %for.cond1.preheader.preheader ], [ %indvars.iv.next46, %for.end13 ]
  %sum.040 = phi i32 [ 0, %for.cond1.preheader.preheader ], [ %sum.1.lcssa, %for.end13 ]
  %Y.addr.037 = phi i32 [ %Y, %for.cond1.preheader.preheader ], [ %div, %for.end13 ]
  %cmp233 = icmp eq i32 %Y.addr.037, 0
  br i1 %cmp233, label %for.end13, label %for.cond4.preheader.lr.ph

for.cond4.preheader.lr.ph:                        ; preds = %for.cond1.preheader
  %wide.trip.count = sext i32 %Y.addr.037 to i64
  br label %for.cond4.preheader

for.cond4.preheader:                              ; preds = %for.inc11, %for.cond4.preheader.lr.ph
  %indvars.iv42 = phi i64 [ 0, %for.cond4.preheader.lr.ph ], [ %indvars.iv.next43, %for.inc11 ]
  %sum.135 = phi i32 [ %sum.040, %for.cond4.preheader.lr.ph ], [ %add.lcssa, %for.inc11 ]
  br label %for.body6

for.body6:                                        ; preds = %for.body6, %for.cond4.preheader
  %indvars.iv = phi i64 [ 0, %for.cond4.preheader ], [ %indvars.iv.next, %for.body6 ]
  %sum.232 = phi i32 [ %sum.135, %for.cond4.preheader ], [ %add, %for.body6 ]
  %arrayidx10 = getelementptr inbounds [10 x [10 x [10 x i32]]], ptr @A, i64 0, i64 %indvars.iv45, i64 %indvars.iv42, i64 %indvars.iv, !intel-tbaa !2
  %0 = load i32, ptr %arrayidx10, align 4, !tbaa !2
  %add = add nsw i32 %0, %sum.232
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 10
  br i1 %exitcond, label %for.inc11, label %for.body6

for.inc11:                                        ; preds = %for.body6
  %add.lcssa = phi i32 [ %add, %for.body6 ]
  %indvars.iv.next43 = add nuw nsw i64 %indvars.iv42, 1
  %exitcond44 = icmp eq i64 %indvars.iv.next43, %wide.trip.count
  br i1 %exitcond44, label %for.end13.loopexit, label %for.cond4.preheader

for.end13.loopexit:                               ; preds = %for.inc11
  %add.lcssa.lcssa = phi i32 [ %add.lcssa, %for.inc11 ]
  br label %for.end13

for.end13:                                        ; preds = %for.end13.loopexit, %for.cond1.preheader
  %sum.1.lcssa = phi i32 [ %sum.040, %for.cond1.preheader ], [ %add.lcssa.lcssa, %for.end13.loopexit ]
  %div = lshr i32 %Y.addr.037, 1
  %indvars.iv.next46 = add nuw nsw i64 %indvars.iv45, 1
  %exitcond48 = icmp eq i64 %indvars.iv.next46, %wide.trip.count47
  br i1 %exitcond48, label %for.end16.loopexit, label %for.cond1.preheader

for.end16.loopexit:                               ; preds = %for.end13
  %sum.1.lcssa.lcssa = phi i32 [ %sum.1.lcssa, %for.end13 ]
  br label %for.end16

for.end16:                                        ; preds = %for.end16.loopexit, %entry
  %sum.0.lcssa = phi i32 [ 0, %entry ], [ %sum.1.lcssa.lcssa, %for.end16.loopexit ]
  %1 = load i32, ptr @A, align 16, !tbaa !2
  %add17 = add i32 %sum.0.lcssa, 1
  %add18 = add i32 %add17, %1
  ret i32 %add18
}

attributes #0 = { norecurse nounwind readonly uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!2 = !{!3, !6, i64 0}
!3 = !{!"array@_ZTSA10_A10_A10_i", !4, i64 0}
!4 = !{!"array@_ZTSA10_A10_i", !5, i64 0}
!5 = !{!"array@_ZTSA10_i", !6, i64 0}
!6 = !{!"int", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C/C++ TBAA"}
