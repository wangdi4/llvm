; RUN: opt -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Original code:
; void foo() {
;   float x;
;   double y;
;   int i;
;   long long int j;
;   char c;
; #pragma omp target firstprivate(x, y, i, j, c)
;   {
;     (void)x;(void)y;(void)i;(void)j;(void)c;
;   }
; }

; Check that all firstprivate values are passed by value:
; CHECK: define{{.*}}void @__omp_offloading_806_1c5423__Z3foo_l7(i64{{[^*,]*}}, i64{{[^*,]*}}, i64{{[^*,]*}}, i64{{[^*,]*}}, i64{{[^*,]*}})

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: noinline nounwind optnone
define hidden spir_func void @foo() #0 {
entry:
  %x = alloca float, align 4
  %x.ascast = addrspacecast float* %x to float addrspace(4)*
  %y = alloca double, align 8
  %y.ascast = addrspacecast double* %y to double addrspace(4)*
  %i = alloca i32, align 4
  %i.ascast = addrspacecast i32* %i to i32 addrspace(4)*
  %j = alloca i64, align 8
  %j.ascast = addrspacecast i64* %j to i64 addrspace(4)*
  %c = alloca i8, align 1
  %c.ascast = addrspacecast i8* %c to i8 addrspace(4)*
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.FIRSTPRIVATE"(float addrspace(4)* %x.ascast), "QUAL.OMP.FIRSTPRIVATE"(double addrspace(4)* %y.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %i.ascast), "QUAL.OMP.FIRSTPRIVATE"(i64 addrspace(4)* %j.ascast), "QUAL.OMP.FIRSTPRIVATE"(i8 addrspace(4)* %c.ascast) ]
  %1 = load float, float addrspace(4)* %x.ascast, align 4
  %2 = load double, double addrspace(4)* %y.ascast, align 8
  %3 = load i32, i32 addrspace(4)* %i.ascast, align 4
  %4 = load i64, i64 addrspace(4)* %j.ascast, align 8
  %5 = load i8, i8 addrspace(4)* %c.ascast, align 1
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone "contains-openmp-target"="true" "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2}
!opencl.used.extensions = !{!3}
!opencl.used.optional.core.features = !{!4}
!opencl.compiler.options = !{!3}
!llvm.ident = !{!5}

!0 = !{i32 0, i32 2054, i32 1856547, !"_Z3foo", i32 7, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"PIC Level", i32 2}
!3 = !{}
!4 = !{!"cl_doubles"}
!5 = !{!"clang version 10.0.0"}
