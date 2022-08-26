; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec \
; RUN:     -vplan-force-vf=4 \
; RUN:     -vplan-enable-peeling \
; RUN:     -debug-only=vplan-scalar-evolution,vplan-alignment-analysis \
; RUN:     -print-before=hir-vplan-vec -disable-output < %s 2>&1 \
; RUN: | FileCheck %s
;
; REQUIRES: asserts
;
; Test handling of strided accesses by VPlanScalarEvolutionHIR.
;
;   void foo(int *dst, int *src, int size) {
;     for (int i = 0; i < size; ++i) {
;       dst[3*i] = src[5*i] + i * i;
;   }
;
; CHECK:       BEGIN REGION { }
; CHECK-NEXT:        %entry.region = @llvm.directive.region.entry(); [ DIR.VPO.AUTO.VEC() ]
; CHECK-NEXT:<{{.*}}>
; CHECK-NEXT:        + DO i1 = 0, zext.i32.i64(%size) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
; CHECK-NEXT:        |   %1 = (%src)[5 * i1];
; CHECK-NEXT:        |   %mul1 = i1  *  i1;
; CHECK-NEXT:        |   (%dst)[3 * i1] = %1 + %mul1;
; CHECK-NEXT:        + END LOOP
; CHECK-NEXT:<{{.*}}>
; CHECK-NEXT:        @llvm.directive.region.exit(%entry.region); [ DIR.VPO.END.AUTO.VEC() ]
; CHECK-NEXT:  END REGION
; CHECK-EMPTY:
; CHECK-NEXT:  computeAddressSCEV([DA: Div] i32 %vp{{.*}} = load i32* [[LD_PTR:%.*]])
; CHECK-NEXT:    -> {(%src),+,20}
; CHECK-NEXT:  computeAddressSCEV([DA: Div] store i32 %vp{{.*}} i32* [[ST_PTR:%.*]])
; CHECK-NEXT:    -> {(%dst),+,12}
;

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(i32* noalias %dst, i32* noalias %src, i32 %size) {
entry:
  %cmp11 = icmp sgt i32 %size, 0
  br i1 %cmp11, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %wide.trip.count15 = zext i32 %size to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.body
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %0 = mul nuw nsw i64 %indvars.iv, 5
  %ptridx = getelementptr inbounds i32, i32* %src, i64 %0
  %1 = load i32, i32* %ptridx, align 4
  %2 = trunc i64 %indvars.iv to i32
  %mul1 = mul nsw i32 %2, %2
  %add = add nsw i32 %1, %mul1
  %3 = mul nuw nsw i64 %indvars.iv, 3
  %ptridx4 = getelementptr inbounds i32, i32* %dst, i64 %3
  store i32 %add, i32* %ptridx4, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count15
  br i1 %exitcond.not, label %for.cond.cleanup.loopexit, label %for.body
}
