; RUN: opt -passes="dse" -S < %s | FileCheck %s

; 25029: from bwaves. DSE should not remove any of these stores. They should
; all be "MayAlias" with each other, and with the loads in the same loop.

; CHECK: store double %"fill2_$Q[][][][]_fetch", ptr %"fill2_$Q[]31[][][]", align 1
; CHECK: store double %"fill2_$Q[][][][]_fetch", ptr %"fill2_$Q[]66[][][]", align 1
; CHECK: store double %"fill2_$Q[]112[][][]_fetch", ptr %"fill2_$Q[]131[][][]", align 1
; CHECK: store double %"fill2_$Q[]112[][][]_fetch", ptr %"fill2_$Q[]163[][][]", align 1


; ModuleID = 'full.ll'
source_filename = "fill2.fppized.f"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree nounwind uwtable
define void @fill2_(ptr noalias nocapture dereferenceable(8) %"fill2_$Q", ptr noalias nocapture readonly dereferenceable(4) %"fill2_$NX", ptr noalias nocapture readonly dereferenceable(4) %"fill2_$NY", ptr noalias nocapture readonly dereferenceable(4) %"fill2_$NZL") local_unnamed_addr #0 {
alloca_0:
  %"fill2_$NX_fetch" = load i32, ptr %"fill2_$NX", align 1
  %"fill2_$NY_fetch" = load i32, ptr %"fill2_$NY", align 1
  %int_sext = sext i32 %"fill2_$NX_fetch" to i64
  %mul = mul nsw i64 %int_sext, 40
  %int_sext7 = sext i32 %"fill2_$NY_fetch" to i64
  %mul8 = mul nsw i64 %mul, %int_sext7
  %rel = icmp slt i32 %"fill2_$NY_fetch", 1
  br i1 %rel, label %bb54, label %bb5.preheader

bb5.preheader:                                    ; preds = %alloca_0
  %rel6 = icmp slt i32 %"fill2_$NX_fetch", 1
  %"fill2_$Q[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 3, i64 1, i64 %mul8, ptr elementtype(double) nonnull %"fill2_$Q", i64 3)
  %"fill2_$NZL_fetch26" = load i32, ptr %"fill2_$NZL", align 1
  %add28 = add nsw i32 %"fill2_$NZL_fetch26", 3
  %int_sext30 = sext i32 %add28 to i64
  %"fill2_$Q[]31" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 3, i64 1, i64 %mul8, ptr elementtype(double) nonnull %"fill2_$Q", i64 %int_sext30)
  %add61 = add nsw i32 %"fill2_$NZL_fetch26", 4
  %int_sext63 = sext i32 %add61 to i64
  %"fill2_$Q[]66" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 3, i64 1, i64 %mul8, ptr elementtype(double) nonnull %"fill2_$Q", i64 %int_sext63)
  %0 = add nuw nsw i32 %"fill2_$NX_fetch", 1
  %1 = add nuw nsw i32 %"fill2_$NY_fetch", 1
  %wide.trip.count281 = sext i32 %1 to i64
  br label %bb5

bb5:                                              ; preds = %bb10, %bb5.preheader
  %indvars.iv279 = phi i64 [ 1, %bb5.preheader ], [ %indvars.iv.next280, %bb10 ]
  br i1 %rel6, label %bb10, label %bb9.preheader

bb9.preheader:                                    ; preds = %bb5
  %"fill2_$Q[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %mul, ptr elementtype(double) nonnull %"fill2_$Q[]", i64 %indvars.iv279)
  %"fill2_$Q[]31[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %mul, ptr elementtype(double) nonnull %"fill2_$Q[]31", i64 %indvars.iv279)
  %"fill2_$Q[]66[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %mul, ptr elementtype(double) nonnull %"fill2_$Q[]66", i64 %indvars.iv279)
  %wide.trip.count277 = sext i32 %0 to i64
  br label %bb9

bb9:                                              ; preds = %bb16, %bb9.preheader
  %indvars.iv275 = phi i64 [ 1, %bb9.preheader ], [ %indvars.iv.next276, %bb16 ]
  %"fill2_$Q[][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 40, ptr elementtype(double) nonnull %"fill2_$Q[][]", i64 %indvars.iv275)
  %"fill2_$Q[]31[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 40, ptr elementtype(double) nonnull %"fill2_$Q[]31[]", i64 %indvars.iv275)
  %"fill2_$Q[]66[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 40, ptr elementtype(double) nonnull %"fill2_$Q[]66[]", i64 %indvars.iv275)
  br label %bb13

bb13:                                             ; preds = %bb13, %bb9
  %indvars.iv272 = phi i64 [ %indvars.iv.next273, %bb13 ], [ 1, %bb9 ]
  %"fill2_$Q[][][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %"fill2_$Q[][][]", i64 %indvars.iv272)
  %"fill2_$Q[][][][]_fetch" = load double, ptr %"fill2_$Q[][][][]", align 1
  %"fill2_$Q[]31[][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %"fill2_$Q[]31[][]", i64 %indvars.iv272)
  store double %"fill2_$Q[][][][]_fetch", ptr %"fill2_$Q[]31[][][]", align 1
  %"fill2_$Q[]66[][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %"fill2_$Q[]66[][]", i64 %indvars.iv272)
  store double %"fill2_$Q[][][][]_fetch", ptr %"fill2_$Q[]66[][][]", align 1
  %indvars.iv.next273 = add nuw nsw i64 %indvars.iv272, 1
  %exitcond274 = icmp eq i64 %indvars.iv.next273, 6
  br i1 %exitcond274, label %bb16, label %bb13

bb16:                                             ; preds = %bb13
  %indvars.iv.next276 = add nuw nsw i64 %indvars.iv275, 1
  %exitcond278 = icmp eq i64 %indvars.iv.next276, %wide.trip.count277
  br i1 %exitcond278, label %bb10, label %bb9

bb10:                                             ; preds = %bb16, %bb5
  %indvars.iv.next280 = add nuw nsw i64 %indvars.iv279, 1
  %exitcond282 = icmp eq i64 %indvars.iv.next280, %wide.trip.count281
  br i1 %exitcond282, label %bb53.preheader, label %bb5

bb53.preheader:                                   ; preds = %bb10
  %add141 = add nsw i32 %"fill2_$NZL_fetch26", 1
  %int_sext142 = sext i32 %add141 to i64
  %"fill2_$Q[]112" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 3, i64 1, i64 %mul8, ptr elementtype(double) nonnull %"fill2_$Q", i64 %int_sext142)
  %"fill2_$Q[]131" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 3, i64 1, i64 %mul8, ptr elementtype(double) nonnull %"fill2_$Q", i64 1)
  %"fill2_$Q[]163" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 3, i64 1, i64 %mul8, ptr elementtype(double) nonnull %"fill2_$Q", i64 2)
  br label %bb53

bb53:                                             ; preds = %bb58, %bb53.preheader
  %indvars.iv268 = phi i64 [ 1, %bb53.preheader ], [ %indvars.iv.next269, %bb58 ]
  br i1 %rel6, label %bb58, label %bb57.preheader

bb57.preheader:                                   ; preds = %bb53
  %"fill2_$Q[]112[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %mul, ptr elementtype(double) nonnull %"fill2_$Q[]112", i64 %indvars.iv268)
  %"fill2_$Q[]131[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %mul, ptr elementtype(double) nonnull %"fill2_$Q[]131", i64 %indvars.iv268)
  %"fill2_$Q[]163[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %mul, ptr elementtype(double) nonnull %"fill2_$Q[]163", i64 %indvars.iv268)
  %wide.trip.count = sext i32 %0 to i64
  br label %bb57

bb57:                                             ; preds = %bb64, %bb57.preheader
  %indvars.iv265 = phi i64 [ 1, %bb57.preheader ], [ %indvars.iv.next266, %bb64 ]
  %"fill2_$Q[]112[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 40, ptr elementtype(double) nonnull %"fill2_$Q[]112[]", i64 %indvars.iv265)
  %"fill2_$Q[]131[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 40, ptr elementtype(double) nonnull %"fill2_$Q[]131[]", i64 %indvars.iv265)
  %"fill2_$Q[]163[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 40, ptr elementtype(double) nonnull %"fill2_$Q[]163[]", i64 %indvars.iv265)
  br label %bb61

bb61:                                             ; preds = %bb61, %bb57
  %indvars.iv = phi i64 [ %indvars.iv.next, %bb61 ], [ 1, %bb57 ]
  %"fill2_$Q[]112[][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %"fill2_$Q[]112[][]", i64 %indvars.iv)
  %"fill2_$Q[]112[][][]_fetch" = load double, ptr %"fill2_$Q[]112[][][]", align 1
  %"fill2_$Q[]131[][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %"fill2_$Q[]131[][]", i64 %indvars.iv)
  store double %"fill2_$Q[]112[][][]_fetch", ptr %"fill2_$Q[]131[][][]", align 1
  %"fill2_$Q[]163[][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %"fill2_$Q[]163[][]", i64 %indvars.iv)
  store double %"fill2_$Q[]112[][][]_fetch", ptr %"fill2_$Q[]163[][][]", align 1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 6
  br i1 %exitcond, label %bb64, label %bb61

bb64:                                             ; preds = %bb61
  %indvars.iv.next266 = add nuw nsw i64 %indvars.iv265, 1
  %exitcond267 = icmp eq i64 %indvars.iv.next266, %wide.trip.count
  br i1 %exitcond267, label %bb58, label %bb57

bb58:                                             ; preds = %bb64, %bb53
  %indvars.iv.next269 = add nuw nsw i64 %indvars.iv268, 1
  %exitcond271 = icmp eq i64 %indvars.iv.next269, %wide.trip.count281
  br i1 %exitcond271, label %bb54, label %bb53

bb54:                                             ; preds = %bb58, %alloca_0
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #1

attributes #0 = { nofree nounwind uwtable "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+mmx,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87" }
attributes #1 = { nounwind readnone speculatable }

!omp_offload.info = !{}
