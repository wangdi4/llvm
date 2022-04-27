; RUN: opt -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-loop-collapse -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt-optimize-data-sharing -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse,vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring,vpo-paropt-optimize-data-sharing),vpo-paropt' -switch-to-offload -S %s | FileCheck %s

; Original code:
;struct s1 {
;  double d[10];
;};
;void foo() {
;  struct s1 a;
;#pragma omp target firstprivate(a)
;  a.d[7] = 1234567;
;}

; Verify that the pass-as-kernel-argument is applied and
; that the store to a.d[7] is not guarded with a master-thread check.
; That would be incorrect, because the same store must be done
; in all threads.
; CHECK: define{{.*}}spir_kernel void @__omp_offloading_805_be2278__Z3foov_l6(<{ [10 x i64] }>* byval(<{ [10 x i64] }>)
; Master-thread check will generate get_local_id calls:
; CHECK-NOT: get_local_id

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

%struct.s1 = type { [10 x double] }

; Function Attrs: convergent mustprogress noinline nounwind
define hidden spir_func void @_Z3foov() #0 {
entry:
  %a = alloca %struct.s1, align 8
  %a.ascast = addrspacecast %struct.s1* %a to %struct.s1 addrspace(4)*
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.FIRSTPRIVATE"(%struct.s1 addrspace(4)* %a.ascast), "QUAL.OMP.MAP.TO"(%struct.s1 addrspace(4)* %a.ascast, %struct.s1 addrspace(4)* %a.ascast, i64 80, i64 161, i8* null, i8* null) ]
  %d = getelementptr inbounds %struct.s1, %struct.s1 addrspace(4)* %a.ascast, i32 0, i32 0, !intel-tbaa !9
  %arrayidx = getelementptr inbounds [10 x double], [10 x double] addrspace(4)* %d, i64 0, i64 7, !intel-tbaa !15
  store double 0x4132D68700000000, double addrspace(4)* %arrayidx, align 8, !tbaa !16
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { convergent mustprogress noinline nounwind "contains-openmp-target"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4, !5}
!opencl.used.extensions = !{!6}
!opencl.used.optional.core.features = !{!7}
!opencl.compiler.options = !{!6}
!llvm.ident = !{!8}

!0 = !{i32 0, i32 2053, i32 12460664, !"_Z3foov", i32 6, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 50}
!3 = !{i32 7, !"openmp-device", i32 50}
!4 = !{i32 7, !"PIC Level", i32 2}
!5 = !{i32 7, !"frame-pointer", i32 2}
!6 = !{}
!7 = !{!"cl_doubles"}
!8 = !{!"clang version 13.0.0"}
!9 = !{!10, !11, i64 0}
!10 = !{!"struct@_ZTS2s1", !11, i64 0}
!11 = !{!"array@_ZTSA10_d", !12, i64 0}
!12 = !{!"double", !13, i64 0}
!13 = !{!"omnipotent char", !14, i64 0}
!14 = !{!"Simple C++ TBAA"}
!15 = !{!11, !12, i64 0}
!16 = !{!10, !12, i64 0}
