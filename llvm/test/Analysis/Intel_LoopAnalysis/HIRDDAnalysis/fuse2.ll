;    for (j=0; j <  n; j++) {
;        B[j] =  1;
;    }
;    for (j=n; j <  2*n; j++) {
;        B[j] =  2;
;    }
;   No dep expected when invoked from demand driven DD  
;
; RUN:  opt < %s -hir-ssa-deconstruction -hir-create-function-level-region | opt -hir-create-function-level-region -hir-dd-test-assume-loop-fusion -hir-dd-analysis  -hir-dd-analysis-verify=Region -analyze  | FileCheck %s 
; CHECK: 'HIR Data Dependence Analysis' 
; CHECK-NOT:  (%B)[0][i1] -->  (%B)[i1 + sext.i32.i64(%n)] 
; ModuleID = 'fuse2.c'
source_filename = "fuse2.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @sub1(float* nocapture %B, i32 %n) local_unnamed_addr #0 {
entry:
  %cmp21 = icmp sgt i32 %n, 0
  br i1 %cmp21, label %for.body.preheader, label %for.cond1.preheader

for.body.preheader:                               ; preds = %entry
  %wide.trip.count25 = sext i32 %n to i64
  br label %for.body

for.cond1.preheader.loopexit:                     ; preds = %for.body
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond1.preheader.loopexit, %entry
  %mul = shl nsw i32 %n, 1
  %cmp219 = icmp sgt i32 %mul, %n
  br i1 %cmp219, label %for.body3.preheader, label %for.end8

for.body3.preheader:                              ; preds = %for.cond1.preheader
  %0 = sext i32 %n to i64
  %wide.trip.count = sext i32 %mul to i64
  br label %for.body3

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv23 = phi i64 [ %indvars.iv.next24, %for.body ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr inbounds float, float* %B, i64 %indvars.iv23
  store float 1.000000e+00, float* %arrayidx, align 4, !tbaa !1
  %indvars.iv.next24 = add nuw nsw i64 %indvars.iv23, 1
  %exitcond26 = icmp eq i64 %indvars.iv.next24, %wide.trip.count25
  br i1 %exitcond26, label %for.cond1.preheader.loopexit, label %for.body

for.body3:                                        ; preds = %for.body3, %for.body3.preheader
  %indvars.iv = phi i64 [ %0, %for.body3.preheader ], [ %indvars.iv.next, %for.body3 ]
  %arrayidx5 = getelementptr inbounds float, float* %B, i64 %indvars.iv
  store float 2.000000e+00, float* %arrayidx5, align 4, !tbaa !1
  %indvars.iv.next = add nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.end8.loopexit, label %for.body3

for.end8.loopexit:                                ; preds = %for.body3
  br label %for.end8

for.end8:                                         ; preds = %for.end8.loopexit, %for.cond1.preheader
  ret void
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 5.0.0 (cfe/trunk)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"float", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
