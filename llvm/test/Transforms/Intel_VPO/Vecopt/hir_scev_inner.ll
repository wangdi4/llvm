; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec \
; RUN:     -vplan-force-vf=4 \
; RUN:     -vplan-enable-peeling-hir -vplan-enable-general-peeling-hir \
; RUN:     -debug-only=vplan-scalar-evolution,vplan-alignment-analysis \
; RUN:     -print-before=hir-vplan-vec -disable-output < %s 2>&1 \
; RUN: | FileCheck %s
;
; REQUIRES: asserts
;
; In this test there are 2 nested loops in HIR region, and only the
; inner loop is vectorized by VPlan. The purpose of this test is to
; check that outer loop IVs are handled properly.
;
;   void foo(int *dst, int *src, int size, int x) {
;     for (int i = 0; i < size; ++i) {
;       dst[i] = dst[i + x];
;       for (int j = 0; j < size; ++j) {
;         dst[1024*i + j] = src[2048*i + j];
;     }
;   }
;
; CHECK:       BEGIN REGION { }
; CHECK-NEXT:        + DO i1 = 0, zext.i32.i64(%size) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
; CHECK-NEXT:        |   %2 = (%dst)[i1 + sext.i32.i64(%x)];
; CHECK-NEXT:        |   (%dst)[i1] = %2;
; CHECK-NEXT:        |   %entry.region = @llvm.directive.region.entry(); [ DIR.VPO.AUTO.VEC() ]
; CHECK-NEXT:        |
; CHECK-NEXT:        |   + DO i2 = 0, zext.i32.i64(%size) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
; CHECK-NEXT:        |   |   %6 = (%src)[2048 * i1 + i2];
; CHECK-NEXT:        |   |   (%dst)[1024 * i1 + i2] = %6;
; CHECK-NEXT:        |   + END LOOP
; CHECK-NEXT:        |
; CHECK-NEXT:        |   @llvm.directive.region.exit(%entry.region); [ DIR.VPO.END.AUTO.VEC() ]
; CHECK-NEXT:        + END LOOP
; CHECK-NEXT:  END REGION
; CHECK-EMPTY:
; CHECK-NEXT:  computeAddressSCEV([DA: Div] i32 %vp{{.*}} = load i32* [[LD_PTR:%.*]])
; CHECK-NEXT:    -> {(8192 * i1 + %src),+,4}
; CHECK-NEXT:  computeAddressSCEV([DA: Div] store i32 %vp{{.*}} i32* [[ST_PTR:%.*]])
; CHECK-NEXT:    -> {(4096 * i1 + %dst),+,4}
; CHECK-NEXT:  computeAddressSCEV(i32 %vp{{.*}} = load i32* [[LD_PTR_CLONE:%.*]])
; CHECK-NEXT:    -> {(8192 * i1 + %src),+,4}
; CHECK-NEXT:  computeAddressSCEV(store i32 %vp{{.*}} i32* [[ST_PTR_CLONE:%.*]])
; CHECK-NEXT:    -> {(4096 * i1 + %dst),+,4}
; CHECK-NEXT:  getMinusExpr({(4096 * i1 + %dst),+,0},
; CHECK-NEXT:               {(8192 * i1 + %src),+,0})
; CHECK-NEXT:    -> {(-4096 * i1 + %dst + -1 * %src),+,0}
;

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(i32* noalias %dst, i32* noalias %src, i32 %x, i32 %size) {
entry:
  %cmp32 = icmp sgt i32 %size, 0
  br i1 %cmp32, label %for.body.us.preheader, label %for.cond.cleanup

for.body.us.preheader:                            ; preds = %entry
  %0 = sext i32 %x to i64
  %wide.trip.count4446 = zext i32 %size to i64
  br label %for.body.us

for.body.us:                                      ; preds = %for.body.us.preheader, %for.cond3.for.cond.cleanup5_crit_edge.us
  %indvars.iv39 = phi i64 [ 0, %for.body.us.preheader ], [ %indvars.iv.next40, %for.cond3.for.cond.cleanup5_crit_edge.us ]
  %1 = add nsw i64 %indvars.iv39, %0
  %ptridx.us = getelementptr inbounds i32, i32* %dst, i64 %1
  %2 = load i32, i32* %ptridx.us, align 4
  %ptridx2.us = getelementptr inbounds i32, i32* %dst, i64 %indvars.iv39
  store i32 %2, i32* %ptridx2.us, align 4
  %3 = shl nsw i64 %indvars.iv39, 11
  %4 = shl nsw i64 %indvars.iv39, 10
  br label %for.body6.us

for.body6.us:                                     ; preds = %for.body.us, %for.body6.us
  %indvars.iv = phi i64 [ 0, %for.body.us ], [ %indvars.iv.next, %for.body6.us ]
  %5 = add nuw nsw i64 %indvars.iv, %3
  %ptridx9.us = getelementptr inbounds i32, i32* %src, i64 %5
  %6 = load i32, i32* %ptridx9.us, align 4
  %7 = add nuw nsw i64 %indvars.iv, %4
  %ptridx13.us = getelementptr inbounds i32, i32* %dst, i64 %7
  store i32 %6, i32* %ptridx13.us, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count4446
  br i1 %exitcond.not, label %for.cond3.for.cond.cleanup5_crit_edge.us, label %for.body6.us

for.cond3.for.cond.cleanup5_crit_edge.us:         ; preds = %for.body6.us
  %indvars.iv.next40 = add nuw nsw i64 %indvars.iv39, 1
  %exitcond45.not = icmp eq i64 %indvars.iv.next40, %wide.trip.count4446
  br i1 %exitcond45.not, label %for.cond.cleanup.loopexit, label %for.body.us

for.cond.cleanup.loopexit:                        ; preds = %for.cond3.for.cond.cleanup5_crit_edge.us
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void
}
