; RUN: opt -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-loop-collapse -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt-optimize-data-sharing -vpo-paropt -vpo-paropt-atomic-free-reduction=false -S %s | FileCheck --check-prefixes=DEF %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse,vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring,vpo-paropt-optimize-data-sharing),vpo-paropt' -switch-to-offload -vpo-paropt-atomic-free-reduction=false -S %s | FileCheck --check-prefixes=DEF %s
; RUN: opt -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-loop-collapse -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt-optimize-data-sharing -vpo-paropt -vpo-paropt-kernel-args-size-limit=0 -vpo-paropt-atomic-free-reduction=false -S %s | FileCheck --check-prefixes=DIS %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse,vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring,vpo-paropt-optimize-data-sharing),vpo-paropt' -switch-to-offload -vpo-paropt-kernel-args-size-limit=0 -vpo-paropt-atomic-free-reduction=false -S %s | FileCheck --check-prefixes=DIS %s
; RUN: opt -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-loop-collapse -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt-optimize-data-sharing -vpo-paropt -vpo-paropt-kernel-args-size-limit=1 -vpo-paropt-atomic-free-reduction=false -S %s | FileCheck --check-prefixes=PART %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse,vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring,vpo-paropt-optimize-data-sharing),vpo-paropt' -switch-to-offload -vpo-paropt-kernel-args-size-limit=1 -vpo-paropt-atomic-free-reduction=false -S %s | FileCheck --check-prefixes=PART %s

; Original code:
; struct s1 {
;   char a;
; };
; struct s2 {
;   double d;
; };
; void foo() {
;   struct s1 a;
;   struct s2 d;
; #pragma omp target firstprivate(a) firstprivate(d)
;   {(void)a; (void)d;}
;}

; By default both arguments must be passed by value:
; DEF: @__omp_offloading_805_be228f__Z3foo_l10_kernel_info = weak target_declare addrspace(1) constant %0 { i32 4, i32 2, [2 x %1] [%1 { i32 1, i32 1 }, %1 { i32 1, i32 8 }], i64 0, i64 0, i64 0 }
; DEF: define weak dso_local spir_kernel void @__omp_offloading_805_be228f__Z3foo_l10(
; DEF-SAME: <{ [1 x i8] }>* byval(<{ [1 x i8] }>){{[% A-Za-z_.0-9]*}},
; DEF-SAME: <{ [1 x i64] }>* byval(<{ [1 x i64] }>){{[% A-Za-z_.0-9]*}})

; By value passing is disabled:
; DIS: %struct.s1 = type { i8 }
; DIS: %struct.s2 = type { double }
; DIS: @__omp_offloading_805_be228f__Z3foo_l10_kernel_info = weak target_declare addrspace(1) constant %0 { i32 4, i32 2, [2 x %1] [%1 { i32 0, i32 8 }, %1 { i32 0, i32 8 }], i64 0, i64 0, i64 0 }
; DIS: define weak dso_local spir_kernel void @__omp_offloading_805_be228f__Z3foo_l10(
; DIS-SAME: %struct.s1 addrspace(1)*{{[% A-Za-z_.0-9]*}},
; DIS-SAME: %struct.s2 addrspace(1)*{{[% A-Za-z_.0-9]*}})

; Only the first argument must be passed by value:
; PART: %struct.s2 = type { double }
; PART: @__omp_offloading_805_be228f__Z3foo_l10_kernel_info = weak target_declare addrspace(1) constant %0 { i32 4, i32 2, [2 x %1] [%1 { i32 1, i32 1 }, %1 { i32 0, i32 8 }], i64 0, i64 0, i64 0 }
; PART: define weak dso_local spir_kernel void @__omp_offloading_805_be228f__Z3foo_l10(
; PART-SAME: <{ [1 x i8] }>* byval(<{ [1 x i8] }>){{[% A-Za-z_.0-9]*}},
; PART-SAME: %struct.s2 addrspace(1)*{{[% A-Za-z_.0-9]*}})

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

%struct.s1 = type { i8 }
%struct.s2 = type { double }

; Function Attrs: convergent noinline nounwind
define hidden spir_func void @foo() #0 {
entry:
  %a = alloca %struct.s1, align 1
  %a.ascast = addrspacecast %struct.s1* %a to %struct.s1 addrspace(4)*
  %d = alloca %struct.s2, align 8
  %d.ascast = addrspacecast %struct.s2* %d to %struct.s2 addrspace(4)*
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.FIRSTPRIVATE"(%struct.s1 addrspace(4)* %a.ascast), "QUAL.OMP.FIRSTPRIVATE"(%struct.s2 addrspace(4)* %d.ascast), "QUAL.OMP.MAP.TO"(%struct.s1 addrspace(4)* %a.ascast, %struct.s1 addrspace(4)* %a.ascast, i64 1, i64 161, i8* null, i8* null), "QUAL.OMP.MAP.TO"(%struct.s2 addrspace(4)* %d.ascast, %struct.s2 addrspace(4)* %d.ascast, i64 8, i64 161, i8* null, i8* null) ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { convergent noinline nounwind "contains-openmp-target"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4, !5}
!opencl.used.extensions = !{!6}
!opencl.used.optional.core.features = !{!7}
!opencl.compiler.options = !{!6}
!llvm.ident = !{!8}

!0 = !{i32 0, i32 2053, i32 12460687, !"_Z3foo", i32 10, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 50}
!3 = !{i32 7, !"openmp-device", i32 50}
!4 = !{i32 7, !"PIC Level", i32 2}
!5 = !{i32 7, !"frame-pointer", i32 2}
!6 = !{}
!7 = !{!"cl_doubles"}
!8 = !{!"clang version 12.0.0"}
