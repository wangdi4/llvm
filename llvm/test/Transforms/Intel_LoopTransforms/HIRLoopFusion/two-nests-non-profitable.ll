; REQUIRES: asserts

; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-fusion,print<hir>" -aa-pipeline="basic-aa" -debug-only=hir-loop-fusion -disable-output < %s 2>&1 | FileCheck %s

; Test checks that we bail out loop fusion for two loop nests to avoid
; loosing vectorization on the innermost loop of the first loop nest.

;   BEGIN REGION { }
;      + DO i1 = 0, 4, 1   <DO_LOOP>
;      |   if (%"sub_$N_fetch.1" >= 1)
;      |   {
;      |      + DO i2 = 0, sext.i32.i64((1 + %"sub_$N_fetch.1")) + -2, 1   <DO_LOOP>
;      |      |   + DO i3 = 0, sext.i32.i64((1 + %"sub_$N_fetch.1")) + -2, 1   <DO_LOOP>  <MAX_TC_EST = 100>
;      |      |   |   + DO i4 = 0, sext.i32.i64((1 + %"sub_$N_fetch.1")) + -2, 1   <DO_LOOP>  <MAX_TC_EST = 100>
;      |      |   |   |   %add.1 = (%"sub_$A")[i2][i3][i4]  +  5.000000e+00;
;      |      |   |   |   (%"sub_$A")[i2][i3][i4] = %add.1;      // SrcRef
;      |      |   |   + END LOOP
;      |      |   + END LOOP
;      |      + END LOOP
;      |
;      |
;      |      + DO i2 = 0, sext.i32.i64((1 + %"sub_$N_fetch.1")) + -2, 1   <DO_LOOP>
;      |      |   + DO i3 = 0, sext.i32.i64((1 + %"sub_$N_fetch.1")) + -2, 1   <DO_LOOP>  <MAX_TC_EST = 100>
;      |      |   |   + DO i4 = 0, sext.i32.i64((1 + %"sub_$N_fetch.1")) + -2, 1   <DO_LOOP>  <MAX_TC_EST = 100>
;      |      |   |   |   %add.5 = (%"sub_$A")[i2][i3][i4]  +  6.000000e+00;
;      |      |   |   |   %add.675 = %add.5;
;      |      |   |   |
;      |      |   |   |   + DO i5 = 0, 7, 1   <DO_LOOP>
;      |      |   |   |   |   %add.675 = %add.675  +  5.000000e+00;
;      |      |   |   |   + END LOOP
;      |      |   |   |
;      |      |   |   |   (%"sub_$A")[i2][i3][i4] = %add.675;   // DstRef
;      |      |   |   + END LOOP
;      |      |   + END LOOP
;      |      + END LOOP
;      |   }
;      + END LOOP
;   END REGION

; CHECK: Bail out for different loop nests, case 1
; CHECK: BEGIN REGION
; CHECK-NOT: modified


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree nosync nounwind uwtable
define void @sub_(ptr noalias nocapture dereferenceable(4) %"sub_$A", ptr noalias nocapture readnone dereferenceable(4) %"sub_$B", ptr noalias nocapture readonly dereferenceable(4) %"sub_$N") local_unnamed_addr {
alloca_0:
  %"sub_$N_fetch.1" = load i32, ptr %"sub_$N", align 1
  %rel.1 = icmp slt i32 %"sub_$N_fetch.1", 1
  %0 = add nuw nsw i32 %"sub_$N_fetch.1", 1
  %wide.trip.count82 = sext i32 %0 to i64
  br label %bb2

bb2:                                              ; preds = %bb19, %alloca_0
  %"sub_$MM.0" = phi i32 [ 1, %alloca_0 ], [ %add.11, %bb19 ]
  br i1 %rel.1, label %bb19, label %bb14.preheader.preheader.preheader

bb14.preheader.preheader.preheader:               ; preds = %bb2
  br label %bb14.preheader.preheader

bb14.preheader.preheader:                         ; preds = %bb14.preheader.preheader.preheader, %bb11
  %indvars.iv80 = phi i64 [ %indvars.iv.next81, %bb11 ], [ 1, %bb14.preheader.preheader.preheader ]
  %"sub_$A_entry[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 40000, ptr nonnull elementtype(float) %"sub_$A", i64 %indvars.iv80)
  br label %bb14.preheader

bb14.preheader:                                   ; preds = %bb14.preheader.preheader, %bb15
  %indvars.iv76 = phi i64 [ 1, %bb14.preheader.preheader ], [ %indvars.iv.next77, %bb15 ]
  %"sub_$A_entry[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 400, ptr nonnull elementtype(float) %"sub_$A_entry[]", i64 %indvars.iv76)
  br label %bb14

bb14:                                             ; preds = %bb14.preheader, %bb14
  %indvars.iv = phi i64 [ 1, %bb14.preheader ], [ %indvars.iv.next, %bb14 ]
  %"sub_$A_entry[][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"sub_$A_entry[][]", i64 %indvars.iv)
  %"sub_$A_entry[][][]_fetch.10" = load float, ptr %"sub_$A_entry[][][]", align 1
  %add.1 = fadd reassoc ninf nsz arcp contract afn float %"sub_$A_entry[][][]_fetch.10", 5.000000e+00
  store float %add.1, ptr %"sub_$A_entry[][][]", align 1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count82
  br i1 %exitcond, label %bb15, label %bb14

bb15:                                             ; preds = %bb14
  %indvars.iv.next77 = add nuw nsw i64 %indvars.iv76, 1
  %exitcond79 = icmp eq i64 %indvars.iv.next77, %wide.trip.count82
  br i1 %exitcond79, label %bb11, label %bb14.preheader

bb11:                                             ; preds = %bb15
  %indvars.iv.next81 = add nuw nsw i64 %indvars.iv80, 1
  %exitcond83 = icmp eq i64 %indvars.iv.next81, %wide.trip.count82
  br i1 %exitcond83, label %bb26.preheader.preheader.preheader, label %bb14.preheader.preheader

bb26.preheader.preheader.preheader:               ; preds = %bb11
  br label %bb26.preheader.preheader

bb26.preheader.preheader:                         ; preds = %bb26.preheader.preheader.preheader, %bb23
  %indvars.iv93 = phi i64 [ %indvars.iv.next94, %bb23 ], [ 1, %bb26.preheader.preheader.preheader ]
  %"sub_$A_entry[]14" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 40000, ptr nonnull elementtype(float) %"sub_$A", i64 %indvars.iv93)
  br label %bb26.preheader

bb26.preheader:                                   ; preds = %bb26.preheader.preheader, %bb27
  %indvars.iv89 = phi i64 [ 1, %bb26.preheader.preheader ], [ %indvars.iv.next90, %bb27 ]
  %"sub_$A_entry[][]15" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 400, ptr nonnull elementtype(float) %"sub_$A_entry[]14", i64 %indvars.iv89)
  br label %bb26

bb26:                                             ; preds = %bb26.preheader, %bb33
  %indvars.iv85 = phi i64 [ 1, %bb26.preheader ], [ %indvars.iv.next86, %bb33 ]
  %"sub_$A_entry[][][]16" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"sub_$A_entry[][]15", i64 %indvars.iv85)
  %"sub_$A_entry[][][]_fetch.32" = load float, ptr %"sub_$A_entry[][][]16", align 1
  %add.5 = fadd reassoc ninf nsz arcp contract afn float %"sub_$A_entry[][][]_fetch.32", 6.000000e+00
  br label %bb30

bb30:                                             ; preds = %bb30, %bb26
  %add.675 = phi float [ %add.5, %bb26 ], [ %add.6, %bb30 ]
  %"sub_$L.0" = phi i32 [ 1, %bb26 ], [ %add.7, %bb30 ]
  %add.6 = fadd reassoc ninf nsz arcp contract afn float %add.675, 5.000000e+00
  %add.7 = add nuw nsw i32 %"sub_$L.0", 1
  %exitcond84.not = icmp eq i32 %add.7, 9
  br i1 %exitcond84.not, label %bb33, label %bb30

bb33:                                             ; preds = %bb30
  %add.6.lcssa = phi float [ %add.6, %bb30 ]
  store float %add.6.lcssa, ptr %"sub_$A_entry[][][]16", align 1
  %indvars.iv.next86 = add nuw nsw i64 %indvars.iv85, 1
  %exitcond88 = icmp eq i64 %indvars.iv.next86, %wide.trip.count82
  br i1 %exitcond88, label %bb27, label %bb26

bb27:                                             ; preds = %bb33
  %indvars.iv.next90 = add nuw nsw i64 %indvars.iv89, 1
  %exitcond92 = icmp eq i64 %indvars.iv.next90, %wide.trip.count82
  br i1 %exitcond92, label %bb23, label %bb26.preheader

bb23:                                             ; preds = %bb27
  %indvars.iv.next94 = add nuw nsw i64 %indvars.iv93, 1
  %exitcond96 = icmp eq i64 %indvars.iv.next94, %wide.trip.count82
  br i1 %exitcond96, label %bb19.loopexit, label %bb26.preheader.preheader

bb19.loopexit:                                    ; preds = %bb23
  br label %bb19

bb19:                                             ; preds = %bb19.loopexit, %bb2
  %add.11 = add nuw nsw i32 %"sub_$MM.0", 1
  %exitcond97.not = icmp eq i32 %add.11, 6
  br i1 %exitcond97.not, label %bb5, label %bb2

bb5:                                              ; preds = %bb19
  ret void
}

; Function Attrs: nofree nosync nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64)

