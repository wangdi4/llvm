; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -instcombine -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt-shared-privatization -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,instcombine,vpo-restore-operands,vpo-cfg-restructuring,vpo-paropt-shared-privatization)' -S %s 2>&1 | FileCheck %s
;
; Test src:
;
; subroutine foo()
;   real(8), allocatable :: enl(:,:)
;   allocate(enl(10,10))
; !$omp parallel
; !$omp do private(enl)
;   do icol = 1, 2
;     enl = 0
;   end do
; !$omp end do
; !$omp end parallel
;   deallocate(enl)
; end
;
; Check that shared privatization pass does not turn shared(enl) on
;'omp parallel' into private.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$i8*$rank2$" = type { i8*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }
%"QNCA_a0$double*$rank2$" = type { double*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }

@"var$2" = internal global %"QNCA_a0$i8*$rank2$" { i8* null, i64 0, i64 0, i64 128, i64 2, i64 0, [2 x { i64, i64, i64 }] zeroinitializer }

; Function Attrs: nounwind uwtable
define void @foo_() #0 {
alloca_0:
  %"$io_ctx" = alloca [8 x i64], align 16
  %"foo_$ICOL" = alloca i32, align 8
  %"foo_$ENL" = alloca %"QNCA_a0$double*$rank2$", align 8
  %"var$3" = alloca i64, align 8
  %"$loop_ctr" = alloca i64, align 8
  %"$loop_ctr31" = alloca i64, align 8
  %"var$6" = alloca i64, align 8
  %"var$7" = alloca i64, align 8
  %"var$8" = alloca i64, align 8
  %fetch.1 = load %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* bitcast (%"QNCA_a0$i8*$rank2$"* @"var$2" to %"QNCA_a0$double*$rank2$"*), align 1, !tbaa !1
  store %"QNCA_a0$double*$rank2$" %fetch.1, %"QNCA_a0$double*$rank2$"* %"foo_$ENL", align 1, !tbaa !1
  %"foo_$ENL.flags$" = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %"foo_$ENL", i32 0, i32 3
  %"foo_$ENL.flags$_fetch.2" = load i64, i64* %"foo_$ENL.flags$", align 1, !tbaa !4
  %and.1 = and i64 %"foo_$ENL.flags$_fetch.2", 256
  %lshr.1 = lshr i64 %and.1, 8
  %shl.1 = shl i64 %lshr.1, 8
  %or.1 = or i64 133, %shl.1
  %and.3 = and i64 %"foo_$ENL.flags$_fetch.2", 1030792151040
  %lshr.2 = lshr i64 %and.3, 36
  %and.4 = and i64 %or.1, -1030792151041
  %shl.2 = shl i64 %lshr.2, 36
  %or.2 = or i64 %and.4, %shl.2
  %"foo_$ENL.flags$7" = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %"foo_$ENL", i32 0, i32 3
  store i64 %or.2, i64* %"foo_$ENL.flags$7", align 1, !tbaa !4
  %"foo_$ENL.reserved$8" = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %"foo_$ENL", i32 0, i32 5
  store i64 0, i64* %"foo_$ENL.reserved$8", align 1, !tbaa !7
  %"foo_$ENL.addr_length$" = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %"foo_$ENL", i32 0, i32 1
  store i64 8, i64* %"foo_$ENL.addr_length$", align 1, !tbaa !8
  %"foo_$ENL.dim$" = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %"foo_$ENL", i32 0, i32 4
  store i64 2, i64* %"foo_$ENL.dim$", align 1, !tbaa !9
  %"foo_$ENL.codim$" = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %"foo_$ENL", i32 0, i32 2
  store i64 0, i64* %"foo_$ENL.codim$", align 1, !tbaa !10
  %"foo_$ENL.dim_info$9" = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %"foo_$ENL", i32 0, i32 6, i32 0
  %"foo_$ENL.dim_info$.lower_bound$10" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"foo_$ENL.dim_info$9", i32 0, i32 2
  %"foo_$ENL.dim_info$.lower_bound$[]11" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"foo_$ENL.dim_info$.lower_bound$10", i32 0)
  store i64 1, i64* %"foo_$ENL.dim_info$.lower_bound$[]11", align 1, !tbaa !11
  %"foo_$ENL.dim_info$12" = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %"foo_$ENL", i32 0, i32 6, i32 0
  %"foo_$ENL.dim_info$.extent$" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"foo_$ENL.dim_info$12", i32 0, i32 0
  %"foo_$ENL.dim_info$.extent$[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"foo_$ENL.dim_info$.extent$", i32 0)
  store i64 10, i64* %"foo_$ENL.dim_info$.extent$[]", align 1, !tbaa !12
  %"foo_$ENL.dim_info$13" = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %"foo_$ENL", i32 0, i32 6, i32 0
  %"foo_$ENL.dim_info$.lower_bound$14" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"foo_$ENL.dim_info$13", i32 0, i32 2
  %"foo_$ENL.dim_info$.lower_bound$[]15" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"foo_$ENL.dim_info$.lower_bound$14", i32 1)
  store i64 1, i64* %"foo_$ENL.dim_info$.lower_bound$[]15", align 1, !tbaa !11
  %"foo_$ENL.dim_info$16" = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %"foo_$ENL", i32 0, i32 6, i32 0
  %"foo_$ENL.dim_info$.extent$17" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"foo_$ENL.dim_info$16", i32 0, i32 0
  %"foo_$ENL.dim_info$.extent$[]18" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"foo_$ENL.dim_info$.extent$17", i32 1)
  store i64 10, i64* %"foo_$ENL.dim_info$.extent$[]18", align 1, !tbaa !12
  %"foo_$ENL.dim_info$19" = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %"foo_$ENL", i32 0, i32 6, i32 0
  %"foo_$ENL.dim_info$.spacing$20" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"foo_$ENL.dim_info$19", i32 0, i32 1
  %"foo_$ENL.dim_info$.spacing$[]21" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"foo_$ENL.dim_info$.spacing$20", i32 0)
  store i64 8, i64* %"foo_$ENL.dim_info$.spacing$[]21", align 1, !tbaa !13
  %"foo_$ENL.dim_info$22" = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %"foo_$ENL", i32 0, i32 6, i32 0
  %"foo_$ENL.dim_info$.spacing$23" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"foo_$ENL.dim_info$22", i32 0, i32 1
  %"foo_$ENL.dim_info$.spacing$[]24" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"foo_$ENL.dim_info$.spacing$23", i32 1)
  store i64 80, i64* %"foo_$ENL.dim_info$.spacing$[]24", align 1, !tbaa !13
  %"foo_$ENL.addr_a0$" = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %"foo_$ENL", i32 0, i32 0
  %"foo_$ENL.flags$25" = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %"foo_$ENL", i32 0, i32 3
  %"foo_$ENL.flags$_fetch.3" = load i64, i64* %"foo_$ENL.flags$25", align 1, !tbaa !4
  %"foo_$ENL.flags$26" = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %"foo_$ENL", i32 0, i32 3
  %"foo_$ENL.flags$_fetch.4" = load i64, i64* %"foo_$ENL.flags$26", align 1, !tbaa !4
  %and.5 = and i64 %"foo_$ENL.flags$_fetch.4", -68451041281
  %or.3 = or i64 %and.5, 1073741824
  %"foo_$ENL.flags$27" = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %"foo_$ENL", i32 0, i32 3
  store i64 %or.3, i64* %"foo_$ENL.flags$27", align 1, !tbaa !4
  %and.6 = and i64 %"foo_$ENL.flags$_fetch.3", 1
  %shl.4 = shl i64 %and.6, 1
  %int_zext = trunc i64 %shl.4 to i32
  %or.4 = or i32 0, %int_zext
  %and.9 = and i32 %or.4, -17
  %or.5 = or i32 %and.9, 0
  %and.10 = and i64 %"foo_$ENL.flags$_fetch.3", 256
  %lshr.3 = lshr i64 %and.10, 8
  %and.11 = and i32 %or.5, -2097153
  %shl.5 = shl i64 %lshr.3, 21
  %int_zext28 = trunc i64 %shl.5 to i32
  %or.6 = or i32 %and.11, %int_zext28
  %and.12 = and i64 %"foo_$ENL.flags$_fetch.3", 1030792151040
  %lshr.4 = lshr i64 %and.12, 36
  %and.13 = and i32 %or.6, -31457281
  %shl.6 = shl i64 %lshr.4, 21
  %int_zext29 = trunc i64 %shl.6 to i32
  %or.7 = or i32 %and.13, %int_zext29
  %and.14 = and i64 %"foo_$ENL.flags$_fetch.3", 1099511627776
  %lshr.5 = lshr i64 %and.14, 40
  %and.15 = and i32 %or.7, -33554433
  %shl.7 = shl i64 %lshr.5, 25
  %int_zext30 = trunc i64 %shl.7 to i32
  %or.8 = or i32 %and.15, %int_zext30
  %and.16 = and i32 %or.8, -2031617
  %or.9 = or i32 %and.16, 262144
  %"foo_$ENL.reserved$" = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %"foo_$ENL", i32 0, i32 5
  %"foo_$ENL.reserved$_fetch.5" = load i64, i64* %"foo_$ENL.reserved$", align 1, !tbaa !7
  %"(i8*)foo_$ENL.reserved$_fetch.5$" = inttoptr i64 %"foo_$ENL.reserved$_fetch.5" to i8*
  %"(i8**)foo_$ENL.addr_a0$$" = bitcast double** %"foo_$ENL.addr_a0$" to i8**
  %func_result = call i32 @for_alloc_allocatable_handle(i64 800, i8** %"(i8**)foo_$ENL.addr_a0$$", i32 %or.9, i8* %"(i8*)foo_$ENL.reserved$_fetch.5$")
  %"foo_$ENL.dim_info$" = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %"foo_$ENL", i32 0, i32 6, i32 0
  %"foo_$ENL.dim_info$.lower_bound$" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"foo_$ENL.dim_info$", i32 0, i32 2
  %"foo_$ENL.dim_info$.lower_bound$[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"foo_$ENL.dim_info$.lower_bound$", i32 0)
  %"foo_$ENL.dim_info$.lower_bound$[]_fetch.6" = load i64, i64* %"foo_$ENL.dim_info$.lower_bound$[]", align 1, !tbaa !11
  %mul.2 = mul nsw i64 %"foo_$ENL.dim_info$.lower_bound$[]_fetch.6", 8
  %"foo_$ENL.dim_info$2" = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %"foo_$ENL", i32 0, i32 6, i32 0
  %"foo_$ENL.dim_info$.lower_bound$4" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"foo_$ENL.dim_info$2", i32 0, i32 2
  %"foo_$ENL.dim_info$.lower_bound$4[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"foo_$ENL.dim_info$.lower_bound$4", i32 1)
  %"foo_$ENL.dim_info$.lower_bound$4[]_fetch.7" = load i64, i64* %"foo_$ENL.dim_info$.lower_bound$4[]", align 1, !tbaa !11
  %"foo_$ENL.dim_info$6" = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %"foo_$ENL", i32 0, i32 6, i32 0
  %"foo_$ENL.dim_info$.spacing$" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"foo_$ENL.dim_info$6", i32 0, i32 1
  %"foo_$ENL.dim_info$.spacing$[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"foo_$ENL.dim_info$.spacing$", i32 1)
  %"foo_$ENL.dim_info$.spacing$[]_fetch.8" = load i64, i64* %"foo_$ENL.dim_info$.spacing$[]", align 1, !tbaa !13, !range !14
  %mul.3 = mul nsw i64 %"foo_$ENL.dim_info$.lower_bound$4[]_fetch.7", %"foo_$ENL.dim_info$.spacing$[]_fetch.8"
  %add.1 = add nsw i64 %mul.2, %mul.3
  br label %bb_new7

omp.pdo.cond9:                                    ; preds = %bb_new12, %loop_exit21
  %omp.pdo.norm.iv_fetch.13 = load i64, i64* %omp.pdo.norm.iv, align 1, !tbaa !1
  %omp.pdo.norm.ub_fetch.14 = load i64, i64* %omp.pdo.norm.ub, align 1, !tbaa !1
  %rel.1 = icmp sle i64 %omp.pdo.norm.iv_fetch.13, %omp.pdo.norm.ub_fetch.14
  br i1 %rel.1, label %omp.pdo.body10, label %omp.pdo.epilog11

omp.pdo.body10:                                   ; preds = %omp.pdo.cond9
  %omp.pdo.norm.iv_fetch.15 = load i64, i64* %omp.pdo.norm.iv, align 1, !tbaa !1
  %int_sext = trunc i64 %omp.pdo.norm.iv_fetch.15 to i32
  %omp.pdo.step_fetch.16 = load i32, i32* %omp.pdo.step, align 1, !tbaa !1
  %mul.4 = mul nsw i32 %int_sext, %omp.pdo.step_fetch.16
  %omp.pdo.start_fetch.17 = load i32, i32* %omp.pdo.start, align 1, !tbaa !1
  %add.2 = add nsw i32 %mul.4, %omp.pdo.start_fetch.17
  store i32 %add.2, i32* %"foo_$ICOL", align 1, !tbaa !15
  %"foo_$ENL.addr_a0$32" = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %"foo_$ENL", i32 0, i32 0
  %"foo_$ENL.addr_a0$_fetch.18" = load double*, double** %"foo_$ENL.addr_a0$32", align 1, !tbaa !17
  %"foo_$ENL.dim_info$33" = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %"foo_$ENL", i32 0, i32 6, i32 0
  %"foo_$ENL.dim_info$.lower_bound$34" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"foo_$ENL.dim_info$33", i32 0, i32 2
  %"foo_$ENL.dim_info$.lower_bound$[]35" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"foo_$ENL.dim_info$.lower_bound$34", i32 0)
  %"foo_$ENL.dim_info$.lower_bound$[]_fetch.19" = load i64, i64* %"foo_$ENL.dim_info$.lower_bound$[]35", align 1, !tbaa !11
  %"foo_$ENL.dim_info$36" = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %"foo_$ENL", i32 0, i32 6, i32 0
  %"foo_$ENL.dim_info$.lower_bound$37" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"foo_$ENL.dim_info$36", i32 0, i32 2
  %"foo_$ENL.dim_info$.lower_bound$[]38" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"foo_$ENL.dim_info$.lower_bound$37", i32 0)
  %"foo_$ENL.dim_info$.lower_bound$[]_fetch.20" = load i64, i64* %"foo_$ENL.dim_info$.lower_bound$[]38", align 1, !tbaa !11
  %"foo_$ENL.dim_info$39" = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %"foo_$ENL", i32 0, i32 6, i32 0
  %"foo_$ENL.dim_info$.lower_bound$40" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"foo_$ENL.dim_info$39", i32 0, i32 2
  %"foo_$ENL.dim_info$.lower_bound$[]41" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"foo_$ENL.dim_info$.lower_bound$40", i32 0)
  %"foo_$ENL.dim_info$.lower_bound$[]_fetch.21" = load i64, i64* %"foo_$ENL.dim_info$.lower_bound$[]41", align 1, !tbaa !11
  %"foo_$ENL.dim_info$42" = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %"foo_$ENL", i32 0, i32 6, i32 0
  %"foo_$ENL.dim_info$.extent$43" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"foo_$ENL.dim_info$42", i32 0, i32 0
  %"foo_$ENL.dim_info$.extent$[]44" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"foo_$ENL.dim_info$.extent$43", i32 0)
  %"foo_$ENL.dim_info$.extent$[]_fetch.22" = load i64, i64* %"foo_$ENL.dim_info$.extent$[]44", align 1, !tbaa !12
  %add.3 = add nsw i64 %"foo_$ENL.dim_info$.lower_bound$[]_fetch.21", %"foo_$ENL.dim_info$.extent$[]_fetch.22"
  %sub.2 = sub nsw i64 %add.3, 1
  %"foo_$ENL.dim_info$45" = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %"foo_$ENL", i32 0, i32 6, i32 0
  %"foo_$ENL.dim_info$.extent$46" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"foo_$ENL.dim_info$45", i32 0, i32 0
  %"foo_$ENL.dim_info$.extent$[]47" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"foo_$ENL.dim_info$.extent$46", i32 0)
  %"foo_$ENL.dim_info$.extent$[]_fetch.23" = load i64, i64* %"foo_$ENL.dim_info$.extent$[]47", align 1, !tbaa !12
  %"foo_$ENL.dim_info$48" = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %"foo_$ENL", i32 0, i32 6, i32 0
  %"foo_$ENL.dim_info$.spacing$49" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"foo_$ENL.dim_info$48", i32 0, i32 1
  %"foo_$ENL.dim_info$.spacing$[]50" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"foo_$ENL.dim_info$.spacing$49", i32 1)
  %"foo_$ENL.dim_info$.spacing$[]_fetch.24" = load i64, i64* %"foo_$ENL.dim_info$.spacing$[]50", align 1, !tbaa !13, !range !14
  %"foo_$ENL.dim_info$51" = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %"foo_$ENL", i32 0, i32 6, i32 0
  %"foo_$ENL.dim_info$.lower_bound$52" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"foo_$ENL.dim_info$51", i32 0, i32 2
  %"foo_$ENL.dim_info$.lower_bound$[]53" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"foo_$ENL.dim_info$.lower_bound$52", i32 1)
  %"foo_$ENL.dim_info$.lower_bound$[]_fetch.25" = load i64, i64* %"foo_$ENL.dim_info$.lower_bound$[]53", align 1, !tbaa !11
  %"foo_$ENL.dim_info$54" = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %"foo_$ENL", i32 0, i32 6, i32 0
  %"foo_$ENL.dim_info$.lower_bound$55" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"foo_$ENL.dim_info$54", i32 0, i32 2
  %"foo_$ENL.dim_info$.lower_bound$[]56" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"foo_$ENL.dim_info$.lower_bound$55", i32 1)
  %"foo_$ENL.dim_info$.lower_bound$[]_fetch.26" = load i64, i64* %"foo_$ENL.dim_info$.lower_bound$[]56", align 1, !tbaa !11
  %"foo_$ENL.dim_info$57" = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %"foo_$ENL", i32 0, i32 6, i32 0
  %"foo_$ENL.dim_info$.lower_bound$58" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"foo_$ENL.dim_info$57", i32 0, i32 2
  %"foo_$ENL.dim_info$.lower_bound$[]59" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"foo_$ENL.dim_info$.lower_bound$58", i32 1)
  %"foo_$ENL.dim_info$.lower_bound$[]_fetch.27" = load i64, i64* %"foo_$ENL.dim_info$.lower_bound$[]59", align 1, !tbaa !11
  %"foo_$ENL.dim_info$60" = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %"foo_$ENL", i32 0, i32 6, i32 0
  %"foo_$ENL.dim_info$.extent$61" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"foo_$ENL.dim_info$60", i32 0, i32 0
  %"foo_$ENL.dim_info$.extent$[]62" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"foo_$ENL.dim_info$.extent$61", i32 1)
  %"foo_$ENL.dim_info$.extent$[]_fetch.28" = load i64, i64* %"foo_$ENL.dim_info$.extent$[]62", align 1, !tbaa !12
  %add.4 = add nsw i64 %"foo_$ENL.dim_info$.lower_bound$[]_fetch.27", %"foo_$ENL.dim_info$.extent$[]_fetch.28"
  %sub.3 = sub nsw i64 %add.4, 1
  %"foo_$ENL.dim_info$63" = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %"foo_$ENL", i32 0, i32 6, i32 0
  %"foo_$ENL.dim_info$.extent$64" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"foo_$ENL.dim_info$63", i32 0, i32 0
  %"foo_$ENL.dim_info$.extent$[]65" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"foo_$ENL.dim_info$.extent$64", i32 1)
  %"foo_$ENL.dim_info$.extent$[]_fetch.29" = load i64, i64* %"foo_$ENL.dim_info$.extent$[]65", align 1, !tbaa !12
  %"foo_$ENL.dim_info$66" = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %"foo_$ENL", i32 0, i32 6, i32 0
  %"foo_$ENL.dim_info$.lower_bound$67" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"foo_$ENL.dim_info$66", i32 0, i32 2
  %"foo_$ENL.dim_info$.lower_bound$[]68" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"foo_$ENL.dim_info$.lower_bound$67", i32 0)
  %"foo_$ENL.dim_info$.lower_bound$[]_fetch.30" = load i64, i64* %"foo_$ENL.dim_info$.lower_bound$[]68", align 1, !tbaa !11
  %mul.5 = mul nsw i64 %"foo_$ENL.dim_info$.lower_bound$[]_fetch.30", 8
  %"foo_$ENL.dim_info$69" = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %"foo_$ENL", i32 0, i32 6, i32 0
  %"foo_$ENL.dim_info$.lower_bound$70" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"foo_$ENL.dim_info$69", i32 0, i32 2
  %"foo_$ENL.dim_info$.lower_bound$[]71" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"foo_$ENL.dim_info$.lower_bound$70", i32 1)
  %"foo_$ENL.dim_info$.lower_bound$[]_fetch.31" = load i64, i64* %"foo_$ENL.dim_info$.lower_bound$[]71", align 1, !tbaa !11
  %"foo_$ENL.dim_info$72" = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %"foo_$ENL", i32 0, i32 6, i32 0
  %"foo_$ENL.dim_info$.spacing$73" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"foo_$ENL.dim_info$72", i32 0, i32 1
  %"foo_$ENL.dim_info$.spacing$[]74" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"foo_$ENL.dim_info$.spacing$73", i32 1)
  %"foo_$ENL.dim_info$.spacing$[]_fetch.32" = load i64, i64* %"foo_$ENL.dim_info$.spacing$[]74", align 1, !tbaa !13, !range !14
  %mul.6 = mul nsw i64 %"foo_$ENL.dim_info$.lower_bound$[]_fetch.31", %"foo_$ENL.dim_info$.spacing$[]_fetch.32"
  %add.5 = add nsw i64 %mul.5, %mul.6
  store i64 %"foo_$ENL.dim_info$.lower_bound$[]_fetch.26", i64* %"var$7", align 1, !tbaa !1
  store i64 1, i64* %"$loop_ctr31", align 1, !tbaa !1
  br label %loop_test19

loop_test15:                                      ; preds = %loop_body20, %loop_body16
  %"$loop_ctr_fetch.35" = load i64, i64* %"$loop_ctr", align 1, !tbaa !1
  %rel.2 = icmp sle i64 %"$loop_ctr_fetch.35", %"foo_$ENL.dim_info$.extent$[]_fetch.23"
  br i1 %rel.2, label %loop_body16, label %loop_exit17

loop_body16:                                      ; preds = %loop_test15
  %"var$7_fetch.33" = load i64, i64* %"var$7", align 1, !tbaa !1
  %"foo_$ENL.addr_a0$_fetch.18[]" = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %"foo_$ENL.dim_info$.lower_bound$[]_fetch.25", i64 %"foo_$ENL.dim_info$.spacing$[]_fetch.24", double* elementtype(double) %"foo_$ENL.addr_a0$_fetch.18", i64 %"var$7_fetch.33")
  %"var$6_fetch.34" = load i64, i64* %"var$6", align 1, !tbaa !1
  %"foo_$ENL.addr_a0$_fetch.18[][]" = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %"foo_$ENL.dim_info$.lower_bound$[]_fetch.19", i64 8, double* elementtype(double) %"foo_$ENL.addr_a0$_fetch.18[]", i64 %"var$6_fetch.34")
  store double 0.000000e+00, double* %"foo_$ENL.addr_a0$_fetch.18[][]", align 1, !tbaa !18
  %"var$6_fetch.36" = load i64, i64* %"var$6", align 1, !tbaa !1
  %add.6 = add nsw i64 %"var$6_fetch.36", 1
  store i64 %add.6, i64* %"var$6", align 1, !tbaa !1
  %"$loop_ctr_fetch.37" = load i64, i64* %"$loop_ctr", align 1, !tbaa !1
  %add.7 = add nsw i64 %"$loop_ctr_fetch.37", 1
  store i64 %add.7, i64* %"$loop_ctr", align 1, !tbaa !1
  br label %loop_test15

loop_exit17:                                      ; preds = %loop_test15
  %"var$7_fetch.39" = load i64, i64* %"var$7", align 1, !tbaa !1
  %add.8 = add nsw i64 %"var$7_fetch.39", 1
  store i64 %add.8, i64* %"var$7", align 1, !tbaa !1
  %"$loop_ctr31_fetch.40" = load i64, i64* %"$loop_ctr31", align 1, !tbaa !1
  %add.9 = add nsw i64 %"$loop_ctr31_fetch.40", 1
  store i64 %add.9, i64* %"$loop_ctr31", align 1, !tbaa !1
  br label %loop_test19

loop_test19:                                      ; preds = %loop_exit17, %omp.pdo.body10
  %"$loop_ctr31_fetch.38" = load i64, i64* %"$loop_ctr31", align 1, !tbaa !1
  %rel.3 = icmp sle i64 %"$loop_ctr31_fetch.38", %"foo_$ENL.dim_info$.extent$[]_fetch.29"
  br i1 %rel.3, label %loop_body20, label %loop_exit21

loop_body20:                                      ; preds = %loop_test19
  store i64 %"foo_$ENL.dim_info$.lower_bound$[]_fetch.20", i64* %"var$6", align 1, !tbaa !1
  store i64 1, i64* %"$loop_ctr", align 1, !tbaa !1
  br label %loop_test15

loop_exit21:                                      ; preds = %loop_test19
  %omp.pdo.norm.iv_fetch.41 = load i64, i64* %omp.pdo.norm.iv, align 1, !tbaa !1
  %add.10 = add nsw i64 %omp.pdo.norm.iv_fetch.41, 1
  store i64 %add.10, i64* %omp.pdo.norm.iv, align 1, !tbaa !1
  br label %omp.pdo.cond9

bb_new12:                                         ; preds = %bb_new7
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(), "QUAL.OMP.PRIVATE:F90_DV"(%"QNCA_a0$double*$rank2$"* %"foo_$ENL"), "QUAL.OMP.PRIVATE"(i64* %"var$7"), "QUAL.OMP.PRIVATE"(i64* %"var$6"), "QUAL.OMP.PRIVATE"(i64* %"$loop_ctr31"), "QUAL.OMP.PRIVATE"(i64* %"$loop_ctr"), "QUAL.OMP.PRIVATE"(i32* %"foo_$ICOL"), "QUAL.OMP.FIRSTPRIVATE"(i64* %omp.pdo.norm.lb), "QUAL.OMP.NORMALIZED.IV"(i64* %omp.pdo.norm.iv), "QUAL.OMP.NORMALIZED.UB"(i64* %omp.pdo.norm.ub) ]
  %omp.pdo.norm.lb_fetch.12 = load i64, i64* %omp.pdo.norm.lb, align 1, !tbaa !1
  store i64 %omp.pdo.norm.lb_fetch.12, i64* %omp.pdo.norm.iv, align 1, !tbaa !1
  br label %omp.pdo.cond9

omp.pdo.epilog11:                                 ; preds = %omp.pdo.cond9
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.LOOP"() ]
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL"() ]
  %"foo_$ENL.addr_a0$77" = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %"foo_$ENL", i32 0, i32 0
  %"foo_$ENL.addr_a0$77_fetch.42" = load double*, double** %"foo_$ENL.addr_a0$77", align 1, !tbaa !17
  %"foo_$ENL.addr_a0$79" = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %"foo_$ENL", i32 0, i32 0
  %"foo_$ENL.addr_a0$79_fetch.43" = load double*, double** %"foo_$ENL.addr_a0$79", align 1, !tbaa !17
  %"foo_$ENL.flags$81" = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %"foo_$ENL", i32 0, i32 3
  %"foo_$ENL.flags$81_fetch.44" = load i64, i64* %"foo_$ENL.flags$81", align 1, !tbaa !4
  %and.17 = and i64 %"foo_$ENL.flags$81_fetch.44", 2
  %lshr.6 = lshr i64 %and.17, 1
  %shl.9 = shl i64 %lshr.6, 2
  %int_zext83 = trunc i64 %shl.9 to i32
  %or.10 = or i32 0, %int_zext83
  %and.19 = and i64 %"foo_$ENL.flags$81_fetch.44", 1
  %and.20 = and i32 %or.10, -3
  %shl.10 = shl i64 %and.19, 1
  %int_zext85 = trunc i64 %shl.10 to i32
  %or.11 = or i32 %and.20, %int_zext85
  %and.21 = and i64 %"foo_$ENL.flags$81_fetch.44", 2048
  %lshr.7 = lshr i64 %and.21, 11
  %and.22 = and i32 %or.11, -257
  %shl.11 = shl i64 %lshr.7, 8
  %int_zext87 = trunc i64 %shl.11 to i32
  %or.12 = or i32 %and.22, %int_zext87
  %and.23 = and i64 %"foo_$ENL.flags$81_fetch.44", 256
  %lshr.8 = lshr i64 %and.23, 8
  %and.24 = and i32 %or.12, -2097153
  %shl.12 = shl i64 %lshr.8, 21
  %int_zext89 = trunc i64 %shl.12 to i32
  %or.13 = or i32 %and.24, %int_zext89
  %and.25 = and i64 %"foo_$ENL.flags$81_fetch.44", 1030792151040
  %lshr.9 = lshr i64 %and.25, 36
  %and.26 = and i32 %or.13, -31457281
  %shl.13 = shl i64 %lshr.9, 21
  %int_zext91 = trunc i64 %shl.13 to i32
  %or.14 = or i32 %and.26, %int_zext91
  %and.27 = and i64 %"foo_$ENL.flags$81_fetch.44", 1099511627776
  %lshr.10 = lshr i64 %and.27, 40
  %and.28 = and i32 %or.14, -33554433
  %shl.14 = shl i64 %lshr.10, 25
  %int_zext93 = trunc i64 %shl.14 to i32
  %or.15 = or i32 %and.28, %int_zext93
  %and.29 = and i32 %or.15, -2031617
  %or.16 = or i32 %and.29, 262144
  %"(i8*)foo_$ENL.addr_a0$79_fetch.43$" = bitcast double* %"foo_$ENL.addr_a0$79_fetch.43" to i8*
  %"foo_$ENL.reserved$95" = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %"foo_$ENL", i32 0, i32 5
  %"foo_$ENL.reserved$95_fetch.45" = load i64, i64* %"foo_$ENL.reserved$95", align 1, !tbaa !7
  %"(i8*)foo_$ENL.reserved$95_fetch.45$" = inttoptr i64 %"foo_$ENL.reserved$95_fetch.45" to i8*
  %func_result97 = call i32 @for_dealloc_allocatable_handle(i8* %"(i8*)foo_$ENL.addr_a0$79_fetch.43$", i32 %or.16, i8* %"(i8*)foo_$ENL.reserved$95_fetch.45$")
  %"foo_$ENL.addr_a0$99" = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %"foo_$ENL", i32 0, i32 0
  %"foo_$ENL.addr_a0$99_fetch.46" = load double*, double** %"foo_$ENL.addr_a0$99", align 1, !tbaa !17
  %rel.4 = icmp eq i32 %func_result97, 0
  br i1 %rel.4, label %bb_new24_then, label %bb9_endif

bb_new7:                                          ; preds = %alloca_0
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.SHARED"(%"QNCA_a0$double*$rank2$"* %"foo_$ENL"), "QUAL.OMP.PRIVATE"(i32* %"foo_$ICOL"), "QUAL.OMP.PRIVATE"(i64* %"var$7"), "QUAL.OMP.PRIVATE"(i64* %"var$6"), "QUAL.OMP.PRIVATE"(i64* %"$loop_ctr31"), "QUAL.OMP.PRIVATE"(i64* %"$loop_ctr") ]
; CHECK: call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"()
; CHECK-SAME: "QUAL.OMP.SHARED"(%"QNCA_a0$double*$rank2$"* %"foo_$ENL")
  %omp.pdo.start = alloca i32, align 4
  store i32 1, i32* %omp.pdo.start, align 1, !tbaa !1
  %omp.pdo.end = alloca i32, align 4
  store i32 2, i32* %omp.pdo.end, align 1, !tbaa !1
  %omp.pdo.step = alloca i32, align 4
  store i32 1, i32* %omp.pdo.step, align 1, !tbaa !1
  %omp.pdo.norm.iv = alloca i64, align 8
  %omp.pdo.norm.lb = alloca i64, align 8
  store i64 0, i64* %omp.pdo.norm.lb, align 1, !tbaa !1
  %omp.pdo.norm.ub = alloca i64, align 8
  %omp.pdo.end_fetch.9 = load i32, i32* %omp.pdo.end, align 1, !tbaa !1
  %omp.pdo.start_fetch.10 = load i32, i32* %omp.pdo.start, align 1, !tbaa !1
  %sub.1 = sub nsw i32 %omp.pdo.end_fetch.9, %omp.pdo.start_fetch.10
  %omp.pdo.step_fetch.11 = load i32, i32* %omp.pdo.step, align 1, !tbaa !1
  %div.1 = sdiv i32 %sub.1, %omp.pdo.step_fetch.11
  %int_sext75 = sext i32 %div.1 to i64
  store i64 %int_sext75, i64* %omp.pdo.norm.ub, align 1, !tbaa !1
  br label %bb_new12

bb_new24_then:                                    ; preds = %omp.pdo.epilog11
  %"foo_$ENL.addr_a0$100" = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %"foo_$ENL", i32 0, i32 0
  store double* null, double** %"foo_$ENL.addr_a0$100", align 1, !tbaa !17
  %"foo_$ENL.flags$101" = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %"foo_$ENL", i32 0, i32 3
  %"foo_$ENL.flags$_fetch.47" = load i64, i64* %"foo_$ENL.flags$101", align 1, !tbaa !4
  %and.30 = and i64 %"foo_$ENL.flags$_fetch.47", -2
  %"foo_$ENL.flags$102" = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %"foo_$ENL", i32 0, i32 3
  store i64 %and.30, i64* %"foo_$ENL.flags$102", align 1, !tbaa !4
  %"foo_$ENL.flags$103" = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %"foo_$ENL", i32 0, i32 3
  %"foo_$ENL.flags$_fetch.48" = load i64, i64* %"foo_$ENL.flags$103", align 1, !tbaa !4
  %and.31 = and i64 %"foo_$ENL.flags$_fetch.48", -2049
  %"foo_$ENL.flags$104" = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %"foo_$ENL", i32 0, i32 3
  store i64 %and.31, i64* %"foo_$ENL.flags$104", align 1, !tbaa !4
  %"foo_$ENL.flags$105" = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %"foo_$ENL", i32 0, i32 3
  %"foo_$ENL.flags$_fetch.49" = load i64, i64* %"foo_$ENL.flags$105", align 1, !tbaa !4
  %and.32 = and i64 %"foo_$ENL.flags$_fetch.49", -1030792151041
  %or.17 = or i64 %and.32, 0
  %"foo_$ENL.flags$106" = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %"foo_$ENL", i32 0, i32 3
  store i64 %or.17, i64* %"foo_$ENL.flags$106", align 1, !tbaa !4
  br label %bb9_endif

bb9_endif:                                        ; preds = %omp.pdo.epilog11, %bb_new24_then
  %"foo_$ENL.flags$108" = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %"foo_$ENL", i32 0, i32 3
  %"foo_$ENL.flags$108_fetch.50" = load i64, i64* %"foo_$ENL.flags$108", align 1, !tbaa !4
  %and.33 = and i64 %"foo_$ENL.flags$108_fetch.50", 1
  %rel.5 = icmp eq i64 %and.33, 0
  br i1 %rel.5, label %dealloc.list.end26, label %dealloc.list.then25

dealloc.list.then25:                              ; preds = %bb9_endif
  %"foo_$ENL.addr_a0$110" = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %"foo_$ENL", i32 0, i32 0
  %"foo_$ENL.addr_a0$110_fetch.51" = load double*, double** %"foo_$ENL.addr_a0$110", align 1, !tbaa !17
  %"foo_$ENL.flags$112" = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %"foo_$ENL", i32 0, i32 3
  %"foo_$ENL.flags$112_fetch.52" = load i64, i64* %"foo_$ENL.flags$112", align 1, !tbaa !4
  %and.34 = and i64 %"foo_$ENL.flags$112_fetch.52", 2
  %lshr.11 = lshr i64 %and.34, 1
  %shl.16 = shl i64 %lshr.11, 2
  %int_zext114 = trunc i64 %shl.16 to i32
  %or.18 = or i32 0, %int_zext114
  %and.36 = and i64 %"foo_$ENL.flags$112_fetch.52", 1
  %and.37 = and i32 %or.18, -3
  %shl.17 = shl i64 %and.36, 1
  %int_zext116 = trunc i64 %shl.17 to i32
  %or.19 = or i32 %and.37, %int_zext116
  %and.38 = and i64 %"foo_$ENL.flags$112_fetch.52", 2048
  %lshr.12 = lshr i64 %and.38, 11
  %and.39 = and i32 %or.19, -257
  %shl.18 = shl i64 %lshr.12, 8
  %int_zext118 = trunc i64 %shl.18 to i32
  %or.20 = or i32 %and.39, %int_zext118
  %and.40 = and i64 %"foo_$ENL.flags$112_fetch.52", 256
  %lshr.13 = lshr i64 %and.40, 8
  %and.41 = and i32 %or.20, -2097153
  %shl.19 = shl i64 %lshr.13, 21
  %int_zext120 = trunc i64 %shl.19 to i32
  %or.21 = or i32 %and.41, %int_zext120
  %and.42 = and i64 %"foo_$ENL.flags$112_fetch.52", 1030792151040
  %lshr.14 = lshr i64 %and.42, 36
  %and.43 = and i32 %or.21, -31457281
  %shl.20 = shl i64 %lshr.14, 21
  %int_zext122 = trunc i64 %shl.20 to i32
  %or.22 = or i32 %and.43, %int_zext122
  %and.44 = and i64 %"foo_$ENL.flags$112_fetch.52", 1099511627776
  %lshr.15 = lshr i64 %and.44, 40
  %and.45 = and i32 %or.22, -33554433
  %shl.21 = shl i64 %lshr.15, 25
  %int_zext124 = trunc i64 %shl.21 to i32
  %or.23 = or i32 %and.45, %int_zext124
  %and.46 = and i32 %or.23, -2031617
  %or.24 = or i32 %and.46, 262144
  %"(i8*)foo_$ENL.addr_a0$110_fetch.51$" = bitcast double* %"foo_$ENL.addr_a0$110_fetch.51" to i8*
  %"foo_$ENL.reserved$126" = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %"foo_$ENL", i32 0, i32 5
  %"foo_$ENL.reserved$126_fetch.53" = load i64, i64* %"foo_$ENL.reserved$126", align 1, !tbaa !7
  %"(i8*)foo_$ENL.reserved$126_fetch.53$" = inttoptr i64 %"foo_$ENL.reserved$126_fetch.53" to i8*
  %func_result128 = call i32 @for_dealloc_allocatable_handle(i8* %"(i8*)foo_$ENL.addr_a0$110_fetch.51$", i32 %or.24, i8* %"(i8*)foo_$ENL.reserved$126_fetch.53$")
  %"foo_$ENL.addr_a0$130" = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %"foo_$ENL", i32 0, i32 0
  %"foo_$ENL.addr_a0$130_fetch.54" = load double*, double** %"foo_$ENL.addr_a0$130", align 1, !tbaa !17
  %rel.6 = icmp eq i32 %func_result128, 0
  br i1 %rel.6, label %bb_new29_then, label %dealloc.list.end26

bb_new29_then:                                    ; preds = %dealloc.list.then25
  %"foo_$ENL.addr_a0$131" = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %"foo_$ENL", i32 0, i32 0
  store double* null, double** %"foo_$ENL.addr_a0$131", align 1, !tbaa !17
  %"foo_$ENL.flags$132" = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %"foo_$ENL", i32 0, i32 3
  %"foo_$ENL.flags$_fetch.55" = load i64, i64* %"foo_$ENL.flags$132", align 1, !tbaa !4
  %and.47 = and i64 %"foo_$ENL.flags$_fetch.55", -2
  %"foo_$ENL.flags$133" = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %"foo_$ENL", i32 0, i32 3
  store i64 %and.47, i64* %"foo_$ENL.flags$133", align 1, !tbaa !4
  %"foo_$ENL.flags$134" = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %"foo_$ENL", i32 0, i32 3
  %"foo_$ENL.flags$_fetch.56" = load i64, i64* %"foo_$ENL.flags$134", align 1, !tbaa !4
  %and.48 = and i64 %"foo_$ENL.flags$_fetch.56", -2049
  %"foo_$ENL.flags$135" = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %"foo_$ENL", i32 0, i32 3
  store i64 %and.48, i64* %"foo_$ENL.flags$135", align 1, !tbaa !4
  br label %dealloc.list.end26

dealloc.list.end26:                               ; preds = %bb_new29_then, %dealloc.list.then25, %bb9_endif
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8, i64, i32, i64*, i32) #1

declare i32 @for_alloc_allocatable_handle(i64, i8** nocapture, i32, i8*)

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind readnone speculatable
declare double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8, i64, i64, double*, i64) #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

declare i32 @for_dealloc_allocatable_handle(i8* nocapture readonly, i32, i8*)

attributes #0 = { nounwind uwtable "denormal-fp-math"="preserve_sign,preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { nounwind readnone speculatable }
attributes #2 = { nounwind }

!omp_offload.info = !{}
!llvm.module.flags = !{!0}

!0 = !{i32 7, !"openmp", i32 50}
!1 = !{!2, !2, i64 0}
!2 = !{!"Generic Fortran Symbol", !3, i64 0}
!3 = !{!"ifx$root$1$foo_"}
!4 = !{!5, !6, i64 24}
!5 = !{!"ifx$descr$1", !6, i64 0, !6, i64 8, !6, i64 16, !6, i64 24, !6, i64 32, !6, i64 40, !6, i64 48, !6, i64 56, !6, i64 64, !6, i64 72, !6, i64 80, !6, i64 88}
!6 = !{!"ifx$descr$field", !2, i64 0}
!7 = !{!5, !6, i64 40}
!8 = !{!5, !6, i64 8}
!9 = !{!5, !6, i64 32}
!10 = !{!5, !6, i64 16}
!11 = !{!5, !6, i64 64}
!12 = !{!5, !6, i64 48}
!13 = !{!5, !6, i64 56}
!14 = !{i64 1, i64 -9223372036854775808}
!15 = !{!16, !16, i64 0}
!16 = !{!"ifx$unique_sym$1", !2, i64 0}
!17 = !{!5, !6, i64 0}
!18 = !{!19, !19, i64 0}
!19 = !{!"ifx$unique_sym$2", !2, i64 0}
