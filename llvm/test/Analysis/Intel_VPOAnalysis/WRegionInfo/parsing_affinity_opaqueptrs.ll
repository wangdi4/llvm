; REQUIRES: asserts
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-wrncollection -analyze -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,print<vpo-wrncollection>)' -S %s 2>&1 | FileCheck %s
;
; Check parsing of the AFFINITY clause
; Test IR was hand-modified by changing QUAL.OMP.DEPARRAY to QUAL.OMP.AFFARRAY.
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
;}

; CHECK: BEGIN TASK ID=1 {
; CHECK:   AFFARRAY( i32 2 , ptr %0 )
; CHECK: } END TASK ID=1

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
    "QUAL.OMP.AFFARRAY"(i32 2, ptr %0) ]

  call void (...) @bar()
  call void @llvm.directive.region.exit(token %9) [ "DIR.OMP.END.TASK"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

declare dso_local void @bar(...)
