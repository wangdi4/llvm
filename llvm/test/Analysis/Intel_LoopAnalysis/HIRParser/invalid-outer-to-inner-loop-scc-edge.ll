; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -disable-output  2>&1 | FileCheck %s

; Verify that we invalidate the SCC (%add41.lcssa -> %add41 -> %add41.lcssa78)
; because collpasing %add41 and %add41.lcssa78 would make it recursive in the
; inner loop.

; CHECK-NOT: SCC1

; CHECK: + DO i1 = 0, %div + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
; CHECK: |   + DO i2 = 0, 9, 1   <DO_LOOP>
; CHECK: |   |   %ld1 = (%ptr1)[2 * i2 + 2];
; CHECK: |   |   %ld2 = (%ptr2)[2 * i2 + 2];
; CHECK: |   |   %mul39 = %ld1  *  %ld2;
; CHECK: |   |   %add41 = %add41.lcssa78  +  %mul39;
; CHECK: |   |   (%ptr1)[2 * i2 + 2] = %add41;
; CHECK: |   + END LOOP
; CHECK: |
; CHECK: |   %add41.lcssa78 = %add41;
; CHECK: + END LOOP


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #0

define void @foo(i32 %div, ptr noalias nocapture %ptr1, ptr noalias nocapture readonly %ptr2) {
alloca_2:
  %rel = icmp slt i32 %div, 1
  br i1 %rel, label %bb95, label %outerloop.preheader

outerloop.preheader:                                  ; preds = %alloca_2
  %0 = add nuw nsw i32 %div, 1
  br label %outerloop

outerloop:                                            ; preds = %bb134, %outerloop.preheader
  %add41.lcssa78 = phi double [ %add41.lcssa, %bb134 ], [ 0.000000e+00, %outerloop.preheader ]
  %j = phi i32 [ %add61, %bb134 ], [ 1, %outerloop.preheader ]
  br label %innerloop

innerloop:                                            ; preds = %innerloop, %outerloop
  %indvars.iv = phi i64 [ %indvars.iv.next, %innerloop ], [ 1, %outerloop ]
  %indvars.iv.next = add nsw i64 %indvars.iv, 2
  %1 = add nsw i64 %indvars.iv, 1
  %gep1 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %ptr1, i64 %indvars.iv.next)
  %ld1 = load double, ptr %gep1, align 1
  %gep2 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %ptr2, i64 %indvars.iv.next)
  %ld2 = load double, ptr %gep2, align 1
  %mul39 = fmul double %ld1, %ld2
  %add41 = fadd double %add41.lcssa78, %mul39
  store double %add41, ptr %gep1, align 1
  %exitcond = icmp eq i64 %indvars.iv.next, 21
  br i1 %exitcond, label %bb134, label %innerloop

bb134:                                            ; preds = %innerloop
  %add41.lcssa = phi double [ %add41, %innerloop ]
  %add61 = add nuw nsw i32 %j, 1
  %exitcond80 = icmp eq i32 %add61, %0
  br i1 %exitcond80, label %bb95.loopexit, label %outerloop

bb95.loopexit:                                    ; preds = %bb134
  %add41.lcssa.lcssa = phi double [ %add41.lcssa, %bb134 ]
  br label %bb95

bb95:                                             ; preds = %bb95.loopexit, %alloca_2
  ret void
}

