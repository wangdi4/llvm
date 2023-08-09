; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src:
;
; #include <omp.h>
; #include <stdio.h>
;
; int main() {
;   int Result[10];
;   int a;
; #pragma omp taskwait
;   printf("Result = %d .... \n", Result[0]);
; }

; This test checks that we are generating @__kmpc_omp_wait_deps in case of depend on taskwait without nowait
; The input IR was hand-modified because front end doesn't yet handle Depend clauses on taskwait
; CHECK: call void @__kmpc_omp_wait_deps(ptr @{{.*}}, i32 %{{.*}}, i32 1, ptr %{{.*}}, i32 0, ptr null)
; CHECK-NOT: call void @__kmpc_omp_taskwait

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

@.str = private unnamed_addr constant [19 x i8] c"Result = %d .... \0A\00", align 1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
entry:
  %Result = alloca [10 x i32], align 16
  %a = alloca i32, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKWAIT"(),
    "QUAL.OMP.DEPARRAY"(i32 1, ptr %a) ]
  fence acq_rel
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TASKWAIT"() ]
  %arrayidx = getelementptr inbounds [10 x i32], ptr %Result, i64 0, i64 0
  %1 = load i32, ptr %arrayidx, align 16
  %call = call i32 (ptr, ...) @printf(ptr noundef @.str, i32 noundef %1)
  ret i32 0
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare dso_local i32 @printf(ptr noundef, ...) #2

attributes #0 = { noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }
attributes #2 = { "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 7, !"frame-pointer", i32 2}
