; REQUIRES: asserts
; RUN: opt -enable-new-pm=0 -vpo-cfg-restructuring -vpo-wrncollection -analyze -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,print<vpo-wrncollection>)' -S %s 2>&1 | FileCheck %s
;
; Check parsing of the DEPARRAY(Num, Array) representation for DEPEND clauses
;
; // C sourse
; #include <omp.h>
; int aaa, bbb;
; void bar();
; void foo() {
;
;   #pragma omp task depend(out:aaa, bbb)
;     bar();
;
;   #pragma omp target depend(out:aaa, bbb)
;     bar();
;
;   #pragma omp taskwait depend(out:aaa, bbb)
;
;   omp_interop_t interop_obj;
;   #pragma omp interop init(targetsync:interop_obj) depend(out:aaa, bbb)
;
;   // No need to test parsing of the remaining constructs that take DEPEND
;   // clauses, as all of them result in a taget-task with the DEPARRAY qual,
;   // similar to what's done with the above target construct with DEPEND
;   //     #pragma omp target enter data
;   //     #pragma omp target exit data
;   //     #pragma omp target update
; }
;
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.kmp_depend_info = type { i64, i64, i8 }

@aaa = dso_local global i32 0, align 4
@bbb = dso_local global i32 0, align 4

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local void @_Z3foov() #0 {
entry:
  %.dep.arr.addr = alloca [2 x %struct.kmp_depend_info], align 8
  %dep.counter.addr = alloca i64, align 8
  %.dep.arr.addr1 = alloca [2 x %struct.kmp_depend_info], align 8
  %dep.counter.addr2 = alloca i64, align 8
  %.dep.arr.addr3 = alloca [2 x %struct.kmp_depend_info], align 8
  %dep.counter.addr4 = alloca i64, align 8
  %interop_obj = alloca i8*, align 8
  %.dep.arr.addr5 = alloca [2 x %struct.kmp_depend_info], align 8
  %dep.counter.addr6 = alloca i64, align 8
  %0 = getelementptr inbounds [2 x %struct.kmp_depend_info], [2 x %struct.kmp_depend_info]* %.dep.arr.addr, i64 0, i64 0
  %1 = getelementptr %struct.kmp_depend_info, %struct.kmp_depend_info* %0, i64 0
  %2 = getelementptr inbounds %struct.kmp_depend_info, %struct.kmp_depend_info* %1, i32 0, i32 0
  store i64 ptrtoint (i32* @aaa to i64), i64* %2, align 8
  %3 = getelementptr inbounds %struct.kmp_depend_info, %struct.kmp_depend_info* %1, i32 0, i32 1
  store i64 4, i64* %3, align 8
  %4 = getelementptr inbounds %struct.kmp_depend_info, %struct.kmp_depend_info* %1, i32 0, i32 2
  store i8 3, i8* %4, align 8
  %5 = getelementptr %struct.kmp_depend_info, %struct.kmp_depend_info* %0, i64 1
  %6 = getelementptr inbounds %struct.kmp_depend_info, %struct.kmp_depend_info* %5, i32 0, i32 0
  store i64 ptrtoint (i32* @bbb to i64), i64* %6, align 8
  %7 = getelementptr inbounds %struct.kmp_depend_info, %struct.kmp_depend_info* %5, i32 0, i32 1
  store i64 4, i64* %7, align 8
  %8 = getelementptr inbounds %struct.kmp_depend_info, %struct.kmp_depend_info* %5, i32 0, i32 2
  store i8 3, i8* %8, align 8
  store i64 2, i64* %dep.counter.addr, align 8
  %9 = bitcast %struct.kmp_depend_info* %0 to i8*
  %10 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(), "QUAL.OMP.DEPARRAY"(i32 2, i8* %9) ]
  call void @_Z3barv() #1
  call void @llvm.directive.region.exit(token %10) [ "DIR.OMP.END.TASK"() ]

; CHECK: BEGIN TASK ID=1 {
; CHECK:   DEPEND clause: UNSPECIFIED
; CHECK:   DEPARRAY( i32 2 , i8* %9 )
; CHECK: } END TASK ID=1

  %11 = getelementptr inbounds [2 x %struct.kmp_depend_info], [2 x %struct.kmp_depend_info]* %.dep.arr.addr1, i64 0, i64 0
  %12 = getelementptr %struct.kmp_depend_info, %struct.kmp_depend_info* %11, i64 0
  %13 = getelementptr inbounds %struct.kmp_depend_info, %struct.kmp_depend_info* %12, i32 0, i32 0
  store i64 ptrtoint (i32* @aaa to i64), i64* %13, align 8
  %14 = getelementptr inbounds %struct.kmp_depend_info, %struct.kmp_depend_info* %12, i32 0, i32 1
  store i64 4, i64* %14, align 8
  %15 = getelementptr inbounds %struct.kmp_depend_info, %struct.kmp_depend_info* %12, i32 0, i32 2
  store i8 3, i8* %15, align 8
  %16 = getelementptr %struct.kmp_depend_info, %struct.kmp_depend_info* %11, i64 1
  %17 = getelementptr inbounds %struct.kmp_depend_info, %struct.kmp_depend_info* %16, i32 0, i32 0
  store i64 ptrtoint (i32* @bbb to i64), i64* %17, align 8
  %18 = getelementptr inbounds %struct.kmp_depend_info, %struct.kmp_depend_info* %16, i32 0, i32 1
  store i64 4, i64* %18, align 8
  %19 = getelementptr inbounds %struct.kmp_depend_info, %struct.kmp_depend_info* %16, i32 0, i32 2
  store i8 3, i8* %19, align 8
  store i64 2, i64* %dep.counter.addr2, align 8
  %20 = bitcast %struct.kmp_depend_info* %11 to i8*
  %21 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(), "QUAL.OMP.IF"(i32 0), "QUAL.OMP.TARGET.TASK"(), "QUAL.OMP.DEPARRAY"(i32 2, i8* %20) ]
  %22 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0) ]
  call void @_Z3barv() #1
  call void @llvm.directive.region.exit(token %22) [ "DIR.OMP.END.TARGET"() ]
  call void @llvm.directive.region.exit(token %21) [ "DIR.OMP.END.TASK"() ]

; CHECK: BEGIN TASK ID=2 {
; CHECK:   TARGET_TASK: true
; CHECK:   DEPEND clause: UNSPECIFIED
; CHECK:   DEPARRAY( i32 2 , i8* %20 )
;
; CHECK:   BEGIN TARGET ID=3 {
; CHECK:     DEPEND clause: UNSPECIFIED
; CHECK:     DEPARRAY: UNSPECIFIED
; CHECK:   } END TARGET ID=3
;
; CHECK: } END TASK ID=2

  %23 = getelementptr inbounds [2 x %struct.kmp_depend_info], [2 x %struct.kmp_depend_info]* %.dep.arr.addr3, i64 0, i64 0
  %24 = getelementptr %struct.kmp_depend_info, %struct.kmp_depend_info* %23, i64 0
  %25 = getelementptr inbounds %struct.kmp_depend_info, %struct.kmp_depend_info* %24, i32 0, i32 0
  store i64 ptrtoint (i32* @aaa to i64), i64* %25, align 8
  %26 = getelementptr inbounds %struct.kmp_depend_info, %struct.kmp_depend_info* %24, i32 0, i32 1
  store i64 4, i64* %26, align 8
  %27 = getelementptr inbounds %struct.kmp_depend_info, %struct.kmp_depend_info* %24, i32 0, i32 2
  store i8 3, i8* %27, align 8
  %28 = getelementptr %struct.kmp_depend_info, %struct.kmp_depend_info* %23, i64 1
  %29 = getelementptr inbounds %struct.kmp_depend_info, %struct.kmp_depend_info* %28, i32 0, i32 0
  store i64 ptrtoint (i32* @bbb to i64), i64* %29, align 8
  %30 = getelementptr inbounds %struct.kmp_depend_info, %struct.kmp_depend_info* %28, i32 0, i32 1
  store i64 4, i64* %30, align 8
  %31 = getelementptr inbounds %struct.kmp_depend_info, %struct.kmp_depend_info* %28, i32 0, i32 2
  store i8 3, i8* %31, align 8
  store i64 2, i64* %dep.counter.addr4, align 8
  %32 = bitcast %struct.kmp_depend_info* %23 to i8*
  %33 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKWAIT"(), "QUAL.OMP.DEPARRAY"(i32 2, i8* %32) ]
  fence acq_rel
  call void @llvm.directive.region.exit(token %33) [ "DIR.OMP.END.TASKWAIT"() ]

; CHECK: BEGIN TASKWAIT ID=4 {
; CHECK:   DEPEND clause: UNSPECIFIED
; CHECK:   DEPARRAY( i32 2 , i8* %32 )
; CHECK: } END TASKWAIT ID=4

  %34 = getelementptr inbounds [2 x %struct.kmp_depend_info], [2 x %struct.kmp_depend_info]* %.dep.arr.addr5, i64 0, i64 0
  %35 = getelementptr %struct.kmp_depend_info, %struct.kmp_depend_info* %34, i64 0
  %36 = getelementptr inbounds %struct.kmp_depend_info, %struct.kmp_depend_info* %35, i32 0, i32 0
  store i64 ptrtoint (i32* @aaa to i64), i64* %36, align 8
  %37 = getelementptr inbounds %struct.kmp_depend_info, %struct.kmp_depend_info* %35, i32 0, i32 1
  store i64 4, i64* %37, align 8
  %38 = getelementptr inbounds %struct.kmp_depend_info, %struct.kmp_depend_info* %35, i32 0, i32 2
  store i8 3, i8* %38, align 8
  %39 = getelementptr %struct.kmp_depend_info, %struct.kmp_depend_info* %34, i64 1
  %40 = getelementptr inbounds %struct.kmp_depend_info, %struct.kmp_depend_info* %39, i32 0, i32 0
  store i64 ptrtoint (i32* @bbb to i64), i64* %40, align 8
  %41 = getelementptr inbounds %struct.kmp_depend_info, %struct.kmp_depend_info* %39, i32 0, i32 1
  store i64 4, i64* %41, align 8
  %42 = getelementptr inbounds %struct.kmp_depend_info, %struct.kmp_depend_info* %39, i32 0, i32 2
  store i8 3, i8* %42, align 8
  store i64 2, i64* %dep.counter.addr6, align 8
  %43 = bitcast %struct.kmp_depend_info* %34 to i8*
  %44 = call token @llvm.directive.region.entry() [ "DIR.OMP.INTEROP"(), "QUAL.OMP.INIT:TARGETSYNC"(i8** %interop_obj), "QUAL.OMP.DEPARRAY"(i32 2, i8* %43) ]
  call void @llvm.directive.region.exit(token %44) [ "DIR.OMP.END.INTEROP"() ]

; CHECK: BEGIN INTEROP ID=5 {
; CHECK:   INIT clause (size=1): (i8** %interop_obj) TARGETSYNC
; CHECK:   DEPEND clause: UNSPECIFIED
; CHECK:   DEPARRAY( i32 2 , i8* %43 )
; CHECK: } END INTEROP ID=5

  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare dso_local void @_Z3barv() #2

attributes #0 = { mustprogress noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }
attributes #2 = { "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4}

!0 = !{i32 0, i32 57, i32 -687794376, !"_Z3foov", i32 9, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 50}
!3 = !{i32 7, !"uwtable", i32 1}
!4 = !{i32 7, !"frame-pointer", i32 2}
