; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -switch-to-offload -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -S %s | FileCheck %s

; Test src:
;
; void foo(int *arg) {
; #pragma omp target map(arg[22:77])
;   arg[33] = 55;
; }

; Verify that the kernel has only one argument:
; CHECK: define weak dso_local spir_kernel void @__omp_offloading_{{.*}}__Z3foo_l2(ptr addrspace(1) {{[^,]*}})

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: convergent noinline nounwind optnone
define protected spir_func void @foo(ptr addrspace(4) noundef %arg) #0 {
entry:
  %arg.addr = alloca ptr addrspace(4), align 8
  %arg.map.ptr.tmp = alloca ptr addrspace(4), align 8
  %arg.addr.ascast = addrspacecast ptr %arg.addr to ptr addrspace(4)
  %arg.map.ptr.tmp.ascast = addrspacecast ptr %arg.map.ptr.tmp to ptr addrspace(4)
  store ptr addrspace(4) %arg, ptr addrspace(4) %arg.addr.ascast, align 8
  %0 = load ptr addrspace(4), ptr addrspace(4) %arg.addr.ascast, align 8
  %1 = load ptr addrspace(4), ptr addrspace(4) %arg.addr.ascast, align 8
  %2 = load ptr addrspace(4), ptr addrspace(4) %arg.addr.ascast, align 8
  %arrayidx = getelementptr inbounds i32, ptr addrspace(4) %2, i64 22
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %1, ptr addrspace(4) %arrayidx, i64 308, i64 35, ptr null, ptr null),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %arg.map.ptr.tmp.ascast, ptr addrspace(4) null, i32 1) ]
  store ptr addrspace(4) %1, ptr addrspace(4) %arg.map.ptr.tmp.ascast, align 8
  %4 = load ptr addrspace(4), ptr addrspace(4) %arg.map.ptr.tmp.ascast, align 8
  %arrayidx1 = getelementptr inbounds i32, ptr addrspace(4) %4, i64 33
  store i32 55, ptr addrspace(4) %arrayidx1, align 4
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { convergent noinline nounwind optnone "approx-func-fp-math"="true" "contains-openmp-target"="true" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4, !5}
!opencl.compiler.options = !{!6}

!0 = !{i32 0, i32 53, i32 -1916375430, !"_Z3foo", i32 2, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 51}
!3 = !{i32 7, !"openmp-device", i32 51}
!4 = !{i32 7, !"PIC Level", i32 2}
!5 = !{i32 7, !"frame-pointer", i32 2}
!6 = !{}
