; RUN: opt -S -vplan-vec -vplan-force-vf=4 -debug-only=vpo-ir-loop-vectorize-legality -disable-output < %s 2>&1 | FileCheck %s
; FIXME: Remove once support for F90_NONPOD is added
; CHECK: F90 non-POD privates are not supported.
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%dyn_typ = type { i8*, %dyn_typ* }
%"XDESC_2_0$i8*$" = type { %"QNCA_a0$i8*$rank2$", %dyn_typ*, i8*, i64, i8*, i8*, i8*, i8*, i8*, i8*, i8* }
%"QNCA_a0$i8*$rank2$" = type { i8*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }
%"XDESC_0_0$i8*$.4" = type { %"QNCA_a0$i8*$rank0$.3", %dyn_typ*, i8*, i64, i8*, i8*, i8*, i8*, i8*, i8*, i8* }
%"QNCA_a0$i8*$rank0$.3" = type { i8*, i64, i64, i64, i64, i64 }
%"XDESC_0_0$i8*$.6" = type { %"QNCA_a0$i8*$rank0$.5", %dyn_typ*, i8*, i64, i8*, i8*, i8*, i8*, i8*, i8*, i8* }
%"QNCA_a0$i8*$rank0$.5" = type { i8*, i64, i64, i64, i64, i64 }
%"XDESC_0_0$i8*$.8" = type { %"QNCA_a0$i8*$rank0$.7", %dyn_typ*, i8*, i64, i8*, i8*, i8*, i8*, i8*, i8*, i8* }
%"QNCA_a0$i8*$rank0$.7" = type { i8*, i64, i64, i64, i64, i64 }
%"WORK$.btINNER_T" = type <{ i32, [4 x i8], %"QNCA_a0$i32*$rank2$" }>
%"QNCA_a0$i32*$rank2$" = type { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }

@_DYNTYPE_RECORD_0 = external hidden global %dyn_typ
@"var$5" = external hidden global %"XDESC_2_0$i8*$"
@_DYNTYPE_RECORD_1 = external hidden global %dyn_typ
@_INFO_LIST_VAR_0 = external hidden global [4 x i8*], align 8
@"var$8" = external hidden global %"XDESC_0_0$i8*$.4"
@_ALLOC_RECORD_LIST_VAR_2 = external hidden global [4 x i8*], align 8
@"var$9" = external hidden global %"XDESC_0_0$i8*$.6"
@_ALLOC_RECORD_LIST_VAR_3 = external hidden global [4 x i8*], align 8
@"var$10" = external hidden global %"XDESC_0_0$i8*$.8"
@_ALLOC_RECORD_LIST_VAR_4 = external hidden global [4 x i8*], align 8

define void @work_(i32* noalias nocapture dereferenceable(4) %"work_$A", i32* noalias nocapture dereferenceable(4) %"work_$B", i32* noalias nocapture dereferenceable(4) %"work_$N", %"WORK$.btINNER_T"** noalias dereferenceable(104) %"work_$COLLECTION") {
DIR.OMP.SIMD.1:
  %"work_$COLLECTION.lpriv" = alloca %"WORK$.btINNER_T"*, align 8
  call void @"%WORK$.btINNER_T.omp.mold_ctor_deref"(%"WORK$.btINNER_T"** nonnull %"work_$COLLECTION.lpriv", %"WORK$.btINNER_T"** nonnull %"work_$COLLECTION")
  %"work_$I.linear.iv" = alloca i32, align 8
  store i64 1248, i64* getelementptr inbounds (%"XDESC_2_0$i8*$", %"XDESC_2_0$i8*$"* @"var$5", i64 0, i32 0, i32 3), align 8
  store i64 2, i64* getelementptr inbounds (%"XDESC_2_0$i8*$", %"XDESC_2_0$i8*$"* @"var$5", i64 0, i32 0, i32 4), align 16
  store i64 0, i64* getelementptr inbounds (%"XDESC_2_0$i8*$", %"XDESC_2_0$i8*$"* @"var$5", i64 0, i32 0, i32 2), align 16
  store %dyn_typ* @_DYNTYPE_RECORD_1, %dyn_typ** getelementptr inbounds (%"XDESC_2_0$i8*$", %"XDESC_2_0$i8*$"* @"var$5", i64 0, i32 1), align 16
  store i8* null, i8** getelementptr inbounds (%"XDESC_2_0$i8*$", %"XDESC_2_0$i8*$"* @"var$5", i64 0, i32 2), align 8
  call void @llvm.memset.p0i8.i64(i8* noundef nonnull align 8 dereferenceable(48) bitcast (i8** getelementptr inbounds (%"XDESC_2_0$i8*$", %"XDESC_2_0$i8*$"* @"var$5", i64 0, i32 4) to i8*), i8 0, i64 48, i1 false)
  %"work_$N_fetch.3" = load i32, i32* %"work_$N", align 1
  %rel.1.not38 = icmp slt i32 %"work_$N_fetch.3", 1
  br i1 %rel.1.not38, label %DIR.OMP.END.SIMD.437, label %DIR.OMP.SIMD.148

omp.pdo.body5:                                    ; preds = %DIR.OMP.SIMD.150, %omp.pdo.body5
  %add.445 = phi i32 [ %"work_$COLLECTION_fetch.29.INNER_ID$.promoted", %DIR.OMP.SIMD.150 ], [ %add.4, %omp.pdo.body5 ]
  %omp.pdo.norm.iv.local.039 = phi i64 [ 0, %DIR.OMP.SIMD.150 ], [ %add.5, %omp.pdo.body5 ]
  %int_sext = trunc i64 %omp.pdo.norm.iv.local.039 to i32
  %add.1 = add nsw i32 %int_sext, 1
  %int_sext1 = sext i32 %add.1 to i64
  %"work_$A[]" = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull elementtype(i32) %"work_$A", i64 %int_sext1)
  %"work_$A[]_fetch.14" = load i32, i32* %"work_$A[]", align 1
  %"work_$B[]" = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull elementtype(i32) %"work_$B", i64 %int_sext1)
  %"work_$B[]_fetch.16" = load i32, i32* %"work_$B[]", align 1
  %add.2 = add nsw i32 %"work_$B[]_fetch.16", %"work_$A[]_fetch.14"
  %"work_$COLLECTION_fetch.17.INNER_STUFF$.addr_a0$_fetch.18[]" = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 %"work_$COLLECTION_fetch.17.INNER_STUFF$.dim_info$.lower_bound$[]_fetch.22", i64 %"work_$COLLECTION_fetch.17.INNER_STUFF$.dim_info$.spacing$[]_fetch.21", i32* elementtype(i32) %"work_$COLLECTION_fetch.17.INNER_STUFF$.addr_a0$_fetch.18", i64 %int_sext1)
  %"work_$COLLECTION_fetch.17.INNER_STUFF$.addr_a0$_fetch.18[][]" = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 %"work_$COLLECTION_fetch.17.INNER_STUFF$.dim_info$.lower_bound$[]_fetch.19", i64 4, i32* elementtype(i32) %"work_$COLLECTION_fetch.17.INNER_STUFF$.addr_a0$_fetch.18[]", i64 %int_sext1)
  store i32 %add.2, i32* %"work_$COLLECTION_fetch.17.INNER_STUFF$.addr_a0$_fetch.18[][]", align 1
  %add.4 = add nsw i32 %add.445, %"work_$A[]_fetch.14"
  %add.5 = add nuw nsw i64 %omp.pdo.norm.iv.local.039, 1
  %exitcond.not = icmp eq i64 %add.5, %1
  br i1 %exitcond.not, label %DIR.OMP.END.SIMD.249, label %omp.pdo.body5

DIR.OMP.SIMD.148:                                 ; preds = %DIR.OMP.SIMD.1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LINEAR:IV"(i32* %"work_$I.linear.iv", i32 1), "QUAL.OMP.LASTPRIVATE:F90_NONPOD"(%"WORK$.btINNER_T"** %"work_$COLLECTION.lpriv", void (%"WORK$.btINNER_T"**, %"WORK$.btINNER_T"**)* @"%WORK$.btINNER_T.omp.mold_ctor_deref", void (%"WORK$.btINNER_T"**, %"WORK$.btINNER_T"**)* @"%WORK$.btINNER_T.omp.copy_assign_deref", void (%"WORK$.btINNER_T"**)* @"%WORK$.btINNER_T.omp.dtor_deref"), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null), "QUAL.OMP.LIVEIN"(i32* %"work_$B"), "QUAL.OMP.LIVEIN"(i32* %"work_$A"), "QUAL.OMP.LIVEIN"(i32* %"work_$N") ]
  br label %DIR.OMP.SIMD.150

DIR.OMP.SIMD.150:                                 ; preds = %DIR.OMP.SIMD.148
  %"work_$COLLECTION_fetch.17" = load %"WORK$.btINNER_T"*, %"WORK$.btINNER_T"** %"work_$COLLECTION.lpriv", align 8
  %"work_$COLLECTION_fetch.17.INNER_STUFF$.addr_a0$" = getelementptr inbounds %"WORK$.btINNER_T", %"WORK$.btINNER_T"* %"work_$COLLECTION_fetch.17", i64 0, i32 2, i32 0
  %"work_$COLLECTION_fetch.17.INNER_STUFF$.addr_a0$_fetch.18" = load i32*, i32** %"work_$COLLECTION_fetch.17.INNER_STUFF$.addr_a0$", align 1
  %"work_$COLLECTION_fetch.17.INNER_STUFF$.dim_info$.lower_bound$" = getelementptr inbounds %"WORK$.btINNER_T", %"WORK$.btINNER_T"* %"work_$COLLECTION_fetch.17", i64 0, i32 2, i32 6, i64 0, i32 2
  %"work_$COLLECTION_fetch.17.INNER_STUFF$.dim_info$.lower_bound$[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %"work_$COLLECTION_fetch.17.INNER_STUFF$.dim_info$.lower_bound$", i32 0)
  %"work_$COLLECTION_fetch.17.INNER_STUFF$.dim_info$.lower_bound$[]_fetch.19" = load i64, i64* %"work_$COLLECTION_fetch.17.INNER_STUFF$.dim_info$.lower_bound$[]", align 1
  %"work_$COLLECTION_fetch.17.INNER_STUFF$.dim_info$.spacing$" = getelementptr inbounds %"WORK$.btINNER_T", %"WORK$.btINNER_T"* %"work_$COLLECTION_fetch.17", i64 0, i32 2, i32 6, i64 0, i32 1
  %"work_$COLLECTION_fetch.17.INNER_STUFF$.dim_info$.spacing$[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %"work_$COLLECTION_fetch.17.INNER_STUFF$.dim_info$.spacing$", i32 1)
  %"work_$COLLECTION_fetch.17.INNER_STUFF$.dim_info$.spacing$[]_fetch.21" = load i64, i64* %"work_$COLLECTION_fetch.17.INNER_STUFF$.dim_info$.spacing$[]", align 1
  %"work_$COLLECTION_fetch.17.INNER_STUFF$.dim_info$.lower_bound$[]11" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %"work_$COLLECTION_fetch.17.INNER_STUFF$.dim_info$.lower_bound$", i32 1)
  %"work_$COLLECTION_fetch.17.INNER_STUFF$.dim_info$.lower_bound$[]_fetch.22" = load i64, i64* %"work_$COLLECTION_fetch.17.INNER_STUFF$.dim_info$.lower_bound$[]11", align 1
  %"work_$COLLECTION_fetch.29.INNER_ID$" = getelementptr inbounds %"WORK$.btINNER_T", %"WORK$.btINNER_T"* %"work_$COLLECTION_fetch.17", i64 0, i32 0
  %1 = zext i32 %"work_$N_fetch.3" to i64
  %"work_$COLLECTION_fetch.29.INNER_ID$.promoted" = load i32, i32* %"work_$COLLECTION_fetch.29.INNER_ID$", align 1
  br label %omp.pdo.body5

DIR.OMP.END.SIMD.249:                             ; preds = %omp.pdo.body5
  %add.4.lcssa = phi i32 [ %add.4, %omp.pdo.body5 ]
  store i32 %"work_$N_fetch.3", i32* %"work_$I.linear.iv", align 8
  store i32 %add.4.lcssa, i32* %"work_$COLLECTION_fetch.29.INNER_ID$", align 1
  br label %DIR.OMP.END.SIMD.2

DIR.OMP.END.SIMD.2:                               ; preds = %DIR.OMP.END.SIMD.249
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:                               ; preds = %DIR.OMP.END.SIMD.2
  %src_fetch.39.i = load %"WORK$.btINNER_T"*, %"WORK$.btINNER_T"** %"work_$COLLECTION.lpriv", align 8
  %rel.3.not.i = icmp eq %"WORK$.btINNER_T"* %src_fetch.39.i, null
  br i1 %rel.3.not.i, label %omp.pdo.cond4.DIR.OMP.END.SIMD.4.loopexit_crit_edge.split.thread, label %omp.pdo.cond4.DIR.OMP.END.SIMD.4.loopexit_crit_edge.split

omp.pdo.cond4.DIR.OMP.END.SIMD.4.loopexit_crit_edge.split.thread: ; preds = %DIR.OMP.END.SIMD.3
  store %"WORK$.btINNER_T"* null, %"WORK$.btINNER_T"** %"work_$COLLECTION", align 1
  br label %DIR.OMP.END.SIMD.437

omp.pdo.cond4.DIR.OMP.END.SIMD.4.loopexit_crit_edge.split: ; preds = %DIR.OMP.END.SIMD.3
  %2 = bitcast %"WORK$.btINNER_T"** %"work_$COLLECTION" to i8**
  %dst_fetch.401.i = load i8*, i8** %2, align 1
  store i64 104, i64* getelementptr inbounds (%"XDESC_0_0$i8*$.4", %"XDESC_0_0$i8*$.4"* @"var$8", i64 0, i32 0, i32 1), align 8
  store i64 0, i64* getelementptr inbounds (%"XDESC_0_0$i8*$.4", %"XDESC_0_0$i8*$.4"* @"var$8", i64 0, i32 0, i32 4), align 16
  store i8* null, i8** getelementptr inbounds (%"XDESC_0_0$i8*$.4", %"XDESC_0_0$i8*$.4"* @"var$8", i64 0, i32 0, i32 0), align 16
  store i64 1091, i64* getelementptr inbounds (%"XDESC_0_0$i8*$.4", %"XDESC_0_0$i8*$.4"* @"var$8", i64 0, i32 0, i32 3), align 8
  store i64 0, i64* getelementptr inbounds (%"XDESC_0_0$i8*$.4", %"XDESC_0_0$i8*$.4"* @"var$8", i64 0, i32 0, i32 2), align 16
  store %dyn_typ* @_DYNTYPE_RECORD_0, %dyn_typ** getelementptr inbounds (%"XDESC_0_0$i8*$.4", %"XDESC_0_0$i8*$.4"* @"var$8", i64 0, i32 1), align 16
  store i8* null, i8** getelementptr inbounds (%"XDESC_0_0$i8*$.4", %"XDESC_0_0$i8*$.4"* @"var$8", i64 0, i32 2), align 8
  call void @llvm.memset.p0i8.i64(i8* noundef nonnull align 16 dereferenceable(24) bitcast (i8** getelementptr inbounds (%"XDESC_0_0$i8*$.4", %"XDESC_0_0$i8*$.4"* @"var$8", i64 0, i32 5) to i8*), i8 0, i64 24, i1 false)
  store i8* bitcast ([4 x i8*]* @_ALLOC_RECORD_LIST_VAR_2 to i8*), i8** getelementptr inbounds (%"XDESC_0_0$i8*$.4", %"XDESC_0_0$i8*$.4"* @"var$8", i64 0, i32 4), align 8
  store i8* bitcast ([4 x i8*]* @_INFO_LIST_VAR_0 to i8*), i8** getelementptr inbounds (%"XDESC_0_0$i8*$.4", %"XDESC_0_0$i8*$.4"* @"var$8", i64 0, i32 8), align 8
  store i8* null, i8** getelementptr inbounds (%"XDESC_0_0$i8*$.4", %"XDESC_0_0$i8*$.4"* @"var$8", i64 0, i32 9), align 16
  store i64 104, i64* getelementptr inbounds (%"XDESC_0_0$i8*$.6", %"XDESC_0_0$i8*$.6"* @"var$9", i64 0, i32 0, i32 1), align 8
  store i64 0, i64* getelementptr inbounds (%"XDESC_0_0$i8*$.6", %"XDESC_0_0$i8*$.6"* @"var$9", i64 0, i32 0, i32 4), align 16
  store i8* null, i8** getelementptr inbounds (%"XDESC_0_0$i8*$.6", %"XDESC_0_0$i8*$.6"* @"var$9", i64 0, i32 0, i32 0), align 16
  store i64 1091, i64* getelementptr inbounds (%"XDESC_0_0$i8*$.6", %"XDESC_0_0$i8*$.6"* @"var$9", i64 0, i32 0, i32 3), align 8
  store i64 0, i64* getelementptr inbounds (%"XDESC_0_0$i8*$.6", %"XDESC_0_0$i8*$.6"* @"var$9", i64 0, i32 0, i32 2), align 16
  store %dyn_typ* @_DYNTYPE_RECORD_0, %dyn_typ** getelementptr inbounds (%"XDESC_0_0$i8*$.6", %"XDESC_0_0$i8*$.6"* @"var$9", i64 0, i32 1), align 16
  store i8* null, i8** getelementptr inbounds (%"XDESC_0_0$i8*$.6", %"XDESC_0_0$i8*$.6"* @"var$9", i64 0, i32 2), align 8
  call void @llvm.memset.p0i8.i64(i8* noundef nonnull align 16 dereferenceable(24) bitcast (i8** getelementptr inbounds (%"XDESC_0_0$i8*$.6", %"XDESC_0_0$i8*$.6"* @"var$9", i64 0, i32 5) to i8*), i8 0, i64 24, i1 false)
  store i8* bitcast ([4 x i8*]* @_ALLOC_RECORD_LIST_VAR_3 to i8*), i8** getelementptr inbounds (%"XDESC_0_0$i8*$.6", %"XDESC_0_0$i8*$.6"* @"var$9", i64 0, i32 4), align 8
  store i8* bitcast ([4 x i8*]* @_INFO_LIST_VAR_0 to i8*), i8** getelementptr inbounds (%"XDESC_0_0$i8*$.6", %"XDESC_0_0$i8*$.6"* @"var$9", i64 0, i32 8), align 8
  store i8* null, i8** getelementptr inbounds (%"XDESC_0_0$i8*$.6", %"XDESC_0_0$i8*$.6"* @"var$9", i64 0, i32 9), align 16
  %"(i8*)src$.i.i" = bitcast %"WORK$.btINNER_T"* %src_fetch.39.i to i8*
  %func_result.i.i = call i32 @for_alloc_assign_v2(i8* bitcast (%"XDESC_0_0$i8*$.6"* @"var$9" to i8*), i8* nonnull %"(i8*)src$.i.i", i8* bitcast (%"XDESC_0_0$i8*$.4"* @"var$8" to i8*), i8* %dst_fetch.401.i, i32 0)
  %old_fetch.42.i.pr = load %"WORK$.btINNER_T"*, %"WORK$.btINNER_T"** %"work_$COLLECTION.lpriv", align 8
  %rel.4.not.i = icmp eq %"WORK$.btINNER_T"* %old_fetch.42.i.pr, null
  br i1 %rel.4.not.i, label %DIR.OMP.END.SIMD.437, label %old_allocated38.i

old_allocated38.i:                                ; preds = %omp.pdo.cond4.DIR.OMP.END.SIMD.4.loopexit_crit_edge.split
  store i64 104, i64* getelementptr inbounds (%"XDESC_0_0$i8*$.8", %"XDESC_0_0$i8*$.8"* @"var$10", i64 0, i32 0, i32 1), align 8
  store i64 0, i64* getelementptr inbounds (%"XDESC_0_0$i8*$.8", %"XDESC_0_0$i8*$.8"* @"var$10", i64 0, i32 0, i32 4), align 16
  store i8* null, i8** getelementptr inbounds (%"XDESC_0_0$i8*$.8", %"XDESC_0_0$i8*$.8"* @"var$10", i64 0, i32 0, i32 0), align 16
  store i64 1091, i64* getelementptr inbounds (%"XDESC_0_0$i8*$.8", %"XDESC_0_0$i8*$.8"* @"var$10", i64 0, i32 0, i32 3), align 8
  store i64 0, i64* getelementptr inbounds (%"XDESC_0_0$i8*$.8", %"XDESC_0_0$i8*$.8"* @"var$10", i64 0, i32 0, i32 2), align 16
  store %dyn_typ* @_DYNTYPE_RECORD_0, %dyn_typ** getelementptr inbounds (%"XDESC_0_0$i8*$.8", %"XDESC_0_0$i8*$.8"* @"var$10", i64 0, i32 1), align 16
  store i8* null, i8** getelementptr inbounds (%"XDESC_0_0$i8*$.8", %"XDESC_0_0$i8*$.8"* @"var$10", i64 0, i32 2), align 8
  call void @llvm.memset.p0i8.i64(i8* noundef nonnull align 16 dereferenceable(24) bitcast (i8** getelementptr inbounds (%"XDESC_0_0$i8*$.8", %"XDESC_0_0$i8*$.8"* @"var$10", i64 0, i32 5) to i8*), i8 0, i64 24, i1 false)
  store i8* bitcast ([4 x i8*]* @_ALLOC_RECORD_LIST_VAR_4 to i8*), i8** getelementptr inbounds (%"XDESC_0_0$i8*$.8", %"XDESC_0_0$i8*$.8"* @"var$10", i64 0, i32 4), align 8
  store i8* bitcast ([4 x i8*]* @_INFO_LIST_VAR_0 to i8*), i8** getelementptr inbounds (%"XDESC_0_0$i8*$.8", %"XDESC_0_0$i8*$.8"* @"var$10", i64 0, i32 8), align 8
  store i8* null, i8** getelementptr inbounds (%"XDESC_0_0$i8*$.8", %"XDESC_0_0$i8*$.8"* @"var$10", i64 0, i32 9), align 16
  %"(i8*)old$.i.i" = bitcast %"WORK$.btINNER_T"* %old_fetch.42.i.pr to i8*
  %func_result.i.i41 = call i32 @for_dealloc_all_nocheck(i8* bitcast (%"XDESC_0_0$i8*$.8"* @"var$10" to i8*), i8* nonnull %"(i8*)old$.i.i", i32 262144)
  %func_result.i = call i32 @for_deallocate(i8* nonnull %"(i8*)old$.i.i", i32 262144)
  br label %DIR.OMP.END.SIMD.437

DIR.OMP.END.SIMD.437:                             ; preds = %old_allocated38.i, %omp.pdo.cond4.DIR.OMP.END.SIMD.4.loopexit_crit_edge.split, %omp.pdo.cond4.DIR.OMP.END.SIMD.4.loopexit_crit_edge.split.thread, %DIR.OMP.SIMD.1
  ret void
}

declare void @"%WORK$.btINNER_T.omp.mold_ctor_deref"(%"WORK$.btINNER_T"**, %"WORK$.btINNER_T"**)

declare void @"%WORK$.btINNER_T.omp.copy_assign_deref"(%"WORK$.btINNER_T"**, %"WORK$.btINNER_T"**)

declare void @"%WORK$.btINNER_T.omp.dtor_deref"(%"WORK$.btINNER_T"**)

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind readnone speculatable
declare i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8, i64, i64, i32*, i64)

; Function Attrs: nounwind readnone speculatable
declare i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8, i64, i32, i64*, i32)

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

declare i32 @for_alloc_assign_v2(i8* nocapture, i8* nocapture, i8*, i8* nocapture, i32)

declare i32 @for_dealloc_all_nocheck(i8* nocapture, i8* nocapture, i32)

declare i32 @for_deallocate(i8* nocapture readonly, i32)

; Function Attrs: argmemonly nofree nounwind willreturn writeonly
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg)
