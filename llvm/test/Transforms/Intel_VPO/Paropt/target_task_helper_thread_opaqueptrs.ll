; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-cfg-restructuring -vpo-paropt -S <%s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-cfg-restructuring),vpo-paropt' -S <%s | FileCheck %s
;
; // C test
; void bar();
; int aaa;
; void foo_target_depend() {
;   #pragma omp target depend(out:aaa)
;   {
;     bar();
;   }
; }
; void foo_target_nowait() {
;   #pragma omp target nowait
;   {
;     bar();
;   }
; }
; 
; We create an implicit TASK (the "target task") around TARGET when it has
; DEPEND or NOWAIT. If NOWAIT is present, the target task is created with
; the flag bit 0x80 set, which tells runtime to use hidden helper threads.
; This bit must not be set if NOWAIT is not specified for TARGET.
; 
; TARGET DEPEND without NOWAIT: Check that the flag bit vector (3rd argument
; in the @__kmpc_omp_task_alloc call) is 1 (ie, bit 0x80 is not set).
; CHECK: define dso_local void @_Z17foo_target_dependv
; CHECK: @__kmpc_omp_task_alloc(ptr {{.*}}, i32 {{.*}}, i32 1
; 
; TARGET NOWAIT: Check that the flag bit vector is 129 (bit 0x80 is set).
; CHECK: define dso_local void @_Z17foo_target_nowaitv
; CHECK: @__kmpc_omp_task_alloc(ptr {{.*}}, i32 {{.*}}, i32 129

source_filename = "target_task_helper_thread.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

%struct.kmp_depend_info = type { i64, i64, i8 }

@aaa = dso_local global i32 0, align 4

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local void @_Z17foo_target_dependv() {
entry:
  %.dep.arr.addr = alloca [1 x %struct.kmp_depend_info], align 8
  %dep.counter.addr = alloca i64, align 8
  %0 = getelementptr inbounds [1 x %struct.kmp_depend_info], ptr %.dep.arr.addr, i64 0, i64 0
  %1 = getelementptr %struct.kmp_depend_info, ptr %0, i64 0
  %2 = getelementptr inbounds %struct.kmp_depend_info, ptr %1, i32 0, i32 0
  store i64 ptrtoint (ptr @aaa to i64), ptr %2, align 8
  %3 = getelementptr inbounds %struct.kmp_depend_info, ptr %1, i32 0, i32 1
  store i64 4, ptr %3, align 8
  %4 = getelementptr inbounds %struct.kmp_depend_info, ptr %1, i32 0, i32 2
  store i8 3, ptr %4, align 8
  store i64 1, ptr %dep.counter.addr, align 8
  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(),
    "QUAL.OMP.IF"(i32 0),
    "QUAL.OMP.TARGET.TASK"(),
    "QUAL.OMP.DEPARRAY"(i32 1, ptr %0) ]

  %6 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0) ]

  call void @_Z3barv()
  call void @llvm.directive.region.exit(token %6) [ "DIR.OMP.END.TARGET"() ]

  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.TASK"() ]

  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

declare dso_local void @_Z3barv()

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local void @_Z17foo_target_nowaitv() {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(),
    "QUAL.OMP.TARGET.TASK"() ]

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 1),
    "QUAL.OMP.NOWAIT"() ]

  call void @_Z3barv()
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET"() ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TASK"() ]

  ret void
}

!omp_offload.info = !{!0, !1}

!0 = !{i32 0, i32 44, i32 -696467376, !"_Z17foo_target_dependv", i32 4, i32 0, i32 0, i32 0}
!1 = !{i32 0, i32 44, i32 -696467376, !"_Z17foo_target_nowaitv", i32 10, i32 0, i32 1, i32 0}
