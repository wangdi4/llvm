; REQUIRES: asserts
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -debug-only=WRegionUtils,vpo-paropt-transform -vpo-paropt-opt-scalar-fp=false -S %s 2>&1 | FileCheck %s
; RUN: opt -aa-pipeline=basic-aa -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -debug-only=WRegionUtils,vpo-paropt-transform -vpo-paropt-opt-scalar-fp=false -S %s 2>&1 | FileCheck %s

; Test src:

; #include <omp.h>
; #include <stdio.h>
; int main()
; {
;   int n_dims = 2;
;   int tmp[n_dims];
;   tmp[0] = 10;
;
; #pragma omp target defaultmap(tofrom:scalar)
;   {
; #pragma omp parallel private(tmp)
;     {
;       printf("%d\n", tmp[0]);
;     }
;   }
;   printf("%d\n", tmp[1]);
; }

; This test contains the host IR corresponding to target_par_priv_vla_tgt.ll.

; Check that we capture the VLA size expression at both parallel and target constructs.
; CHECK: VPOParopt Transform: PARALLEL construct
; CHECK: collectNonPointerValuesToBeUsedInOutlinedRegion: Non-pointer values to be passed into the outlined region: 'i64 %5 '
; CHECK: captureAndAddCollectedNonPointerValuesToSharedClause: Added implicit shared/map(to)/firstprivate clause for: 'ptr [[SIZE_ADDR1:%[^ ]+]]'

; CHECK: VPOParopt Transform: TARGET construct
; CHECK: collectNonPointerValuesToBeUsedInOutlinedRegion: Non-pointer values to be passed into the outlined region: 'i64 %1 i64 %1 '
; CHECK: captureAndAddCollectedNonPointerValuesToSharedClause: Added implicit shared/map(to)/firstprivate clause for: 'ptr [[SIZE_ADDR2:%[^ ]+]]'
; CHECK: captureAndAddCollectedNonPointerValuesToSharedClause: Added implicit shared/map(to)/firstprivate clause for: 'ptr [[SIZE_ADDR3:%[^ ]+]]'

; Check that the map-type for the VLA (second one) is 161 (TO|PRIVATE)
; CHECK: @.offload_maptypes = private unnamed_addr constant [4 x i64] [i64 160, i64 161, i64 161, i64 161]

; Check that we capture vla size and emit a map for the firstprivate VLA on target.
; CHECK: define dso_local i32 @main()
; CHECK: [[SIZE_ADDR3]] = alloca i64, align 8
; CHECK: [[SIZE_ADDR2]] = alloca i64, align 8
; CHECK: [[N_DIMS:%n_dims]] = alloca i32, align 4
; CHECK: store i32 2, ptr [[N_DIMS]], align 4
; CHECK: [[LOAD_DIMS:%[0-9]+]] = load i32, ptr [[N_DIMS]], align 4
; CHECK: [[DIMS_EXT:%[0-9]+]] = zext i32 [[LOAD_DIMS]] to i64
; CHECK: [[SIZE_IN_BYTES:%.+]] = mul i64 [[DIMS_EXT]], 4

; CHECK: [[BASE_GEP:%.+]] = getelementptr inbounds [4 x ptr], ptr %.offload_baseptrs, i32 0, i32 0
; CHECK: store ptr %vla, ptr [[BASE_GEP]], align 8
; CHECK: [[START_GEP:%.+]] = getelementptr inbounds [4 x ptr], ptr %.offload_ptrs, i32 0, i32 0
; CHECK: store ptr %vla, ptr [[START_GEP]], align 8
; CHECK: [[SIZE_GEP:%.+]] = getelementptr inbounds [4 x i64], ptr %.offload_sizes, i32 0, i32 0
; CHECK: store i64 [[SIZE_IN_BYTES]], ptr [[SIZE_GEP]], align 8

; Check that the captured VLA size is passed in to the outlined function for the target region.
; CHECK: call void @__omp_offloading{{.*}}main{{.*}}(ptr %vla, ptr %omp.vla.tmp, ptr [[SIZE_ADDR2]], ptr [[SIZE_ADDR3]])

; Check that the captured VLA size is used in the parallel region for allocation of the private VLA.
; CHECK: define internal void @main.DIR.OMP.PARALLEL{{.*}}(ptr %tid, ptr %bid, ptr [[SIZE_ADDR1]], ptr %omp.vla.tmp1)
; CHECK: [[SIZE_VLA1:%[^ ]+]] = load i64, ptr [[SIZE_ADDR1]], align 8
; CHECK: %vla.priv = alloca i32, i64 [[SIZE_VLA1]], align 16

; Check that the captured VLA size is used in the target region for allocation of the private VLA.
; CHECK: define internal void @__omp_offloading{{.*}}main{{.*}}(ptr noalias %vla, ptr %omp.vla.tmp, ptr noalias [[SIZE_ADDR2]], ptr noalias [[SIZE_ADDR3]])
; CHECK: [[SIZE_ADDR3]].fpriv = alloca i64, align 8
; CHECK: [[SIZE_ADDR2]].fpriv = alloca i64, align 8
; CHECK: [[SIZE_ADDR1:%.addr]] = alloca i64, align 8
; CHECK: [[SIZE_VAL3:%[0-9]+]] = load i64, ptr [[SIZE_ADDR3]], align 8
; CHECK: store i64 [[SIZE_VAL3]], ptr [[SIZE_ADDR3]].fpriv, align 8
; CHECK: [[SIZE_VAL2:%.+]] = load i64, ptr [[SIZE_ADDR2]], align 8
; CHECK: store i64 [[SIZE_VAL2]], ptr [[SIZE_ADDR2]].fpriv, align 8
; CHECK: [[SIZE_FP_VAL3:%.+]] = load i64, ptr [[SIZE_ADDR3]].fpriv, align 8
; CHECK: [[SIZE_FP_VAL2:%.+]] = load i64, ptr [[SIZE_ADDR2]].fpriv, align 8
; CHECK: %vla.priv = alloca i32, i64 [[SIZE_FP_VAL2]], align 16

; Check that the captured VLA size is passed in to the outlined function for the parallel region.
; CHECK: [[VLA_TMP_FP_VAL:%[0-9]+]] = load i64, ptr %omp.vla.tmp.fpriv, align 8
; CHECK: store i64 [[VLA_TMP_FP_VAL]], ptr %omp.vla.tmp1.priv, align 8
; CHECK: store i64 [[VLA_TMP_FP_VAL]], ptr [[SIZE_ADDR1]], align 8
; CHECK: call void (ptr, i32, ptr, ...) @__kmpc_fork_call(ptr {{.+}}, i32 2, ptr @main.DIR.OMP.PARALLEL{{.*}}, ptr [[SIZE_ADDR1]], ptr %omp.vla.tmp1.priv)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

@.str = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1
@llvm.global_ctors = appending global [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 0, ptr @.omp_offloading.requires_reg, ptr null }]

define dso_local i32 @main() {
entry:
  %n_dims = alloca i32, align 4
  %saved_stack = alloca ptr, align 8
  %__vla_expr0 = alloca i64, align 8
  %omp.vla.tmp = alloca i64, align 8
  %omp.vla.tmp1 = alloca i64, align 8
  store i32 2, ptr %n_dims, align 4
  %0 = load i32, ptr %n_dims, align 4
  %1 = zext i32 %0 to i64
  %2 = call ptr @llvm.stacksave()
  store ptr %2, ptr %saved_stack, align 8
  %vla = alloca i32, i64 %1, align 16
  store i64 %1, ptr %__vla_expr0, align 8
  %arrayidx = getelementptr inbounds i32, ptr %vla, i64 0
  store i32 10, ptr %arrayidx, align 16
  store i64 %1, ptr %omp.vla.tmp, align 8
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.DEFAULTMAP.TOFROM:SCALAR"(),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %vla, i32 0, i64 %1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %omp.vla.tmp, i64 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %omp.vla.tmp1, i64 0, i32 1) ]

  %4 = load i64, ptr %omp.vla.tmp, align 8
  store i64 %4, ptr %omp.vla.tmp1, align 8
  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %vla, i32 0, i64 %4),
    "QUAL.OMP.SHARED:TYPED"(ptr %omp.vla.tmp1, i64 0, i32 1) ]

  %6 = load i64, ptr %omp.vla.tmp1, align 8
  %arrayidx2 = getelementptr inbounds i32, ptr %vla, i64 0
  %7 = load i32, ptr %arrayidx2, align 16
  %call = call i32 (ptr, ...) @printf(ptr noundef @.str, i32 noundef %7) #2
  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.PARALLEL"() ]

  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.TARGET"() ]

  %arrayidx3 = getelementptr inbounds i32, ptr %vla, i64 1
  %8 = load i32, ptr %arrayidx3, align 4
  %call4 = call i32 (ptr, ...) @printf(ptr noundef @.str, i32 noundef %8)
  %9 = load ptr, ptr %saved_stack, align 8
  call void @llvm.stackrestore(ptr %9)
  ret i32 0
}

declare ptr @llvm.stacksave()
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare dso_local i32 @printf(ptr noundef, ...)
declare void @llvm.stackrestore(ptr)
declare void @.omp_offloading.requires_reg()
declare void @__tgt_register_requires(i64)

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 66312, i32 226246865, !"_Z4main", i32 9, i32 0, i32 0, i32 0}
