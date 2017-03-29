; Testing for Distance Vector
;  M[k][j-1][i] = M[k+4][j][i] + M[k-m][j+2][i-2] +1;
; RUN:  opt < %s   -hir-ssa-deconstruction | opt  -hir-dd-analysis  -hir-dd-analysis-verify=Region  -analyze  | FileCheck %s
; CHECK-DAG: FLOW (= = =) (0 0 0)  
; CHECK-DAG: ANTI (< < =) (4 1 0)  
; CHECK-DAG: FLOW (* < >) (? 3 -2)  
;
; ModuleID = 'distanceVector.c'
source_filename = "distanceVector.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@M = common local_unnamed_addr global [10 x [10 x [10 x i32]]] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define double @sub(double* nocapture readnone %A, double* nocapture readnone %B, i32 %n, i32 %m) local_unnamed_addr #0 {
entry:
  %cmp56 = icmp sgt i32 %n, 0
  br i1 %cmp56, label %for.cond1.preheader.lr.ph, label %for.end33

for.cond1.preheader.lr.ph:                        ; preds = %entry
  %cmp552 = icmp eq i32 %n, 1
  %0 = sext i32 %n to i64
  %1 = sext i32 %m to i64
  %wide.trip.count = zext i32 %n to i64
  %wide.trip.count67 = zext i32 %n to i64
  br label %for.cond4.preheader.lr.ph

for.cond4.preheader.lr.ph:                        ; preds = %for.cond1.preheader.lr.ph, %for.inc31
  %indvars.iv63 = phi i64 [ 0, %for.cond1.preheader.lr.ph ], [ %indvars.iv.next64, %for.inc31 ]
  %2 = add nuw nsw i64 %indvars.iv63, 4
  %3 = sub nsw i64 %indvars.iv63, %1
  br label %for.cond4.preheader

for.cond4.preheader:                              ; preds = %for.inc28, %for.cond4.preheader.lr.ph
  %indvars.iv59 = phi i64 [ 0, %for.cond4.preheader.lr.ph ], [ %indvars.iv.next60, %for.inc28 ]
  br i1 %cmp552, label %for.inc28, label %for.body6.lr.ph

for.body6.lr.ph:                                  ; preds = %for.cond4.preheader
  %4 = add nuw nsw i64 %indvars.iv59, 2
  %5 = add nsw i64 %indvars.iv59, -1
  br label %for.body6

for.body6:                                        ; preds = %for.body6.lr.ph, %for.body6
  %indvars.iv = phi i64 [ 1, %for.body6.lr.ph ], [ %indvars.iv.next, %for.body6 ]
  %arrayidx10 = getelementptr inbounds [10 x [10 x [10 x i32]]], [10 x [10 x [10 x i32]]]* @M, i64 0, i64 %2, i64 %indvars.iv59, i64 %indvars.iv
  %6 = load i32, i32* %arrayidx10, align 4, !tbaa !1
  %7 = add nsw i64 %indvars.iv, -2
  %arrayidx18 = getelementptr inbounds [10 x [10 x [10 x i32]]], [10 x [10 x [10 x i32]]]* @M, i64 0, i64 %3, i64 %4, i64 %7
  %8 = load i32, i32* %arrayidx18, align 4, !tbaa !1
  %add19 = add i32 %6, 1
  %add20 = add i32 %add19, %8
  %arrayidx27 = getelementptr inbounds [10 x [10 x [10 x i32]]], [10 x [10 x [10 x i32]]]* @M, i64 0, i64 %indvars.iv63, i64 %5, i64 %indvars.iv
  store i32 %add20, i32* %arrayidx27, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %cmp5 = icmp slt i64 %indvars.iv.next, %0
  br i1 %cmp5, label %for.body6, label %for.inc28.loopexit

for.inc28.loopexit:                               ; preds = %for.body6
  br label %for.inc28

for.inc28:                                        ; preds = %for.inc28.loopexit, %for.cond4.preheader
  %indvars.iv.next60 = add nuw nsw i64 %indvars.iv59, 1
  %exitcond = icmp eq i64 %indvars.iv.next60, %wide.trip.count
  br i1 %exitcond, label %for.inc31, label %for.cond4.preheader

for.inc31:                                        ; preds = %for.inc28
  %indvars.iv.next64 = add nuw nsw i64 %indvars.iv63, 1
  %exitcond68 = icmp eq i64 %indvars.iv.next64, %wide.trip.count67
  br i1 %exitcond68, label %for.end33.loopexit, label %for.cond4.preheader.lr.ph

for.end33.loopexit:                               ; preds = %for.inc31
  br label %for.end33

for.end33:                                        ; preds = %for.end33.loopexit, %entry
  ret double 0.000000e+00
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (trunk 20848) (llvm/branches/loopopt 21002)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
