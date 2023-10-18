; REQUIRES: asserts
; RUN: opt -passes=local-array-transpose -debug-only=local-array-transpose -S < %s 2>&1 | FileCheck %s

; Test local array transpose after fixing CMPLRLLVM-52391

; Check that ACOX is not transposed because its subscripts include both (I,J) and (J,I) indexing.

; CHECK: LocalArrayTranspose: BEGIN Valid Candidates for foo_
; CHECK: %"foo_$ACOX" = alloca %"QNCA_a0$float*$rank2$", align 8, !llfort.type_idx !2
; CHECK: LocalArrayTranspose: END Valid Candidates for foo_
; CHECK: LocalArrayTranspose: No Profitable Candidates for foo_

; Check that the subscripting of ACOX remains unchanged.

; CHECK: %indvars.iv197 = phi
; CHECK: %indvars.iv = phi
; CHECK: %"foo_$ACOX.addr_a0$_fetch.26[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %"foo_$ACOX.dim_info$.lower_bound$[]_fetch.30", i64 %"foo_$ACOX.dim_info$.spacing$[]_fetch.29", ptr elementtype(float) %"foo_$ACOX.addr_a0$113_fetch.71.pre", i64 %indvars.iv), !llfort.type_idx !30
; CHECK: %"foo_$ACOX.addr_a0$_fetch.26[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %"foo_$ACOX.dim_info$.lower_bound$[]_fetch.27", i64 4, ptr elementtype(float) %"foo_$ACOX.addr_a0$_fetch.26[]", i64 %indvars.iv197), !llfort.type_idx !30

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$float*$rank2$" = type { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }

; Function Attrs: nounwind uwtable
define void @foo_(ptr noalias nocapture readonly dereferenceable(96) "ptrnoalias" %"foo_$INPUT", ptr noalias nocapture readonly dereferenceable(96) "ptrnoalias" %"foo_$RESULT", ptr noalias nocapture readonly dereferenceable(4) %"foo_$N") local_unnamed_addr #0 {
alloca_0:
  %"foo_$ACOX" = alloca %"QNCA_a0$float*$rank2$", align 8, !llfort.type_idx !2
  %"var$4" = alloca i64, align 8, !llfort.type_idx !3
  %"var$2_fetch.1.fca.0.gep" = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %"foo_$ACOX", i64 0, i32 0
  store ptr null, ptr %"var$2_fetch.1.fca.0.gep", align 1, !tbaa !4
  %"var$2_fetch.1.fca.1.gep" = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %"foo_$ACOX", i64 0, i32 1
  store i64 0, ptr %"var$2_fetch.1.fca.1.gep", align 1, !tbaa !4
  %"var$2_fetch.1.fca.2.gep" = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %"foo_$ACOX", i64 0, i32 2
  store i64 0, ptr %"var$2_fetch.1.fca.2.gep", align 1, !tbaa !4
  %"var$2_fetch.1.fca.3.gep" = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %"foo_$ACOX", i64 0, i32 3
  store i64 128, ptr %"var$2_fetch.1.fca.3.gep", align 1, !tbaa !4
  %"var$2_fetch.1.fca.4.gep" = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %"foo_$ACOX", i64 0, i32 4
  store i64 2, ptr %"var$2_fetch.1.fca.4.gep", align 1, !tbaa !4
  %"var$2_fetch.1.fca.5.gep" = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %"foo_$ACOX", i64 0, i32 5
  store i64 0, ptr %"var$2_fetch.1.fca.5.gep", align 1, !tbaa !4
  %"var$2_fetch.1.fca.6.0.0.gep" = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %"foo_$ACOX", i64 0, i32 6, i64 0, i32 0
  store i64 0, ptr %"var$2_fetch.1.fca.6.0.0.gep", align 1, !tbaa !4
  %"var$2_fetch.1.fca.6.0.1.gep" = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %"foo_$ACOX", i64 0, i32 6, i64 0, i32 1
  store i64 0, ptr %"var$2_fetch.1.fca.6.0.1.gep", align 1, !tbaa !4
  %"var$2_fetch.1.fca.6.0.2.gep" = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %"foo_$ACOX", i64 0, i32 6, i64 0, i32 2
  store i64 0, ptr %"var$2_fetch.1.fca.6.0.2.gep", align 1, !tbaa !4
  %"var$2_fetch.1.fca.6.1.0.gep" = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %"foo_$ACOX", i64 0, i32 6, i64 1, i32 0
  store i64 0, ptr %"var$2_fetch.1.fca.6.1.0.gep", align 1, !tbaa !4
  %"var$2_fetch.1.fca.6.1.1.gep" = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %"foo_$ACOX", i64 0, i32 6, i64 1, i32 1
  store i64 0, ptr %"var$2_fetch.1.fca.6.1.1.gep", align 1, !tbaa !4
  %"var$2_fetch.1.fca.6.1.2.gep" = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %"foo_$ACOX", i64 0, i32 6, i64 1, i32 2
  store i64 0, ptr %"var$2_fetch.1.fca.6.1.2.gep", align 1, !tbaa !4
  store i64 133, ptr %"var$2_fetch.1.fca.3.gep", align 8, !tbaa !8
  store i64 0, ptr %"var$2_fetch.1.fca.5.gep", align 8, !tbaa !11
  store i64 4, ptr %"var$2_fetch.1.fca.1.gep", align 8, !tbaa !12
  store i64 2, ptr %"var$2_fetch.1.fca.4.gep", align 8, !tbaa !13
  store i64 0, ptr %"var$2_fetch.1.fca.2.gep", align 8, !tbaa !14
  %"foo_$N_fetch.3" = load i32, ptr %"foo_$N", align 1, !tbaa !15
  %int_sext = sext i32 %"foo_$N_fetch.3" to i64, !llfort.type_idx !3
  %"foo_$ACOX.dim_info$.lower_bound$21" = getelementptr inbounds { i64, i64, i64 }, ptr %"var$2_fetch.1.fca.6.0.0.gep", i64 0, i32 2, !llfort.type_idx !18
  %"foo_$ACOX.dim_info$.lower_bound$[]22" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"foo_$ACOX.dim_info$.lower_bound$21", i32 0), !llfort.type_idx !18
  store i64 1, ptr %"foo_$ACOX.dim_info$.lower_bound$[]22", align 1, !tbaa !19
  %rel.1 = icmp sgt i64 %int_sext, 0
  %slct.1 = select i1 %rel.1, i64 %int_sext, i64 0
  %"foo_$ACOX.dim_info$.extent$" = getelementptr inbounds { i64, i64, i64 }, ptr %"var$2_fetch.1.fca.6.0.0.gep", i64 0, i32 0, !llfort.type_idx !20
  %"foo_$ACOX.dim_info$.extent$[]" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"foo_$ACOX.dim_info$.extent$", i32 0), !llfort.type_idx !20
  store i64 %slct.1, ptr %"foo_$ACOX.dim_info$.extent$[]", align 1, !tbaa !21
  %"foo_$ACOX.dim_info$.spacing$" = getelementptr inbounds { i64, i64, i64 }, ptr %"var$2_fetch.1.fca.6.0.0.gep", i64 0, i32 1, !llfort.type_idx !22
  %"foo_$ACOX.dim_info$.spacing$[]" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"foo_$ACOX.dim_info$.spacing$", i32 0), !llfort.type_idx !22
  store i64 4, ptr %"foo_$ACOX.dim_info$.spacing$[]", align 1, !tbaa !23
  %mul.1 = shl nuw nsw i64 %slct.1, 2
  %"foo_$ACOX.dim_info$.lower_bound$[]28" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"foo_$ACOX.dim_info$.lower_bound$21", i32 1), !llfort.type_idx !18
  store i64 1, ptr %"foo_$ACOX.dim_info$.lower_bound$[]28", align 1, !tbaa !19
  %"foo_$ACOX.dim_info$.extent$[]31" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"foo_$ACOX.dim_info$.extent$", i32 1), !llfort.type_idx !20
  store i64 %slct.1, ptr %"foo_$ACOX.dim_info$.extent$[]31", align 1, !tbaa !21
  %"foo_$ACOX.dim_info$.spacing$[]34" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"foo_$ACOX.dim_info$.spacing$", i32 1), !llfort.type_idx !22
  store i64 %mul.1, ptr %"foo_$ACOX.dim_info$.spacing$[]34", align 1, !tbaa !23
  %func_result = call i32 (ptr, i32, ...) @for_check_mult_overflow64(ptr nonnull %"var$4", i32 3, i64 %slct.1, i64 %slct.1, i64 4) #3, !llfort.type_idx !24
  %"var$4_fetch.5" = load i64, ptr %"var$4", align 8, !tbaa !25, !llfort.type_idx !3
  store i64 1073741957, ptr %"var$2_fetch.1.fca.3.gep", align 8, !tbaa !8
  %and.8 = shl i32 %func_result, 4
  %shl.5 = and i32 %and.8, 16
  %or.9 = or i32 %shl.5, 262146
  %func_result12 = call i32 @for_alloc_allocatable_handle(i64 %"var$4_fetch.5", ptr nonnull %"var$2_fetch.1.fca.0.gep", i32 %or.9, ptr null) #3, !llfort.type_idx !24
  %rel.7 = icmp slt i32 %"foo_$N_fetch.3", 1
  %"foo_$ACOX.addr_a0$113_fetch.71.pre" = load ptr, ptr %"var$2_fetch.1.fca.0.gep", align 8, !tbaa !26
  br i1 %rel.7, label %do.end_do19, label %do.body9.preheader

do.body9.preheader:                               ; preds = %alloca_0
  %"foo_$INPUT.addr_a0$" = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %"foo_$INPUT", i64 0, i32 0, !llfort.type_idx !27
  %"foo_$INPUT.addr_a0$_fetch.16" = load ptr, ptr %"foo_$INPUT.addr_a0$", align 1, !tbaa !28, !llfort.type_idx !30
  %"foo_$INPUT.dim_info$.lower_bound$" = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %"foo_$INPUT", i64 0, i32 6, i64 0, i32 2
  %"foo_$INPUT.dim_info$.lower_bound$[]" = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"foo_$INPUT.dim_info$.lower_bound$", i32 0), !llfort.type_idx !18
  %"foo_$INPUT.dim_info$.lower_bound$[]_fetch.17" = load i64, ptr %"foo_$INPUT.dim_info$.lower_bound$[]", align 1, !tbaa !31
  %"foo_$INPUT.dim_info$.spacing$" = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %"foo_$INPUT", i64 0, i32 6, i64 0, i32 1
  %"foo_$INPUT.dim_info$.spacing$[]" = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"foo_$INPUT.dim_info$.spacing$", i32 1), !llfort.type_idx !22
  %"foo_$INPUT.dim_info$.spacing$[]_fetch.19" = load i64, ptr %"foo_$INPUT.dim_info$.spacing$[]", align 1, !tbaa !32, !range !33
  %"foo_$INPUT.dim_info$.lower_bound$[]40" = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"foo_$INPUT.dim_info$.lower_bound$", i32 1), !llfort.type_idx !18
  %"foo_$INPUT.dim_info$.lower_bound$[]_fetch.20" = load i64, ptr %"foo_$INPUT.dim_info$.lower_bound$[]40", align 1, !tbaa !31
  %"foo_$ACOX.dim_info$.lower_bound$[]_fetch.27" = load i64, ptr %"foo_$ACOX.dim_info$.lower_bound$[]22", align 1, !tbaa !19
  %"foo_$ACOX.dim_info$.spacing$[]_fetch.29" = load i64, ptr %"foo_$ACOX.dim_info$.spacing$[]34", align 1, !tbaa !23, !range !33
  %"foo_$ACOX.dim_info$.lower_bound$[]_fetch.30" = load i64, ptr %"foo_$ACOX.dim_info$.lower_bound$[]28", align 1, !tbaa !19
  %0 = add nuw nsw i32 %"foo_$N_fetch.3", 1
  %wide.trip.count199 = sext i32 %0 to i64
  br label %do.body9

do.body9:                                         ; preds = %do.body9.preheader, %do.end_do14.loopexit
  %indvars.iv197 = phi i64 [ 1, %do.body9.preheader ], [ %indvars.iv.next198, %do.end_do14.loopexit ]
  %"foo_$INPUT.addr_a0$_fetch.16[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %"foo_$INPUT.dim_info$.lower_bound$[]_fetch.20", i64 %"foo_$INPUT.dim_info$.spacing$[]_fetch.19", ptr elementtype(float) %"foo_$INPUT.addr_a0$_fetch.16", i64 %indvars.iv197), !llfort.type_idx !30
  br label %do.body13

do.body13:                                        ; preds = %do.body9, %do.body13
  %indvars.iv = phi i64 [ 1, %do.body9 ], [ %indvars.iv.next, %do.body13 ]
  %"foo_$INPUT.addr_a0$_fetch.16[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %"foo_$INPUT.dim_info$.lower_bound$[]_fetch.17", i64 4, ptr elementtype(float) %"foo_$INPUT.addr_a0$_fetch.16[]", i64 %indvars.iv), !llfort.type_idx !30
  %"foo_$INPUT.addr_a0$_fetch.16[][]_fetch.25" = load float, ptr %"foo_$INPUT.addr_a0$_fetch.16[][]", align 1, !tbaa !34, !llfort.type_idx !30
  %"foo_$ACOX.addr_a0$_fetch.26[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %"foo_$ACOX.dim_info$.lower_bound$[]_fetch.30", i64 %"foo_$ACOX.dim_info$.spacing$[]_fetch.29", ptr elementtype(float) %"foo_$ACOX.addr_a0$113_fetch.71.pre", i64 %indvars.iv), !llfort.type_idx !30
  %"foo_$ACOX.addr_a0$_fetch.26[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %"foo_$ACOX.dim_info$.lower_bound$[]_fetch.27", i64 4, ptr elementtype(float) %"foo_$ACOX.addr_a0$_fetch.26[]", i64 %indvars.iv197), !llfort.type_idx !30
  store float %"foo_$INPUT.addr_a0$_fetch.16[][]_fetch.25", ptr %"foo_$ACOX.addr_a0$_fetch.26[][]", align 4, !tbaa !36
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count199
  br i1 %exitcond, label %do.end_do14.loopexit, label %do.body13

do.end_do14.loopexit:                             ; preds = %do.body13
  %indvars.iv.next198 = add nuw nsw i64 %indvars.iv197, 1
  %exitcond200 = icmp eq i64 %indvars.iv.next198, %wide.trip.count199
  br i1 %exitcond200, label %do.body18.preheader, label %do.body9

do.body18.preheader:                              ; preds = %do.end_do14.loopexit
  %"foo_$RESULT.addr_a0$" = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %"foo_$RESULT", i64 0, i32 0, !llfort.type_idx !38
  %"foo_$RESULT.addr_a0$_fetch.55" = load ptr, ptr %"foo_$RESULT.addr_a0$", align 1, !tbaa !39, !llfort.type_idx !30
  %"foo_$RESULT.dim_info$.lower_bound$" = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %"foo_$RESULT", i64 0, i32 6, i64 0, i32 2
  %"foo_$RESULT.dim_info$.lower_bound$[]" = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"foo_$RESULT.dim_info$.lower_bound$", i32 0), !llfort.type_idx !18
  %"foo_$RESULT.dim_info$.lower_bound$[]_fetch.56" = load i64, ptr %"foo_$RESULT.dim_info$.lower_bound$[]", align 1, !tbaa !41
  %"foo_$RESULT.dim_info$.spacing$" = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %"foo_$RESULT", i64 0, i32 6, i64 0, i32 1
  %"foo_$RESULT.dim_info$.spacing$[]" = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"foo_$RESULT.dim_info$.spacing$", i32 1), !llfort.type_idx !22
  %"foo_$RESULT.dim_info$.spacing$[]_fetch.58" = load i64, ptr %"foo_$RESULT.dim_info$.spacing$[]", align 1, !tbaa !42, !range !33
  %"foo_$RESULT.dim_info$.lower_bound$[]99" = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"foo_$RESULT.dim_info$.lower_bound$", i32 1), !llfort.type_idx !18
  %"foo_$RESULT.dim_info$.lower_bound$[]_fetch.59" = load i64, ptr %"foo_$RESULT.dim_info$.lower_bound$[]99", align 1, !tbaa !41
  br label %do.body18

do.body18:                                        ; preds = %do.body18.preheader, %do.end_do23.loopexit
  %indvars.iv205 = phi i64 [ 1, %do.body18.preheader ], [ %indvars.iv.next206, %do.end_do23.loopexit ]
  %"foo_$ACOX.addr_a0$_fetch.45[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %"foo_$ACOX.dim_info$.lower_bound$[]_fetch.30", i64 %"foo_$ACOX.dim_info$.spacing$[]_fetch.29", ptr elementtype(float) %"foo_$ACOX.addr_a0$113_fetch.71.pre", i64 %indvars.iv205), !llfort.type_idx !30
  br label %do.body22

do.body22:                                        ; preds = %do.body18, %do.body22
  %indvars.iv201 = phi i64 [ 1, %do.body18 ], [ %indvars.iv.next202, %do.body22 ]
  %"foo_$ACOX.addr_a0$_fetch.45[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %"foo_$ACOX.dim_info$.lower_bound$[]_fetch.27", i64 4, ptr elementtype(float) %"foo_$ACOX.addr_a0$_fetch.45[]", i64 %indvars.iv201), !llfort.type_idx !30
  %"foo_$ACOX.addr_a0$_fetch.45[][]_fetch.54" = load float, ptr %"foo_$ACOX.addr_a0$_fetch.45[][]", align 4, !tbaa !36, !llfort.type_idx !30
  %"foo_$RESULT.addr_a0$_fetch.55[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %"foo_$RESULT.dim_info$.lower_bound$[]_fetch.59", i64 %"foo_$RESULT.dim_info$.spacing$[]_fetch.58", ptr elementtype(float) %"foo_$RESULT.addr_a0$_fetch.55", i64 %indvars.iv201), !llfort.type_idx !30
  %"foo_$RESULT.addr_a0$_fetch.55[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %"foo_$RESULT.dim_info$.lower_bound$[]_fetch.56", i64 4, ptr elementtype(float) %"foo_$RESULT.addr_a0$_fetch.55[]", i64 %indvars.iv205), !llfort.type_idx !30
  store float %"foo_$ACOX.addr_a0$_fetch.45[][]_fetch.54", ptr %"foo_$RESULT.addr_a0$_fetch.55[][]", align 1, !tbaa !43
  %indvars.iv.next202 = add nuw nsw i64 %indvars.iv201, 1
  %exitcond204 = icmp eq i64 %indvars.iv.next202, %wide.trip.count199
  br i1 %exitcond204, label %do.end_do23.loopexit, label %do.body22

do.end_do23.loopexit:                             ; preds = %do.body22
  %indvars.iv.next206 = add nuw nsw i64 %indvars.iv205, 1
  %exitcond208 = icmp eq i64 %indvars.iv.next206, %wide.trip.count199
  br i1 %exitcond208, label %do.end_do19.loopexit, label %do.body18

do.end_do19.loopexit:                             ; preds = %do.end_do23.loopexit
  br label %do.end_do19

do.end_do19:                                      ; preds = %do.end_do19.loopexit, %alloca_0
  %"foo_$ACOX.flags$115_fetch.72" = load i64, ptr %"var$2_fetch.1.fca.3.gep", align 8, !tbaa !8
  %"foo_$ACOX.flags$115_fetch.72.tr" = trunc i64 %"foo_$ACOX.flags$115_fetch.72" to i32
  %1 = shl i32 %"foo_$ACOX.flags$115_fetch.72.tr", 1
  %or.11 = and i32 %1, 6
  %2 = lshr i32 %"foo_$ACOX.flags$115_fetch.72.tr", 3
  %int_zext121 = and i32 %2, 256
  %3 = lshr i64 %"foo_$ACOX.flags$115_fetch.72", 15
  %4 = trunc i64 %3 to i32
  %5 = and i32 %4, 65011712
  %or.12 = or i32 %int_zext121, %or.11
  %or.15 = or i32 %or.12, %5
  %or.16 = or i32 %or.15, 262144
  %"foo_$ACOX.reserved$129_fetch.73" = load i64, ptr %"var$2_fetch.1.fca.5.gep", align 8, !tbaa !11
  %"(ptr)foo_$ACOX.reserved$129_fetch.73$" = inttoptr i64 %"foo_$ACOX.reserved$129_fetch.73" to ptr, !llfort.type_idx !45
  %func_result131 = tail call i32 @for_dealloc_allocatable_handle(ptr %"foo_$ACOX.addr_a0$113_fetch.71.pre", i32 %or.16, ptr %"(ptr)foo_$ACOX.reserved$129_fetch.73$") #3, !llfort.type_idx !24
  %rel.15 = icmp eq i32 %func_result131, 0
  br i1 %rel.15, label %bb_new29_then, label %bb3_endif

bb_new29_then:                                    ; preds = %do.end_do19
  store ptr null, ptr %"var$2_fetch.1.fca.0.gep", align 8, !tbaa !26
  %and.32 = and i64 %"foo_$ACOX.flags$115_fetch.72", -1030792153090
  store i64 %and.32, ptr %"var$2_fetch.1.fca.3.gep", align 8, !tbaa !8
  br label %bb3_endif

bb3_endif:                                        ; preds = %do.end_do19, %bb_new29_then
  %"foo_$ACOX.addr_a0$142_fetch.78" = phi ptr [ %"foo_$ACOX.addr_a0$113_fetch.71.pre", %do.end_do19 ], [ null, %bb_new29_then ]
  %"foo_$ACOX.flags$140_fetch.77" = phi i64 [ %"foo_$ACOX.flags$115_fetch.72", %do.end_do19 ], [ %and.32, %bb_new29_then ]
  %and.33 = and i64 %"foo_$ACOX.flags$140_fetch.77", 1
  %rel.16 = icmp eq i64 %and.33, 0
  br i1 %rel.16, label %dealloc.list.end31, label %dealloc.list.then30

dealloc.list.then30:                              ; preds = %bb3_endif
  %"foo_$ACOX.flags$140_fetch.77.tr" = trunc i64 %"foo_$ACOX.flags$140_fetch.77" to i32
  %and.33.tr = trunc i64 %and.33 to i32
  %6 = and i32 %"foo_$ACOX.flags$140_fetch.77.tr", 2
  %7 = or i32 %6, %and.33.tr
  %or.19 = shl nuw nsw i32 %7, 1
  %8 = lshr i32 %"foo_$ACOX.flags$140_fetch.77.tr", 3
  %int_zext150 = and i32 %8, 256
  %or.20 = or i32 %or.19, %int_zext150
  %9 = lshr i64 %"foo_$ACOX.flags$140_fetch.77", 15
  %10 = trunc i64 %9 to i32
  %11 = and i32 %10, 65011712
  %and.46 = or i32 %or.20, %11
  %or.24 = or i32 %and.46, 262144
  %func_result160 = tail call i32 @for_dealloc_allocatable_handle(ptr %"foo_$ACOX.addr_a0$142_fetch.78", i32 %or.24, ptr %"(ptr)foo_$ACOX.reserved$129_fetch.73$") #3, !llfort.type_idx !24
  %rel.17 = icmp eq i32 %func_result160, 0
  br i1 %rel.17, label %bb_new34_then, label %dealloc.list.end31

bb_new34_then:                                    ; preds = %dealloc.list.then30
  store ptr null, ptr %"var$2_fetch.1.fca.0.gep", align 8, !tbaa !26
  %and.48 = and i64 %"foo_$ACOX.flags$140_fetch.77", -2050
  store i64 %and.48, ptr %"var$2_fetch.1.fca.3.gep", align 8, !tbaa !8
  br label %dealloc.list.end31

dealloc.list.end31:                               ; preds = %bb_new34_then, %dealloc.list.then30, %bb3_endif
  ret void
}

; Function Attrs: mustprogress nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none)
declare ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8, i64, i32, ptr, i32) #1

; Function Attrs: nofree
declare !llfort.intrin_id !46 i32 @for_check_mult_overflow64(ptr nocapture, i32, ...) local_unnamed_addr #2

; Function Attrs: nofree
declare !llfort.intrin_id !47 i32 @for_alloc_allocatable_handle(i64, ptr nocapture, i32, ptr) local_unnamed_addr #2

; Function Attrs: mustprogress nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none)
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #1

; Function Attrs: nofree
declare !llfort.intrin_id !48 i32 @for_dealloc_allocatable_handle(ptr nocapture readonly, i32, ptr) local_unnamed_addr #2

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
!24 = !{i64 2}
!25 = !{!17, !17, i64 0}
!26 = !{!9, !10, i64 0}
!27 = !{i64 56}
!28 = !{!29, !10, i64 0}
!29 = !{!"ifx$descr$2", !10, i64 0, !10, i64 8, !10, i64 16, !10, i64 24, !10, i64 32, !10, i64 40, !10, i64 48, !10, i64 56, !10, i64 64, !10, i64 72, !10, i64 80, !10, i64 88}
!30 = !{i64 5}
!31 = !{!29, !10, i64 64}
!32 = !{!29, !10, i64 56}
!33 = !{i64 1, i64 -9223372036854775808}
!34 = !{!35, !35, i64 0}
!35 = !{!"ifx$unique_sym$4", !17, i64 0}
!36 = !{!37, !37, i64 0}
!37 = !{!"ifx$unique_sym$5", !17, i64 0}
!38 = !{i64 78}
!39 = !{!40, !10, i64 0}
!40 = !{!"ifx$descr$3", !10, i64 0, !10, i64 8, !10, i64 16, !10, i64 24, !10, i64 32, !10, i64 40, !10, i64 48, !10, i64 56, !10, i64 64, !10, i64 72, !10, i64 80, !10, i64 88}
!41 = !{!40, !10, i64 64}
!42 = !{!40, !10, i64 56}
!43 = !{!44, !44, i64 0}
!44 = !{!"ifx$unique_sym$6", !17, i64 0}
!45 = !{i64 11}
!46 = !{i32 102}
!47 = !{i32 94}
!48 = !{i32 95}
