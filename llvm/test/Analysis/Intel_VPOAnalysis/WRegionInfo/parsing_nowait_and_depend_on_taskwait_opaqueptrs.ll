; REQUIRES: asserts
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-wrncollection -analyze -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,print<vpo-wrncollection>)' -S %s 2>&1 | FileCheck %s
;
; Test src:
;
; #include<stdio.h>
; int main() {
; int y=2;
; #pragma omp taskwait depend(in: y) nowait
;   printf("y = %d \n", y);
; }

; Check that the NOWAIT clause with DEPEND on TASKWAIT is parsed internally as DEPARRAY on TASK
; CHECK:BEGIN TASK ID=1 {
; CHEK-NOT: NOWAIT clause
; CHECK:  DEPEND clause: UNSPECIFIED
; CHECK:  DEPARRAY( i32 1 , ptr %{{.*}})
; CHECK: } END TASK ID=1

source_filename = "test.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.kmp_depend_info = type { i64, i64, i8 }

@.str = private unnamed_addr constant [9 x i8] c"y = %d \0A\00", align 1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
entry:
  %y = alloca i32, align 4
  %.dep.arr.addr = alloca [1 x %struct.kmp_depend_info], align 8
  %dep.counter.addr = alloca i64, align 8
  store i32 2, ptr %y, align 4
  %0 = getelementptr inbounds [1 x %struct.kmp_depend_info], ptr %.dep.arr.addr, i64 0, i64 0
  %1 = ptrtoint ptr %y to i64
  %2 = getelementptr %struct.kmp_depend_info, ptr %0, i64 0
  %3 = getelementptr inbounds %struct.kmp_depend_info, ptr %2, i32 0, i32 0
  store i64 %1, ptr %3, align 8
  %4 = getelementptr inbounds %struct.kmp_depend_info, ptr %2, i32 0, i32 1
  store i64 4, ptr %4, align 8
  %5 = getelementptr inbounds %struct.kmp_depend_info, ptr %2, i32 0, i32 2
  store i8 1, ptr %5, align 8
  store i64 1, ptr %dep.counter.addr, align 8
  %6 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKWAIT"(),
      "QUAL.OMP.DEPARRAY"(i32 1, ptr %0),
      "QUAL.OMP.NOWAIT"()]

  fence acq_rel
  call void @llvm.directive.region.exit(token %6) [ "DIR.OMP.END.TASKWAIT"() ]
  %7 = load i32, ptr %y, align 4
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
!1 = !{i32 7, !"openmp", i32 50}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 7, !"frame-pointer", i32 2}
