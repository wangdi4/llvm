; REQUIRES: asserts
; RUN: opt -opaque-pointers=0 -enable-new-pm=0 -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -debug -S %s 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers=0 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -debug -S %s 2>&1 | FileCheck %s

; Test checks the codegen for the detach clause on task

; Test src:
;
; #include <omp.h>
; void foo() {
;  omp_event_handle_t e;
;  omp_event_handle_t &e_ref = e;
; #pragma omp parallel num_threads(2) private(e)
;  {
; #pragma omp task detach(e_ref) {}
;  }
;}

; check that we add a FirstprivateItem for the DetachItem. This is done because the existing firstprivate codegen is needed also for Detach.
; CHECK: addFirstprivateForDetach: Created Firstprivate Clause for Detach clause '%e_ref'.

; check the code generated for Detach
; CHECK: %.task.alloc = call i8* @__kmpc_omp_task_alloc(%struct.ident_t* @{{.*}}, i32 %{{.*}}, i32 65, i64 {{.*}}, i64 {{.*}}, i32 (i32, i8*)* bitcast (void (i32, %__struct.kmp_task_t_with_privates*)* @_Z3foov.DIR.OMP.TASK{{.*}} to i32 (i32, i8*)*))
; CHECK: %.event = call i8* @__kmpc_task_allow_completion_event(%struct.ident_t* @{{.*}}, i32 %{{.*}}, i8* %.task.alloc)
; CHECK: %e_ref.orig.deref = load i64*, i64** %e_ref, align 8
; CHECK: %e_ref.orig.deref.cast = bitcast i64* %e_ref.orig.deref to i8**
; CHECK: store i8* %.event, i8** %e_ref.orig.deref.cast, align 8

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local void @_Z3foov() #0 {
entry:
  %e = alloca i64, align 8
  %e_ref = alloca i64*, align 8
  store i64* %e, i64** %e_ref, align 8
  %0 = load i64*, i64** %e_ref, align 8
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.NUM_THREADS"(i32 2),
    "QUAL.OMP.PRIVATE"(i64* %e),
    "QUAL.OMP.SHARED:BYREF"(i64** %e_ref) ]

  %2 = load i64*, i64** %e_ref, align 8
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(),
    "QUAL.OMP.DETACH:BYREF.TYPED"(i64** %e_ref, i64 0, i32 1)]

  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.TASK"() ]
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
