; REQUIRES: asserts
; RUN: opt -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -debug-only=WRegionUtils,vpo-paropt-transform -vpo-paropt-opt-scalar-fp=false -S %s 2>&1 | FileCheck %s
; RUN: opt -switch-to-offload -aa-pipeline=basic-aa -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -debug-only=WRegionUtils,vpo-paropt-transform -vpo-paropt-opt-scalar-fp=false -S %s 2>&1 | FileCheck %s

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

; This test contains the host IR corresponding to target_par_priv_vla_host.ll.

; Check that we capture the VLA size expression at the target construct.
; CHECK: VPOParopt Transform: TARGET construct
; CHECK: collectNonPointerValuesToBeUsedInOutlinedRegion: Non-pointer values to be passed into the outlined region: 'i64 %1 i64 %1 '
; CHECK: captureAndAddCollectedNonPointerValuesToSharedClause: Added implicit shared/map(to)/firstprivate clause for: 'ptr addrspace(4) [[SIZE_ADDR1:%[^ ]+]]'
; CHECK: captureAndAddCollectedNonPointerValuesToSharedClause: Added implicit shared/map(to)/firstprivate clause for: 'ptr addrspace(4) [[SIZE_ADDR2:%[^ ]+]]'

; CHECK: define weak dso_local spir_kernel void @__omp_offloading_10308_d7c40d1__Z4main_l9(ptr addrspace(1) noalias %vla.ascast, ptr addrspace(1) %omp.vla.tmp.ascast, ptr noalias byval(<{ [1 x i64] }>) [[SIZE_ADDR1]], ptr noalias byval(<{ [1 x i64] }>) [[SIZE_ADDR2]])
; CHECK: [[SIZE_ADDR2]].fpriv = alloca i64, align 8
; CHECK: [[SIZE_ADDR1]].fpriv = alloca i64, align 8
; CHECK: [[VLA_TMP_FPRIV:%.+]] = alloca i64, align 8
; CHECK: [[SIZE_ARG_LOAD2:%.+]] = load i64, ptr [[SIZE_ADDR2]], align 8
; CHECK: store i64 [[SIZE_ARG_LOAD2]], ptr [[SIZE_ADDR2]].fpriv, align 8
; CHECK: [[SIZE_ARG_LOAD1:%.+]] = load i64, ptr [[SIZE_ADDR1]], align 8
; CHECK: store i64 [[SIZE_ARG_LOAD1]], ptr [[SIZE_ADDR1]].fpriv, align 8

; Make sure that we don't allocate a WILocal copy for %vla.ascast for the firstprivate clause on target.
; CHECK-NOT:   %vla.ascast.fpriv
; Check that the captured VLA size is used in the target region for allocation of the private VLA (for the inner parallel).
; CHECK: [[VLA_TMP:%.+]] = load i64, ptr addrspace(1) %omp.vla.tmp.ascast, align 8
; CHECK: store i64 [[VLA_TMP]], ptr [[VLA_TMP_FPRIV]], align 8
; CHECK: [[VLA_TMP_FPRIV_VAL:%.+]] = load i64, ptr [[VLA_TMP_FPRIV]], align 8
; CHECK: %vla.ascast.priv = alloca i32, i64 [[VLA_TMP_FPRIV_VAL]], align 4

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

@.str = private unnamed_addr addrspace(2) constant [4 x i8] c"%d\0A\00", align 1
@.str.1 = private unnamed_addr addrspace(1) constant [4 x i8] c"%d\0A\00", align 1

define i32 @main() {
entry:
  %retval = alloca i32, align 4
  %n_dims = alloca i32, align 4
  %saved_stack = alloca ptr, align 8
  %__vla_expr0 = alloca i64, align 8
  %omp.vla.tmp = alloca i64, align 8
  %omp.vla.tmp1 = alloca i64, align 8
  %retval.ascast = addrspacecast ptr %retval to ptr addrspace(4)
  %n_dims.ascast = addrspacecast ptr %n_dims to ptr addrspace(4)
  %saved_stack.ascast = addrspacecast ptr %saved_stack to ptr addrspace(4)
  %__vla_expr0.ascast = addrspacecast ptr %__vla_expr0 to ptr addrspace(4)
  %omp.vla.tmp.ascast = addrspacecast ptr %omp.vla.tmp to ptr addrspace(4)
  %omp.vla.tmp1.ascast = addrspacecast ptr %omp.vla.tmp1 to ptr addrspace(4)
  store i32 2, ptr addrspace(4) %n_dims.ascast, align 4
  %0 = load i32, ptr addrspace(4) %n_dims.ascast, align 4
  %1 = zext i32 %0 to i64
  %2 = call ptr @llvm.stacksave()
  store ptr %2, ptr addrspace(4) %saved_stack.ascast, align 8
  %vla = alloca i32, i64 %1, align 4
  %vla.ascast = addrspacecast ptr %vla to ptr addrspace(4)
  store i64 %1, ptr addrspace(4) %__vla_expr0.ascast, align 8
  %arrayidx = getelementptr inbounds i32, ptr addrspace(4) %vla.ascast, i64 0
  store i32 10, ptr addrspace(4) %arrayidx, align 4
  store i64 %1, ptr addrspace(4) %omp.vla.tmp.ascast, align 8
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.DEFAULTMAP.TOFROM:SCALAR"(),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %vla.ascast, i32 0, i64 %1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %omp.vla.tmp.ascast, i64 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %omp.vla.tmp1.ascast, i64 0, i32 1) ]

  %4 = load i64, ptr addrspace(4) %omp.vla.tmp.ascast, align 8
  store i64 %4, ptr addrspace(4) %omp.vla.tmp1.ascast, align 8
  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %vla.ascast, i32 0, i64 %4),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %omp.vla.tmp1.ascast, i64 0, i32 1) ]

  %6 = load i64, ptr addrspace(4) %omp.vla.tmp1.ascast, align 8
  %arrayidx2 = getelementptr inbounds i32, ptr addrspace(4) %vla.ascast, i64 0
  %7 = load i32, ptr addrspace(4) %arrayidx2, align 4
  %8 = call i32 (ptr addrspace(2), ...) @_Z18__spirv_ocl_printfPU3AS2cz(ptr addrspace(2) @.str, i32 %7)
  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.PARALLEL"() ]

  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.TARGET"() ]

  %arrayidx3 = getelementptr inbounds i32, ptr addrspace(4) %vla.ascast, i64 1
  %9 = load i32, ptr addrspace(4) %arrayidx3, align 4
  %call = call spir_func i32 (ptr addrspace(4), ...) @printf(ptr addrspace(4) noundef addrspacecast (ptr addrspace(1) @.str.1 to ptr addrspace(4)), i32 noundef %9) #4
  %10 = load ptr, ptr addrspace(4) %saved_stack.ascast, align 8
  call void @llvm.stackrestore(ptr %10)
  ret i32 0
}

declare ptr @llvm.stacksave()
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare spir_func i32 @_Z18__spirv_ocl_printfPU3AS2cz(ptr addrspace(2), ...)
declare spir_func i32 @printf(ptr addrspace(4) noundef, ...)
declare void @llvm.stackrestore(ptr)

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 66312, i32 226246865, !"_Z4main", i32 9, i32 0, i32 0, i32 0}
