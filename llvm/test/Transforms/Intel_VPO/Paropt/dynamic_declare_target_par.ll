; RUN: opt -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S -switch-to-offload %s 2>&1 | FileCheck %s

; Test src:
;
; #pragma omp declare target
; void foo(int *x, int n);
; #pragma omp end declare target
;
; void foo(int *x, int n) {
; #pragma omp parallel
;   for (int i = 0; i < n; i++)
;     {
; //      #pragma omp atomic
;       x[i] = x[i] + 1;
;     }
; }

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

; Check that warning is emitted for the ignored construct.
; CHECK: warning:{{.*}}'parallel' construct, in a declare target function, was ignored for calls from target regions.

; Function Attrs: noinline nounwind optnone
define hidden spir_func void @foo(i32 addrspace(4)* %x, i32 %n) #0 {
entry:
  %x.addr = alloca i32 addrspace(4)*, align 8
  %x.addr.ascast = addrspacecast i32 addrspace(4)** %x.addr to i32 addrspace(4)* addrspace(4)*
  %n.addr = alloca i32, align 4
  %n.addr.ascast = addrspacecast i32* %n.addr to i32 addrspace(4)*
  %i = alloca i32, align 4
  %i.ascast = addrspacecast i32* %i to i32 addrspace(4)*
  store i32 addrspace(4)* %x, i32 addrspace(4)* addrspace(4)* %x.addr.ascast, align 8
  store i32 %n, i32 addrspace(4)* %n.addr.ascast, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.SHARED"(i32 addrspace(4)* addrspace(4)* %x.addr.ascast), "QUAL.OMP.SHARED"(i32 addrspace(4)* %n.addr.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast) ]
  store i32 0, i32 addrspace(4)* %i.ascast, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %1 = load i32, i32 addrspace(4)* %i.ascast, align 4
  %2 = load i32, i32 addrspace(4)* %n.addr.ascast, align 4
  %cmp = icmp slt i32 %1, %2
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %3 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %x.addr.ascast, align 8
  %4 = load i32, i32 addrspace(4)* %i.ascast, align 4
  %idxprom = sext i32 %4 to i64
  %ptridx = getelementptr inbounds i32, i32 addrspace(4)* %3, i64 %idxprom
  %5 = load i32, i32 addrspace(4)* %ptridx, align 4
  %add = add nsw i32 %5, 1
  %6 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %x.addr.ascast, align 8
  %7 = load i32, i32 addrspace(4)* %i.ascast, align 4
  %idxprom1 = sext i32 %7 to i64
  %ptridx2 = getelementptr inbounds i32, i32 addrspace(4)* %6, i64 %idxprom1
  store i32 %add, i32 addrspace(4)* %ptridx2, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %8 = load i32, i32 addrspace(4)* %i.ascast, align 4
  %inc = add nsw i32 %8, 1
  store i32 %inc, i32 addrspace(4)* %i.ascast, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
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
