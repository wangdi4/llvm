; RUN: opt -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-atomic-free-reduction=false -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -vpo-paropt-atomic-free-reduction=false -S %s | FileCheck %s

; Original code:
;void foo() {
;  int s = 0;
;#pragma omp target teams reduction(+: s)
;  ++s;
;}

; Check that the HasTeamsReduction (0x1) attribute is set:
; CHECK: @__omp_offloading_805_b43487__Z3foo_l3_kernel_info = weak target_declare addrspace(1) constant %0 { i32 4, i32 1, [1 x %1] [%1 { i32 0, i32 8 }], i64 1, i64 0, i64 0 }

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: convergent noinline nounwind
define hidden spir_func void @foo() #0 {
entry:
  %s = alloca i32, align 4
  %s.ascast = addrspacecast i32* %s to i32 addrspace(4)*
  store i32 0, i32 addrspace(4)* %s.ascast, align 4, !tbaa !8
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TOFROM"(i32 addrspace(4)* %s.ascast, i32 addrspace(4)* %s.ascast, i64 4, i64 547, i8* null, i8* null) ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(), "QUAL.OMP.REDUCTION.ADD"(i32 addrspace(4)* %s.ascast) ]
  %2 = load i32, i32 addrspace(4)* %s.ascast, align 4, !tbaa !8
  %inc = add nsw i32 %2, 1
  store i32 %inc, i32 addrspace(4)* %s.ascast, align 4, !tbaa !8
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TEAMS"() ]
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
!opencl.used.optional.core.features = !{!6}
!opencl.compiler.options = !{!6}
!llvm.ident = !{!7}

!0 = !{i32 0, i32 2053, i32 11809927, !"_Z3foo", i32 3, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 50}
!3 = !{i32 7, !"openmp-device", i32 50}
!4 = !{i32 7, !"PIC Level", i32 2}
!5 = !{i32 7, !"frame-pointer", i32 2}
!6 = !{}
!7 = !{!"clang version 13.0.0"}
!8 = !{!9, !9, i64 0}
!9 = !{!"int", !10, i64 0}
!10 = !{!"omnipotent char", !11, i64 0}
!11 = !{!"Simple C/C++ TBAA"}
