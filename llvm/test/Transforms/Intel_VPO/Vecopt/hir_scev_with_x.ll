; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec \
; RUN:     -vplan-force-vf=4 \
; RUN:     -vplan-enable-peeling-hir -vplan-enable-general-peeling-hir \
; RUN:     -debug-only=vplan-scalar-evolution,vplan-alignment-analysis \
; RUN:     -print-before=hir-vplan-vec -disable-output < %s 2>&1 \
; RUN: | FileCheck %s
;
; REQUIRES: asserts
;
; This is a test for memory accesses with an unknown address component
; (%x). Such pointers are very important for peeling analysis. If
; pointers like (%src + %x + %i1) and (%dst + %x + %i1) can be properly
; represented and subtracted, we can prove that the two accesses can
; be aligned simultaneously even if we don't know anything about %x.
;
;   void foo(int *dst, int *src, long x, long size) {
;     for (long i = 0; i < size; ++i) {
;       dst[5*x + i + 7] = src[5*x + i + 7] + i * i;
;   }
;
; CHECK:       BEGIN REGION { }
; CHECK-NEXT:        %entry.region = @llvm.directive.region.entry(); [ DIR.VPO.AUTO.VEC() ]
; CHECK-NEXT:<{{.*}}>
; CHECK-NEXT:        + DO i1 = 0, %size + -1, 1   <DO_LOOP>
; CHECK-NEXT:        |   %0 = (%src)[i1 + 5 * %x + 7];
; CHECK-NEXT:        |   %1 = i1  *  i1;
; CHECK-NEXT:        |   %2 = trunc.i64.i32(%1);
; CHECK-NEXT:        |   (%dst)[i1 + 5 * %x + 7] = %0 + %2;
; CHECK-NEXT:        + END LOOP
; CHECK-NEXT:<{{.*}}>
; CHECK-NEXT:        @llvm.directive.region.exit(%entry.region); [ DIR.VPO.END.AUTO.VEC() ]
; CHECK-NEXT:  END REGION
; CHECK-EMPTY:
; CHECK-NEXT:  computeAddressSCEV([DA: Div] i32 %vp{{.*}} = load i32* [[LD_PTR:%.*]])
; CHECK-NEXT:    -> {(%src + 20 * %x + 28),+,4}
; CHECK-NEXT:  computeAddressSCEV([DA: Div] store i32 %vp{{.*}} i32* [[ST_PTR:%.*]])
; CHECK-NEXT:    -> {(20 * %x + %dst + 28),+,4}
; CHECK-NEXT:  computeAddressSCEV(i32 %vp{{.*}} = load i32* [[LD_PTR_CLONE:%.*]])
; CHECK-NEXT:    -> {(%src + 20 * %x + 28),+,4}
; CHECK-NEXT:  computeAddressSCEV(store i32 %vp{{.*}} i32* [[ST_PTR_CLONE:%.*]])
; CHECK-NEXT:    -> {(20 * %x + %dst + 28),+,4}
; CHECK-NEXT:  getMinusExpr({(20 * %x + %dst + 28),+,0},
; CHECK-NEXT:               {(%src + 20 * %x + 28),+,0})
; CHECK-NEXT:    -> {(-1 * %src + %dst),+,0}
;

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(i32* noalias %dst, i32* noalias %src, i64 %x, i64 %size) {
entry:
  %cmp16 = icmp sgt i64 %size, 0
  br i1 %cmp16, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:                                   ; preds = %entry
  %mul = mul nsw i64 %x, 5
  %add = add i64 %mul, 7
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.body
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body.lr.ph, %for.body
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %add1 = add i64 %add, %indvars.iv
  %ptridx = getelementptr inbounds i32, i32* %src, i64 %add1
  %0 = load i32, i32* %ptridx, align 4
  %1 = mul nsw i64 %indvars.iv, %indvars.iv
  %2 = trunc i64 %1 to i32
  %add3 = add nsw i32 %0, %2
  %ptridx8 = getelementptr inbounds i32, i32* %dst, i64 %add1
  store i32 %add3, i32* %ptridx8, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %size
  br i1 %exitcond.not, label %for.cond.cleanup.loopexit, label %for.body
}
