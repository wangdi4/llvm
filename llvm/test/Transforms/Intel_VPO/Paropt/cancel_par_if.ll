; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src:
;
; #include <stdio.h>
; #include <omp.h>
;
; void foo() {
; #pragma omp parallel num_threads(12)
;   {
;
;     int i = omp_get_thread_num();
;     printf("%d: before\n", i);
;
; #pragma omp cancel parallel if (i == 9)
;
;     printf("%d: after\n", i);
;   }
; }

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [12 x i8] c"%d: before\0A\00", align 1
@.str.1 = private unnamed_addr constant [11 x i8] c"%d: after\0A\00", align 1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo() #0 {
entry:
  %i = alloca i32, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.NUM_THREADS"(i32 12),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1) ]
  %call = call i32 @omp_get_thread_num() #1
  store i32 %call, ptr %i, align 4
  %1 = load i32, ptr %i, align 4
  %call1 = call i32 (ptr, ...) @printf(ptr noundef @.str, i32 noundef %1) #1
  %2 = load i32, ptr %i, align 4
  %cmp = icmp eq i32 %2, 9
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.CANCEL"(),
    "QUAL.OMP.CANCEL.PARALLEL"(),
    "QUAL.OMP.IF"(i1 %cmp) ]
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.CANCEL"() ]

  ; CHECK: %[[CANCEL:[^ ]+]] = call i32 @__kmpc_cancel(ptr @{{[^ ]+}}, i32 %{{[^ ]+}}, i32 1)
  ; CHECK: %[[CHECK1:[^ ]+]] = icmp ne i32 %[[CANCEL]], 0
  ; CHECK: br i1 %[[CHECK1]], label %[[CANCELLED:[^ ]+]], label %{{[^ ]+}}

  ; CHECK: %[[CP:[^ ]+]] = call i32 @__kmpc_cancellationpoint(ptr @{{[^ ]+}}, i32 %{{[^ ]+}}, i32 1)
  ; CHECK: %[[CHECK2:[^ ]+]] = icmp ne i32 %[[CP]], 0
  ; CHECK: br i1 %[[CHECK2]], label %[[CANCELLED]], label %{{[^ ]+}}

  %4 = load i32, ptr %i, align 4
  %call2 = call i32 (ptr, ...) @printf(ptr noundef @.str.1, i32 noundef %4) #1
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: nounwind
declare dso_local i32 @omp_get_thread_num() #2

declare dso_local i32 @printf(ptr noundef, ...) #3

attributes #0 = { noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }
attributes #2 = { nounwind "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #3 = { "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 7, !"frame-pointer", i32 2}
