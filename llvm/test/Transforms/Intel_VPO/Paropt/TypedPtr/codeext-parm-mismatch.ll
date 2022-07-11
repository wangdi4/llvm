; REQUIRES: asserts
; RUN: opt -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -debug-only=code-extractor -S %s 2>&1 | FileCheck %s

; TEST 1
;
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

; TEST 2:
; additional call to the outlined function is created with the depend
; clause
; int depends() {
;   int dep_1[N];
;
; #pragma omp target depend(out: dep_1) map(tofrom: dep_1[0:N])
;  {
;   for (int i = 0; i < N; i++)
;     dep_1[i] = 1;
;  }
;  return dep_1[4];
; }


define hidden spir_func i32 @depends() #0 {
entry:
  %retval = alloca i32, align 4
  %retval.ascast = addrspacecast i32* %retval to i32 addrspace(4)*
  %dep_1 = alloca [1000 x i32], align 4
  %dep_1.ascast = addrspacecast [1000 x i32]* %dep_1 to [1000 x i32] addrspace(4)*
  %i = alloca i32, align 4
  %i.ascast = addrspacecast i32* %i to i32 addrspace(4)*
  %arrayidx = getelementptr inbounds [1000 x i32], [1000 x i32] addrspace(4)* %dep_1.ascast, i64 0, i64 0
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(), "QUAL.OMP.IF"(i32 0), "QUAL.OMP.TARGET.TASK"(), "QUAL.OMP.DEPEND.OUT"([1000 x i32] addrspace(4)* %dep_1.ascast), "QUAL.OMP.SHARED"([1000 x i32] addrspace(4)* %dep_1.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast) ]
  %arrayidx1 = getelementptr inbounds [1000 x i32], [1000 x i32] addrspace(4)* %dep_1.ascast, i64 0, i64 0
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 1), "QUAL.OMP.MAP.TOFROM"([1000 x i32] addrspace(4)* %dep_1.ascast, i32 addrspace(4)* %arrayidx1, i64 4000, i64 35), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast) ]
  store i32 0, i32 addrspace(4)* %i.ascast, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %2 = load i32, i32 addrspace(4)* %i.ascast, align 4
  %cmp = icmp slt i32 %2, 1000
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %3 = load i32, i32 addrspace(4)* %i.ascast, align 4
  %idxprom = sext i32 %3 to i64
  %arrayidx2 = getelementptr inbounds [1000 x i32], [1000 x i32] addrspace(4)* %dep_1.ascast, i64 0, i64 %idxprom
  store i32 1, i32 addrspace(4)* %arrayidx2, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %4 = load i32, i32 addrspace(4)* %i.ascast, align 4
  %inc = add nsw i32 %4, 1
  store i32 %inc, i32 addrspace(4)* %i.ascast, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TASK"() ]
  %arrayidx3 = getelementptr inbounds [1000 x i32], [1000 x i32] addrspace(4)* %dep_1.ascast, i64 0, i64 4
  %5 = load i32, i32 addrspace(4)* %arrayidx3, align 4
  ret i32 %5
}


declare dso_local spir_func i32 @foo(...) #0

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone uwtable "contains-openmp-target"="true" "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!omp_offload.info = !{!0, !1}
!llvm.module.flags = !{!2}

!0 = !{i32 0, i32 64773, i32 1088323344, !"b", i32 7, i32 0, i32 0}
!1 = !{i32 0, i32 64773, i32 1088323344, !"depends", i32 15, i32 1, i32 0}
!2 = !{i32 1, !"wchar_size", i32 4}
