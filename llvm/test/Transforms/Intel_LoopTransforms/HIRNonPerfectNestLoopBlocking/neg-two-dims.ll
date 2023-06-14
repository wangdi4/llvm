; REQUIRES : asserts
; RUN: opt -intel-libirc-allowed --passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-non-perfect-nest-loop-blocking,print<hir>" -disable-hir-non-perfect-nest-loop-blocking=false -disable-output -debug-only=hir-non-perfect-nest-loop-blocking 2>&1 < %s | FileCheck %s

; Verify that the loop is not a candidate for HIRNonPerfectNestLoopBlocking
; because currently, the outermost level IV i1 in dimensions are not allowed.
;

; CHECK: Invalid Dimension info.
; CHECK: Not Passed legality check in a innermost loop.
;
; CHECK: Function: foo_
;
; CHECK:         BEGIN REGION { }
; CHECK:               + DO i1 = 0, zext.i32.i64(%"foo_$M_fetch.1") + -1, 1
; CHECK:               |   + DO i2 = 0, zext.i32.i64(%"foo_$N_fetch.3") + -1, 1
; CHECK:               |   |   %"foo_$A[][]_fetch.7" = (%"foo_$A")[i1 + -1][i2 + -1];
; CHECK:               |   |   %"foo_$B[][]_fetch.10" = (%"foo_$B")[i1 + -1][i2 + -1];
; CHECK:               |   |   (%"foo_$C")[i1 + -1][i2 + -1] = %"foo_$A[][]_fetch.7" + %"foo_$B[][]_fetch.10";
; CHECK:               |   + END LOOP
;                      |
; CHECK:               |   + DO i2 = 0, zext.i32.i64(%"foo_$N_fetch.3") + -1, 1
; CHECK:               |   |   %"foo_$A[][]_fetch.20" = (%"foo_$A")[i1 + -1][i2 + -1];
; CHECK:               |   |   %"foo_$B[][]_fetch.23" = (%"foo_$B")[i1 + -1][i2 + -1];
; CHECK:               |   |   (%"foo_$A")[i1 + -1][i2 + -1] = 2 * %"foo_$A[][]_fetch.20" + %"foo_$B[][]_fetch.23";
; CHECK:               |   + END LOOP
; CHECK:               + END LOOP
; CHECK:         END REGION


; ModuleID = 'test-two-dims.f90'
source_filename = "test-two-dims.f90"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nosync nounwind memory(argmem: readwrite) uwtable
define void @foo_(ptr noalias nocapture dereferenceable(4) %"foo_$A", ptr noalias nocapture readonly dereferenceable(4) %"foo_$B", ptr noalias nocapture writeonly dereferenceable(4) %"foo_$C", ptr noalias nocapture readonly dereferenceable(4) %"foo_$M", ptr noalias nocapture readonly dereferenceable(4) %"foo_$N") local_unnamed_addr #0 {
alloca_0:
  %"foo_$M_fetch.1" = load i32, ptr %"foo_$M", align 1
  %rel.1 = icmp slt i32 %"foo_$M_fetch.1", 1
  br i1 %rel.1, label %do.end_do3, label %do.body2.preheader

do.body2.preheader:                               ; preds = %alloca_0
  %"foo_$N_fetch.3" = load i32, ptr %"foo_$N", align 1
  %rel.2 = icmp slt i32 %"foo_$N_fetch.3", 1
  br i1 %rel.2, label %do.end_do3, label %do.body2.preheader42

do.body2.preheader42:                             ; preds = %do.body2.preheader
  %wide.trip.count50 = zext i32 %"foo_$M_fetch.1" to i64
  %wide.trip.count = zext i32 %"foo_$N_fetch.3" to i64
  br label %do.body2

do.body2:                                         ; preds = %do.body2.preheader42, %do.end_do12.loopexit
  %indvars.iv48 = phi i64 [ 0, %do.body2.preheader42 ], [ %indvars.iv.next49, %do.end_do12.loopexit ]
  %"foo_$A[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 4096, ptr nonnull elementtype(i32) %"foo_$A", i64 %indvars.iv48)
  %"foo_$B[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 4096, ptr nonnull elementtype(i32) %"foo_$B", i64 %indvars.iv48)
  %"foo_$C[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 4096, ptr nonnull elementtype(i32) %"foo_$C", i64 %indvars.iv48)
  br label %do.body6

do.body6:                                         ; preds = %do.body2, %do.body6
  %indvars.iv = phi i64 [ 0, %do.body2 ], [ %indvars.iv.next, %do.body6 ]
  %"foo_$A[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(i32) %"foo_$A[]", i64 %indvars.iv)
  %"foo_$A[][]_fetch.7" = load i32, ptr %"foo_$A[][]", align 1
  %"foo_$B[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(i32) %"foo_$B[]", i64 %indvars.iv)
  %"foo_$B[][]_fetch.10" = load i32, ptr %"foo_$B[][]", align 1
  %add.1 = add nsw i32 %"foo_$B[][]_fetch.10", %"foo_$A[][]_fetch.7"
  %"foo_$C[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(i32) %"foo_$C[]", i64 %indvars.iv)
  store i32 %add.1, ptr %"foo_$C[][]", align 1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %do.body11.preheader, label %do.body6

do.body11.preheader:                              ; preds = %do.body6
  br label %do.body11

do.body11:                                        ; preds = %do.body11.preheader, %do.body11
  %indvars.iv44 = phi i64 [ %indvars.iv.next45, %do.body11 ], [ 0, %do.body11.preheader ]
  %"foo_$A[][]11" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(i32) %"foo_$A[]", i64 %indvars.iv44)
  %"foo_$A[][]_fetch.20" = load i32, ptr %"foo_$A[][]11", align 1
  %mul.5 = shl nsw i32 %"foo_$A[][]_fetch.20", 1
  %"foo_$B[][]15" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(i32) %"foo_$B[]", i64 %indvars.iv44)
  %"foo_$B[][]_fetch.23" = load i32, ptr %"foo_$B[][]15", align 1
  %add.3 = add nsw i32 %mul.5, %"foo_$B[][]_fetch.23"
  store i32 %add.3, ptr %"foo_$A[][]11", align 1
  %indvars.iv.next45 = add nuw nsw i64 %indvars.iv44, 1
  %exitcond47 = icmp eq i64 %indvars.iv.next45, %wide.trip.count
  br i1 %exitcond47, label %do.end_do12.loopexit, label %do.body11

do.end_do12.loopexit:                             ; preds = %do.body11
  %indvars.iv.next49 = add nuw nsw i64 %indvars.iv48, 1
  %exitcond51 = icmp eq i64 %indvars.iv.next49, %wide.trip.count50
  br i1 %exitcond51, label %do.end_do3.loopexit, label %do.body2

do.end_do3.loopexit:                              ; preds = %do.end_do12.loopexit
  br label %do.end_do3

do.end_do3:                                       ; preds = %do.end_do3.loopexit, %do.body2.preheader, %alloca_0
  ret void
}

declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #1
