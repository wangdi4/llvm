; INTEL_CUSTOMIZATION
; CMPLRLLVM-50598
;
; REQUIRES: asserts
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt -debug-only=vpo-paropt-transform -debug-only=vpo-paropt-target -S <%s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring),vpo-paropt' -debug-only=vpo-paropt-transform -debug-only=vpo-paropt-target -S <%s 2>&1 | FileCheck %s

; subroutine foo(lb, ub, n, ccc_arrsect)
;   integer              :: lb, ub, n, i
;   integer              :: aaa_scalar
;   integer              :: bbb_array(100)
;   integer              :: ccc_arrsect(*)
;   integer              :: ggg_arrsect_var_lb(100)
;   integer              :: hhh_arrsect_var_ub(100)
;   integer              :: iii_vla(n)
;   integer, allocatable :: jjj_alloc_array(:)
;   allocate (jjj_alloc_array(100))
;   !$omp target teams distribute parallel do reduction(+: aaa_scalar, bbb_array, ccc_arrsect(20:60), &
;   !$omp              ggg_arrsect_var_lb(lb:90), hhh_arrsect_var_ub(40:ub), iii_vla, jjj_alloc_array)
;   do i = 1, 10
;     aaa_scalar             = 111  ! expected to use HOST_MEM
;     bbb_array(22)          = 222  ! expected to use HOST_MEM
;     ccc_arrsect(33)        = 333  ! expected to use HOST_MEM
;     ggg_arrsect_var_lb(44) = 444  ! not AFReduction; don't use HOST_MEM
;     hhh_arrsect_var_ub(55) = 555  ! not AFReduction; don't use HOST_MEM
;     iii_vla(66)            = 666  ! not AFReduction; don't use HOST_MEM
;     jjj_alloc_array(77)    = 777  ! not AFReduction; don't use HOST_MEM
;   end do
;   !$omp end target teams distribute parallel do
; end subroutine foo
;
; The FFE adds the USE_HOST_MEM MapType (bit 0x8000) to the implicit MAP of a
; REDUCTION var when compiled with "-switch use-host-usm-for-implicit-reduction-map".
; This MapType should only be used if the var is in an atomic-free reduction, so Paropt
; removes the bit if it finds that the reduction is not atomic-free.
;
; CHECK-LABEL: Enter VPOParoptTransform::genReductionCode
;
; CHECK: ptr %"foo_$AAA_SCALAR" is used in atomic-free reduction.
; CHECK: ptr %"foo_$BBB_ARRAY" is used in atomic-free reduction.
; CHECK: ptr %"foo_$CCC_ARRSECT" is used in atomic-free reduction.
; CHECK-NOT: ptr %"foo_$GGG_ARRSECT_VAR_LB" is used in atomic-free reduction.
; CHECK-NOT: ptr %"foo_$HHH_ARRSECT_VAR_UB" is used in atomic-free reduction.
; CHECK-NOT: ptr %"foo_$III_VLA{{.*}}" is used in atomic-free reduction.
; CHECK-NOT: ptr %"foo_$JJJ_ALLOC_ARRAY" is used in atomic-free reduction.
;
;
; CHECK-LABEL: Enter VPOParoptTransform::genTgtInformationForPtrs
;
; CHECK: Working with Map Item: 'CHAIN(<ptr %"foo_$AAA_SCALAR", ptr %"foo_$AAA_SCALAR", i64 4, 33315 (0x0000000000008223), null, null> ) '.
; CHECK: MapType 33315 has HOST_MEM bit and it is honored.

; CHECK: Working with Map Item: 'CHAIN(<ptr %"foo_$BBB_ARRAY", ptr %"foo_$BBB_ARRAY", i64 400, 33315 (0x0000000000008223), null, null> ) '.
; CHECK: MapType 33315 has HOST_MEM bit and it is honored.

; CHECK: Working with Map Item: 'CHAIN(<ptr %"foo_$CCC_ARRSECT", ptr %"foo_$CCC_ARRSECT", i64 0, 33315 (0x0000000000008223), null, null> ) '.
; CHECK: MapType 33315 has HOST_MEM bit and it is honored.

; CHECK: Working with Map Item: 'CHAIN(<ptr %"foo_$GGG_ARRSECT_VAR_LB", ptr %"foo_$GGG_ARRSECT_VAR_LB", i64 400, 33315 (0x0000000000008223), null, null> ) '.
; CHECK: MapType 33315 has HOST_MEM bit but it is removed.
; CHECK: MapType changed from '33315 (0x0000000000008223)' to '547 (0x0000000000000223)'.

; CHECK: Working with Map Item: 'CHAIN(<ptr %"foo_$HHH_ARRSECT_VAR_UB", ptr %"foo_$HHH_ARRSECT_VAR_UB", i64 400, 33315 (0x0000000000008223), null, null> ) '.
; CHECK: MapType 33315 has HOST_MEM bit but it is removed.
; CHECK: MapType changed from '33315 (0x0000000000008223)' to '547 (0x0000000000000223)'.

; CHECK: Working with Map Item: 'CHAIN,VARLEN(<ptr %"foo_$III_VLA2", ptr %"foo_$III_VLA2", i64 %mul.3, 33315 (0x0000000000008223), null, null> ) '.
; CHECK: MapType 33315 has HOST_MEM bit but it is removed.
; CHECK: MapType changed from '33315 (0x0000000000008223)' to '547 (0x0000000000000223)'.

; CHECK: Working with Map Item: 'CHAIN(<ptr %"foo_$JJJ_ALLOC_ARRAY", ptr %"foo_$JJJ_ALLOC_ARRAY", i64 72, 32800 (0x0000000000008020), null, null> <ptr %"foo_$JJJ_ALLOC_ARRAY", ptr %"foo_$JJJ_ALLOC_ARRAY.addr_a0$18_fetch.12", i64 %5, 1970324837007891 (0x0007000000008213), null, null> <ptr %"foo_$JJJ_ALLOC_ARRAY", ptr %fetch.1.fca.1.gep, i64 64, 1970324837007361 (0x0007000000008001), null, null> ) '.
; CHECK: MapType 32800 has HOST_MEM bit but it is removed.
; CHECK-NEXT: MapType changed from '32800 (0x0000000000008020)' to '32 (0x0000000000000020)'.
; CHECK-NEXT: MapType 1970324837007891 has HOST_MEM bit but it is removed.
; CHECK-NEXT: MapType changed from '1970324837007891 (0x0007000000008213)' to '1970324836975123 (0x0007000000000213)'.
; CHECK-NEXT: MapType 1970324837007361 has HOST_MEM bit but it is removed.
; CHECK-NEXT: MapType changed from '1970324837007361 (0x0007000000008001)' to '1970324836974593 (0x0007000000000001)'.
;
;
; CHECK: @.offload_maptypes = private unnamed_addr constant [25 x i64] [i64 33315, i64 33315, i64 33315, i64 547, i64 547, i64 547, i64 32, i64 1970324836975123, i64 1970324836974593

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

%"QNCA_a0$ptr$rank1$.0" = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }

; Function Attrs: nounwind uwtable
define void @foo_(ptr noalias dereferenceable(4) %"foo_$LB", ptr noalias dereferenceable(4) %"foo_$UB", ptr noalias dereferenceable(4) %"foo_$N", ptr noalias dereferenceable(4) %"foo_$CCC_ARRSECT") local_unnamed_addr {
alloca_0:
  %"foo_$HHH_ARRSECT_VAR_UB" = alloca [100 x i32], align 16
  %"foo_$GGG_ARRSECT_VAR_LB" = alloca [100 x i32], align 16
  %"foo_$BBB_ARRAY" = alloca [100 x i32], align 16
  %"foo_$AAA_SCALAR" = alloca i32, align 4
  %"foo_$I" = alloca i32, align 4
  %"foo_$JJJ_ALLOC_ARRAY" = alloca %"QNCA_a0$ptr$rank1$.0", align 8
  %"var$3" = alloca i32, align 4
  %"foo_$III_VLA2.array.elements" = alloca i64, align 8
  %"foo_$N_fetch.3" = load i32, ptr %"foo_$N", align 1
  store i32 %"foo_$N_fetch.3", ptr %"var$3", align 4
  %fetch.1.fca.0.gep = getelementptr inbounds %"QNCA_a0$ptr$rank1$.0", ptr %"foo_$JJJ_ALLOC_ARRAY", i64 0, i32 0
  store ptr null, ptr %fetch.1.fca.0.gep, align 8
  %fetch.1.fca.1.gep = getelementptr inbounds %"QNCA_a0$ptr$rank1$.0", ptr %"foo_$JJJ_ALLOC_ARRAY", i64 0, i32 1
  store i64 0, ptr %fetch.1.fca.1.gep, align 8
  %fetch.1.fca.2.gep = getelementptr inbounds %"QNCA_a0$ptr$rank1$.0", ptr %"foo_$JJJ_ALLOC_ARRAY", i64 0, i32 2
  store i64 0, ptr %fetch.1.fca.2.gep, align 8
  %fetch.1.fca.3.gep = getelementptr inbounds %"QNCA_a0$ptr$rank1$.0", ptr %"foo_$JJJ_ALLOC_ARRAY", i64 0, i32 3
  store i64 128, ptr %fetch.1.fca.3.gep, align 8
  %fetch.1.fca.4.gep = getelementptr inbounds %"QNCA_a0$ptr$rank1$.0", ptr %"foo_$JJJ_ALLOC_ARRAY", i64 0, i32 4
  store i64 1, ptr %fetch.1.fca.4.gep, align 8
  %fetch.1.fca.5.gep = getelementptr inbounds %"QNCA_a0$ptr$rank1$.0", ptr %"foo_$JJJ_ALLOC_ARRAY", i64 0, i32 5
  store i64 0, ptr %fetch.1.fca.5.gep, align 8
  %fetch.1.fca.6.0.0.gep = getelementptr inbounds %"QNCA_a0$ptr$rank1$.0", ptr %"foo_$JJJ_ALLOC_ARRAY", i64 0, i32 6, i64 0, i32 0
  store i64 0, ptr %fetch.1.fca.6.0.0.gep, align 8
  %fetch.1.fca.6.0.1.gep = getelementptr inbounds %"QNCA_a0$ptr$rank1$.0", ptr %"foo_$JJJ_ALLOC_ARRAY", i64 0, i32 6, i64 0, i32 1
  store i64 0, ptr %fetch.1.fca.6.0.1.gep, align 8
  %fetch.1.fca.6.0.2.gep = getelementptr inbounds %"QNCA_a0$ptr$rank1$.0", ptr %"foo_$JJJ_ALLOC_ARRAY", i64 0, i32 6, i64 0, i32 2
  store i64 0, ptr %fetch.1.fca.6.0.2.gep, align 8
  %"var$3_fetch.2" = load i32, ptr %"var$3", align 4
  %int_sext = sext i32 %"var$3_fetch.2" to i64
  %slct.1 = call i64 @llvm.smax.i64(i64 %int_sext, i64 0)
  %"foo_$III_VLA2" = alloca i32, i64 %slct.1, align 4
  %slct.2 = call i64 @llvm.smax.i64(i64 %int_sext, i64 0)
  store i64 %slct.2, ptr %"foo_$III_VLA2.array.elements", align 8
  %"foo_$JJJ_ALLOC_ARRAY.flags$_fetch.4" = load i64, ptr %fetch.1.fca.3.gep, align 8
  %or.1 = and i64 %"foo_$JJJ_ALLOC_ARRAY.flags$_fetch.4", 1030792151296
  %or.2 = or i64 %or.1, 133
  store i64 %or.2, ptr %fetch.1.fca.3.gep, align 8
  store i64 0, ptr %fetch.1.fca.5.gep, align 8
  store i64 4, ptr %fetch.1.fca.1.gep, align 8
  store i64 1, ptr %fetch.1.fca.4.gep, align 8
  store i64 0, ptr %fetch.1.fca.2.gep, align 8
  %"foo_$JJJ_ALLOC_ARRAY.dim_info$.lower_bound$6" = getelementptr inbounds %"QNCA_a0$ptr$rank1$.0", ptr %"foo_$JJJ_ALLOC_ARRAY", i64 0, i32 6, i64 0, i32 2
  %"foo_$JJJ_ALLOC_ARRAY.dim_info$.lower_bound$[]7" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"foo_$JJJ_ALLOC_ARRAY.dim_info$.lower_bound$6", i32 0)
  store i64 1, ptr %"foo_$JJJ_ALLOC_ARRAY.dim_info$.lower_bound$[]7", align 1
  %"foo_$JJJ_ALLOC_ARRAY.dim_info$.extent$" = getelementptr inbounds %"QNCA_a0$ptr$rank1$.0", ptr %"foo_$JJJ_ALLOC_ARRAY", i64 0, i32 6, i64 0, i32 0
  %"foo_$JJJ_ALLOC_ARRAY.dim_info$.extent$[]" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"foo_$JJJ_ALLOC_ARRAY.dim_info$.extent$", i32 0)
  store i64 100, ptr %"foo_$JJJ_ALLOC_ARRAY.dim_info$.extent$[]", align 1
  %"foo_$JJJ_ALLOC_ARRAY.dim_info$.spacing$" = getelementptr inbounds %"QNCA_a0$ptr$rank1$.0", ptr %"foo_$JJJ_ALLOC_ARRAY", i64 0, i32 6, i64 0, i32 1
  %"foo_$JJJ_ALLOC_ARRAY.dim_info$.spacing$[]" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"foo_$JJJ_ALLOC_ARRAY.dim_info$.spacing$", i32 0)
  store i64 4, ptr %"foo_$JJJ_ALLOC_ARRAY.dim_info$.spacing$[]", align 1
  %"foo_$JJJ_ALLOC_ARRAY.flags$_fetch.5" = load i64, ptr %fetch.1.fca.3.gep, align 8
  %and.5 = and i64 %"foo_$JJJ_ALLOC_ARRAY.flags$_fetch.5", -68451041281
  %or.3 = or i64 %and.5, 1073741824
  store i64 %or.3, ptr %fetch.1.fca.3.gep, align 8
  %"foo_$JJJ_ALLOC_ARRAY.flags$_fetch.5.tr" = trunc i64 %"foo_$JJJ_ALLOC_ARRAY.flags$_fetch.5" to i32
  %0 = shl i32 %"foo_$JJJ_ALLOC_ARRAY.flags$_fetch.5.tr", 1
  %int_zext = and i32 %0, 2
  %1 = lshr i64 %"foo_$JJJ_ALLOC_ARRAY.flags$_fetch.5", 15
  %2 = trunc i64 %1 to i32
  %int_zext14 = and i32 %2, 31457280
  %or.7 = or i32 %int_zext, %int_zext14
  %3 = lshr i64 %"foo_$JJJ_ALLOC_ARRAY.flags$_fetch.5", 15
  %4 = trunc i64 %3 to i32
  %int_zext15 = and i32 %4, 33554432
  %or.8 = or i32 %or.7, %int_zext15
  %or.9 = or i32 %or.8, 262144
  %"foo_$JJJ_ALLOC_ARRAY.reserved$_fetch.7" = load i64, ptr %fetch.1.fca.5.gep, align 8
  %"(ptr)foo_$JJJ_ALLOC_ARRAY.reserved$_fetch.7$" = inttoptr i64 %"foo_$JJJ_ALLOC_ARRAY.reserved$_fetch.7" to ptr
  %func_result = call i32 @for_alloc_allocatable_handle(i64 400, ptr nonnull %fetch.1.fca.0.gep, i32 %or.9, ptr %"(ptr)foo_$JJJ_ALLOC_ARRAY.reserved$_fetch.7$")
  %"foo_$III_VLA2.array.elements_fetch.11" = load i64, ptr %"foo_$III_VLA2.array.elements", align 8
  %mul.3 = shl nsw i64 %"foo_$III_VLA2.array.elements_fetch.11", 2
  %"foo_$JJJ_ALLOC_ARRAY.addr_a0$18_fetch.12" = load ptr, ptr %fetch.1.fca.0.gep, align 8
  %rel.5.not = icmp eq ptr %"foo_$JJJ_ALLOC_ARRAY.addr_a0$18_fetch.12", null
  %"foo_$JJJ_ALLOC_ARRAY.addr_length$20_fetch.13" = load i64, ptr %fetch.1.fca.1.gep, align 8
  %"foo_$JJJ_ALLOC_ARRAY.dim_info$22.extent$[]_fetch.14" = load i64, ptr %"foo_$JJJ_ALLOC_ARRAY.dim_info$.extent$[]", align 1
  %mul.4 = mul nsw i64 %"foo_$JJJ_ALLOC_ARRAY.addr_length$20_fetch.13", %"foo_$JJJ_ALLOC_ARRAY.dim_info$22.extent$[]_fetch.14"
  %5 = select i1 %rel.5.not, i64 0, i64 %mul.4
  %omp.pdo.norm.iv = alloca i32, align 4
  %omp.pdo.norm.lb = alloca i32, align 4
  store i32 0, ptr %omp.pdo.norm.lb, align 4
  %omp.pdo.norm.ub = alloca i32, align 4
  store volatile i32 9, ptr %omp.pdo.norm.ub, align 4
  %end.dir.temp77 = alloca i1, align 1
  br label %DIR.OMP.TARGET.1

DIR.OMP.TARGET.1:                                 ; preds = %alloca_0
  br label %DIR.OMP.TARGET.2

DIR.OMP.TARGET.2:                                 ; preds = %DIR.OMP.TARGET.1
  %6 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(ptr %"foo_$AAA_SCALAR", ptr %"foo_$AAA_SCALAR", i64 4, i64 33315, ptr null, ptr null), ; MAP type: 33315 = 0x8223 = USE_HOST_MEM (0x8000) | IMPLICIT (0x200) | TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.MAP.TOFROM"(ptr %"foo_$BBB_ARRAY", ptr %"foo_$BBB_ARRAY", i64 400, i64 33315, ptr null, ptr null), ; MAP type: 33315 = 0x8223 = USE_HOST_MEM (0x8000) | IMPLICIT (0x200) | TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.MAP.TOFROM"(ptr %"foo_$CCC_ARRSECT", ptr %"foo_$CCC_ARRSECT", i64 0, i64 33315, ptr null, ptr null), ; MAP type: 33315 = 0x8223 = USE_HOST_MEM (0x8000) | IMPLICIT (0x200) | TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.MAP.TOFROM"(ptr %"foo_$GGG_ARRSECT_VAR_LB", ptr %"foo_$GGG_ARRSECT_VAR_LB", i64 400, i64 33315, ptr null, ptr null), ; MAP type: 33315 = 0x8223 = USE_HOST_MEM (0x8000) | IMPLICIT (0x200) | TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.MAP.TOFROM"(ptr %"foo_$HHH_ARRSECT_VAR_UB", ptr %"foo_$HHH_ARRSECT_VAR_UB", i64 400, i64 33315, ptr null, ptr null), ; MAP type: 33315 = 0x8223 = USE_HOST_MEM (0x8000) | IMPLICIT (0x200) | TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.MAP.TOFROM:VARLEN"(ptr %"foo_$III_VLA2", ptr %"foo_$III_VLA2", i64 %mul.3, i64 33315, ptr null, ptr null), ; MAP type: 33315 = 0x8223 = USE_HOST_MEM (0x8000) | IMPLICIT (0x200) | TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.MAP.TOFROM"(ptr %"foo_$JJJ_ALLOC_ARRAY", ptr %"foo_$JJJ_ALLOC_ARRAY", i64 72, i64 32800, ptr null, ptr null), ; MAP type: 32800 = 0x8020 = USE_HOST_MEM (0x8000) | TARGET_PARAM (0x20)
    "QUAL.OMP.MAP.TOFROM:CHAIN"(ptr %"foo_$JJJ_ALLOC_ARRAY", ptr %"foo_$JJJ_ALLOC_ARRAY.addr_a0$18_fetch.12", i64 %5, i64 1970324837007891, ptr null, ptr null), ; MAP type: 1970324837007891 = 0x7000000008213 = MEMBER_OF_7 (0x7000000000000) | USE_HOST_MEM (0x8000) | IMPLICIT (0x200) | PTR_AND_OBJ (0x10) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.MAP.TOFROM:CHAIN"(ptr %"foo_$JJJ_ALLOC_ARRAY", ptr %fetch.1.fca.1.gep, i64 64, i64 1970324837007361, ptr null, ptr null), ; MAP type: 1970324837007361 = 0x7000000008001 = MEMBER_OF_7 (0x7000000000000) | USE_HOST_MEM (0x8000) | TO (0x1)
    "QUAL.OMP.PRIVATE:TYPED.WILOCAL"(ptr %omp.pdo.norm.iv, i32 0, i64 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED.WILOCAL"(ptr %omp.pdo.norm.ub, i32 0, i64 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED.WILOCAL"(ptr %omp.pdo.norm.lb, i32 0, i64 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED.WILOCAL"(ptr %"foo_$III_VLA2.array.elements", i64 0, i64 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED.WILOCAL"(ptr %"var$3", i32 0, i64 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED.WILOCAL"(ptr %"foo_$I", i32 0, i64 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED.WILOCAL"(ptr %"foo_$UB", i32 0, i64 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED.WILOCAL"(ptr %"foo_$LB", i32 0, i64 1),
    "QUAL.OMP.OFFLOAD.NDRANGE"(ptr %omp.pdo.norm.ub, i32 0),
    "QUAL.OMP.OFFLOAD.HAS.TEAMS.REDUCTION"(),
    "QUAL.OMP.JUMP.TO.END.IF"(ptr %end.dir.temp77) ]

  br label %DIR.OMP.TARGET.3

DIR.OMP.TARGET.3:                                 ; preds = %DIR.OMP.TARGET.2
  %temp.load78 = load volatile i1, ptr %end.dir.temp77, align 1
  br i1 %temp.load78, label %DIR.OMP.END.TARGET.7, label %DIR.OMP.TEAMS.13

omp.pdo.body26:                                   ; preds = %omp.pdo.body26.preheader, %omp.pdo.body26
  %omp.pdo.norm.iv_fetch.27 = load volatile i32, ptr %omp.pdo.norm.iv, align 4
  %add.9 = add nsw i32 %omp.pdo.norm.iv_fetch.27, 1
  store i32 %add.9, ptr %"foo_$I", align 4
  store i32 111, ptr %"foo_$AAA_SCALAR", align 4
  %"foo_$BBB_ARRAY[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %"foo_$BBB_ARRAY", i64 22)
  store i32 222, ptr %"foo_$BBB_ARRAY[]", align 4
  %"foo_$CCC_ARRSECT[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %"foo_$CCC_ARRSECT", i64 33)
  store i32 333, ptr %"foo_$CCC_ARRSECT[]", align 1
  %"foo_$GGG_ARRSECT_VAR_LB[]39" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %"foo_$GGG_ARRSECT_VAR_LB", i64 44)
  store i32 444, ptr %"foo_$GGG_ARRSECT_VAR_LB[]39", align 4
  %"foo_$HHH_ARRSECT_VAR_UB[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %"foo_$HHH_ARRSECT_VAR_UB", i64 55)
  store i32 555, ptr %"foo_$HHH_ARRSECT_VAR_UB[]", align 4
  %"foo_$III_VLA2[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %"foo_$III_VLA2", i64 66)
  store i32 666, ptr %"foo_$III_VLA2[]", align 4
  %"foo_$JJJ_ALLOC_ARRAY.addr_a0$40" = getelementptr inbounds %"QNCA_a0$ptr$rank1$.0", ptr %"foo_$JJJ_ALLOC_ARRAY", i64 0, i32 0
  %"foo_$JJJ_ALLOC_ARRAY.addr_a0$_fetch.28" = load ptr, ptr %"foo_$JJJ_ALLOC_ARRAY.addr_a0$40", align 8
  %"foo_$JJJ_ALLOC_ARRAY.dim_info$.lower_bound$42" = getelementptr inbounds %"QNCA_a0$ptr$rank1$.0", ptr %"foo_$JJJ_ALLOC_ARRAY", i64 0, i32 6, i64 0, i32 2
  %"foo_$JJJ_ALLOC_ARRAY.dim_info$.lower_bound$[]43" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"foo_$JJJ_ALLOC_ARRAY.dim_info$.lower_bound$42", i32 0)
  %"foo_$JJJ_ALLOC_ARRAY.dim_info$.lower_bound$[]_fetch.29" = load i64, ptr %"foo_$JJJ_ALLOC_ARRAY.dim_info$.lower_bound$[]43", align 1
  %"foo_$JJJ_ALLOC_ARRAY.addr_a0$_fetch.28[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %"foo_$JJJ_ALLOC_ARRAY.dim_info$.lower_bound$[]_fetch.29", i64 4, ptr elementtype(i32) %"foo_$JJJ_ALLOC_ARRAY.addr_a0$_fetch.28", i64 77)
  store i32 777, ptr %"foo_$JJJ_ALLOC_ARRAY.addr_a0$_fetch.28[]", align 4
  %omp.pdo.norm.iv_fetch.31 = load volatile i32, ptr %omp.pdo.norm.iv, align 4
  %add.10 = add nsw i32 %omp.pdo.norm.iv_fetch.31, 1
  store volatile i32 %add.10, ptr %omp.pdo.norm.iv, align 4
  %omp.pdo.norm.iv_fetch.32 = load volatile i32, ptr %omp.pdo.norm.iv, align 4
  %omp.pdo.norm.ub_fetch.33 = load volatile i32, ptr %omp.pdo.norm.ub, align 4
  %rel.11.not = icmp sgt i32 %omp.pdo.norm.iv_fetch.32, %omp.pdo.norm.ub_fetch.33
  br i1 %rel.11.not, label %DIR.OMP.END.DISTRIBUTE.PARLOOP.4.loopexit, label %omp.pdo.body26

DIR.OMP.DISTRIBUTE.PARLOOP.2:                     ; preds = %DIR.OMP.DISTRIBUTE.PARLOOP.1086
  %omp.pdo.norm.lb_fetch.24 = load i32, ptr %omp.pdo.norm.lb.fp, align 4
  store volatile i32 %omp.pdo.norm.lb_fetch.24, ptr %omp.pdo.norm.iv, align 4
  %omp.pdo.norm.iv_fetch.25 = load volatile i32, ptr %omp.pdo.norm.iv, align 4
  %omp.pdo.norm.ub_fetch.26 = load volatile i32, ptr %omp.pdo.norm.ub, align 4
  %rel.10.not = icmp sgt i32 %omp.pdo.norm.iv_fetch.25, %omp.pdo.norm.ub_fetch.26
  br i1 %rel.10.not, label %DIR.OMP.END.DISTRIBUTE.PARLOOP.4, label %omp.pdo.body26.preheader

omp.pdo.body26.preheader:                         ; preds = %DIR.OMP.DISTRIBUTE.PARLOOP.2
  br label %omp.pdo.body26

DIR.OMP.END.DISTRIBUTE.PARLOOP.4.loopexit:        ; preds = %omp.pdo.body26
  br label %DIR.OMP.END.DISTRIBUTE.PARLOOP.4

DIR.OMP.END.DISTRIBUTE.PARLOOP.4:                 ; preds = %DIR.OMP.END.DISTRIBUTE.PARLOOP.4.loopexit, %DIR.OMP.DISTRIBUTE.PARLOOP.1086, %DIR.OMP.DISTRIBUTE.PARLOOP.2
  br label %DIR.OMP.END.DISTRIBUTE.PARLOOP.483

DIR.OMP.END.DISTRIBUTE.PARLOOP.483:               ; preds = %DIR.OMP.END.DISTRIBUTE.PARLOOP.4
  call void @llvm.directive.region.exit(token %9) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]

  br label %DIR.OMP.END.TEAMS.5

DIR.OMP.END.TEAMS.5:                              ; preds = %DIR.OMP.END.DISTRIBUTE.PARLOOP.483, %DIR.OMP.TEAMS.1387
  br label %DIR.OMP.END.TEAMS.584

DIR.OMP.END.TEAMS.584:                            ; preds = %DIR.OMP.END.TEAMS.5
  call void @llvm.directive.region.exit(token %12) [ "DIR.OMP.END.TEAMS"() ]

  br label %DIR.OMP.END.TARGET.7

DIR.OMP.END.TARGET.7:                             ; preds = %DIR.OMP.TARGET.3, %DIR.OMP.END.TEAMS.584
  br label %DIR.OMP.END.TARGET.6

DIR.OMP.END.TARGET.6:                             ; preds = %DIR.OMP.END.TARGET.7
  call void @llvm.directive.region.exit(token %6) [ "DIR.OMP.END.TARGET"() ]

  br label %DIR.OMP.END.TARGET.785

DIR.OMP.END.TARGET.785:                           ; preds = %DIR.OMP.END.TARGET.6
  %"foo_$JJJ_ALLOC_ARRAY.flags$55" = getelementptr inbounds %"QNCA_a0$ptr$rank1$.0", ptr %"foo_$JJJ_ALLOC_ARRAY", i64 0, i32 3
  %"foo_$JJJ_ALLOC_ARRAY.flags$55_fetch.35" = load i64, ptr %"foo_$JJJ_ALLOC_ARRAY.flags$55", align 8
  %and.17 = and i64 %"foo_$JJJ_ALLOC_ARRAY.flags$55_fetch.35", 1
  %rel.12 = icmp eq i64 %and.17, 0
  br i1 %rel.12, label %dealloc.list.end39, label %dealloc.list.then38

DIR.OMP.DISTRIBUTE.PARLOOP.10:                    ; preds = %DIR.OMP.TEAMS.1387
  %"foo_$LB_fetch.20" = load i32, ptr %"foo_$LB", align 1
  %int_sext29 = sext i32 %"foo_$LB_fetch.20" to i64
  %add.6 = sub nsw i64 91, %int_sext29
  %slct.5 = call i64 @llvm.smax.i64(i64 %add.6, i64 0)
  %"foo_$GGG_ARRSECT_VAR_LB[]32" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %"foo_$GGG_ARRSECT_VAR_LB", i64 %int_sext29)
  %PtrToInt33 = ptrtoint ptr %"foo_$GGG_ARRSECT_VAR_LB[]32" to i64
  %PtrToInt34 = ptrtoint ptr %"foo_$GGG_ARRSECT_VAR_LB" to i64
  %sub.11 = sub nsw i64 %PtrToInt33, %PtrToInt34
  %div.3 = sdiv i64 %sub.11, 4
  %"foo_$UB_fetch.22" = load i32, ptr %"foo_$UB", align 1
  %7 = call i32 @llvm.smax.i32(i32 %"foo_$UB_fetch.22", i32 39)
  %8 = zext i32 %7 to i64
  %slct.6 = add nsw i64 %8, -39
  %"foo_$III_VLA2.array.elements_fetch.23" = load i64, ptr %"foo_$III_VLA2.array.elements", align 8
  %end.dir.temp = alloca i1, align 1
  br label %DIR.OMP.DISTRIBUTE.PARLOOP.8

DIR.OMP.DISTRIBUTE.PARLOOP.8:                     ; preds = %DIR.OMP.DISTRIBUTE.PARLOOP.10
  br label %DIR.OMP.DISTRIBUTE.PARLOOP.9

DIR.OMP.DISTRIBUTE.PARLOOP.9:                     ; preds = %DIR.OMP.DISTRIBUTE.PARLOOP.8
  %9 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr %"foo_$AAA_SCALAR", i32 0, i64 1),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr %"foo_$BBB_ARRAY", i32 0, i64 100),
    "QUAL.OMP.REDUCTION.ADD:ARRSECT.TYPED"(ptr %"foo_$CCC_ARRSECT", i32 0, i64 41, i64 19),
    "QUAL.OMP.REDUCTION.ADD:ARRSECT.TYPED"(ptr %"foo_$GGG_ARRSECT_VAR_LB", i32 0, i64 %slct.5, i64 %div.3),
    "QUAL.OMP.REDUCTION.ADD:ARRSECT.TYPED"(ptr %"foo_$HHH_ARRSECT_VAR_UB", i32 0, i64 %slct.6, i64 39),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr %"foo_$III_VLA2", i32 0, i64 %"foo_$III_VLA2.array.elements_fetch.23"),
    "QUAL.OMP.REDUCTION.ADD:F90_DV.TYPED"(ptr %"foo_$JJJ_ALLOC_ARRAY", %"QNCA_a0$ptr$rank1$.0" zeroinitializer, i32 0),
    "QUAL.OMP.SHARED:TYPED"(ptr null, i32 0, i64 1),
    "QUAL.OMP.SHARED:TYPED"(ptr null, i32 0, i64 1),
    "QUAL.OMP.SHARED:TYPED"(ptr null, i32 0, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %"foo_$I", i32 0, i64 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %omp.pdo.norm.lb.fp, i32 0, i64 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr null, i64 0, i64 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %omp.pdo.norm.iv, i32 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %omp.pdo.norm.ub, i32 0),
    "QUAL.OMP.JUMP.TO.END.IF"(ptr %end.dir.temp),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %"foo_$III_VLA2.array.elements", i64 0, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %"var$3", i32 0, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %"foo_$UB", i32 0, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %"foo_$LB", i32 0, i64 1) ]

  br label %DIR.OMP.DISTRIBUTE.PARLOOP.1086

DIR.OMP.DISTRIBUTE.PARLOOP.1086:                  ; preds = %DIR.OMP.DISTRIBUTE.PARLOOP.9
  %temp.load = load volatile i1, ptr %end.dir.temp, align 1
  br i1 %temp.load, label %DIR.OMP.END.DISTRIBUTE.PARLOOP.4, label %DIR.OMP.DISTRIBUTE.PARLOOP.2

DIR.OMP.TEAMS.13:                                 ; preds = %DIR.OMP.TARGET.3
  %"foo_$LB_fetch.17" = load i32, ptr %"foo_$LB", align 1
  %int_sext25 = sext i32 %"foo_$LB_fetch.17" to i64
  %add.3 = sub nsw i64 91, %int_sext25
  %slct.3 = call i64 @llvm.smax.i64(i64 %add.3, i64 0)
  %"foo_$GGG_ARRSECT_VAR_LB[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %"foo_$GGG_ARRSECT_VAR_LB", i64 %int_sext25)
  %PtrToInt = ptrtoint ptr %"foo_$GGG_ARRSECT_VAR_LB[]" to i64
  %PtrToInt27 = ptrtoint ptr %"foo_$GGG_ARRSECT_VAR_LB" to i64
  %sub.5 = sub nsw i64 %PtrToInt, %PtrToInt27
  %div.2 = sdiv i64 %sub.5, 4
  %"foo_$UB_fetch.19" = load i32, ptr %"foo_$UB", align 1
  %10 = call i32 @llvm.smax.i32(i32 %"foo_$UB_fetch.19", i32 39)
  %11 = zext i32 %10 to i64
  %slct.4 = add nsw i64 %11, -39
  %"foo_$III_VLA2.array.elements_fetch.34" = load i64, ptr %"foo_$III_VLA2.array.elements", align 8
  %end.dir.temp74 = alloca i1, align 1
  br label %DIR.OMP.TEAMS.11

DIR.OMP.TEAMS.11:                                 ; preds = %DIR.OMP.TEAMS.13
  br label %DIR.OMP.TEAMS.12

DIR.OMP.TEAMS.12:                                 ; preds = %DIR.OMP.TEAMS.11
  %12 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr %"foo_$AAA_SCALAR", i32 0, i64 1),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr %"foo_$BBB_ARRAY", i32 0, i64 100),
    "QUAL.OMP.REDUCTION.ADD:ARRSECT.TYPED"(ptr %"foo_$CCC_ARRSECT", i32 0, i64 41, i64 19),
    "QUAL.OMP.REDUCTION.ADD:ARRSECT.TYPED"(ptr %"foo_$GGG_ARRSECT_VAR_LB", i32 0, i64 %slct.3, i64 %div.2),
    "QUAL.OMP.REDUCTION.ADD:ARRSECT.TYPED"(ptr %"foo_$HHH_ARRSECT_VAR_UB", i32 0, i64 %slct.4, i64 39),
    "QUAL.OMP.SHARED:TYPED"(ptr %omp.pdo.norm.ub, i32 0, i64 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %omp.pdo.norm.lb, i32 0, i64 1),
    "QUAL.OMP.SHARED:TYPED"(ptr null, i32 0, i64 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %"foo_$III_VLA2", i32 0, i64 %"foo_$III_VLA2.array.elements_fetch.34"),
    "QUAL.OMP.SHARED:TYPED"(ptr %"foo_$JJJ_ALLOC_ARRAY", %"QNCA_a0$ptr$rank1$.0" zeroinitializer, i64 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %"foo_$UB", i32 0, i64 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %"foo_$LB", i32 0, i64 1),
    "QUAL.OMP.PRIVATE:TYPED.WILOCAL"(ptr %omp.pdo.norm.iv, i32 0, i64 1),
    "QUAL.OMP.PRIVATE:TYPED.WILOCAL"(ptr %"foo_$I", i32 0, i64 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED.WILOCAL"(ptr %"foo_$III_VLA2.array.elements", i64 0, i64 1),
    "QUAL.OMP.JUMP.TO.END.IF"(ptr %end.dir.temp74),
    "QUAL.OMP.PRIVATE:TYPED.WILOCAL"(ptr %"var$3", i32 0, i64 1) ]

  br label %DIR.OMP.TEAMS.12.split

DIR.OMP.TEAMS.12.split:                           ; preds = %DIR.OMP.TEAMS.12
  %omp.pdo.norm.lb.fp = alloca i32, align 4
  %omp.pdo.norm.lb.v = load i32, ptr %omp.pdo.norm.lb, align 4
  store i32 %omp.pdo.norm.lb.v, ptr %omp.pdo.norm.lb.fp, align 4
  br label %DIR.OMP.TEAMS.1387

DIR.OMP.TEAMS.1387:                               ; preds = %DIR.OMP.TEAMS.12.split
  %temp.load75 = load volatile i1, ptr %end.dir.temp74, align 1
  br i1 %temp.load75, label %DIR.OMP.END.TEAMS.5, label %DIR.OMP.DISTRIBUTE.PARLOOP.10

dealloc.list.then38:                              ; preds = %DIR.OMP.END.TARGET.785
  %"foo_$JJJ_ALLOC_ARRAY.addr_a0$57_fetch.36" = load ptr, ptr %fetch.1.fca.0.gep, align 8
  %"foo_$JJJ_ALLOC_ARRAY.flags$59" = getelementptr inbounds %"QNCA_a0$ptr$rank1$.0", ptr %"foo_$JJJ_ALLOC_ARRAY", i64 0, i32 3
  %"foo_$JJJ_ALLOC_ARRAY.flags$59_fetch.37" = load i64, ptr %"foo_$JJJ_ALLOC_ARRAY.flags$59", align 8
  %"foo_$JJJ_ALLOC_ARRAY.flags$59_fetch.37.tr" = trunc i64 %"foo_$JJJ_ALLOC_ARRAY.flags$59_fetch.37" to i32
  %13 = shl i32 %"foo_$JJJ_ALLOC_ARRAY.flags$59_fetch.37.tr", 1
  %int_zext61 = and i32 %13, 4
  %"foo_$JJJ_ALLOC_ARRAY.flags$59_fetch.37.tr81" = trunc i64 %"foo_$JJJ_ALLOC_ARRAY.flags$59_fetch.37" to i32
  %14 = shl i32 %"foo_$JJJ_ALLOC_ARRAY.flags$59_fetch.37.tr81", 1
  %int_zext63 = and i32 %14, 2
  %or.11 = or i32 %int_zext61, %int_zext63
  %15 = trunc i64 %"foo_$JJJ_ALLOC_ARRAY.flags$59_fetch.37" to i32
  %16 = lshr i32 %15, 3
  %int_zext65 = and i32 %16, 256
  %or.12 = or i32 %or.11, %int_zext65
  %17 = lshr i64 %"foo_$JJJ_ALLOC_ARRAY.flags$59_fetch.37", 15
  %18 = trunc i64 %17 to i32
  %int_zext69 = and i32 %18, 31457280
  %or.14 = or i32 %or.12, %int_zext69
  %19 = lshr i64 %"foo_$JJJ_ALLOC_ARRAY.flags$59_fetch.37", 15
  %20 = trunc i64 %19 to i32
  %int_zext71 = and i32 %20, 33554432
  %or.15 = or i32 %or.14, %int_zext71
  %or.16 = or i32 %or.15, 262144
  %"foo_$JJJ_ALLOC_ARRAY.reserved$73_fetch.38" = load i64, ptr %fetch.1.fca.5.gep, align 8
  %"(ptr)foo_$JJJ_ALLOC_ARRAY.reserved$73_fetch.38$" = inttoptr i64 %"foo_$JJJ_ALLOC_ARRAY.reserved$73_fetch.38" to ptr
  %func_result75 = call i32 @for_dealloc_allocatable_handle(ptr %"foo_$JJJ_ALLOC_ARRAY.addr_a0$57_fetch.36", i32 %or.16, ptr %"(ptr)foo_$JJJ_ALLOC_ARRAY.reserved$73_fetch.38$")
  %rel.13 = icmp eq i32 %func_result75, 0
  br i1 %rel.13, label %bb_new42_then, label %dealloc.list.end39

bb_new42_then:                                    ; preds = %dealloc.list.then38
  store ptr null, ptr %fetch.1.fca.0.gep, align 8
  %"foo_$JJJ_ALLOC_ARRAY.flags$_fetch.39" = load i64, ptr %"foo_$JJJ_ALLOC_ARRAY.flags$59", align 8
  %and.32 = and i64 %"foo_$JJJ_ALLOC_ARRAY.flags$_fetch.39", -2050
  store i64 %and.32, ptr %"foo_$JJJ_ALLOC_ARRAY.flags$59", align 8
  br label %dealloc.list.end39

dealloc.list.end39:                               ; preds = %bb_new42_then, %dealloc.list.then38, %DIR.OMP.END.TARGET.785
  ret void
}

declare ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8, i64, i32, ptr, i32)
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64)
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare i32 @for_alloc_allocatable_handle(i64, ptr nocapture, i32, ptr) local_unnamed_addr
declare i32 @for_dealloc_allocatable_handle(ptr nocapture readonly, i32, ptr) local_unnamed_addr
declare i64 @llvm.smax.i64(i64, i64)
declare i32 @llvm.smax.i32(i32, i32)

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 44, i32 -677940430, !"foo_", i32 11, i32 0, i32 0}
; end INTEL_CUSTOMIZATION
