; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-wrncollection -analyze -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,print<vpo-wrncollection>)' -S %s 2>&1 | FileCheck %s

; This test checks if THREAD_LIMIT:TYPED clause on target construct is parsed correctly.
; source IR was hand modified because front end does not yet support the THREAD_LIMIT clause.

; Test src:
;
; #include <omp.h>
; void foo1(long long int n)
; {
;   #pragma omp target thread_limit(n)
;   { }
; }
; void foo2()
; {
;   int local = 2;
;   #pragma omp target thread_limit(local)
;   { }
; }
; void foo2c()
; {
;   int local = 2;
;   #pragma omp target thread_limit(local+2)
;   { }
; }
; void foo3()
; {
;   #pragma omp target thread_limit(4)
;   { }
; }

; CHECK: THREAD_LIMIT:   %n.addr = alloca i32, align 4
; CHECK: THREAD_LIMIT:   %local = alloca i32, align 4
; CHECK: THREAD_LIMIT:   %omp.clause.tmp = alloca i32, align 4
; CHECK: THREAD_LIMIT: i32 4

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo1(i32 noundef %n) #0 {
entry:
  %n.addr = alloca i32, align 4
  store i32 %n, ptr %n.addr, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.THREAD_LIMIT:TYPED"(ptr %n.addr, i32 0) ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo2() #0 {
entry:
  %local = alloca i32, align 4
  store i32 2, ptr %local, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 1),
    "QUAL.OMP.THREAD_LIMIT:TYPED"(ptr %local, i32 0) ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  ret void
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo2c() #0 {
entry:
  %local = alloca i32, align 4
  %omp.clause.tmp = alloca i32, align 4
  store i32 2, ptr %local, align 4
  %0 = load i32, ptr %local, align 4
  %add = add nsw i32 %0, 2
  store i32 %add, ptr %omp.clause.tmp, align 4
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 2),
    "QUAL.OMP.THREAD_LIMIT:TYPED"(ptr %omp.clause.tmp, i32 0) ]

  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET"() ]

  ret void
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo3() #0 {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 3),
    "QUAL.OMP.THREAD_LIMIT"(i32 4) ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  ret void
}

attributes #0 = { noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }

!omp_offload.info = !{!0, !1, !2, !3}
!llvm.module.flags = !{!4, !5, !6, !7}

!0 = !{i32 0, i32 90, i32 -675040325, !"_Z4foo1", i32 4, i32 0, i32 0, i32 0}
!1 = !{i32 0, i32 90, i32 -675040325, !"_Z4foo2", i32 11, i32 0, i32 1, i32 0}
!2 = !{i32 0, i32 90, i32 -675040325, !"_Z4foo3", i32 24, i32 0, i32 3, i32 0}
!3 = !{i32 0, i32 90, i32 -675040325, !"_Z5foo2c", i32 18, i32 0, i32 2, i32 0}
!4 = !{i32 1, !"wchar_size", i32 4}
!5 = !{i32 7, !"openmp", i32 51}
!6 = !{i32 7, !"uwtable", i32 2}
!7 = !{i32 7, !"frame-pointer", i32 2}
