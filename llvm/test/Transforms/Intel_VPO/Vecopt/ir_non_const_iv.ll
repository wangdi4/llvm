; RUN: opt -S -VPlanDriver -disable-vplan-predicator -disable-vplan-subregions -vplan-build-stress-test -instcombine < %s | FileCheck %s

;int inc_x;
;int foo(int * __restrict__ A, int N, int init) {
;  int x = init;
;#pragma omp simd
;  for (int i=0; i < N; i++) {
;    A[i] = x;
;    x += inc_x;
;  }
;  return x;
;}

; CHECK-LABEL: foo
; CHECK: vector.ph:                                        ; preds = %min.iters.checked
; CHECK:  %[[SPLAT:.*splat[0-9]*]] = shufflevector <4 x i32> {{.*}}, <4 x i32> undef, <4 x i32> zeroinitializer
; CHECK:  br label %vector.body
; CHECK: vector.body:                                      ; preds = %vector.body, %vector.ph
; CHECK:   %index = phi i64 [ 0, %vector.ph ], [ %index.next, %vector.body ]
; CHECK:   %[[VEC_IND:.*]] = phi <4 x i32> [ %induction, %vector.ph ], [ %[[VEC_IND_NEXT:.*]], %vector.body ]
; CHECK:   %[[VEC_IND_NEXT]] = add <4 x i32> %[[VEC_IND]], %.splat


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

@inc_x = common global i32 0, align 4

define i32 @foo(i32* noalias nocapture %A, i32 %N, i32 %init) local_unnamed_addr #0 {
entry:
  %cmp7 = icmp sgt i32 %N, 0
  br i1 %cmp7, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:                                   ; preds = %entry
  %0 = load i32, i32* @inc_x, align 4
  %1 = mul i32 %0, %N
  %wide.trip.count = zext i32 %N to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.body
  %2 = add i32 %1, %init
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  %x.0.lcssa = phi i32 [ %init, %entry ], [ %2, %for.cond.cleanup.loopexit ]
  ret i32 %x.0.lcssa

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %x.08 = phi i32 [ %init, %for.body.lr.ph ], [ %add, %for.body ]
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %indvars.iv
  store i32 %x.08, i32* %arrayidx, align 4
  %add = add nsw i32 %0, %x.08
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup.loopexit, label %for.body
}
