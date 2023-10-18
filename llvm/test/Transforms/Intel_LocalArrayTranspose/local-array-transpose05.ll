; REQUIRES: asserts
; RUN: opt -passes=local-array-transpose -debug-only=local-array-transpose -S < %s 2>&1 | FileCheck %s

; Test local array transpose after fixing CMPLRLLVM-52391

; Check that ACOX is not a candidate because it is not square:

; CHECK: LocalArrayTranspose: No Valid Candidates for foo_

; Check that the subscripts of ACOX are not inverted.

; CHECK: %indvars.iv221 = phi
; CHECK: %indvars.iv = phi
; CHECK: %"foo_$ACOX.addr_a0$_fetch.27[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %"foo_$ACOX.dim_info$.lower_bound$[]_fetch.31", i64 %"foo_$ACOX.dim_info$.spacing$[]_fetch.30", ptr elementtype(float) %"foo_$ACOX.addr_a0$113_fetch.72.pre", i64 %indvars.iv), !llfort.type_idx !30
; CHECK: %"foo_$ACOX.addr_a0$_fetch.27[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %"foo_$ACOX.dim_info$.lower_bound$[]_fetch.28", i64 4, ptr elementtype(float) %"foo_$ACOX.addr_a0$_fetch.27[]", i64 %indvars.iv221), !llfort.type_idx !30

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$float*$rank2$" = type { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }

; Function Attrs: nounwind uwtable
define void @foo_(ptr noalias nocapture readonly dereferenceable(96) "ptrnoalias" %"foo_$INPUT", ptr noalias nocapture readonly dereferenceable(96) "ptrnoalias" %"foo_$RESULT", ptr noalias nocapture readonly dereferenceable(4) %"foo_$M", ptr noalias nocapture readonly dereferenceable(4) %"foo_$N") local_unnamed_addr #0 {
alloca_0:
  %"foo_$ACOX" = alloca %"QNCA_a0$float*$rank2$", align 8, !llfort.type_idx !2
  %"var$4" = alloca i64, align 8, !llfort.type_idx !3
  %"var$2_fetch.2.fca.0.gep" = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %"foo_$ACOX", i64 0, i32 0
  store ptr null, ptr %"var$2_fetch.2.fca.0.gep", align 1, !tbaa !4
  %"var$2_fetch.2.fca.1.gep" = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %"foo_$ACOX", i64 0, i32 1
  store i64 0, ptr %"var$2_fetch.2.fca.1.gep", align 1, !tbaa !4
  %"var$2_fetch.2.fca.2.gep" = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %"foo_$ACOX", i64 0, i32 2
  store i64 0, ptr %"var$2_fetch.2.fca.2.gep", align 1, !tbaa !4
  %"var$2_fetch.2.fca.3.gep" = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %"foo_$ACOX", i64 0, i32 3
  store i64 128, ptr %"var$2_fetch.2.fca.3.gep", align 1, !tbaa !4
  %"var$2_fetch.2.fca.4.gep" = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %"foo_$ACOX", i64 0, i32 4
  store i64 2, ptr %"var$2_fetch.2.fca.4.gep", align 1, !tbaa !4
  %"var$2_fetch.2.fca.5.gep" = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %"foo_$ACOX", i64 0, i32 5
  store i64 0, ptr %"var$2_fetch.2.fca.5.gep", align 1, !tbaa !4
  %"var$2_fetch.2.fca.6.0.0.gep" = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %"foo_$ACOX", i64 0, i32 6, i64 0, i32 0
  store i64 0, ptr %"var$2_fetch.2.fca.6.0.0.gep", align 1, !tbaa !4
  %"var$2_fetch.2.fca.6.0.1.gep" = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %"foo_$ACOX", i64 0, i32 6, i64 0, i32 1
  store i64 0, ptr %"var$2_fetch.2.fca.6.0.1.gep", align 1, !tbaa !4
  %"var$2_fetch.2.fca.6.0.2.gep" = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %"foo_$ACOX", i64 0, i32 6, i64 0, i32 2
  store i64 0, ptr %"var$2_fetch.2.fca.6.0.2.gep", align 1, !tbaa !4
  %"var$2_fetch.2.fca.6.1.0.gep" = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %"foo_$ACOX", i64 0, i32 6, i64 1, i32 0
  store i64 0, ptr %"var$2_fetch.2.fca.6.1.0.gep", align 1, !tbaa !4
  %"var$2_fetch.2.fca.6.1.1.gep" = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %"foo_$ACOX", i64 0, i32 6, i64 1, i32 1
  store i64 0, ptr %"var$2_fetch.2.fca.6.1.1.gep", align 1, !tbaa !4
  %"var$2_fetch.2.fca.6.1.2.gep" = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %"foo_$ACOX", i64 0, i32 6, i64 1, i32 2
  store i64 0, ptr %"var$2_fetch.2.fca.6.1.2.gep", align 1, !tbaa !4
  store i64 133, ptr %"var$2_fetch.2.fca.3.gep", align 8, !tbaa !8
  store i64 0, ptr %"var$2_fetch.2.fca.5.gep", align 8, !tbaa !11
  store i64 4, ptr %"var$2_fetch.2.fca.1.gep", align 8, !tbaa !12
  store i64 2, ptr %"var$2_fetch.2.fca.4.gep", align 8, !tbaa !13
  store i64 0, ptr %"var$2_fetch.2.fca.2.gep", align 8, !tbaa !14
  %"foo_$M_fetch.4" = load i32, ptr %"foo_$M", align 1, !tbaa !15
  %int_sext = sext i32 %"foo_$M_fetch.4" to i64, !llfort.type_idx !3
  %"foo_$ACOX.dim_info$.lower_bound$21" = getelementptr inbounds { i64, i64, i64 }, ptr %"var$2_fetch.2.fca.6.0.0.gep", i64 0, i32 2, !llfort.type_idx !18
  %"foo_$ACOX.dim_info$.lower_bound$[]22" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"foo_$ACOX.dim_info$.lower_bound$21", i32 0), !llfort.type_idx !18
  store i64 1, ptr %"foo_$ACOX.dim_info$.lower_bound$[]22", align 1, !tbaa !19
  %rel.1 = icmp sgt i64 %int_sext, 0
  %slct.1 = select i1 %rel.1, i64 %int_sext, i64 0
  %"foo_$ACOX.dim_info$.extent$" = getelementptr inbounds { i64, i64, i64 }, ptr %"var$2_fetch.2.fca.6.0.0.gep", i64 0, i32 0, !llfort.type_idx !20
  %"foo_$ACOX.dim_info$.extent$[]" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"foo_$ACOX.dim_info$.extent$", i32 0), !llfort.type_idx !20
  store i64 %slct.1, ptr %"foo_$ACOX.dim_info$.extent$[]", align 1, !tbaa !21
  %"foo_$ACOX.dim_info$.spacing$" = getelementptr inbounds { i64, i64, i64 }, ptr %"var$2_fetch.2.fca.6.0.0.gep", i64 0, i32 1, !llfort.type_idx !22
  %"foo_$ACOX.dim_info$.spacing$[]" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"foo_$ACOX.dim_info$.spacing$", i32 0), !llfort.type_idx !22
  store i64 4, ptr %"foo_$ACOX.dim_info$.spacing$[]", align 1, !tbaa !23
  %mul.1 = shl nuw nsw i64 %slct.1, 2
  %"foo_$N_fetch.5" = load i32, ptr %"foo_$N", align 1, !tbaa !24
  %int_sext25 = sext i32 %"foo_$N_fetch.5" to i64, !llfort.type_idx !3
  %"foo_$ACOX.dim_info$.lower_bound$[]28" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"foo_$ACOX.dim_info$.lower_bound$21", i32 1), !llfort.type_idx !18
  store i64 1, ptr %"foo_$ACOX.dim_info$.lower_bound$[]28", align 1, !tbaa !19
  %rel.4 = icmp sgt i64 %int_sext25, 0
  %slct.2 = select i1 %rel.4, i64 %int_sext25, i64 0
  %"foo_$ACOX.dim_info$.extent$[]31" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"foo_$ACOX.dim_info$.extent$", i32 1), !llfort.type_idx !20
  store i64 %slct.2, ptr %"foo_$ACOX.dim_info$.extent$[]31", align 1, !tbaa !21
  %"foo_$ACOX.dim_info$.spacing$[]34" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"foo_$ACOX.dim_info$.spacing$", i32 1), !llfort.type_idx !22
  store i64 %mul.1, ptr %"foo_$ACOX.dim_info$.spacing$[]34", align 1, !tbaa !23
  %func_result = call i32 (ptr, i32, ...) @for_check_mult_overflow64(ptr nonnull %"var$4", i32 3, i64 %slct.1, i64 %slct.2, i64 4) #3, !llfort.type_idx !26
  %"var$4_fetch.6" = load i64, ptr %"var$4", align 8, !tbaa !27, !llfort.type_idx !3
  store i64 1073741957, ptr %"var$2_fetch.2.fca.3.gep", align 8, !tbaa !8
  %and.8 = shl i32 %func_result, 4
  %shl.5 = and i32 %and.8, 16
  %or.9 = or i32 %shl.5, 262146
  %func_result12 = call i32 @for_alloc_allocatable_handle(i64 %"var$4_fetch.6", ptr nonnull %"var$2_fetch.2.fca.0.gep", i32 %or.9, ptr null) #3, !llfort.type_idx !26
  %rel.7 = icmp slt i32 %"foo_$M_fetch.4", 1
  %"foo_$ACOX.addr_a0$113_fetch.72.pre" = load ptr, ptr %"var$2_fetch.2.fca.0.gep", align 8, !tbaa !28
  br i1 %rel.7, label %do.end_do19, label %do.body9.preheader

do.body9.preheader:                               ; preds = %alloca_0
  %rel.8 = icmp slt i32 %"foo_$N_fetch.5", 1
  %"foo_$INPUT.addr_a0$" = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %"foo_$INPUT", i64 0, i32 0
  %"foo_$INPUT.addr_a0$_fetch.17" = load ptr, ptr %"foo_$INPUT.addr_a0$", align 1
  %"foo_$INPUT.dim_info$.lower_bound$" = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %"foo_$INPUT", i64 0, i32 6, i64 0, i32 2
  %"foo_$INPUT.dim_info$.lower_bound$[]" = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"foo_$INPUT.dim_info$.lower_bound$", i32 0)
  %"foo_$INPUT.dim_info$.lower_bound$[]_fetch.18" = load i64, ptr %"foo_$INPUT.dim_info$.lower_bound$[]", align 1
  %"foo_$INPUT.dim_info$.spacing$" = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %"foo_$INPUT", i64 0, i32 6, i64 0, i32 1
  %"foo_$INPUT.dim_info$.spacing$[]" = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"foo_$INPUT.dim_info$.spacing$", i32 1)
  %"foo_$INPUT.dim_info$.spacing$[]_fetch.20" = load i64, ptr %"foo_$INPUT.dim_info$.spacing$[]", align 1, !range !29
  %"foo_$INPUT.dim_info$.lower_bound$[]40" = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"foo_$INPUT.dim_info$.lower_bound$", i32 1)
  %"foo_$INPUT.dim_info$.lower_bound$[]_fetch.21" = load i64, ptr %"foo_$INPUT.dim_info$.lower_bound$[]40", align 1
  %"foo_$ACOX.dim_info$.lower_bound$[]_fetch.28" = load i64, ptr %"foo_$ACOX.dim_info$.lower_bound$[]22", align 1
  %"foo_$ACOX.dim_info$.spacing$[]_fetch.30" = load i64, ptr %"foo_$ACOX.dim_info$.spacing$[]34", align 1, !range !29
  %"foo_$ACOX.dim_info$.lower_bound$[]_fetch.31" = load i64, ptr %"foo_$ACOX.dim_info$.lower_bound$[]28", align 1
  %0 = add nuw nsw i32 %"foo_$N_fetch.5", 1
  %1 = add nuw nsw i32 %"foo_$M_fetch.4", 1
  %wide.trip.count223 = sext i32 %1 to i64
  br label %do.body9

do.body9:                                         ; preds = %do.body9.preheader, %do.end_do14
  %indvars.iv221 = phi i64 [ 1, %do.body9.preheader ], [ %indvars.iv.next222, %do.end_do14 ]
  br i1 %rel.8, label %do.end_do14, label %do.body13.preheader

do.body13.preheader:                              ; preds = %do.body9
  %wide.trip.count = sext i32 %0 to i64
  br label %do.body13

do.body13:                                        ; preds = %do.body13.preheader, %do.body13
  %indvars.iv = phi i64 [ 1, %do.body13.preheader ], [ %indvars.iv.next, %do.body13 ]
  %"foo_$INPUT.addr_a0$_fetch.17[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %"foo_$INPUT.dim_info$.lower_bound$[]_fetch.21", i64 %"foo_$INPUT.dim_info$.spacing$[]_fetch.20", ptr elementtype(float) %"foo_$INPUT.addr_a0$_fetch.17", i64 %indvars.iv), !llfort.type_idx !30
  %"foo_$INPUT.addr_a0$_fetch.17[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %"foo_$INPUT.dim_info$.lower_bound$[]_fetch.18", i64 4, ptr elementtype(float) %"foo_$INPUT.addr_a0$_fetch.17[]", i64 %indvars.iv221), !llfort.type_idx !30
  %"foo_$INPUT.addr_a0$_fetch.17[][]_fetch.26" = load float, ptr %"foo_$INPUT.addr_a0$_fetch.17[][]", align 1, !tbaa !31, !llfort.type_idx !30
  %"foo_$ACOX.addr_a0$_fetch.27[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %"foo_$ACOX.dim_info$.lower_bound$[]_fetch.31", i64 %"foo_$ACOX.dim_info$.spacing$[]_fetch.30", ptr elementtype(float) %"foo_$ACOX.addr_a0$113_fetch.72.pre", i64 %indvars.iv), !llfort.type_idx !30
  %"foo_$ACOX.addr_a0$_fetch.27[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %"foo_$ACOX.dim_info$.lower_bound$[]_fetch.28", i64 4, ptr elementtype(float) %"foo_$ACOX.addr_a0$_fetch.27[]", i64 %indvars.iv221), !llfort.type_idx !30
  store float %"foo_$INPUT.addr_a0$_fetch.17[][]_fetch.26", ptr %"foo_$ACOX.addr_a0$_fetch.27[][]", align 4, !tbaa !33
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %do.end_do14.loopexit, label %do.body13

do.end_do14.loopexit:                             ; preds = %do.body13
  br label %do.end_do14

do.end_do14:                                      ; preds = %do.end_do14.loopexit, %do.body9
  %indvars.iv.next222 = add nuw nsw i64 %indvars.iv221, 1
  %exitcond224 = icmp eq i64 %indvars.iv.next222, %wide.trip.count223
  br i1 %exitcond224, label %do.body18.preheader, label %do.body9

do.body18.preheader:                              ; preds = %do.end_do14
  %"foo_$RESULT.addr_a0$" = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %"foo_$RESULT", i64 0, i32 0
  %"foo_$RESULT.addr_a0$_fetch.56" = load ptr, ptr %"foo_$RESULT.addr_a0$", align 1
  %"foo_$RESULT.dim_info$.lower_bound$" = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %"foo_$RESULT", i64 0, i32 6, i64 0, i32 2
  %"foo_$RESULT.dim_info$.lower_bound$[]" = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"foo_$RESULT.dim_info$.lower_bound$", i32 0)
  %"foo_$RESULT.dim_info$.lower_bound$[]_fetch.57" = load i64, ptr %"foo_$RESULT.dim_info$.lower_bound$[]", align 1
  %"foo_$RESULT.dim_info$.spacing$" = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %"foo_$RESULT", i64 0, i32 6, i64 0, i32 1
  %"foo_$RESULT.dim_info$.spacing$[]" = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"foo_$RESULT.dim_info$.spacing$", i32 1)
  %"foo_$RESULT.dim_info$.spacing$[]_fetch.59" = load i64, ptr %"foo_$RESULT.dim_info$.spacing$[]", align 1, !range !29
  %"foo_$RESULT.dim_info$.lower_bound$[]99" = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"foo_$RESULT.dim_info$.lower_bound$", i32 1)
  %"foo_$RESULT.dim_info$.lower_bound$[]_fetch.60" = load i64, ptr %"foo_$RESULT.dim_info$.lower_bound$[]99", align 1
  br label %do.body18

do.body18:                                        ; preds = %do.body18.preheader, %do.end_do23
  %indvars.iv229 = phi i64 [ 1, %do.body18.preheader ], [ %indvars.iv.next230, %do.end_do23 ]
  br i1 %rel.8, label %do.end_do23, label %do.body22.preheader

do.body22.preheader:                              ; preds = %do.body18
  %wide.trip.count227 = sext i32 %0 to i64
  br label %do.body22

do.body22:                                        ; preds = %do.body22.preheader, %do.body22
  %indvars.iv225 = phi i64 [ 1, %do.body22.preheader ], [ %indvars.iv.next226, %do.body22 ]
  %"foo_$ACOX.addr_a0$_fetch.46[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %"foo_$ACOX.dim_info$.lower_bound$[]_fetch.31", i64 %"foo_$ACOX.dim_info$.spacing$[]_fetch.30", ptr elementtype(float) %"foo_$ACOX.addr_a0$113_fetch.72.pre", i64 %indvars.iv225), !llfort.type_idx !30
  %"foo_$ACOX.addr_a0$_fetch.46[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %"foo_$ACOX.dim_info$.lower_bound$[]_fetch.28", i64 4, ptr elementtype(float) %"foo_$ACOX.addr_a0$_fetch.46[]", i64 %indvars.iv229), !llfort.type_idx !30
  %"foo_$ACOX.addr_a0$_fetch.46[][]_fetch.55" = load float, ptr %"foo_$ACOX.addr_a0$_fetch.46[][]", align 4, !tbaa !33, !llfort.type_idx !30
  %"foo_$RESULT.addr_a0$_fetch.56[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %"foo_$RESULT.dim_info$.lower_bound$[]_fetch.60", i64 %"foo_$RESULT.dim_info$.spacing$[]_fetch.59", ptr elementtype(float) %"foo_$RESULT.addr_a0$_fetch.56", i64 %indvars.iv225), !llfort.type_idx !30
  %"foo_$RESULT.addr_a0$_fetch.56[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %"foo_$RESULT.dim_info$.lower_bound$[]_fetch.57", i64 4, ptr elementtype(float) %"foo_$RESULT.addr_a0$_fetch.56[]", i64 %indvars.iv229), !llfort.type_idx !30
  store float %"foo_$ACOX.addr_a0$_fetch.46[][]_fetch.55", ptr %"foo_$RESULT.addr_a0$_fetch.56[][]", align 1, !tbaa !35
  %indvars.iv.next226 = add nuw nsw i64 %indvars.iv225, 1
  %exitcond228 = icmp eq i64 %indvars.iv.next226, %wide.trip.count227
  br i1 %exitcond228, label %do.end_do23.loopexit, label %do.body22

do.end_do23.loopexit:                             ; preds = %do.body22
  br label %do.end_do23

do.end_do23:                                      ; preds = %do.end_do23.loopexit, %do.body18
  %indvars.iv.next230 = add nuw nsw i64 %indvars.iv229, 1
  %exitcond232 = icmp eq i64 %indvars.iv.next230, %wide.trip.count223
  br i1 %exitcond232, label %do.end_do19.loopexit, label %do.body18

do.end_do19.loopexit:                             ; preds = %do.end_do23
  br label %do.end_do19

do.end_do19:                                      ; preds = %do.end_do19.loopexit, %alloca_0
  %"foo_$ACOX.flags$115_fetch.73" = load i64, ptr %"var$2_fetch.2.fca.3.gep", align 8, !tbaa !8
  %"foo_$ACOX.flags$115_fetch.73.tr" = trunc i64 %"foo_$ACOX.flags$115_fetch.73" to i32
  %2 = shl i32 %"foo_$ACOX.flags$115_fetch.73.tr", 1
  %or.11 = and i32 %2, 6
  %3 = lshr i32 %"foo_$ACOX.flags$115_fetch.73.tr", 3
  %int_zext121 = and i32 %3, 256
  %4 = lshr i64 %"foo_$ACOX.flags$115_fetch.73", 15
  %5 = trunc i64 %4 to i32
  %6 = and i32 %5, 65011712
  %or.12 = or i32 %int_zext121, %or.11
  %or.15 = or i32 %or.12, %6
  %or.16 = or i32 %or.15, 262144
  %"foo_$ACOX.reserved$129_fetch.74" = load i64, ptr %"var$2_fetch.2.fca.5.gep", align 8, !tbaa !11
  %"(ptr)foo_$ACOX.reserved$129_fetch.74$" = inttoptr i64 %"foo_$ACOX.reserved$129_fetch.74" to ptr, !llfort.type_idx !37
  %func_result131 = tail call i32 @for_dealloc_allocatable_handle(ptr %"foo_$ACOX.addr_a0$113_fetch.72.pre", i32 %or.16, ptr %"(ptr)foo_$ACOX.reserved$129_fetch.74$") #3, !llfort.type_idx !26
  %rel.15 = icmp eq i32 %func_result131, 0
  br i1 %rel.15, label %bb_new29_then, label %bb3_endif

bb_new29_then:                                    ; preds = %do.end_do19
  store ptr null, ptr %"var$2_fetch.2.fca.0.gep", align 8, !tbaa !28
  %and.32 = and i64 %"foo_$ACOX.flags$115_fetch.73", -1030792153090
  store i64 %and.32, ptr %"var$2_fetch.2.fca.3.gep", align 8, !tbaa !8
  br label %bb3_endif

bb3_endif:                                        ; preds = %do.end_do19, %bb_new29_then
  %"foo_$ACOX.addr_a0$142_fetch.79" = phi ptr [ %"foo_$ACOX.addr_a0$113_fetch.72.pre", %do.end_do19 ], [ null, %bb_new29_then ]
  %"foo_$ACOX.flags$140_fetch.78" = phi i64 [ %"foo_$ACOX.flags$115_fetch.73", %do.end_do19 ], [ %and.32, %bb_new29_then ]
  %and.33 = and i64 %"foo_$ACOX.flags$140_fetch.78", 1
  %rel.16 = icmp eq i64 %and.33, 0
  br i1 %rel.16, label %dealloc.list.end36, label %dealloc.list.then30

dealloc.list.then30:                              ; preds = %bb3_endif
  %"foo_$ACOX.flags$140_fetch.78.tr" = trunc i64 %"foo_$ACOX.flags$140_fetch.78" to i32
  %and.33.tr = trunc i64 %and.33 to i32
  %7 = and i32 %"foo_$ACOX.flags$140_fetch.78.tr", 2
  %8 = or i32 %7, %and.33.tr
  %or.19 = shl nuw nsw i32 %8, 1
  %9 = lshr i32 %"foo_$ACOX.flags$140_fetch.78.tr", 3
  %int_zext150 = and i32 %9, 256
  %or.20 = or i32 %or.19, %int_zext150
  %10 = lshr i64 %"foo_$ACOX.flags$140_fetch.78", 15
  %11 = trunc i64 %10 to i32
  %12 = and i32 %11, 65011712
  %and.46 = or i32 %or.20, %12
  %or.24 = or i32 %and.46, 262144
  %func_result160 = tail call i32 @for_dealloc_allocatable_handle(ptr %"foo_$ACOX.addr_a0$142_fetch.79", i32 %or.24, ptr %"(ptr)foo_$ACOX.reserved$129_fetch.74$") #3, !llfort.type_idx !26
  %rel.17 = icmp eq i32 %func_result160, 0
  br i1 %rel.17, label %bb_new34_then, label %dealloc.list.end36

bb_new34_then:                                    ; preds = %dealloc.list.then30
  store ptr null, ptr %"var$2_fetch.2.fca.0.gep", align 8, !tbaa !28
  %and.48 = and i64 %"foo_$ACOX.flags$140_fetch.78", -2050
  store i64 %and.48, ptr %"var$2_fetch.2.fca.3.gep", align 8, !tbaa !8
  br label %dealloc.list.end36

dealloc.list.end36:                               ; preds = %bb3_endif, %dealloc.list.then30, %bb_new34_then
  ret void
}

; Function Attrs: mustprogress nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none)
declare ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8, i64, i32, ptr, i32) #1

; Function Attrs: nofree
declare !llfort.intrin_id !38 i32 @for_check_mult_overflow64(ptr nocapture, i32, ...) local_unnamed_addr #2

; Function Attrs: nofree
declare !llfort.intrin_id !39 i32 @for_alloc_allocatable_handle(i64, ptr nocapture, i32, ptr) local_unnamed_addr #2

; Function Attrs: mustprogress nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none)
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #1

; Function Attrs: nofree
declare !llfort.intrin_id !40 i32 @for_dealloc_allocatable_handle(ptr nocapture readonly, i32, ptr) local_unnamed_addr #2

attributes #0 = { nounwind uwtable "denormal-fp-math"="preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cmov,+crc32,+cx16,+cx8,+evex512,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { mustprogress nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none) }
attributes #2 = { nofree "intel-lang"="fortran" }
attributes #3 = { nounwind }

!omp_offload.info = !{}
!llvm.module.flags = !{!0, !1}

!0 = !{i32 1, !"ThinLTO", i32 0}
!1 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!2 = !{i64 25}
!3 = !{i64 3}
!4 = !{!5, !5, i64 0}
!5 = !{!"Fortran Dope Vector Symbol", !6, i64 0}
!6 = !{!"Generic Fortran Symbol", !7, i64 0}
!7 = !{!"ifx$root$1$foo_"}
!8 = !{!9, !10, i64 24}
!9 = !{!"ifx$descr$1", !10, i64 0, !10, i64 8, !10, i64 16, !10, i64 24, !10, i64 32, !10, i64 40, !10, i64 48, !10, i64 56, !10, i64 64, !10, i64 72, !10, i64 80, !10, i64 88}
!10 = !{!"ifx$descr$field", !5, i64 0}
!11 = !{!9, !10, i64 40}
!12 = !{!9, !10, i64 8}
!13 = !{!9, !10, i64 32}
!14 = !{!9, !10, i64 16}
!15 = !{!16, !16, i64 0}
!16 = !{!"ifx$unique_sym$1", !17, i64 0}
!17 = !{!"Fortran Data Symbol", !6, i64 0}
!18 = !{i64 20}
!19 = !{!9, !10, i64 64}
!20 = !{i64 18}
!21 = !{!9, !10, i64 48}
!22 = !{i64 19}
!23 = !{!9, !10, i64 56}
!24 = !{!25, !25, i64 0}
!25 = !{!"ifx$unique_sym$2", !17, i64 0}
!26 = !{i64 2}
!27 = !{!17, !17, i64 0}
!28 = !{!9, !10, i64 0}
!29 = !{i64 1, i64 -9223372036854775808}
!30 = !{i64 5}
!31 = !{!32, !32, i64 0}
!32 = !{!"ifx$unique_sym$5", !17, i64 0}
!33 = !{!34, !34, i64 0}
!34 = !{!"ifx$unique_sym$6", !17, i64 0}
!35 = !{!36, !36, i64 0}
!36 = !{!"ifx$unique_sym$7", !17, i64 0}
!37 = !{i64 11}
!38 = !{i32 102}
!39 = !{i32 94}
!40 = !{i32 95}
