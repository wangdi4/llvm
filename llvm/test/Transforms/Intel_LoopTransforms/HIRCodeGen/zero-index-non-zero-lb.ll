; RUN: opt -passes="hir-ssa-deconstruction,hir-cg" -force-hir-cg -S < %s | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Check that (%A)[%L:0:8(double*:0)] would be CG'ed
; with respect to non-zero LB(%L) even if the index is zero.

;  BEGIN REGION { }
;        + DO i1 = 0, 8, 1   <DO_LOOP>
;        |   (%A)[%L:0:8(double*:0)] = 1.000000e+00;
;        + END LOOP
;  END REGION

; CHECK: region.0:
; The invariant dimension/GEP is generated in loop preheader.

; CHECK: store i64 %L, ptr [[TMP1:%t[0-9]*]]
; CHECK: [[TMP2:%.*]] = load i64, ptr [[TMP1]]
; CHECK: [[TMP3:%.*]] = sub nsw i64 0, [[TMP2]]
; CHECK: [[GEP:%.*]] = getelementptr inbounds double, ptr %A, i64 [[TMP3]]

; CHECK: loop.{{[0-9]*}}:
; CHECK: store double 1.000000e+00, ptr [[GEP]], align 8

define void @test_IP_foo_(ptr noalias nocapture readonly dereferenceable(8) %A, ptr noalias nocapture readonly dereferenceable(4) %L_ptr) {
entry:
  %L = load i64, ptr %L_ptr, align 1
  br label %bb23.preheader

bb23.preheader:
  br label %bb23

bb23:
  %indvars.iv = phi i64 [ 1, %bb23.preheader ], [ %indvars.iv.next, %bb23 ]

  %"A[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %L, i64 8, ptr elementtype(double) nonnull %A, i64 0)
  store double 1.000000, ptr %"A[]"

  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 10
  br i1 %exitcond, label %bb24.loopexit, label %bb23

bb24.loopexit:
  br label %bb24

bb24:
  ret void
}

declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #0
attributes #0 = { nounwind readnone speculatable }

