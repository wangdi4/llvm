; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck --check-prefix=CHECK %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck --check-prefix=CHECK %s

; Test src:
;
; #include <omp.h>
; #include <stdio.h>
;
; int main() {
;   int Result[10];
;   int a;
; #pragma omp taskwait depend(in : a)
;   printf("Result = %d .... \n", Result[0]);
; }

; CHECK: [[DEPNUMBYTES:%[0-9A-Za-z._]+]] = getelementptr inbounds %struct.kmp_depend_info, ptr %{{.*}}, i32 0, i32 1
; CHECK: store i64 4, ptr [[DEPNUMBYTES]]
; CHECK: call void @__kmpc_omp_wait_deps(ptr @{{.*}}, i32 %{{.*}}, i32 1, ptr %{{.*}}, i32 0, ptr null)
; CHECK-NOT: call void @__kmpc_omp_taskwait

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

%struct.kmp_depend_info = type { i64, i64, i8 }

@.str = private unnamed_addr constant [19 x i8] c"Result = %d .... \0A\00", align 1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
entry:
  %Result = alloca [10 x i32], align 16
  %a = alloca i32, align 4
  %.dep.arr.addr = alloca [1 x %struct.kmp_depend_info], align 8
  %dep.counter.addr = alloca i64, align 8
  %0 = getelementptr inbounds [1 x %struct.kmp_depend_info], ptr %.dep.arr.addr, i64 0, i64 0
  %1 = ptrtoint ptr %a to i64
  %2 = getelementptr %struct.kmp_depend_info, ptr %0, i64 0
  %3 = getelementptr inbounds %struct.kmp_depend_info, ptr %2, i32 0, i32 0
  store i64 %1, ptr %3, align 8
  %4 = getelementptr inbounds %struct.kmp_depend_info, ptr %2, i32 0, i32 1
  store i64 4, ptr %4, align 8
  %5 = getelementptr inbounds %struct.kmp_depend_info, ptr %2, i32 0, i32 2
  store i8 1, ptr %5, align 8
  store i64 1, ptr %dep.counter.addr, align 8
  %6 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKWAIT"(),
    "QUAL.OMP.DEPARRAY"(i32 1, ptr %0) ]
  fence acq_rel
  call void @llvm.directive.region.exit(token %6) [ "DIR.OMP.END.TASKWAIT"() ]
  %arrayidx = getelementptr inbounds [10 x i32], ptr %Result, i64 0, i64 0
  %7 = load i32, ptr %arrayidx, align 16
  %call = call i32 (ptr, ...) @printf(ptr noundef @.str, i32 noundef %7)
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
