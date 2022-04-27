; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s
;
; The purpose of this test is to make sure that the compiler doesn't
; comp-fail while handling nested tasks.
;
; Test src:
; double x;
;
; void foo() {
; #pragma omp task
;   {
; #pragma omp task
;     {
;       x++;
;     }
;   }
; }
;
; Check there are two calls to kmps_omp_task_alloc, one for each task.
; CHECK: call i8* @__kmpc_omp_task_alloc
; CHECK: call i8* @__kmpc_omp_task_alloc

; ModuleID = 'task_nested.c'
source_filename = "task_nested.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@x = common dso_local global double 0.000000e+00, align 8

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo() #0 {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(), "QUAL.OMP.SHARED"(double* @x) ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(), "QUAL.OMP.SHARED"(double* @x) ]
  %2 = load double, double* @x, align 8
  %inc = fadd double %2, 1.000000e+00
  store double %inc, double* @x, align 8
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TASK"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TASK"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0}

!0 = !{i32 1, !"wchar_size", i32 4}
