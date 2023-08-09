; RUN: opt -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src:
;
; struct {
; } a;
; void b() {
; #pragma omp target private(a)
;   ;
; }

; Check that we are casting the parameters of _constr and destr to addrspace(4)
; CHECK: %{{.*}} = call spir_func ptr addrspace(4) @"_ZTS3$_0.omp.def_constr"(ptr addrspace(4) addrspacecast (ptr addrspace(1) @a.priv.__global to ptr addrspace(4)))
; CHECK: call spir_func void @"_ZTS3$_0.omp.destr"(ptr addrspace(4) addrspacecast (ptr addrspace(1) @a.priv.__global to ptr addrspace(4)))

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

%struct.anon = type { i8 }

@a = external addrspace(1) global %struct.anon, align 1

; Function Attrs: convergent mustprogress noinline nounwind optnone
define protected spir_func void @_Z1bv() #0 {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.PRIVATE:NONPOD.TYPED"(ptr addrspace(4) addrspacecast (ptr addrspace(1) @a to ptr addrspace(4)), %struct.anon zeroinitializer, i32 1, ptr @"_ZTS3$_0.omp.def_constr", ptr @"_ZTS3$_0.omp.destr") ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: convergent noinline nounwind
declare spir_func noundef ptr addrspace(4) @"_ZTS3$_0.omp.def_constr"(ptr addrspace(4) noundef %0) #2

; Function Attrs: convergent noinline nounwind
declare spir_func void @"_ZTS3$_0.omp.destr"(ptr addrspace(4) noundef %0) #2

attributes #0 = { convergent mustprogress noinline nounwind optnone "approx-func-fp-math"="true" "contains-openmp-target"="true" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }
attributes #2 = { convergent noinline nounwind "approx-func-fp-math"="true" "frame-pointer"="all" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "openmp-target-declare"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4, !5}
!opencl.compiler.options = !{!6}

!0 = !{i32 0, i32 53, i32 -1926233775, !"_Z1bv", i32 4, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 51}
!3 = !{i32 7, !"openmp-device", i32 51}
!4 = !{i32 7, !"PIC Level", i32 2}
!5 = !{i32 7, !"frame-pointer", i32 2}
!6 = !{}
