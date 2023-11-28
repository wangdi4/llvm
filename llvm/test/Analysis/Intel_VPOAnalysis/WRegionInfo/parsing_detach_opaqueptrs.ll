; REQUIRES: asserts
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-wrncollection -analyze -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,print<vpo-wrncollection>)' -S %s 2>&1 | FileCheck %s

; This test checks if DETACH:TYPED and DETACH:BYREF.TYPED clauses are parsed correctly.
; source IR was hand modified because front end does not support yet the DETACH clause.

; Test src:
;
; #include <omp.h>
; void foo() {
;   omp_event_handle_t e;
;   omp_event_handle_t &e_ref = e;
; #pragma omp parallel num_threads(2) private(e) private(e_ref)
;   {
; #pragma omp task detach(e) {}
; #pragma omp task detach(e_ref) {}
;   }
; }

; CHECK: DETACH clause (size=1): TYPED(ptr %e, TYPE: i64, NUM_ELEMENTS: i32 1)
; CHECK: DETACH clause (size=1): BYREF,TYPED(ptr %e_ref, TYPE: i64, NUM_ELEMENTS: i32 1)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local void @_Z3foov() #0 {
entry:
  %e = alloca i64, align 8
  %e_ref = alloca ptr, align 8
  store ptr %e, ptr %e_ref, align 8
  %0 = load ptr, ptr %e_ref, align 8
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
     "QUAL.OMP.NUM_THREADS"(i32 2),
     "QUAL.OMP.PRIVATE:TYPED"(ptr %e, i64 0, i32 1),
     "QUAL.OMP.PRIVATE:BYREF.TYPED"(ptr %e_ref, i64 0, i32 1) ]

  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(),
     "QUAL.OMP.DETACH:TYPED"(ptr %e, i64 0, i32 1) ]

  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TASK"() ]
  %3 = load ptr, ptr %e_ref, align 8
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(),
     "QUAL.OMP.DETACH:BYREF.TYPED"(ptr %e_ref, i64 0, i32 1) ]

  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.TASK"() ]
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { mustprogress noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 50}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 7, !"frame-pointer", i32 2}
