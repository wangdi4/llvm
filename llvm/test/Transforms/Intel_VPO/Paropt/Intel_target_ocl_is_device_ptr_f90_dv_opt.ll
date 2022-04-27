; RUN: opt -switch-to-offload -lower-subscript -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt-optimize-data-sharing -S %s | FileCheck %s
; RUN: opt -switch-to-offload -passes='function(lower-subscript,vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring,vpo-paropt-optimize-data-sharing)' -S %s | FileCheck %s

; The test checks that is_device_ptr() clause is transformed into FIRSTPRIVATE,
; and then the FIRSTPRIVATE clause is optimized with WILOCAL.

; Original code:
; program main
;   use iso_c_binding
; contains
;   subroutine foo(P, A)
;     type(c_ptr) :: P
;     integer :: A(:)
;     !$omp target data use_device_ptr(A)
;     !$omp target is_device_ptr(A) has_device_addr(P)
;     A(2) = 20
;     !$omp end target
;     !$omp end target data
;   end subroutine foo
; end program main

; // CHECK-LABEL: @main_IP_foo_
; // CHECK-NOT: IS_DEVICE_PTR
; // CHECK-NOT: HAS_DEVICE_ADDR
; // CHECK: "QUAL.OMP.MAP.TO"(
; // CHECK: FIRSTPRIVATE:WILOCAL

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

%"ISO_C_BINDING$.btC_PTR.0" = type { i64 }
%"QNCA_a0$i32 addrspace(4)*$rank1$" = type { i32 addrspace(4)*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }

; Function Attrs: noinline nounwind uwtable
define void @MAIN__() #0 {
alloca_0:
  %"var$2" = alloca [8 x i64], align 8
  ret void
}

; Function Attrs: noinline nounwind uwtable
define void @main_IP_foo_(%"ISO_C_BINDING$.btC_PTR.0" addrspace(4)* dereferenceable(8) %P, %"QNCA_a0$i32 addrspace(4)*$rank1$" addrspace(4)* dereferenceable(72) "assumed_shape" "ptrnoalias" %A) #0 {
alloca_1:
  %"var$3" = alloca [8 x i64], align 8
  br label %bb2

bb2:                                              ; preds = %alloca_1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.DATA"(), "QUAL.OMP.USE_DEVICE_PTR:F90_DV"(%"QNCA_a0$i32 addrspace(4)*$rank1$" addrspace(4)* %A) ]
  br label %bb3

bb3:                                              ; preds = %bb2
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.HAS_DEVICE_ADDR"(%"ISO_C_BINDING$.btC_PTR.0" addrspace(4)* %P), "QUAL.OMP.IS_DEVICE_PTR:F90_DV"(%"QNCA_a0$i32 addrspace(4)*$rank1$" addrspace(4)* %A) ]
  %"A.addr_a0$" = getelementptr inbounds %"QNCA_a0$i32 addrspace(4)*$rank1$", %"QNCA_a0$i32 addrspace(4)*$rank1$" addrspace(4)* %A, i32 0, i32 0
  %addr_cast_node2 = addrspacecast i32 addrspace(4)* addrspace(4)* %"A.addr_a0$" to i32 addrspace(4)**
  %addr_cast_node_fetch = load i32 addrspace(4)*, i32 addrspace(4)** %addr_cast_node2, align 1
  %"A.dim_info$3" = getelementptr inbounds %"QNCA_a0$i32 addrspace(4)*$rank1$", %"QNCA_a0$i32 addrspace(4)*$rank1$" addrspace(4)* %A, i32 0, i32 6, i32 0
  %addr_cast_node4 = addrspacecast { i64, i64, i64 } addrspace(4)* %"A.dim_info$3" to { i64, i64, i64 }*
  %"A.dim_info$.spacing$5" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %addr_cast_node4, i32 0, i32 1
  %"A.dim_info$.spacing$[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"A.dim_info$.spacing$5", i32 0)
  %"A.dim_info$.spacing$[]_fetch" = load i64, i64* %"A.dim_info$.spacing$[]", align 1
  %"A.dim_info$" = getelementptr inbounds %"QNCA_a0$i32 addrspace(4)*$rank1$", %"QNCA_a0$i32 addrspace(4)*$rank1$" addrspace(4)* %A, i32 0, i32 6, i32 0
  %addr_cast_node = addrspacecast { i64, i64, i64 } addrspace(4)* %"A.dim_info$" to { i64, i64, i64 }*
  %"A.dim_info$.spacing$" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %addr_cast_node, i32 0, i32 1
  %"A.dim_info$.spacing$[]1" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"A.dim_info$.spacing$", i32 0)
  %"A.dim_info$.spacing$[]1_fetch" = load i64, i64* %"A.dim_info$.spacing$[]1", align 1
  %"addr_cast_node_fetch[]" = call i32 addrspace(4)* @llvm.intel.subscript.p4i32.i64.i64.p4i32.i64(i8 0, i64 1, i64 %"A.dim_info$.spacing$[]_fetch", i32 addrspace(4)* elementtype(i32) %addr_cast_node_fetch, i64 2)
  store i32 20, i32 addrspace(4)* %"addr_cast_node_fetch[]", align 1
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET.DATA"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind readnone speculatable
declare i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 %0, i64 %1, i32 %2, i64* elementtype(i64) %3, i32 %4) #2

; Function Attrs: nounwind readnone speculatable
declare i32 addrspace(4)* @llvm.intel.subscript.p4i32.i64.i64.p4i32.i64(i8 %0, i64 %1, i64 %2, i32 addrspace(4)* elementtype(i32) %3, i64 %4) #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token %0) #1

attributes #0 = { noinline nounwind uwtable "contains-openmp-target"="true" "frame-pointer"="all" "intel-lang"="fortran" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" }
attributes #1 = { nounwind }
attributes #2 = { nounwind readnone speculatable }

!omp_offload.info = !{!0}

!0 = !{i32 0, i32 2053, i32 12462035, !"main_IP_foo_", i32 8, i32 0, i32 0}
