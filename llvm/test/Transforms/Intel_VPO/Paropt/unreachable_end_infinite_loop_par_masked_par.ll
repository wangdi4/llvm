; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src:

; void a() {
; #pragma omp parallel
; #pragma omp master
; #pragma omp parallel
;   for (;;);
; }

; Make sure that we don't comp-fail during codegen due to code-extractor issues
; caused by the infinite loop in the innermost parallel region.
;
; The test was crashing because the IR for the outer region, after handling the
; inner regions, had an early return, breaking the single-entry-single-exit
; expectations:
;
;  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"() ]
;  %2 = call i32 @__kmpc_masked(...)
;  call void @a.DIR.OMP.PARALLEL.58.split()
;  ret void
;  unreachable
;  call void @__kmpc_end_masked(...)
;  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
;  ret void

; CHECK: call void {{.*}} @__kmpc_fork_call({{.*}})
; CHECK: call i32 @__kmpc_masked({{.*}})
; CHECK: call void {{.*}} @__kmpc_fork_call({{.*}})

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @a() #0 {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"() ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.MASTER"() ]
  fence acquire
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"() ]
  br label %for.cond

for.cond:                                         ; preds = %for.cond, %entry
  br label %for.cond

dummy:                                            ; No predecessors!
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.PARALLEL"() ]
  fence release
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.MASTER"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 50}
!2 = !{i32 7, !"uwtable", i32 1}
!3 = !{i32 7, !"frame-pointer", i32 2}
