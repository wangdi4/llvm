; RUN: opt -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -switch-to-offload -S %s | FileCheck %s
; RUN: opt -passes='function(loop(loop-rotate),vpo-cfg-restructuring,vpo-paropt-prepare,loop-simplify,sroa,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -S %s | FileCheck %s

; Original code:
; void foo(int *a) {
;   int i;
; #pragma omp target data map(a[0:100])
; #pragma omp target
;   for (i = 0; i < 100; ++i) {
;     a[i] = 0;
;   }
; }

; Make sure that no data mapping related structures are left in IR.
; In addition, check that there is only one Function in IR.
; CHECK: define weak dso_local
; CHECK-NOT: define{{.*}}dso_local
; CHECK-NOT: offload_baseptrs
; CHECK-NOT: offload_ptrs

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: noinline nounwind uwtable
define dso_local spir_func void @foo(i32 addrspace(4)* %a) #0 {
entry:
  %a.addr = alloca i32 addrspace(4)*, align 8
  %0 = addrspacecast i32 addrspace(4)** %a.addr to i32 addrspace(4)* addrspace(4)*
  %i = alloca i32, align 4
  %1 = addrspacecast i32* %i to i32 addrspace(4)*
  store i32 addrspace(4)* %a, i32 addrspace(4)* addrspace(4)* %0, align 8
  %2 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %0, align 8
  %arrayidx = getelementptr inbounds i32, i32 addrspace(4)* %2, i64 0
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.DATA"(), "QUAL.OMP.MAP.TOFROM:AGGRHEAD"(i32 addrspace(4)* addrspace(4)* %0, i32 addrspace(4)* addrspace(4)* %0, i64 8), "QUAL.OMP.MAP.TOFROM:AGGR"(i32 addrspace(4)* addrspace(4)* %0, i32 addrspace(4)* %arrayidx, i64 400) ]
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %1), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* addrspace(4)* %0) ]
  store i32 0, i32 addrspace(4)* %1, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %5 = load i32, i32 addrspace(4)* %1, align 4
  %cmp = icmp slt i32 %5, 100
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %6 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %0, align 8
  %7 = load i32, i32 addrspace(4)* %1, align 4
  %idxprom = sext i32 %7 to i64
  %arrayidx1 = getelementptr inbounds i32, i32 addrspace(4)* %6, i64 %idxprom
  store i32 0, i32 addrspace(4)* %arrayidx1, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %8 = load i32, i32 addrspace(4)* %1, align 4
  %inc = add nsw i32 %8, 1
  store i32 %inc, i32 addrspace(4)* %1, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.TARGET"() ]
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.TARGET.DATA"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind uwtable "contains-openmp-target"="true" "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!3}

!0 = !{i32 0, i32 2052, i32 85985690, !"foo", i32 4, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{}
!3 = !{!"clang version 9.0.0"}
