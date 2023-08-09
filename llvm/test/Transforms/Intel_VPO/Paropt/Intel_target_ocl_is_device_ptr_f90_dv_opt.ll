; INTEL_CUSTOMIZATION
; RUN: opt -bugpoint-enable-legacy-pm -switch-to-offload -lower-subscript -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt-optimize-data-sharing -S %s | FileCheck %s
; RUN: opt -switch-to-offload -passes='function(lower-subscript,vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring,vpo-paropt-optimize-data-sharing)' -S %s | FileCheck %s

; The test checks that is_device_ptr() clause is transformed into FIRSTPRIVATE,
; and then the FIRSTPRIVATE clause is optimized with WILOCAL.

; Test src:

; Original code:
; program main
; contains
;   subroutine foo(A)
;     integer :: A(:)
;     !$omp target data use_device_ptr(A)
;     !$omp target is_device_ptr(A)
;     A(2) = 20
;     !$omp end target
;     !$omp end target data
;   end subroutine foo
; end program main

; The frontend sends a map(TO|PRIVATE) and FIRSTPRIVATE for is_device_ptr on F90_DVs.
; We make sure we can make it WILocal here.
; // CHECK-LABEL: @main_IP_foo_
; // CHECK: "QUAL.OMP.MAP.TO"(
; // CHECK: FIRSTPRIVATE:TYPED.WILOCAL

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

%"QNCA_a0$i32 addrspace(4)*$rank1$" = type { ptr addrspace(4), i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }

; Function Attrs: noinline nounwind optnone uwtable
define void @MAIN__() #0 {
alloca_0:
  %"$io_ctx" = alloca [8 x i64], align 8
  ret void
}

; Function Attrs: noinline nounwind optnone uwtable
define void @main_IP_foo_(ptr addrspace(4) noalias dereferenceable(72) "assumed_shape" "ptrnoalias" %"A$argptr") #0 {
alloca_1:
  %"$io_ctx" = alloca [8 x i64], align 8
  %"A$locptr" = alloca ptr addrspace(4), align 8
  store ptr addrspace(4) %"A$argptr", ptr %"A$locptr", align 1
  %A.1 = load ptr addrspace(4), ptr %"A$locptr", align 1
  br label %bb_new3

bb_new3:                                          ; preds = %alloca_1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.DATA"(),
    "QUAL.OMP.USE_DEVICE_PTR:F90_DV.TYPED"(ptr addrspace(4) %A.1, %"QNCA_a0$i32 addrspace(4)*$rank1$" zeroinitializer, i32 0) ]
  br label %bb_new4

bb_new4:                                          ; preds = %bb_new3
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TO"(ptr addrspace(4) %A.1, ptr addrspace(4) %A.1, i64 72, i64 161, ptr addrspace(4) null, ptr addrspace(4) null),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %A.1, %"QNCA_a0$i32 addrspace(4)*$rank1$" zeroinitializer, i32 1) ]

  %"A.1.addr_a0$" = getelementptr inbounds %"QNCA_a0$i32 addrspace(4)*$rank1$", ptr addrspace(4) %A.1, i32 0, i32 0
  %"A.1.addr_a0$_fetch.2" = load ptr addrspace(4), ptr addrspace(4) %"A.1.addr_a0$", align 1
  %"A.1.dim_info$" = getelementptr inbounds %"QNCA_a0$i32 addrspace(4)*$rank1$", ptr addrspace(4) %A.1, i32 0, i32 6, i32 0
  %"A.1.dim_info$.spacing$" = getelementptr inbounds { i64, i64, i64 }, ptr addrspace(4) %"A.1.dim_info$", i32 0, i32 1
  %"A.1.dim_info$.spacing$[]" = call ptr addrspace(4) @llvm.intel.subscript.p4.i64.i32.p4.i32(i8 0, i64 0, i32 24, ptr addrspace(4) elementtype(i64) %"A.1.dim_info$.spacing$", i32 0)
  %"A.1.dim_info$.spacing$[]_fetch.3" = load i64, ptr addrspace(4) %"A.1.dim_info$.spacing$[]", align 1
  %"A.1.dim_info$1" = getelementptr inbounds %"QNCA_a0$i32 addrspace(4)*$rank1$", ptr addrspace(4) %A.1, i32 0, i32 6, i32 0
  %"A.1.dim_info$.spacing$2" = getelementptr inbounds { i64, i64, i64 }, ptr addrspace(4) %"A.1.dim_info$1", i32 0, i32 1
  %"A.1.dim_info$.spacing$[]3" = call ptr addrspace(4) @llvm.intel.subscript.p4.i64.i32.p4.i32(i8 0, i64 0, i32 24, ptr addrspace(4) elementtype(i64) %"A.1.dim_info$.spacing$2", i32 0)
  %"A.1.dim_info$.spacing$[]_fetch.4" = load i64, ptr addrspace(4) %"A.1.dim_info$.spacing$[]3", align 1
  %"A.1.addr_a0$_fetch.2[]" = call ptr addrspace(4) @llvm.intel.subscript.p4.i64.i64.p4.i64(i8 0, i64 1, i64 %"A.1.dim_info$.spacing$[]_fetch.3", ptr addrspace(4) elementtype(i32) %"A.1.addr_a0$_fetch.2", i64 2)
  store i32 20, ptr addrspace(4) %"A.1.addr_a0$_fetch.2[]", align 1

  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET.DATA"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token %0) #1

; Function Attrs: nounwind readnone speculatable
declare ptr addrspace(4) @llvm.intel.subscript.p4.i64.i32.p4.i32(i8 %0, i64 %1, i32 %2, ptr addrspace(4) %3, i32 %4) #2

; Function Attrs: nounwind readnone speculatable
declare ptr addrspace(4) @llvm.intel.subscript.p4.i64.i64.p4.i64(i8 %0, i64 %1, i64 %2, ptr addrspace(4) %3, i64 %4) #2

attributes #0 = { noinline nounwind optnone uwtable "contains-openmp-target"="true" "frame-pointer"="all" "intel-lang"="fortran" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" }
attributes #1 = { nounwind }
attributes #2 = { nounwind readnone speculatable }

!omp_offload.info = !{!0}

!0 = !{i32 0, i32 66313, i32 186596919, !"main_IP_foo_", i32 6, i32 0, i32 0}
; end INTEL_CUSTOMIZATION
