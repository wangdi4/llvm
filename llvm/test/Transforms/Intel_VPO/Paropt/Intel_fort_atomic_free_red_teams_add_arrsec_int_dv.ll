; RUN: opt -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-atomic-free-reduction-ctrl=3 -S %s | FileCheck %s
; RUN: opt -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-atomic-free-reduction-ctrl=3 -S %s | FileCheck %s

; Check that dope vector reduction update loop is properly
; separated from the main reditem update loop
;
; Original code (used to trigger CodeExtractor assertion):
;
;   SUBROUTINE gpu_ompmod_twoei_jk(nintn)
;
;      INTEGER, INTENT(INOUT) :: nintn
;
;      integer :: NSHELL
;      INTEGER :: ii
;      double precision,DIMENSION(:,:),ALLOCATABLE :: fa2d
;
;   !$omp target teams &
;   !$omp private(ii) &
;   !$omp reduction(+:fa2d,nintn)
;
;   !$omp  distribute parallel do
;       DO ii = 1, nshell
;       END DO
;   !$omp end distribute parallel do
;
;   !$omp end target teams
;
;   END SUBROUTINE gpu_ompmod_twoei_jk
;

; CHECK-LABEL: atomic.free.red.global.update.store:
; CHECK: %red.update.isempty =
; CHECK: br i1 %red.update.isempty, label %red.update.done, label %red.update.body
; CHECK-LABEL: red.update.body:
; CHECK: br i1 %red.cpy.done{{[.0-9]*}}, label %red.update.done, label %red.update.body

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

%"QNCA_a0$i8 addrspace(4)*$rank2$" = type { ptr addrspace(4), i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }
%"QNCA_a0$double addrspace(4)*$rank2$" = type { ptr addrspace(4), i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }

@"var$2" = internal addrspace(1) global %"QNCA_a0$i8 addrspace(4)*$rank2$" { ptr addrspace(4) null, i64 0, i64 0, i64 128, i64 2, i64 0, [2 x { i64, i64, i64 }] zeroinitializer }

define void @gpu_ompmod_mp_gpu_ompmod_twoei_jk_(ptr addrspace(4) dereferenceable(4) %NINTN) {
alloca_1:
  %"$io_ctx" = alloca [8 x i64], align 16
  %"gpu_ompmod_mp_gpu_ompmod_twoei_jk_$II" = alloca i32, align 8
  %"gpu_ompmod_mp_gpu_ompmod_twoei_jk_$NSHELL" = alloca i32, align 8
  %"gpu_ompmod_mp_gpu_ompmod_twoei_jk_$FA2D" = alloca %"QNCA_a0$double addrspace(4)*$rank2$", align 8
  %"ascastB$val" = addrspacecast ptr %"gpu_ompmod_mp_gpu_ompmod_twoei_jk_$FA2D" to ptr addrspace(4)
  %"ascast$val" = addrspacecast ptr %"gpu_ompmod_mp_gpu_ompmod_twoei_jk_$II" to ptr addrspace(4)
  %"ascast$val7" = addrspacecast ptr %"gpu_ompmod_mp_gpu_ompmod_twoei_jk_$NSHELL" to ptr addrspace(4)
  %fetch.1 = load %"QNCA_a0$double addrspace(4)*$rank2$", ptr addrspace(1) @"var$2", align 1
  store %"QNCA_a0$double addrspace(4)*$rank2$" %fetch.1, ptr %"gpu_ompmod_mp_gpu_ompmod_twoei_jk_$FA2D", align 1
  %"ascastB$val.addr_a0$" = getelementptr inbounds %"QNCA_a0$double addrspace(4)*$rank2$", ptr addrspace(4) %"ascastB$val", i32 0, i32 0
  %"ascastB$val.addr_a0$_fetch.2" = load ptr addrspace(4), ptr addrspace(4) %"ascastB$val.addr_a0$", align 1
  %"ascastB$val.addr_length$" = getelementptr inbounds %"QNCA_a0$double addrspace(4)*$rank2$", ptr addrspace(4) %"ascastB$val", i32 0, i32 1
  %"ascastB$val.addr_length$2" = getelementptr inbounds %"QNCA_a0$double addrspace(4)*$rank2$", ptr addrspace(4) %"ascastB$val", i32 0, i32 1
  %"ascastB$val.addr_length$2_fetch.3" = load i64, ptr addrspace(4) %"ascastB$val.addr_length$2", align 1
  %"ascastB$val.dim_info$" = getelementptr inbounds %"QNCA_a0$double addrspace(4)*$rank2$", ptr addrspace(4) %"ascastB$val", i32 0, i32 6, i32 0
  %"ascastB$val.dim_info$.extent$" = getelementptr inbounds { i64, i64, i64 }, ptr addrspace(4) %"ascastB$val.dim_info$", i32 0, i32 0
  %"ascastB$val.dim_info$.extent$[]" = call ptr addrspace(4) @llvm.intel.subscript.p4.i64.i32.p4.i32(i8 0, i64 0, i32 24, ptr addrspace(4) elementtype(i64) %"ascastB$val.dim_info$.extent$", i32 0)
  %"ascastB$val.dim_info$.extent$[]_fetch.4" = load i64, ptr addrspace(4) %"ascastB$val.dim_info$.extent$[]", align 1
  %"ascastB$val.dim_info$4" = getelementptr inbounds %"QNCA_a0$double addrspace(4)*$rank2$", ptr addrspace(4) %"ascastB$val", i32 0, i32 6, i32 0
  %"ascastB$val.dim_info$.extent$6" = getelementptr inbounds { i64, i64, i64 }, ptr addrspace(4) %"ascastB$val.dim_info$4", i32 0, i32 0
  %"ascastB$val.dim_info$.extent$6[]" = call ptr addrspace(4) @llvm.intel.subscript.p4.i64.i32.p4.i32(i8 0, i64 0, i32 24, ptr addrspace(4) elementtype(i64) %"ascastB$val.dim_info$.extent$6", i32 1)
  %"ascastB$val.dim_info$.extent$6[]_fetch.5" = load i64, ptr addrspace(4) %"ascastB$val.dim_info$.extent$6[]", align 1
  %mul.1 = mul nsw i64 %"ascastB$val.dim_info$.extent$[]_fetch.4", %"ascastB$val.dim_info$.extent$6[]_fetch.5"
  %mul.2 = mul nsw i64 %"ascastB$val.addr_length$2_fetch.3", %mul.1
  %temp = alloca i32, align 4
  %omp.pdo.start = addrspacecast ptr %temp to ptr addrspace(4)
  store i32 1, ptr addrspace(4) %omp.pdo.start, align 1
  %temp8 = alloca i32, align 4
  %omp.pdo.end = addrspacecast ptr %temp8 to ptr addrspace(4)
  %"ascast$val7_fetch.6" = load i32, ptr addrspace(4) %"ascast$val7", align 1
  store i32 %"ascast$val7_fetch.6", ptr addrspace(4) %omp.pdo.end, align 1
  %temp9 = alloca i32, align 4
  %omp.pdo.step = addrspacecast ptr %temp9 to ptr addrspace(4)
  store i32 1, ptr addrspace(4) %omp.pdo.step, align 1
  %temp10 = alloca i64, align 8
  %omp.pdo.norm.iv = addrspacecast ptr %temp10 to ptr addrspace(4)
  %temp11 = alloca i64, align 8
  %omp.pdo.norm.lb = addrspacecast ptr %temp11 to ptr addrspace(4)
  store i64 0, ptr addrspace(4) %omp.pdo.norm.lb, align 1
  %temp12 = alloca i64, align 8
  %omp.pdo.norm.ub = addrspacecast ptr %temp12 to ptr addrspace(4)
  %omp.pdo.end_fetch.7 = load i32, ptr addrspace(4) %omp.pdo.end, align 1
  %omp.pdo.start_fetch.8 = load i32, ptr addrspace(4) %omp.pdo.start, align 1
  %sub.1 = sub nsw i32 %omp.pdo.end_fetch.7, %omp.pdo.start_fetch.8
  %omp.pdo.step_fetch.9 = load i32, ptr addrspace(4) %omp.pdo.step, align 1
  %div.1 = sdiv i32 %sub.1, %omp.pdo.step_fetch.9
  %int_sext13 = sext i32 %div.1 to i64
  store i64 %int_sext13, ptr addrspace(4) %omp.pdo.norm.ub, align 1
  br label %bb_new3

omp.pdo.cond6:                                    ; preds = %bb_new9, %omp.pdo.body7
  %omp.pdo.norm.iv_fetch.11 = load i64, ptr addrspace(4) %omp.pdo.norm.iv, align 1
  %omp.pdo.norm.ub_fetch.12 = load i64, ptr addrspace(4) %omp.pdo.norm.ub, align 1
  %rel.1 = icmp sle i64 %omp.pdo.norm.iv_fetch.11, %omp.pdo.norm.ub_fetch.12
  br i1 %rel.1, label %omp.pdo.body7, label %omp.pdo.epilog8

omp.pdo.body7:                                    ; preds = %omp.pdo.cond6
  %omp.pdo.norm.iv_fetch.13 = load i64, ptr addrspace(4) %omp.pdo.norm.iv, align 1
  %int_sext = trunc i64 %omp.pdo.norm.iv_fetch.13 to i32
  %omp.pdo.step_fetch.14 = load i32, ptr addrspace(4) %omp.pdo.step, align 1
  %mul.3 = mul nsw i32 %int_sext, %omp.pdo.step_fetch.14
  %omp.pdo.start_fetch.15 = load i32, ptr addrspace(4) %omp.pdo.start, align 1
  %add.1 = add nsw i32 %mul.3, %omp.pdo.start_fetch.15
  store i32 %add.1, ptr addrspace(4) %"ascast$val", align 1
  %omp.pdo.norm.iv_fetch.16 = load i64, ptr addrspace(4) %omp.pdo.norm.iv, align 1
  %add.2 = add nsw i64 %omp.pdo.norm.iv_fetch.16, 1
  store i64 %add.2, ptr addrspace(4) %omp.pdo.norm.iv, align 1
  br label %omp.pdo.cond6

omp.pdo.epilog8:                                  ; preds = %omp.pdo.cond6
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]

  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TEAMS"() ]

  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET"() ]

  %"ascastB$val.flags$" = getelementptr inbounds %"QNCA_a0$double addrspace(4)*$rank2$", ptr addrspace(4) %"ascastB$val", i32 0, i32 3
  %"ascastB$val.flags$_fetch.17" = load i64, ptr addrspace(4) %"ascastB$val.flags$", align 1
  %and.1 = and i64 %"ascastB$val.flags$_fetch.17", 1
  %rel.2 = icmp eq i64 %and.1, 0
  br i1 %rel.2, label %dealloc.list.end12, label %dealloc.list.then11

bb_new9:                                          ; preds = %bb_new4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %"ascast$val7", i32 0, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %"ascast$val", i32 0, i64 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %omp.pdo.step, i32 0, i64 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %omp.pdo.end, i64 0, i64 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %omp.pdo.start, i64 0, i64 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %omp.pdo.norm.lb, i64 0, i64 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %omp.pdo.norm.iv, i64 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %omp.pdo.norm.ub, i64 0) ]

  %omp.pdo.norm.lb_fetch.10 = load i64, ptr addrspace(4) %omp.pdo.norm.lb, align 1
  store i64 %omp.pdo.norm.lb_fetch.10, ptr addrspace(4) %omp.pdo.norm.iv, align 1
  br label %omp.pdo.cond6

bb_new3:                                          ; preds = %alloca_1
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %"ascastB$val", ptr addrspace(4) %"ascastB$val", i64 96, i64 32, ptr addrspace(4) null, ptr addrspace(4) null), ; MAP type: 32 = 0x20 = TARGET_PARAM (0x20)
    "QUAL.OMP.MAP.TOFROM:CHAIN"(ptr addrspace(4) %"ascastB$val", ptr addrspace(4) %"ascastB$val.addr_a0$_fetch.2", i64 %mul.2, i64 281474976711187, ptr addrspace(4) null, ptr addrspace(4) null), ; MAP type: 281474976711187 = 0x1000000000213 = MEMBER_OF_1 (0x1000000000000) | IMPLICIT (0x200) | PTR_AND_OBJ (0x10) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.MAP.TOFROM:CHAIN"(ptr addrspace(4) %"ascastB$val", ptr addrspace(4) %"ascastB$val.addr_length$", i64 88, i64 281474976710657, ptr addrspace(4) null, ptr addrspace(4) null), ; MAP type: 281474976710657 = 0x1000000000001 = MEMBER_OF_1 (0x1000000000000) | TO (0x1)
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %NINTN, ptr addrspace(4) %NINTN, i64 4, i64 547, ptr addrspace(4) null, ptr addrspace(4) null), ; MAP type: 547 = 0x223 = IMPLICIT (0x200) | TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %"ascast$val", i32 0, i64 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %"ascast$val7", i32 0, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %omp.pdo.norm.iv, i64 0, i64 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %omp.pdo.norm.lb, i64 0, i64 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %omp.pdo.norm.ub, i64 0, i64 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %omp.pdo.step, i32 0, i64 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %omp.pdo.end, i32 0, i64 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %omp.pdo.start, i32 0, i64 1) ]

  br label %bb_new4

bb_new4:                                          ; preds = %bb_new3
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr addrspace(4) %NINTN, i32 0, i64 1),
    "QUAL.OMP.REDUCTION.ADD:F90_DV.TYPED"(ptr addrspace(4) %"ascastB$val", %"QNCA_a0$double addrspace(4)*$rank2$" zeroinitializer, double 0.000000e+00),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %"ascast$val", i32 0, i64 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %omp.pdo.norm.lb, i64 0, i64 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %omp.pdo.norm.ub, i64 0, i64 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %omp.pdo.step, i32 0, i64 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %omp.pdo.end, i32 0, i64 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %omp.pdo.start, i32 0, i64 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %"ascast$val7", i32 0, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %omp.pdo.norm.iv, i64 0, i64 1) ]

  br label %bb_new9

dealloc.list.then11:                              ; preds = %omp.pdo.epilog8
  %"ascastB$val.addr_a0$15" = getelementptr inbounds %"QNCA_a0$double addrspace(4)*$rank2$", ptr addrspace(4) %"ascastB$val", i32 0, i32 0
  %"ascastB$val.addr_a0$15_fetch.18" = load ptr addrspace(4), ptr addrspace(4) %"ascastB$val.addr_a0$15", align 1
  %"ascastB$val.flags$17" = getelementptr inbounds %"QNCA_a0$double addrspace(4)*$rank2$", ptr addrspace(4) %"ascastB$val", i32 0, i32 3
  %"ascastB$val.flags$17_fetch.19" = load i64, ptr addrspace(4) %"ascastB$val.flags$17", align 1
  %and.2 = and i64 %"ascastB$val.flags$17_fetch.19", 2
  %lshr.1 = lshr i64 %and.2, 1
  %shl.1 = shl i64 %lshr.1, 2
  %int_zext = trunc i64 %shl.1 to i32
  %or.1 = or i32 0, %int_zext
  %and.4 = and i64 %"ascastB$val.flags$17_fetch.19", 1
  %and.5 = and i32 %or.1, -3
  %shl.2 = shl i64 %and.4, 1
  %int_zext19 = trunc i64 %shl.2 to i32
  %or.2 = or i32 %and.5, %int_zext19
  %and.6 = and i64 %"ascastB$val.flags$17_fetch.19", 2048
  %lshr.2 = lshr i64 %and.6, 11
  %and.7 = and i32 %or.2, -257
  %shl.3 = shl i64 %lshr.2, 8
  %int_zext21 = trunc i64 %shl.3 to i32
  %or.3 = or i32 %and.7, %int_zext21
  %and.8 = and i64 %"ascastB$val.flags$17_fetch.19", 256
  %lshr.3 = lshr i64 %and.8, 8
  %and.9 = and i32 %or.3, -2097153
  %shl.4 = shl i64 %lshr.3, 21
  %int_zext23 = trunc i64 %shl.4 to i32
  %or.4 = or i32 %and.9, %int_zext23
  %and.10 = and i64 %"ascastB$val.flags$17_fetch.19", 1030792151040
  %lshr.4 = lshr i64 %and.10, 36
  %and.11 = and i32 %or.4, -31457281
  %shl.5 = shl i64 %lshr.4, 21
  %int_zext25 = trunc i64 %shl.5 to i32
  %or.5 = or i32 %and.11, %int_zext25
  %and.12 = and i64 %"ascastB$val.flags$17_fetch.19", 1099511627776
  %lshr.5 = lshr i64 %and.12, 40
  %and.13 = and i32 %or.5, -33554433
  %shl.6 = shl i64 %lshr.5, 25
  %int_zext27 = trunc i64 %shl.6 to i32
  %or.6 = or i32 %and.13, %int_zext27
  %and.14 = and i32 %or.6, -2031617
  %or.7 = or i32 %and.14, 262144
  %"(i8 addrspace(4)*)ascastB$val.addr_a0$15_fetch.18$" = bitcast ptr addrspace(4) %"ascastB$val.addr_a0$15_fetch.18" to ptr addrspace(4)
  %"ascastB$val.reserved$" = getelementptr inbounds %"QNCA_a0$double addrspace(4)*$rank2$", ptr addrspace(4) %"ascastB$val", i32 0, i32 5
  %"ascastB$val.reserved$_fetch.20" = load i64, ptr addrspace(4) %"ascastB$val.reserved$", align 1
  %"(i8 addrspace(4)*)ascastB$val.reserved$_fetch.20$" = inttoptr i64 %"ascastB$val.reserved$_fetch.20" to ptr addrspace(4)
  %func_result = call i32 @for_dealloc_allocatable_handle(ptr addrspace(4) %"(i8 addrspace(4)*)ascastB$val.addr_a0$15_fetch.18$", i32 %or.7, ptr addrspace(4) %"(i8 addrspace(4)*)ascastB$val.reserved$_fetch.20$")
  %"ascastB$val.addr_a0$29" = getelementptr inbounds %"QNCA_a0$double addrspace(4)*$rank2$", ptr addrspace(4) %"ascastB$val", i32 0, i32 0
  %"ascastB$val.addr_a0$29_fetch.21" = load ptr addrspace(4), ptr addrspace(4) %"ascastB$val.addr_a0$29", align 1
  %rel.3 = icmp eq i32 %func_result, 0
  br i1 %rel.3, label %bb_new15_then, label %bb1_else

bb_new15_then:                                    ; preds = %dealloc.list.then11
  %"ascastB$val.addr_a0$30" = getelementptr inbounds %"QNCA_a0$double addrspace(4)*$rank2$", ptr addrspace(4) %"ascastB$val", i32 0, i32 0
  store ptr addrspace(4) null, ptr addrspace(4) %"ascastB$val.addr_a0$30", align 1
  %"ascastB$val.flags$31" = getelementptr inbounds %"QNCA_a0$double addrspace(4)*$rank2$", ptr addrspace(4) %"ascastB$val", i32 0, i32 3
  %"ascastB$val.flags$_fetch.22" = load i64, ptr addrspace(4) %"ascastB$val.flags$31", align 1
  %and.15 = and i64 %"ascastB$val.flags$_fetch.22", -2
  %"ascastB$val.flags$32" = getelementptr inbounds %"QNCA_a0$double addrspace(4)*$rank2$", ptr addrspace(4) %"ascastB$val", i32 0, i32 3
  store i64 %and.15, ptr addrspace(4) %"ascastB$val.flags$32", align 1
  %"ascastB$val.flags$33" = getelementptr inbounds %"QNCA_a0$double addrspace(4)*$rank2$", ptr addrspace(4) %"ascastB$val", i32 0, i32 3
  %"ascastB$val.flags$_fetch.23" = load i64, ptr addrspace(4) %"ascastB$val.flags$33", align 1
  %and.16 = and i64 %"ascastB$val.flags$_fetch.23", -2049
  %"ascastB$val.flags$34" = getelementptr inbounds %"QNCA_a0$double addrspace(4)*$rank2$", ptr addrspace(4) %"ascastB$val", i32 0, i32 3
  store i64 %and.16, ptr addrspace(4) %"ascastB$val.flags$34", align 1
  br label %bb2_endif

bb1_else:                                         ; preds = %dealloc.list.then11
  br label %bb2_endif

bb2_endif:                                        ; preds = %bb1_else, %bb_new15_then
  br label %dealloc.list.end12

dealloc.list.end12:                               ; preds = %bb2_endif, %omp.pdo.epilog8
  ret void
}

declare ptr addrspace(4) @llvm.intel.subscript.p4.i64.i32.p4.i32(i8, i64, i32, ptr addrspace(4), i32)
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare i32 @for_dealloc_allocatable_handle(ptr addrspace(4) nocapture readonly, i32, ptr addrspace(4))

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 66311, i32 42218157, !"_Z4main", i32 5, i32 0, i32 0}
