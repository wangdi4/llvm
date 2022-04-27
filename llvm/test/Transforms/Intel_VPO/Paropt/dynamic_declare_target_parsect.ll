; RUN: opt -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-loop-collapse -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse,vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S -switch-to-offload %s 2>&1 | FileCheck %s

; Test src:
;
; #pragma omp declare target
; void foo(int *x, int n);
; #pragma omp end declare target
;
; void foo(int *x, int n) {
; #pragma omp parallel sections
;   {
; #pragma omp section
;     x[1] = x[1] + 1;
; #pragma omp section
;     x[2] = x[2] + 1;
;   }
; }

; Check that warning is emitted for the ignored construct.
; CHECK: warning:{{.*}}'parallel sections' construct, in a declare target function, was ignored for calls from target regions.

; Check that scheduling code for parallel-sections is not generated.
; CHECK-NOT: call spir_func i64 @_Z14get_local_sizej(i32 0)
; CHECK-NOT: call spir_func i64 @_Z12get_local_idj(i32 0)

; Check that warning is emitted for the ignored construct.
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: noinline nounwind optnone
define hidden spir_func void @foo(i32 addrspace(4)* %x, i32 %n) #0 {
entry:
  %x.addr = alloca i32 addrspace(4)*, align 8
  %x.addr.ascast = addrspacecast i32 addrspace(4)** %x.addr to i32 addrspace(4)* addrspace(4)*
  %n.addr = alloca i32, align 4
  %n.addr.ascast = addrspacecast i32* %n.addr to i32 addrspace(4)*
  store i32 addrspace(4)* %x, i32 addrspace(4)* addrspace(4)* %x.addr.ascast, align 8
  store i32 %n, i32 addrspace(4)* %n.addr.ascast, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.SECTIONS"(), "QUAL.OMP.SHARED"(i32 addrspace(4)* addrspace(4)* %x.addr.ascast) ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SECTION"() ]
  %2 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %x.addr.ascast, align 8
  %ptridx = getelementptr inbounds i32, i32 addrspace(4)* %2, i64 1
  %3 = load i32, i32 addrspace(4)* %ptridx, align 4
  %add = add nsw i32 %3, 1
  %4 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %x.addr.ascast, align 8
  %ptridx1 = getelementptr inbounds i32, i32 addrspace(4)* %4, i64 1
  store i32 %add, i32 addrspace(4)* %ptridx1, align 4
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.SECTION"() ]
  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.SECTION"() ]
  %6 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %x.addr.ascast, align 8
  %ptridx2 = getelementptr inbounds i32, i32 addrspace(4)* %6, i64 2
  %7 = load i32, i32 addrspace(4)* %ptridx2, align 4
  %add3 = add nsw i32 %7, 1
  %8 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %x.addr.ascast, align 8
  %ptridx4 = getelementptr inbounds i32, i32 addrspace(4)* %8, i64 2
  store i32 %add3, i32 addrspace(4)* %ptridx4, align 4
  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.SECTION"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL.SECTIONS"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "openmp-target-declare"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0, !1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"PIC Level", i32 2}
!2 = !{}
!3 = !{!"clang version 10.0.0"}
