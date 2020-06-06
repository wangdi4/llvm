; REQUIRES: asserts
; RUN: opt < %s -switch-to-offload=true -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -debug-only=code-extractor -S 2>&1 | FileCheck %s

; void b() {
;  int ix;
;  #pragma omp target nowait
;  for (ix = 3; ix < 10; ix++)
;  ;
; }

; The target code is outlined into a temporary stub function that takes an
; addrspace(4) argument. The enclosing task makes a firstprivate value copy
; in addrspace(0) and passes it to the stub function. The verifier aborts on
; the addrspace mismatch between the actual parameter and the declaration.

; CHECK-NOT: verification{{.*}}failed

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: noinline nounwind optnone uwtable
define hidden spir_func void @b() #0 {
entry:
  %ix = alloca i32, align 4
  %ix.ascast = addrspacecast i32* %ix to i32 addrspace(4)*
  %call = call spir_func i32 bitcast (i32 (...)* @foo to i32 ()*)()
  store i32 %call, i32 addrspace(4)* %ix.ascast, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(), "QUAL.OMP.TARGET.TASK"(), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %ix.ascast) ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.NOWAIT"(), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %ix.ascast) ]
  store i32 3, i32 addrspace(4)* %ix.ascast, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %2 = load i32, i32 addrspace(4)* %ix.ascast, align 4
  %cmp = icmp slt i32 %2, 10
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %3 = load i32, i32 addrspace(4)* %ix.ascast, align 4
  %inc = add nsw i32 %3, 1
  store i32 %inc, i32 addrspace(4)* %ix.ascast, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TASK"() ]
  ret void
}

declare dso_local spir_func i32 @foo(...) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

attributes #0 = { noinline nounwind optnone uwtable "contains-openmp-target"="true" "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}

!0 = !{i32 0, i32 64773, i32 1090982192, !"_Z1b", i32 4, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{}
