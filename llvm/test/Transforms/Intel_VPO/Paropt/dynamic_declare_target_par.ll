; RUN: opt -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s 2>&1 | FileCheck %s
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

; Check that warning is emitted for the ignored construct.
; CHECK: warning:{{.*}} do/for/loop construct, in a declare target function, was ignored for calls from target regions.

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: convergent noinline nounwind optnone
define protected spir_func void @foo(ptr addrspace(4) noundef %x, i32 noundef %n) #0 {
entry:
  %x.addr = alloca ptr addrspace(4), align 8
  %n.addr = alloca i32, align 4
  %i = alloca i32, align 4
  %x.addr.ascast = addrspacecast ptr %x.addr to ptr addrspace(4)
  %n.addr.ascast = addrspacecast ptr %n.addr to ptr addrspace(4)
  %i.ascast = addrspacecast ptr %i to ptr addrspace(4)
  store ptr addrspace(4) %x, ptr addrspace(4) %x.addr.ascast, align 8
  store i32 %n, ptr addrspace(4) %n.addr.ascast, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %x.addr.ascast, ptr addrspace(4) null, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %n.addr.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1) ]
  store i32 0, ptr addrspace(4) %i.ascast, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %1 = load i32, ptr addrspace(4) %i.ascast, align 4
  %2 = load i32, ptr addrspace(4) %n.addr.ascast, align 4
  %cmp = icmp slt i32 %1, %2
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %3 = load ptr addrspace(4), ptr addrspace(4) %x.addr.ascast, align 8
  %4 = load i32, ptr addrspace(4) %i.ascast, align 4
  %idxprom = sext i32 %4 to i64
  %arrayidx = getelementptr inbounds i32, ptr addrspace(4) %3, i64 %idxprom
  %5 = load i32, ptr addrspace(4) %arrayidx, align 4
  %add = add nsw i32 %5, 1
  %6 = load ptr addrspace(4), ptr addrspace(4) %x.addr.ascast, align 8
  %7 = load i32, ptr addrspace(4) %i.ascast, align 4
  %idxprom1 = sext i32 %7 to i64
  %arrayidx2 = getelementptr inbounds i32, ptr addrspace(4) %6, i64 %idxprom1
  store i32 %add, ptr addrspace(4) %arrayidx2, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %8 = load i32, ptr addrspace(4) %i.ascast, align 4
  %inc = add nsw i32 %8, 1
  store i32 %inc, ptr addrspace(4) %i.ascast, align 4
  br label %for.cond, !llvm.loop !7

for.end:                                          ; preds = %for.cond
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { convergent noinline nounwind optnone "approx-func-fp-math"="true" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "openmp-target-declare"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!opencl.compiler.options = !{!5}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"openmp-device", i32 51}
!3 = !{i32 7, !"PIC Level", i32 2}
!4 = !{i32 7, !"frame-pointer", i32 2}
!5 = !{}
!7 = distinct !{!7, !8}
!8 = !{!"llvm.loop.mustprogress"}
