; REQUIRES: asserts
; RUN: opt -S -passes=vplan-vec -debug-only=VPlanHCFGBuilder -disable-output < %s 2>&1 | FileCheck %s

;float fp_inc;
;void fp_iv_loop(float init, float * __restrict__ A, int N) {
;  float x = init;
;  for (int i=0; i < N; ++i) {
;    A[i] = x;
;    x -= fp_inc;
;  }
;}

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@fp_inc = local_unnamed_addr global float 0.000000e+00, align 4

define void @fp_iv_loop(float %init, ptr noalias nocapture %A, i32 %N) local_unnamed_addr #0 {
;
; CHECK-NOT: The trip count for loop
; CHECK: Could not estimate trip count for loop 1 using ScalarEvolution
;
entry:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %DIR.QUAL.LIST.END.2

DIR.QUAL.LIST.END.2:
  %cmp6 = icmp sgt i32 %N, 0
  br i1 %cmp6, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:                                   ; preds = %DIR.QUAL.LIST.END.2
  %0 = load float, ptr @fp_inc, align 4
  %wide.trip.count = zext i32 %N to i64
  br label %for.body

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %x.07 = phi float [ %init, %for.body.lr.ph ], [ %sub, %for.body ]
  %arrayidx = getelementptr inbounds float, ptr %A, i64 %indvars.iv
  store float %x.07, ptr %arrayidx, align 4
  %sub = fsub fast float %x.07, %0
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup.loopexit, label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.body
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %DIR.QUAL.LIST.END.2
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.QUAL.LIST.END.3

DIR.QUAL.LIST.END.3:
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
