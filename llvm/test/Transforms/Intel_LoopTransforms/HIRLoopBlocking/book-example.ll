; RUN: opt -intel-libirc-allowed -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-loop-blocking,print<hir>" -hir-loop-blocking-skip-anti-pattern-check=false -disable-output 2>&1 < %s | FileCheck %s
; RUN: opt -intel-libirc-allowed -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-loop-blocking,print<hir>" -hir-loop-blocking-skip-anti-pattern-check=true -disable-output 2>&1 < %s | FileCheck %s

; Verify that the loop is blocked.
; Notice that the input is not recognized a trivial anti-pattern even when anti-pattern-check is turned on.

; Before Transformation
;
; CHECK: Function: subblock_
;
; CHECK:        BEGIN REGION { }
; CHECK:              + DO i1 = 0, zext.i32.i64(%"subblock_$N_fetch.1") + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1000>  <LEGAL_MAX_TC = 2147483647>
; CHECK:              |   + DO i2 = 0, sext.i32.i64(%"subblock_$M_fetch.3") + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1000>  <LEGAL_MAX_TC = 2147483647>
; CHECK:              |   |   %add.1 = (%"subblock_$B")[i2][i1]  +  (%"subblock_$A")[i1][i2];
; CHECK:              |   |   (%"subblock_$A")[i1][i2] = %add.1;
; CHECK:              |   + END LOOP
; CHECK:              + END LOOP
; CHECK:        END REGION

; After Transformation
; CHECK: Function: subblock_
;
; CHECK:          BEGIN REGION { modified }
; CHECK:                + DO i1 = 0, (zext.i32.i64(%"subblock_$N_fetch.1") + -1)/u64, 1   <DO_LOOP>  <MAX_TC_EST = 1000>  <LEGAL_MAX_TC = 2147483647>
; CHECK:                |   %min = (-64 * i1 + zext.i32.i64(%"subblock_$N_fetch.1") + -1 <= 63) ? -64 * i1 + zext.i32.i64(%"subblock_$N_fetch.1") + -1 : 63;
;                       |
; CHECK:                |   + DO i2 = 0, (sext.i32.i64(%"subblock_$M_fetch.3") + -1)/u64, 1   <DO_LOOP>  <MAX_TC_EST = 1000>  <LEGAL_MAX_TC = 2147483647>
; CHECK:                |   |   %min2 = (-64 * i2 + sext.i32.i64(%"subblock_$M_fetch.3") + -1 <= 63) ? -64 * i2 + sext.i32.i64(%"subblock_$M_fetch.3") + -1 : 63;
;                       |   |
; CHECK:                |   |   + DO i3 = 0, %min, 1   <DO_LOOP>  <MAX_TC_EST = 64>  <LEGAL_MAX_TC = 64>
; CHECK:                |   |   |   + DO i4 = 0, %min2, 1   <DO_LOOP>  <MAX_TC_EST = 64>  <LEGAL_MAX_TC = 64>
; CHECK:                |   |   |   |   %add.1 = (%"subblock_$B")[64 * i2 + i4][64 * i1 + i3]  +  (%"subblock_$A")[64 * i1 + i3][64 * i2 + i4];
; CHECK:                |   |   |   |   (%"subblock_$A")[64 * i1 + i3][64 * i2 + i4] = %add.1;
; CHECK:                |   |   |   + END LOOP
; CHECK:                |   |   + END LOOP
; CHECK:                |   + END LOOP
; CHECK:                + END LOOP
; CHECK:          END REGION
;
;
;Module Before HIR
; ModuleID = 'book-example.f90'
source_filename = "book-example.f90"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nosync nounwind memory(argmem: readwrite) uwtable
define void @subblock_(ptr noalias nocapture dereferenceable(8) %"subblock_$A", ptr noalias nocapture readonly dereferenceable(8) %"subblock_$B", ptr noalias nocapture readonly dereferenceable(4) %"subblock_$N", ptr noalias nocapture readonly dereferenceable(4) %"subblock_$M") local_unnamed_addr #0 {
alloca_0:
  %"subblock_$N_fetch.1" = load i32, ptr %"subblock_$N", align 1
  %rel.1 = icmp slt i32 %"subblock_$N_fetch.1", 1
  br i1 %rel.1, label %do.end_do3, label %do.body2.preheader

do.body2.preheader:                               ; preds = %alloca_0
  %"subblock_$M_fetch.3" = load i32, ptr %"subblock_$M", align 1
  %rel.2 = icmp slt i32 %"subblock_$M_fetch.3", 1
  %0 = add nuw nsw i32 %"subblock_$M_fetch.3", 1
  %1 = add nuw nsw i32 %"subblock_$N_fetch.1", 1
  %wide.trip.count21 = zext i32 %1 to i64
  %wide.trip.count = sext i32 %0 to i64
  br label %do.body2

do.body2:                                         ; preds = %do.body2.preheader, %do.end_do7
  %indvars.iv19 = phi i64 [ 1, %do.body2.preheader ], [ %indvars.iv.next20, %do.end_do7 ]
  br i1 %rel.2, label %do.end_do7, label %do.body6.preheader

do.body6.preheader:                               ; preds = %do.body2
  %"subblock_$A[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 8000, ptr nonnull elementtype(double) %"subblock_$A", i64 %indvars.iv19)
  br label %do.body6

do.body6:                                         ; preds = %do.body6.preheader, %do.body6
  %indvars.iv = phi i64 [ 1, %do.body6.preheader ], [ %indvars.iv.next, %do.body6 ]
  %"subblock_$A[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %"subblock_$A[]", i64 %indvars.iv)
  %"subblock_$A[][]_fetch.7" = load double, ptr %"subblock_$A[][]", align 1
  %"subblock_$B[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 8000, ptr nonnull elementtype(double) %"subblock_$B", i64 %indvars.iv)
  %"subblock_$B[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %"subblock_$B[]", i64 %indvars.iv19)
  %"subblock_$B[][]_fetch.10" = load double, ptr %"subblock_$B[][]", align 1
  %add.1 = fadd reassoc ninf nsz arcp contract afn double %"subblock_$B[][]_fetch.10", %"subblock_$A[][]_fetch.7"
  store double %add.1, ptr %"subblock_$A[][]", align 1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %do.end_do7.loopexit, label %do.body6

do.end_do7.loopexit:                              ; preds = %do.body6
  br label %do.end_do7

do.end_do7:                                       ; preds = %do.end_do7.loopexit, %do.body2
  %indvars.iv.next20 = add nuw nsw i64 %indvars.iv19, 1
  %exitcond22 = icmp eq i64 %indvars.iv.next20, %wide.trip.count21
  br i1 %exitcond22, label %do.end_do3.loopexit, label %do.body2

do.end_do3.loopexit:                              ; preds = %do.end_do7
  br label %do.end_do3

do.end_do3:                                       ; preds = %do.end_do3.loopexit, %alloca_0
  ret void
}

; Function Attrs: mustprogress nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none)
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #1
