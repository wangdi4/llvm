; The sin and cos calls should not be conditionalized to thread 0,
; as their return values are used in the loop.
; This test may need adjustment after hierarchical parallelism is able
; to handle these cases.
;
;#pragma omp target map(to:f[0:100]) map(from:r[0:100])
;  for (i = 0; i < 100; i++)
;    r[i] = sinf(f[i]) + cosf(f[i]);

; RUN: opt -switch-to-offload -vpo-restore-operands -domtree -loops -vpo-cfg-restructuring -vpo-paropt  -S < %s | FileCheck %s
; CHECK: call{{.*}}sin
; CHECK-NOT: :
; CHECK: call{{.*}}cos
; CHECK-NOT: :
; CHECK: load

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local spir_func void @_Z3fooPfS_(float addrspace(4)* noalias %f, float addrspace(4)* noalias %r) #0 {
entry:
  %f.addr = alloca float addrspace(4)*, align 8
  %f.addr.ascast = addrspacecast float addrspace(4)** %f.addr to float addrspace(4)* addrspace(4)*
  %r.addr = alloca float addrspace(4)*, align 8
  %r.addr.ascast = addrspacecast float addrspace(4)** %r.addr to float addrspace(4)* addrspace(4)*
  %i = alloca i32, align 4
  %i.ascast = addrspacecast i32* %i to i32 addrspace(4)*
  store float addrspace(4)* %f, float addrspace(4)* addrspace(4)* %f.addr.ascast, align 8
  store float addrspace(4)* %r, float addrspace(4)* addrspace(4)* %r.addr.ascast, align 8
  %0 = load float addrspace(4)*, float addrspace(4)* addrspace(4)* %f.addr.ascast, align 8
  %arrayidx = getelementptr inbounds float, float addrspace(4)* %0, i64 0
  %1 = load float addrspace(4)*, float addrspace(4)* addrspace(4)* %r.addr.ascast, align 8
  %arrayidx1 = getelementptr inbounds float, float addrspace(4)* %1, i64 0
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TO:AGGRHEAD"(float addrspace(4)* addrspace(4)* %f.addr.ascast, float addrspace(4)* addrspace(4)* %f.addr.ascast, i64 8), "QUAL.OMP.MAP.TO:AGGR"(float addrspace(4)* addrspace(4)* %f.addr.ascast, float addrspace(4)* %arrayidx, i64 400), "QUAL.OMP.MAP.FROM:AGGRHEAD"(float addrspace(4)* addrspace(4)* %r.addr.ascast, float addrspace(4)* addrspace(4)* %r.addr.ascast, i64 8), "QUAL.OMP.MAP.FROM:AGGR"(float addrspace(4)* addrspace(4)* %r.addr.ascast, float addrspace(4)* %arrayidx1, i64 400), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %i.ascast) ]
  store i32 0, i32 addrspace(4)* %i.ascast, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %3 = load i32, i32 addrspace(4)* %i.ascast, align 4
  %cmp = icmp slt i32 %3, 100
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %4 = load float addrspace(4)*, float addrspace(4)* addrspace(4)* %f.addr.ascast, align 8
  %5 = load i32, i32 addrspace(4)* %i.ascast, align 4
  %idxprom = sext i32 %5 to i64
  %arrayidx2 = getelementptr inbounds float, float addrspace(4)* %4, i64 %idxprom
  %6 = load float, float addrspace(4)* %arrayidx2, align 4
  %call = call spir_func float @sinf(float %6) #1
  %7 = load float addrspace(4)*, float addrspace(4)* addrspace(4)* %f.addr.ascast, align 8
  %8 = load i32, i32 addrspace(4)* %i.ascast, align 4
  %idxprom3 = sext i32 %8 to i64
  %arrayidx4 = getelementptr inbounds float, float addrspace(4)* %7, i64 %idxprom3
  %9 = load float, float addrspace(4)* %arrayidx4, align 4
  %call5 = call spir_func float @cosf(float %9) #1
  %add = fadd float %call, %call5
  %10 = load float addrspace(4)*, float addrspace(4)* addrspace(4)* %r.addr.ascast, align 8
  %11 = load i32, i32 addrspace(4)* %i.ascast, align 4
  %idxprom6 = sext i32 %11 to i64
  %arrayidx7 = getelementptr inbounds float, float addrspace(4)* %10, i64 %idxprom6
  store float %add, float addrspace(4)* %arrayidx7, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %12 = load i32, i32 addrspace(4)* %i.ascast, align 4
  %inc = add nsw i32 %12, 1
  store i32 %inc, i32 addrspace(4)* %i.ascast, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: nounwind
declare dso_local spir_func float @sinf(float) #2

; Function Attrs: nounwind
declare dso_local spir_func float @cosf(float) #2

attributes #0 = { noinline nounwind optnone uwtable "contains-openmp-target"="true" "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "openmp-target-declare"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!3}

!0 = !{i32 0, i32 64770, i32 1077853693, !"_Z3fooPfS_", i32 6, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{}
!3 = !{!"icx (ICX) dev.8.x.0"}
