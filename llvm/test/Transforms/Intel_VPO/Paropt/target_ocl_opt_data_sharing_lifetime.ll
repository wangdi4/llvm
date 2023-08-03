; RUN: opt -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-loop-collapse -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt-optimize-data-sharing -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse,vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring,vpo-paropt-optimize-data-sharing),vpo-paropt' -switch-to-offload -S %s | FileCheck %s

; Original code:
; void foo(int N) {
; #pragma omp target
;   for (int i = 0; i < N; ++i);
; }

; CHECK-NOT: spirv_ControlBarrier

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: noinline nounwind
define hidden spir_func void @foo(i32 %N) #0 {
entry:
  %N.addr = alloca i32, align 4
  %N.addr.ascast = addrspacecast ptr %N.addr to ptr addrspace(4)
  %i = alloca i32, align 4
  %i.ascast = addrspacecast ptr %i to ptr addrspace(4)
  %cleanup.dest.slot = alloca i32, align 4
  store i32 %N, ptr addrspace(4) %N.addr.ascast, align 4, !tbaa !5
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %N.addr.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1) ]

  %1 = addrspacecast ptr addrspace(4) %i.ascast to ptr
  call void @llvm.lifetime.start.p0i8(i64 4, ptr %1) #1
  store i32 0, ptr addrspace(4) %i.ascast, align 4, !tbaa !5
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %2 = load i32, ptr addrspace(4) %i.ascast, align 4, !tbaa !5
  %3 = load i32, ptr addrspace(4) %N.addr.ascast, align 4, !tbaa !5
  %cmp = icmp slt i32 %2, %3
  br i1 %cmp, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  %4 = addrspacecast ptr addrspace(4) %i.ascast to ptr
  call void @llvm.lifetime.end.p0i8(i64 4, ptr %4) #1
  br label %for.end

for.body:                                         ; preds = %for.cond
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %5 = load i32, ptr addrspace(4) %i.ascast, align 4, !tbaa !5
  %inc = add nsw i32 %5, 1
  store i32 %inc, ptr addrspace(4) %i.ascast, align 4, !tbaa !5
  br label %for.cond, !llvm.loop !9

for.end:                                          ; preds = %for.cond.cleanup
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, ptr nocapture) #2

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, ptr nocapture) #2

attributes #0 = { noinline nounwind "contains-openmp-target"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { argmemonly nofree nosync nounwind willreturn }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2}
!opencl.used.extensions = !{!3}
!opencl.used.optional.core.features = !{!3}
!opencl.compiler.options = !{!3}
!llvm.ident = !{!4}

!0 = !{i32 0, i32 2053, i32 12462058, !"_Z3foo", i32 2, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"PIC Level", i32 2}
!3 = !{}
!4 = !{!"clang version 11.0.0"}
!5 = !{!6, !6, i64 0}
!6 = !{!"int", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C/C++ TBAA"}
!9 = distinct !{!9, !10}
!10 = !{!"llvm.loop.mustprogress"}
