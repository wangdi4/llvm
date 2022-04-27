; RUN: opt -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -S %s | FileCheck %s

; Original code:
; extern void foo(void);
; void bar() {
;   void (*fptr)(void) = foo;
; #pragma omp target firstprivate(fptr)
;   fptr();
; }

; CHECK: define{{.*}}spir_kernel void @__omp_offloading_806_1c54a3__Z3bar_l4(i64{{[^*]*}})
; CHECK-DAG: [[CAST:%[A-Za-z0-9._]+]] = inttoptr i64{{.*}} to void ()*
; CHECK-DAG: [[AI:%[A-Za-z0-9._]+]] = alloca void ()*
; CHECK: store void ()* [[CAST]], void ()** [[AI]]
; CHECK: [[FPTR:%[A-Za-z0-9._]+]] = load void ()*, void ()** [[AI]]
; CHECK: call spir_func void [[FPTR]]()

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: noinline nounwind optnone
define hidden spir_func void @bar() #0 {
entry:
  %fptr = alloca void ()*, align 8
  %fptr.ascast = addrspacecast void ()** %fptr to void ()* addrspace(4)*
  %fptr.map.ptr.tmp = alloca void ()*, align 8
  %fptr.map.ptr.tmp.ascast = addrspacecast void ()** %fptr.map.ptr.tmp to void ()* addrspace(4)*
  store void ()* @foo, void ()* addrspace(4)* %fptr.ascast, align 8
  %0 = load void ()*, void ()* addrspace(4)* %fptr.ascast, align 8
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.FIRSTPRIVATE"(void ()* addrspace(4)* %fptr.ascast) ]
  %2 = load void ()*, void ()* addrspace(4)* %fptr.ascast, align 8
  call spir_func void %2() #2
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

declare spir_func void @foo() #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

attributes #0 = { noinline nounwind optnone "contains-openmp-target"="true" "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2}
!opencl.used.extensions = !{!3}
!opencl.used.optional.core.features = !{!3}
!opencl.compiler.options = !{!3}
!llvm.ident = !{!4}

!0 = !{i32 0, i32 2054, i32 1856675, !"_Z3bar", i32 4, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"PIC Level", i32 2}
!3 = !{}
!4 = !{!"clang version 10.0.0"}
