; REQUIRES: asserts
; RUN: opt -passes=local-array-transpose -debug-only=local-array-transpose -S < %s 2>&1 | FileCheck %s

; Test local array transpose after fixing CMPLRLLVM-52391

; Check that BCOX and ACOX are valid, but only ACOX is profitable in the trace

; CHECK: LocalArrayTranspose: BEGIN Valid Candidates for foo_
; CHECK: %"foo_$BCOX" = alloca %"QNCA_a0$float*$rank2$", align 8, !llfort.type_idx !2
; CHECK: %"foo_$ACOX" = alloca %"QNCA_a0$float*$rank2$", align 8, !llfort.type_idx !2
; CHECK: LocalArrayTranspose: END Valid Candidates for foo_
; CHECK: LocalArrayTranspose: BEGIN Profitable Candidates for foo_
; CHECK:  %"foo_$ACOX" = alloca %"QNCA_a0$float*$rank2$", align 8, !llfort.type_idx !2
; CHECK: LocalArrayTranspose: END Profitable Candidates for foo_

; Check that ACOX's subscript indexing has been inverted, but BCOX's has not.

; CHECK: %indvars.iv372 = phi
; CHECK: %"foo_$ACOX.addr_a0$_fetch.37[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %"foo_$ACOX.dim_info$.lower_bound$[]_fetch.41", i64 %"foo_$ACOX.dim_info$.spacing$[]_fetch.40", ptr elementtype(float) %"foo_$ACOX.addr_a0$215_fetch.111.pre", i64 %indvars.iv372), !llfort.type_idx !43
; CHECK: %indvars.iv = phi i64 
; CHECK: %"foo_$ACOX.addr_a0$_fetch.37[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %"foo_$ACOX.dim_info$.lower_bound$[]_fetch.38", i64 4, ptr elementtype(float) %"foo_$ACOX.addr_a0$_fetch.37[]", i64 %indvars.iv), !llfort.type_idx !43
; CHECK: %"foo_$BCOX.addr_a0$_fetch.56[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %"foo_$BCOX.dim_info$.lower_bound$[]_fetch.60", i64 %"foo_$BCOX.dim_info$.spacing$[]_fetch.59", ptr elementtype(float) %"foo_$BCOX.addr_a0$_fetch.56", i64 %indvars.iv), !llfort.type_idx !43
; CHECK: %"foo_$BCOX.addr_a0$_fetch.56[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %"foo_$BCOX.dim_info$.lower_bound$[]_fetch.57", i64 4, ptr elementtype(float) %"foo_$BCOX.addr_a0$_fetch.56[]", i64 %indvars.iv372), !llfort.type_idx !43

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$float*$rank2$" = type { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }

; Function Attrs: nounwind uwtable
define void @foo_(ptr noalias nocapture readonly dereferenceable(96) "ptrnoalias" %"foo_$INPUT", ptr noalias nocapture readonly dereferenceable(96) "ptrnoalias" %"foo_$RESULT", ptr noalias nocapture readonly dereferenceable(4) %"foo_$N") local_unnamed_addr #0 {
alloca_0:
  %"foo_$BCOX" = alloca %"QNCA_a0$float*$rank2$", align 8, !llfort.type_idx !2
  %"foo_$ACOX" = alloca %"QNCA_a0$float*$rank2$", align 8, !llfort.type_idx !2
  %"var$4" = alloca i64, align 8, !llfort.type_idx !3
  %"var$6" = alloca i64, align 8, !llfort.type_idx !3
  %"var$2_fetch.1.fca.0.gep" = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %"foo_$BCOX", i64 0, i32 0
  store ptr null, ptr %"var$2_fetch.1.fca.0.gep", align 1, !tbaa !4
  %"var$2_fetch.1.fca.1.gep" = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %"foo_$BCOX", i64 0, i32 1
  store i64 0, ptr %"var$2_fetch.1.fca.1.gep", align 1, !tbaa !4
  %"var$2_fetch.1.fca.2.gep" = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %"foo_$BCOX", i64 0, i32 2
  store i64 0, ptr %"var$2_fetch.1.fca.2.gep", align 1, !tbaa !4
  %"var$2_fetch.1.fca.3.gep" = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %"foo_$BCOX", i64 0, i32 3
  store i64 128, ptr %"var$2_fetch.1.fca.3.gep", align 1, !tbaa !4
  %"var$2_fetch.1.fca.4.gep" = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %"foo_$BCOX", i64 0, i32 4
  store i64 2, ptr %"var$2_fetch.1.fca.4.gep", align 1, !tbaa !4
  %"var$2_fetch.1.fca.5.gep" = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %"foo_$BCOX", i64 0, i32 5
  store i64 0, ptr %"var$2_fetch.1.fca.5.gep", align 1, !tbaa !4
  %"var$2_fetch.1.fca.6.0.0.gep" = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %"foo_$BCOX", i64 0, i32 6, i64 0, i32 0
  store i64 0, ptr %"var$2_fetch.1.fca.6.0.0.gep", align 1, !tbaa !4
  %"var$2_fetch.1.fca.6.0.1.gep" = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %"foo_$BCOX", i64 0, i32 6, i64 0, i32 1
  store i64 0, ptr %"var$2_fetch.1.fca.6.0.1.gep", align 1, !tbaa !4
  %"var$2_fetch.1.fca.6.0.2.gep" = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %"foo_$BCOX", i64 0, i32 6, i64 0, i32 2
  store i64 0, ptr %"var$2_fetch.1.fca.6.0.2.gep", align 1, !tbaa !4
  %"var$2_fetch.1.fca.6.1.0.gep" = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %"foo_$BCOX", i64 0, i32 6, i64 1, i32 0
  store i64 0, ptr %"var$2_fetch.1.fca.6.1.0.gep", align 1, !tbaa !4
  %"var$2_fetch.1.fca.6.1.1.gep" = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %"foo_$BCOX", i64 0, i32 6, i64 1, i32 1
  store i64 0, ptr %"var$2_fetch.1.fca.6.1.1.gep", align 1, !tbaa !4
  %"var$2_fetch.1.fca.6.1.2.gep" = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %"foo_$BCOX", i64 0, i32 6, i64 1, i32 2
  store i64 0, ptr %"var$2_fetch.1.fca.6.1.2.gep", align 1, !tbaa !4
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
  %"foo_$N_fetch.4" = load i32, ptr %"foo_$N", align 1, !tbaa !15
  %int_sext = sext i32 %"foo_$N_fetch.4" to i64, !llfort.type_idx !3
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
  %"foo_$ACOX.dim_info$.lower_bound$[]28" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"foo_$ACOX.dim_info$.lower_bound$21", i32 1), !llfort.type_idx !18
  store i64 1, ptr %"foo_$ACOX.dim_info$.lower_bound$[]28", align 1, !tbaa !19
  %"foo_$ACOX.dim_info$.extent$[]31" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"foo_$ACOX.dim_info$.extent$", i32 1), !llfort.type_idx !20
  store i64 %slct.1, ptr %"foo_$ACOX.dim_info$.extent$[]31", align 1, !tbaa !21
  %"foo_$ACOX.dim_info$.spacing$[]34" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"foo_$ACOX.dim_info$.spacing$", i32 1), !llfort.type_idx !22
  store i64 %mul.1, ptr %"foo_$ACOX.dim_info$.spacing$[]34", align 1, !tbaa !23
  %func_result = call i32 (ptr, i32, ...) @for_check_mult_overflow64(ptr nonnull %"var$4", i32 3, i64 %slct.1, i64 %slct.1, i64 4) #3, !llfort.type_idx !24
  %"var$4_fetch.6" = load i64, ptr %"var$4", align 8, !tbaa !25, !llfort.type_idx !3
  store i64 1073741957, ptr %"var$2_fetch.2.fca.3.gep", align 8, !tbaa !8
  %and.8 = shl i32 %func_result, 4
  %shl.5 = and i32 %and.8, 16
  %or.9 = or i32 %shl.5, 262146
  %func_result12 = call i32 @for_alloc_allocatable_handle(i64 %"var$4_fetch.6", ptr nonnull %"var$2_fetch.2.fca.0.gep", i32 %or.9, ptr null) #3, !llfort.type_idx !24
  store i64 133, ptr %"var$2_fetch.1.fca.3.gep", align 8, !tbaa !26
  store i64 0, ptr %"var$2_fetch.1.fca.5.gep", align 8, !tbaa !28
  store i64 4, ptr %"var$2_fetch.1.fca.1.gep", align 8, !tbaa !29
  store i64 2, ptr %"var$2_fetch.1.fca.4.gep", align 8, !tbaa !30
  store i64 0, ptr %"var$2_fetch.1.fca.2.gep", align 8, !tbaa !31
  %"foo_$BCOX.dim_info$.lower_bound$60" = getelementptr inbounds { i64, i64, i64 }, ptr %"var$2_fetch.1.fca.6.0.0.gep", i64 0, i32 2, !llfort.type_idx !18
  %"foo_$BCOX.dim_info$.lower_bound$[]61" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"foo_$BCOX.dim_info$.lower_bound$60", i32 0), !llfort.type_idx !18
  store i64 1, ptr %"foo_$BCOX.dim_info$.lower_bound$[]61", align 1, !tbaa !32
  %"foo_$BCOX.dim_info$.extent$" = getelementptr inbounds { i64, i64, i64 }, ptr %"var$2_fetch.1.fca.6.0.0.gep", i64 0, i32 0, !llfort.type_idx !20
  %"foo_$BCOX.dim_info$.extent$[]" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"foo_$BCOX.dim_info$.extent$", i32 0), !llfort.type_idx !20
  store i64 %slct.1, ptr %"foo_$BCOX.dim_info$.extent$[]", align 1, !tbaa !33
  %"foo_$BCOX.dim_info$.spacing$" = getelementptr inbounds { i64, i64, i64 }, ptr %"var$2_fetch.1.fca.6.0.0.gep", i64 0, i32 1, !llfort.type_idx !22
  %"foo_$BCOX.dim_info$.spacing$[]" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"foo_$BCOX.dim_info$.spacing$", i32 0), !llfort.type_idx !22
  store i64 4, ptr %"foo_$BCOX.dim_info$.spacing$[]", align 1, !tbaa !34
  %"foo_$BCOX.dim_info$.lower_bound$[]67" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"foo_$BCOX.dim_info$.lower_bound$60", i32 1), !llfort.type_idx !18
  store i64 1, ptr %"foo_$BCOX.dim_info$.lower_bound$[]67", align 1, !tbaa !32
  %"foo_$BCOX.dim_info$.extent$[]70" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"foo_$BCOX.dim_info$.extent$", i32 1), !llfort.type_idx !20
  store i64 %slct.1, ptr %"foo_$BCOX.dim_info$.extent$[]70", align 1, !tbaa !33
  %"foo_$BCOX.dim_info$.spacing$[]73" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"foo_$BCOX.dim_info$.spacing$", i32 1), !llfort.type_idx !22
  store i64 %mul.1, ptr %"foo_$BCOX.dim_info$.spacing$[]73", align 1, !tbaa !34
  %func_result36 = call i32 (ptr, i32, ...) @for_check_mult_overflow64(ptr nonnull %"var$6", i32 3, i64 %slct.1, i64 %slct.1, i64 4) #3, !llfort.type_idx !24
  %"var$6_fetch.16" = load i64, ptr %"var$6", align 8, !tbaa !25, !llfort.type_idx !3
  store i64 1073741957, ptr %"var$2_fetch.1.fca.3.gep", align 8, !tbaa !26
  %and.24 = shl i32 %func_result36, 4
  %shl.14 = and i32 %and.24, 16
  %or.18 = or i32 %shl.14, 262146
  %func_result50 = call i32 @for_alloc_allocatable_handle(i64 %"var$6_fetch.16", ptr nonnull %"var$2_fetch.1.fca.0.gep", i32 %or.18, ptr null) #3, !llfort.type_idx !24
  %rel.13 = icmp slt i32 %"foo_$N_fetch.4", 1
  %"foo_$ACOX.addr_a0$215_fetch.111.pre" = load ptr, ptr %"var$2_fetch.2.fca.0.gep", align 8, !tbaa !35
  br i1 %rel.13, label %do.end_do26, label %do.body15.preheader

do.body15.preheader:                              ; preds = %alloca_0
  %"foo_$INPUT.addr_a0$" = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %"foo_$INPUT", i64 0, i32 0, !llfort.type_idx !36
  %"foo_$INPUT.addr_a0$_fetch.27" = load ptr, ptr %"foo_$INPUT.addr_a0$", align 1, !tbaa !37
  %"foo_$INPUT.dim_info$.lower_bound$" = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %"foo_$INPUT", i64 0, i32 6, i64 0, i32 2
  %"foo_$INPUT.dim_info$.lower_bound$[]" = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"foo_$INPUT.dim_info$.lower_bound$", i32 0), !llfort.type_idx !18
  %"foo_$INPUT.dim_info$.lower_bound$[]_fetch.28" = load i64, ptr %"foo_$INPUT.dim_info$.lower_bound$[]", align 1, !tbaa !39
  %"foo_$INPUT.dim_info$.spacing$" = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %"foo_$INPUT", i64 0, i32 6, i64 0, i32 1
  %"foo_$INPUT.dim_info$.spacing$[]" = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"foo_$INPUT.dim_info$.spacing$", i32 1), !llfort.type_idx !22
  %"foo_$INPUT.dim_info$.spacing$[]_fetch.30" = load i64, ptr %"foo_$INPUT.dim_info$.spacing$[]", align 1, !tbaa !40, !range !41
  %"foo_$INPUT.dim_info$.lower_bound$[]79" = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"foo_$INPUT.dim_info$.lower_bound$", i32 1), !llfort.type_idx !18
  %"foo_$INPUT.dim_info$.lower_bound$[]_fetch.31" = load i64, ptr %"foo_$INPUT.dim_info$.lower_bound$[]79", align 1, !tbaa !39
  %"foo_$ACOX.dim_info$.lower_bound$[]_fetch.38" = load i64, ptr %"foo_$ACOX.dim_info$.lower_bound$[]22", align 1, !tbaa !19
  %"foo_$ACOX.dim_info$.spacing$[]_fetch.40" = load i64, ptr %"foo_$ACOX.dim_info$.spacing$[]34", align 1, !tbaa !23, !range !41
  %"foo_$ACOX.dim_info$.lower_bound$[]_fetch.41" = load i64, ptr %"foo_$ACOX.dim_info$.lower_bound$[]28", align 1, !tbaa !19
  %"foo_$BCOX.addr_a0$_fetch.56" = load ptr, ptr %"var$2_fetch.1.fca.0.gep", align 8, !tbaa !42
  %"foo_$BCOX.dim_info$.lower_bound$[]_fetch.57" = load i64, ptr %"foo_$BCOX.dim_info$.lower_bound$[]61", align 1, !tbaa !32
  %"foo_$BCOX.dim_info$.spacing$[]_fetch.59" = load i64, ptr %"foo_$BCOX.dim_info$.spacing$[]73", align 1, !tbaa !34, !range !41
  %"foo_$BCOX.dim_info$.lower_bound$[]_fetch.60" = load i64, ptr %"foo_$BCOX.dim_info$.lower_bound$[]67", align 1, !tbaa !32
  %0 = add nuw nsw i32 %"foo_$N_fetch.4", 1
  %wide.trip.count374 = sext i32 %0 to i64
  br label %do.body15

do.body15:                                        ; preds = %do.body15.preheader, %do.end_do20.loopexit
  %indvars.iv372 = phi i64 [ 1, %do.body15.preheader ], [ %indvars.iv.next373, %do.end_do20.loopexit ]
  %"foo_$INPUT.addr_a0$_fetch.27[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %"foo_$INPUT.dim_info$.lower_bound$[]_fetch.31", i64 %"foo_$INPUT.dim_info$.spacing$[]_fetch.30", ptr elementtype(float) %"foo_$INPUT.addr_a0$_fetch.27", i64 %indvars.iv372), !llfort.type_idx !43
  br label %do.body19

do.body19:                                        ; preds = %do.body15, %do.body19
  %indvars.iv = phi i64 [ 1, %do.body15 ], [ %indvars.iv.next, %do.body19 ]
  %"foo_$INPUT.addr_a0$_fetch.27[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %"foo_$INPUT.dim_info$.lower_bound$[]_fetch.28", i64 4, ptr elementtype(float) %"foo_$INPUT.addr_a0$_fetch.27[]", i64 %indvars.iv), !llfort.type_idx !43
  %"foo_$INPUT.addr_a0$_fetch.27[][]_fetch.36" = load float, ptr %"foo_$INPUT.addr_a0$_fetch.27[][]", align 1, !tbaa !44
  %"foo_$ACOX.addr_a0$_fetch.37[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %"foo_$ACOX.dim_info$.lower_bound$[]_fetch.41", i64 %"foo_$ACOX.dim_info$.spacing$[]_fetch.40", ptr elementtype(float) %"foo_$ACOX.addr_a0$215_fetch.111.pre", i64 %indvars.iv), !llfort.type_idx !43
  %"foo_$ACOX.addr_a0$_fetch.37[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %"foo_$ACOX.dim_info$.lower_bound$[]_fetch.38", i64 4, ptr elementtype(float) %"foo_$ACOX.addr_a0$_fetch.37[]", i64 %indvars.iv372), !llfort.type_idx !43
  store float %"foo_$INPUT.addr_a0$_fetch.27[][]_fetch.36", ptr %"foo_$ACOX.addr_a0$_fetch.37[][]", align 4, !tbaa !46
  %"foo_$BCOX.addr_a0$_fetch.56[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %"foo_$BCOX.dim_info$.lower_bound$[]_fetch.60", i64 %"foo_$BCOX.dim_info$.spacing$[]_fetch.59", ptr elementtype(float) %"foo_$BCOX.addr_a0$_fetch.56", i64 %indvars.iv), !llfort.type_idx !43
  %"foo_$BCOX.addr_a0$_fetch.56[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %"foo_$BCOX.dim_info$.lower_bound$[]_fetch.57", i64 4, ptr elementtype(float) %"foo_$BCOX.addr_a0$_fetch.56[]", i64 %indvars.iv372), !llfort.type_idx !43
  store float %"foo_$INPUT.addr_a0$_fetch.27[][]_fetch.36", ptr %"foo_$BCOX.addr_a0$_fetch.56[][]", align 4, !tbaa !48
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count374
  br i1 %exitcond, label %do.end_do20.loopexit, label %do.body19

do.end_do20.loopexit:                             ; preds = %do.body19
  %indvars.iv.next373 = add nuw nsw i64 %indvars.iv372, 1
  %exitcond375 = icmp eq i64 %indvars.iv.next373, %wide.trip.count374
  br i1 %exitcond375, label %do.body25.preheader, label %do.body15

do.body25.preheader:                              ; preds = %do.end_do20.loopexit
  %"foo_$RESULT.addr_a0$" = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %"foo_$RESULT", i64 0, i32 0, !llfort.type_idx !50
  %"foo_$RESULT.addr_a0$_fetch.95" = load ptr, ptr %"foo_$RESULT.addr_a0$", align 1, !tbaa !51, !llfort.type_idx !43
  %"foo_$RESULT.dim_info$.lower_bound$" = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %"foo_$RESULT", i64 0, i32 6, i64 0, i32 2
  %"foo_$RESULT.dim_info$.lower_bound$[]" = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"foo_$RESULT.dim_info$.lower_bound$", i32 0), !llfort.type_idx !18
  %"foo_$RESULT.dim_info$.lower_bound$[]_fetch.96" = load i64, ptr %"foo_$RESULT.dim_info$.lower_bound$[]", align 1, !tbaa !53
  %"foo_$RESULT.dim_info$.spacing$" = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %"foo_$RESULT", i64 0, i32 6, i64 0, i32 1
  %"foo_$RESULT.dim_info$.spacing$[]" = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"foo_$RESULT.dim_info$.spacing$", i32 1), !llfort.type_idx !22
  %"foo_$RESULT.dim_info$.spacing$[]_fetch.98" = load i64, ptr %"foo_$RESULT.dim_info$.spacing$[]", align 1, !tbaa !54, !range !41
  %"foo_$RESULT.dim_info$.lower_bound$[]201" = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"foo_$RESULT.dim_info$.lower_bound$", i32 1), !llfort.type_idx !18
  %"foo_$RESULT.dim_info$.lower_bound$[]_fetch.99" = load i64, ptr %"foo_$RESULT.dim_info$.lower_bound$[]201", align 1, !tbaa !53
  br label %do.body25

do.body25:                                        ; preds = %do.body25.preheader, %do.end_do30.loopexit
  %indvars.iv380 = phi i64 [ 1, %do.body25.preheader ], [ %indvars.iv.next381, %do.end_do30.loopexit ]
  %"foo_$BCOX.addr_a0$_fetch.85[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %"foo_$BCOX.dim_info$.lower_bound$[]_fetch.60", i64 %"foo_$BCOX.dim_info$.spacing$[]_fetch.59", ptr elementtype(float) %"foo_$BCOX.addr_a0$_fetch.56", i64 %indvars.iv380), !llfort.type_idx !43
  br label %do.body29

do.body29:                                        ; preds = %do.body25, %do.body29
  %indvars.iv376 = phi i64 [ 1, %do.body25 ], [ %indvars.iv.next377, %do.body29 ]
  %"foo_$ACOX.addr_a0$_fetch.75[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %"foo_$ACOX.dim_info$.lower_bound$[]_fetch.41", i64 %"foo_$ACOX.dim_info$.spacing$[]_fetch.40", ptr elementtype(float) %"foo_$ACOX.addr_a0$215_fetch.111.pre", i64 %indvars.iv376), !llfort.type_idx !43
  %"foo_$ACOX.addr_a0$_fetch.75[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %"foo_$ACOX.dim_info$.lower_bound$[]_fetch.38", i64 4, ptr elementtype(float) %"foo_$ACOX.addr_a0$_fetch.75[]", i64 %indvars.iv380), !llfort.type_idx !43
  %"foo_$ACOX.addr_a0$_fetch.75[][]_fetch.84" = load float, ptr %"foo_$ACOX.addr_a0$_fetch.75[][]", align 4, !tbaa !46, !llfort.type_idx !43
  %"foo_$BCOX.addr_a0$_fetch.85[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %"foo_$BCOX.dim_info$.lower_bound$[]_fetch.57", i64 4, ptr elementtype(float) %"foo_$BCOX.addr_a0$_fetch.85[]", i64 %indvars.iv376), !llfort.type_idx !43
  %"foo_$BCOX.addr_a0$_fetch.85[][]_fetch.94" = load float, ptr %"foo_$BCOX.addr_a0$_fetch.85[][]", align 4, !tbaa !48, !llfort.type_idx !43
  %add.15 = fadd fast float %"foo_$BCOX.addr_a0$_fetch.85[][]_fetch.94", %"foo_$ACOX.addr_a0$_fetch.75[][]_fetch.84"
  %"foo_$RESULT.addr_a0$_fetch.95[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %"foo_$RESULT.dim_info$.lower_bound$[]_fetch.99", i64 %"foo_$RESULT.dim_info$.spacing$[]_fetch.98", ptr elementtype(float) %"foo_$RESULT.addr_a0$_fetch.95", i64 %indvars.iv376), !llfort.type_idx !43
  %"foo_$RESULT.addr_a0$_fetch.95[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %"foo_$RESULT.dim_info$.lower_bound$[]_fetch.96", i64 4, ptr elementtype(float) %"foo_$RESULT.addr_a0$_fetch.95[]", i64 %indvars.iv380), !llfort.type_idx !43
  store float %add.15, ptr %"foo_$RESULT.addr_a0$_fetch.95[][]", align 1, !tbaa !55
  %indvars.iv.next377 = add nuw nsw i64 %indvars.iv376, 1
  %exitcond379 = icmp eq i64 %indvars.iv.next377, %wide.trip.count374
  br i1 %exitcond379, label %do.end_do30.loopexit, label %do.body29

do.end_do30.loopexit:                             ; preds = %do.body29
  %indvars.iv.next381 = add nuw nsw i64 %indvars.iv380, 1
  %exitcond383 = icmp eq i64 %indvars.iv.next381, %wide.trip.count374
  br i1 %exitcond383, label %do.end_do26.loopexit, label %do.body25

do.end_do26.loopexit:                             ; preds = %do.end_do30.loopexit
  br label %do.end_do26

do.end_do26:                                      ; preds = %do.end_do26.loopexit, %alloca_0
  %"foo_$ACOX.flags$217_fetch.112" = load i64, ptr %"var$2_fetch.2.fca.3.gep", align 8, !tbaa !8
  %"foo_$ACOX.flags$217_fetch.112.tr" = trunc i64 %"foo_$ACOX.flags$217_fetch.112" to i32
  %1 = shl i32 %"foo_$ACOX.flags$217_fetch.112.tr", 1
  %or.20 = and i32 %1, 6
  %2 = lshr i32 %"foo_$ACOX.flags$217_fetch.112.tr", 3
  %int_zext223 = and i32 %2, 256
  %3 = lshr i64 %"foo_$ACOX.flags$217_fetch.112", 15
  %4 = trunc i64 %3 to i32
  %5 = and i32 %4, 65011712
  %or.21 = or i32 %int_zext223, %or.20
  %or.24 = or i32 %or.21, %5
  %or.25 = or i32 %or.24, 262144
  %"foo_$ACOX.reserved$231_fetch.113" = load i64, ptr %"var$2_fetch.2.fca.5.gep", align 8, !tbaa !11
  %"(ptr)foo_$ACOX.reserved$231_fetch.113$" = inttoptr i64 %"foo_$ACOX.reserved$231_fetch.113" to ptr, !llfort.type_idx !57
  %func_result233 = tail call i32 @for_dealloc_allocatable_handle(ptr %"foo_$ACOX.addr_a0$215_fetch.111.pre", i32 %or.25, ptr %"(ptr)foo_$ACOX.reserved$231_fetch.113$") #3, !llfort.type_idx !24
  %rel.21 = icmp eq i32 %func_result233, 0
  br i1 %rel.21, label %bb_new36_then, label %bb3_endif

bb_new36_then:                                    ; preds = %do.end_do26
  store ptr null, ptr %"var$2_fetch.2.fca.0.gep", align 8, !tbaa !35
  %and.48 = and i64 %"foo_$ACOX.flags$217_fetch.112", -1030792153090
  store i64 %and.48, ptr %"var$2_fetch.2.fca.3.gep", align 8, !tbaa !8
  br label %bb3_endif

bb3_endif:                                        ; preds = %do.end_do26, %bb_new36_then
  %"foo_$ACOX.addr_a0$273_fetch.125" = phi ptr [ %"foo_$ACOX.addr_a0$215_fetch.111.pre", %do.end_do26 ], [ null, %bb_new36_then ]
  %"foo_$ACOX.flags$271_fetch.124" = phi i64 [ %"foo_$ACOX.flags$217_fetch.112", %do.end_do26 ], [ %and.48, %bb_new36_then ]
  %"foo_$BCOX.addr_a0$244_fetch.118" = load ptr, ptr %"var$2_fetch.1.fca.0.gep", align 8, !tbaa !42
  %"foo_$BCOX.flags$246_fetch.119" = load i64, ptr %"var$2_fetch.1.fca.3.gep", align 8, !tbaa !26
  %"foo_$BCOX.flags$246_fetch.119.tr" = trunc i64 %"foo_$BCOX.flags$246_fetch.119" to i32
  %6 = shl i32 %"foo_$BCOX.flags$246_fetch.119.tr", 1
  %or.28 = and i32 %6, 6
  %7 = lshr i32 %"foo_$BCOX.flags$246_fetch.119.tr", 3
  %int_zext252 = and i32 %7, 256
  %8 = lshr i64 %"foo_$BCOX.flags$246_fetch.119", 15
  %9 = trunc i64 %8 to i32
  %10 = and i32 %9, 65011712
  %or.29 = or i32 %int_zext252, %or.28
  %or.32 = or i32 %or.29, %10
  %or.33 = or i32 %or.32, 262144
  %"foo_$BCOX.reserved$260_fetch.120" = load i64, ptr %"var$2_fetch.1.fca.5.gep", align 8, !tbaa !28
  %"(ptr)foo_$BCOX.reserved$260_fetch.120$" = inttoptr i64 %"foo_$BCOX.reserved$260_fetch.120" to ptr, !llfort.type_idx !57
  %func_result262 = tail call i32 @for_dealloc_allocatable_handle(ptr %"foo_$BCOX.addr_a0$244_fetch.118", i32 %or.33, ptr %"(ptr)foo_$BCOX.reserved$260_fetch.120$") #3, !llfort.type_idx !24
  %rel.22 = icmp eq i32 %func_result262, 0
  br i1 %rel.22, label %bb_new39_then, label %bb6_endif

bb_new39_then:                                    ; preds = %bb3_endif
  store ptr null, ptr %"var$2_fetch.1.fca.0.gep", align 8, !tbaa !42
  %and.64 = and i64 %"foo_$BCOX.flags$246_fetch.119", -1030792153090
  store i64 %and.64, ptr %"var$2_fetch.1.fca.3.gep", align 8, !tbaa !26
  br label %bb6_endif

bb6_endif:                                        ; preds = %bb3_endif, %bb_new39_then
  %"foo_$BCOX.addr_a0$300_fetch.131" = phi ptr [ %"foo_$BCOX.addr_a0$244_fetch.118", %bb3_endif ], [ null, %bb_new39_then ]
  %"foo_$BCOX.flags$298_fetch.130" = phi i64 [ %"foo_$BCOX.flags$246_fetch.119", %bb3_endif ], [ %and.64, %bb_new39_then ]
  %and.65 = and i64 %"foo_$ACOX.flags$271_fetch.124", 1
  %rel.23 = icmp eq i64 %and.65, 0
  br i1 %rel.23, label %dealloc.list.end41, label %dealloc.list.then40

dealloc.list.then40:                              ; preds = %bb6_endif
  %"foo_$ACOX.flags$271_fetch.124.tr" = trunc i64 %"foo_$ACOX.flags$271_fetch.124" to i32
  %and.65.tr = trunc i64 %and.65 to i32
  %11 = and i32 %"foo_$ACOX.flags$271_fetch.124.tr", 2
  %12 = or i32 %11, %and.65.tr
  %or.36 = shl nuw nsw i32 %12, 1
  %13 = lshr i32 %"foo_$ACOX.flags$271_fetch.124.tr", 3
  %int_zext281 = and i32 %13, 256
  %or.37 = or i32 %or.36, %int_zext281
  %14 = lshr i64 %"foo_$ACOX.flags$271_fetch.124", 15
  %15 = trunc i64 %14 to i32
  %16 = and i32 %15, 65011712
  %and.78 = or i32 %or.37, %16
  %or.41 = or i32 %and.78, 262144
  %func_result291 = tail call i32 @for_dealloc_allocatable_handle(ptr %"foo_$ACOX.addr_a0$273_fetch.125", i32 %or.41, ptr %"(ptr)foo_$ACOX.reserved$231_fetch.113$") #3, !llfort.type_idx !24
  %rel.24 = icmp eq i32 %func_result291, 0
  br i1 %rel.24, label %bb_new44_then, label %dealloc.list.end41

bb_new44_then:                                    ; preds = %dealloc.list.then40
  store ptr null, ptr %"var$2_fetch.2.fca.0.gep", align 8, !tbaa !35
  %and.80 = and i64 %"foo_$ACOX.flags$271_fetch.124", -2050
  store i64 %and.80, ptr %"var$2_fetch.2.fca.3.gep", align 8, !tbaa !8
  br label %dealloc.list.end41

dealloc.list.end41:                               ; preds = %bb_new44_then, %dealloc.list.then40, %bb6_endif
  %and.81 = and i64 %"foo_$BCOX.flags$298_fetch.130", 1
  %rel.25 = icmp eq i64 %and.81, 0
  br i1 %rel.25, label %dealloc.list.end46, label %dealloc.list.then45

dealloc.list.then45:                              ; preds = %dealloc.list.end41
  %"foo_$BCOX.flags$298_fetch.130.tr" = trunc i64 %"foo_$BCOX.flags$298_fetch.130" to i32
  %17 = shl i32 %"foo_$BCOX.flags$298_fetch.130.tr", 1
  %int_zext304 = and i32 %17, 4
  %18 = lshr i32 %"foo_$BCOX.flags$298_fetch.130.tr", 3
  %int_zext308 = and i32 %18, 256
  %19 = lshr i64 %"foo_$BCOX.flags$298_fetch.130", 15
  %20 = trunc i64 %19 to i32
  %21 = and i32 %20, 65011712
  %or.43 = or i32 %int_zext308, %int_zext304
  %or.46 = or i32 %or.43, %21
  %or.48 = or i32 %or.46, 262146
  %func_result318 = tail call i32 @for_dealloc_allocatable_handle(ptr %"foo_$BCOX.addr_a0$300_fetch.131", i32 %or.48, ptr %"(ptr)foo_$BCOX.reserved$260_fetch.120$") #3, !llfort.type_idx !24
  %rel.26 = icmp eq i32 %func_result318, 0
  br i1 %rel.26, label %bb_new49_then, label %dealloc.list.end46

bb_new49_then:                                    ; preds = %dealloc.list.then45
  store ptr null, ptr %"var$2_fetch.1.fca.0.gep", align 8, !tbaa !42
  %and.96 = and i64 %"foo_$BCOX.flags$298_fetch.130", -2050
  store i64 %and.96, ptr %"var$2_fetch.1.fca.3.gep", align 8, !tbaa !26
  br label %dealloc.list.end46

dealloc.list.end46:                               ; preds = %bb_new49_then, %dealloc.list.then45, %dealloc.list.end41
  ret void
}

; Function Attrs: mustprogress nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none)
declare ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8, i64, i32, ptr, i32) #1

; Function Attrs: nofree
declare !llfort.intrin_id !58 i32 @for_check_mult_overflow64(ptr nocapture, i32, ...) local_unnamed_addr #2

; Function Attrs: nofree
declare !llfort.intrin_id !59 i32 @for_alloc_allocatable_handle(i64, ptr nocapture, i32, ptr) local_unnamed_addr #2

; Function Attrs: mustprogress nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none)
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #1

; Function Attrs: nofree
declare !llfort.intrin_id !60 i32 @for_dealloc_allocatable_handle(ptr nocapture readonly, i32, ptr) local_unnamed_addr #2

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
!26 = !{!27, !10, i64 24}
!27 = !{!"ifx$descr$2", !10, i64 0, !10, i64 8, !10, i64 16, !10, i64 24, !10, i64 32, !10, i64 40, !10, i64 48, !10, i64 56, !10, i64 64, !10, i64 72, !10, i64 80, !10, i64 88}
!28 = !{!27, !10, i64 40}
!29 = !{!27, !10, i64 8}
!30 = !{!27, !10, i64 32}
!31 = !{!27, !10, i64 16}
!32 = !{!27, !10, i64 64}
!33 = !{!27, !10, i64 48}
!34 = !{!27, !10, i64 56}
!35 = !{!9, !10, i64 0}
!36 = !{i64 56}
!37 = !{!38, !10, i64 0}
!38 = !{!"ifx$descr$3", !10, i64 0, !10, i64 8, !10, i64 16, !10, i64 24, !10, i64 32, !10, i64 40, !10, i64 48, !10, i64 56, !10, i64 64, !10, i64 72, !10, i64 80, !10, i64 88}
!39 = !{!38, !10, i64 64}
!40 = !{!38, !10, i64 56}
!41 = !{i64 1, i64 -9223372036854775808}
!42 = !{!27, !10, i64 0}
!43 = !{i64 5}
!44 = !{!45, !45, i64 0}
!45 = !{!"ifx$unique_sym$4", !17, i64 0}
!46 = !{!47, !47, i64 0}
!47 = !{!"ifx$unique_sym$5", !17, i64 0}
!48 = !{!49, !49, i64 0}
!49 = !{!"ifx$unique_sym$6", !17, i64 0}
!50 = !{i64 78}
!51 = !{!52, !10, i64 0}
!52 = !{!"ifx$descr$4", !10, i64 0, !10, i64 8, !10, i64 16, !10, i64 24, !10, i64 32, !10, i64 40, !10, i64 48, !10, i64 56, !10, i64 64, !10, i64 72, !10, i64 80, !10, i64 88}
!53 = !{!52, !10, i64 64}
!54 = !{!52, !10, i64 56}
!55 = !{!56, !56, i64 0}
!56 = !{!"ifx$unique_sym$7", !17, i64 0}
!57 = !{i64 11}
!58 = !{i32 102}
!59 = !{i32 94}
!60 = !{i32 95}
