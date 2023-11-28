; REQUIRES: asserts
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-wrncollection -analyze -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,print<vpo-wrncollection>)' -S %s 2>&1 | FileCheck %s
;
; Check parsing of the DEPARRAY(Num, Array) representation for DEPEND clauses
;
; Test src:
;
; #include <omp.h>
; int aaa, bbb;
; void bar();
; void foo() {
;
; #pragma omp task depend(out : aaa, bbb)
;   bar();
;
; #pragma omp target depend(out : aaa, bbb)
;   bar();
;
; #pragma omp taskwait depend(out : aaa, bbb)
;
;   omp_interop_t interop_obj;
; #pragma omp interop init(targetsync : interop_obj) depend(out : aaa, bbb)
;
;   // No need to test parsing of the remaining constructs that take DEPEND
;   // clauses, as all of them result in a taget-task with the DEPARRAY qual,
;   // similar to what's done with the above target construct with DEPEND
;   //     #pragma omp target enter data
;   //     #pragma omp target exit data
;   //     #pragma omp target update
; }

; CHECK: BEGIN TASK ID=1 {
; CHECK:   DEPEND clause: UNSPECIFIED
; CHECK:   DEPARRAY( i32 2 , ptr {{.*}} )
; CHECK: } END TASK ID=1

; CHECK: BEGIN TASK ID=2 {
; CHECK:   TARGET_TASK: true
; CHECK:   DEPEND clause: UNSPECIFIED
; CHECK:   DEPARRAY( i32 2 , ptr {{.*}} )
;
; CHECK:   BEGIN TARGET ID=3 {
; CHECK:     DEPEND clause: UNSPECIFIED
; CHECK:     DEPARRAY: UNSPECIFIED
; CHECK:   } END TARGET ID=3
;
; CHECK: } END TASK ID=2

; CHECK: BEGIN TASKWAIT ID=4 {
; CHECK:   DEPEND clause: UNSPECIFIED
; CHECK:   DEPARRAY( i32 2 , ptr {{.*}} )
; CHECK: } END TASKWAIT ID=4

; CHECK: BEGIN INTEROP ID=5 {
; CHECK:   INIT clause (size=1): (ptr %interop_obj) TARGETSYNC
; CHECK:   DEPEND clause: UNSPECIFIED
; CHECK:   DEPARRAY( i32 2 , ptr {{.*}} )
; CHECK: } END INTEROP ID=5

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.kmp_depend_info = type { i64, i64, i8 }

@aaa = dso_local global i32 0, align 4
@bbb = dso_local global i32 0, align 4

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo() #0 {
entry:
  %.dep.arr.addr = alloca [2 x %struct.kmp_depend_info], align 8
  %dep.counter.addr = alloca i64, align 8
  %.dep.arr.addr1 = alloca [2 x %struct.kmp_depend_info], align 8
  %dep.counter.addr2 = alloca i64, align 8
  %.dep.arr.addr3 = alloca [2 x %struct.kmp_depend_info], align 8
  %dep.counter.addr4 = alloca i64, align 8
  %interop_obj = alloca ptr, align 8
  %.dep.arr.addr5 = alloca [2 x %struct.kmp_depend_info], align 8
  %dep.counter.addr6 = alloca i64, align 8
  %0 = getelementptr inbounds [2 x %struct.kmp_depend_info], ptr %.dep.arr.addr, i64 0, i64 0
  %1 = getelementptr %struct.kmp_depend_info, ptr %0, i64 0
  %2 = getelementptr inbounds %struct.kmp_depend_info, ptr %1, i32 0, i32 0
  store i64 ptrtoint (ptr @aaa to i64), ptr %2, align 8
  %3 = getelementptr inbounds %struct.kmp_depend_info, ptr %1, i32 0, i32 1
  store i64 4, ptr %3, align 8
  %4 = getelementptr inbounds %struct.kmp_depend_info, ptr %1, i32 0, i32 2
  store i8 3, ptr %4, align 8
  %5 = getelementptr %struct.kmp_depend_info, ptr %0, i64 1
  %6 = getelementptr inbounds %struct.kmp_depend_info, ptr %5, i32 0, i32 0
  store i64 ptrtoint (ptr @bbb to i64), ptr %6, align 8
  %7 = getelementptr inbounds %struct.kmp_depend_info, ptr %5, i32 0, i32 1
  store i64 4, ptr %7, align 8
  %8 = getelementptr inbounds %struct.kmp_depend_info, ptr %5, i32 0, i32 2
  store i8 3, ptr %8, align 8
  store i64 2, ptr %dep.counter.addr, align 8
  %9 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(),
    "QUAL.OMP.DEPARRAY"(i32 2, ptr %0) ]
  call void (...) @bar() #1
  call void @llvm.directive.region.exit(token %9) [ "DIR.OMP.END.TASK"() ]
  %10 = getelementptr inbounds [2 x %struct.kmp_depend_info], ptr %.dep.arr.addr1, i64 0, i64 0
  %11 = getelementptr %struct.kmp_depend_info, ptr %10, i64 0
  %12 = getelementptr inbounds %struct.kmp_depend_info, ptr %11, i32 0, i32 0
  store i64 ptrtoint (ptr @aaa to i64), ptr %12, align 8
  %13 = getelementptr inbounds %struct.kmp_depend_info, ptr %11, i32 0, i32 1
  store i64 4, ptr %13, align 8
  %14 = getelementptr inbounds %struct.kmp_depend_info, ptr %11, i32 0, i32 2
  store i8 3, ptr %14, align 8
  %15 = getelementptr %struct.kmp_depend_info, ptr %10, i64 1
  %16 = getelementptr inbounds %struct.kmp_depend_info, ptr %15, i32 0, i32 0
  store i64 ptrtoint (ptr @bbb to i64), ptr %16, align 8
  %17 = getelementptr inbounds %struct.kmp_depend_info, ptr %15, i32 0, i32 1
  store i64 4, ptr %17, align 8
  %18 = getelementptr inbounds %struct.kmp_depend_info, ptr %15, i32 0, i32 2
  store i8 3, ptr %18, align 8
  store i64 2, ptr %dep.counter.addr2, align 8
  %19 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(),
    "QUAL.OMP.IF"(i32 0),
    "QUAL.OMP.TARGET.TASK"(),
    "QUAL.OMP.DEPARRAY"(i32 2, ptr %10) ]
  %20 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0) ]
  call void (...) @bar() #1
  call void @llvm.directive.region.exit(token %20) [ "DIR.OMP.END.TARGET"() ]
  call void @llvm.directive.region.exit(token %19) [ "DIR.OMP.END.TASK"() ]
  %21 = getelementptr inbounds [2 x %struct.kmp_depend_info], ptr %.dep.arr.addr3, i64 0, i64 0
  %22 = getelementptr %struct.kmp_depend_info, ptr %21, i64 0
  %23 = getelementptr inbounds %struct.kmp_depend_info, ptr %22, i32 0, i32 0
  store i64 ptrtoint (ptr @aaa to i64), ptr %23, align 8
  %24 = getelementptr inbounds %struct.kmp_depend_info, ptr %22, i32 0, i32 1
  store i64 4, ptr %24, align 8
  %25 = getelementptr inbounds %struct.kmp_depend_info, ptr %22, i32 0, i32 2
  store i8 3, ptr %25, align 8
  %26 = getelementptr %struct.kmp_depend_info, ptr %21, i64 1
  %27 = getelementptr inbounds %struct.kmp_depend_info, ptr %26, i32 0, i32 0
  store i64 ptrtoint (ptr @bbb to i64), ptr %27, align 8
  %28 = getelementptr inbounds %struct.kmp_depend_info, ptr %26, i32 0, i32 1
  store i64 4, ptr %28, align 8
  %29 = getelementptr inbounds %struct.kmp_depend_info, ptr %26, i32 0, i32 2
  store i8 3, ptr %29, align 8
  store i64 2, ptr %dep.counter.addr4, align 8
  %30 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKWAIT"(),
    "QUAL.OMP.DEPARRAY"(i32 2, ptr %21) ]
  fence acq_rel
  call void @llvm.directive.region.exit(token %30) [ "DIR.OMP.END.TASKWAIT"() ]
  %31 = getelementptr inbounds [2 x %struct.kmp_depend_info], ptr %.dep.arr.addr5, i64 0, i64 0
  %32 = getelementptr %struct.kmp_depend_info, ptr %31, i64 0
  %33 = getelementptr inbounds %struct.kmp_depend_info, ptr %32, i32 0, i32 0
  store i64 ptrtoint (ptr @aaa to i64), ptr %33, align 8
  %34 = getelementptr inbounds %struct.kmp_depend_info, ptr %32, i32 0, i32 1
  store i64 4, ptr %34, align 8
  %35 = getelementptr inbounds %struct.kmp_depend_info, ptr %32, i32 0, i32 2
  store i8 3, ptr %35, align 8
  %36 = getelementptr %struct.kmp_depend_info, ptr %31, i64 1
  %37 = getelementptr inbounds %struct.kmp_depend_info, ptr %36, i32 0, i32 0
  store i64 ptrtoint (ptr @bbb to i64), ptr %37, align 8
  %38 = getelementptr inbounds %struct.kmp_depend_info, ptr %36, i32 0, i32 1
  store i64 4, ptr %38, align 8
  %39 = getelementptr inbounds %struct.kmp_depend_info, ptr %36, i32 0, i32 2
  store i8 3, ptr %39, align 8
  store i64 2, ptr %dep.counter.addr6, align 8
  %40 = call token @llvm.directive.region.entry() [ "DIR.OMP.INTEROP"(),
    "QUAL.OMP.INIT:TARGETSYNC"(ptr %interop_obj),
    "QUAL.OMP.DEPARRAY"(i32 2, ptr %31) ]
  call void @llvm.directive.region.exit(token %40) [ "DIR.OMP.END.INTEROP"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare dso_local void @bar(...) #2

attributes #0 = { noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }
attributes #2 = { "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4}

!0 = !{i32 0, i32 53, i32 -1928141563, !"_Z3foo", i32 9, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 51}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{i32 7, !"frame-pointer", i32 2}
