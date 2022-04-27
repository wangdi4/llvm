; RUN: opt -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-opt-scalar-fp=false -S %s | FileCheck %s
; RUN: opt -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-opt-scalar-fp=false -S %s | FileCheck %s

; Original code:
; PROGRAM test_target_enter_data_components_alloc
;   INTEGER, TARGET :: justATarget
;   INTEGER, POINTER :: myPtr
; CONTAINS
;   SUBROUTINE test_map_derived_type_alloc()
;     !$omp target map(alloc: myPtr)
;     myPtr => justATarget
;     !$omp end target
;   END SUBROUTINE test_map_derived_type_alloc
; END PROGRAM test_target_enter_data_components_alloc

; Check that firsrprivatization for an addressspacecasted GEP works:
; CHECK: define weak dso_local spir_kernel void @__omp_offloading_805_73_test_target_enter_data_components_alloc_IP_test_map_derived_type_alloc__l6(i32 addrspace(4)* addrspace(1)*{{[^,]*}}, i32 addrspace(1)*{{[^,]*}})

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"
target device_triples = "spir64"

%uplevel_type.0 = type { i32, i32 }
define void @MAIN__() #0 {
alloca:
  %"var$1" = alloca [8 x i64], align 8
  %uplevel_rec = alloca %uplevel_type.0
  %ul_loc_0.0_var = getelementptr inbounds %uplevel_type.0, %uplevel_type.0* %uplevel_rec, i32 0, i32 0
  %0 = addrspacecast i32* %ul_loc_0.0_var to i32 addrspace(4)*
  %"test_target_enter_data_components_alloc_$MYPTR" = bitcast i32 addrspace(4)* %0 to i32 addrspace(4)* addrspace(4)*
  %ul_loc_1.1_var = getelementptr inbounds %uplevel_type.0, %uplevel_type.0* %uplevel_rec, i32 0, i32 1
  %1 = addrspacecast i32* %ul_loc_1.1_var to i32 addrspace(4)*
  br label %bb1

bb1:                                              ; preds = %alloca
  ret void
}
define void @test_target_enter_data_components_alloc_IP_test_map_derived_type_alloc_(%uplevel_type.0* nest %uplevel_ptr0) #0 {
alloca:
  %"var$2" = alloca [8 x i64], align 8
  %ul_loc_0.2_var = getelementptr inbounds %uplevel_type.0, %uplevel_type.0* %uplevel_ptr0, i32 0, i32 0
  %0 = addrspacecast i32* %ul_loc_0.2_var to i32 addrspace(4)*
  %"test_map_derived_type_alloc$MYPTR$_0" = bitcast i32 addrspace(4)* %0 to i32 addrspace(4)* addrspace(4)*
  %ul_loc_1.3_var = getelementptr inbounds %uplevel_type.0, %uplevel_type.0* %uplevel_ptr0, i32 0, i32 1
  %1 = addrspacecast i32* %ul_loc_1.3_var to i32 addrspace(4)*
  br label %bb3

bb3:                                              ; preds = %alloca
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.ALLOC"(i32 addrspace(4)* addrspace(4)* %"test_map_derived_type_alloc$MYPTR$_0", i32 addrspace(4)* addrspace(4)* %"test_map_derived_type_alloc$MYPTR$_0", i64 4, i64 32, i8 addrspace(4)* null, i8 addrspace(4)* null), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %1) ]
  br label %bb4

bb4:                                              ; preds = %bb3
  store i32 addrspace(4)* %1, i32 addrspace(4)* addrspace(4)* %"test_map_derived_type_alloc$MYPTR$_0"
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TARGET"() ]
  br label %bb2

bb2:                                              ; preds = %bb4
  ret void
}
; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1
; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token %0) #1

attributes #0 = { "contains-openmp-target"="true" "min-legal-vector-width"="0" }
attributes #1 = { nounwind }

!omp_offload.info = !{!0}

!0 = !{i32 0, i32 2053, i32 115, !"test_target_enter_data_components_alloc_IP_test_map_derived_type_alloc_", i32 6, i32 0, i32 0}
