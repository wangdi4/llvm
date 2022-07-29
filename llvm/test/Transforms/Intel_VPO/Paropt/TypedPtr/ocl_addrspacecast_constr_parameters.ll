; RUN: opt -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src:

; struct {
; } a;
; b() {
; #pragma omp target private(a)
;   ;
; }

; Check that we are casting the parameters of _constr and destr to addrspace(4)
; CHECK: %{{.*}} = call spir_func %struct.anon addrspace(4)* @"_ZTS3$_0.omp.def_constr"(%struct.anon addrspace(4)* addrspacecast (%struct.anon addrspace(1)* @a.priv.__global to %struct.anon addrspace(4)*))
; CHECK:  call spir_func void @"_ZTS3$_0.omp.destr"(%struct.anon addrspace(4)* addrspacecast (%struct.anon addrspace(1)* @a.priv.__global to %struct.anon addrspace(4)*))

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

%struct.anon = type { i8 }

@a = external addrspace(1) global %struct.anon, align 1

; Function Attrs: convergent mustprogress noinline nounwind optnone
define hidden spir_func i32 @_Z1bv() #0 {
entry:
  %retval = alloca i32, align 4
  %retval.ascast = addrspacecast i32* %retval to i32 addrspace(4)*

  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.PRIVATE:NONPOD"(%struct.anon addrspace(4)* addrspacecast (%struct.anon addrspace(1)* @a to %struct.anon addrspace(4)*), %struct.anon addrspace(4)* (%struct.anon addrspace(4)*)* @"_ZTS3$_0.omp.def_constr", void (%struct.anon addrspace(4)*)* @"_ZTS3$_0.omp.destr") ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  %1 = load i32, i32 addrspace(4)* %retval.ascast, align 4
  ret i32 %1
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: convergent noinline nounwind
declare spir_func %struct.anon addrspace(4)* @"_ZTS3$_0.omp.def_constr"(%struct.anon addrspace(4)* %0) #2
; Function Attrs: convergent noinline nounwind
declare spir_func void @"_ZTS3$_0.omp.destr"(%struct.anon addrspace(4)* %0) #2

attributes #0 = { convergent mustprogress noinline nounwind optnone "contains-openmp-target"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }
attributes #2 = { convergent noinline nounwind "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "openmp-target-declare"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4, !5}
!opencl.used.extensions = !{!6}
!opencl.used.optional.core.features = !{!6}
!opencl.compiler.options = !{!6}

!0 = !{i32 0, i32 66313, i32 64500121, !"_Z1bv", i32 4, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 50}
!3 = !{i32 7, !"openmp-device", i32 50}
!4 = !{i32 7, !"PIC Level", i32 2}
!5 = !{i32 7, !"frame-pointer", i32 2}
!6 = !{}
