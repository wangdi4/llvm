; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s| FileCheck %s

; Test checks the codegen for the affinity clause on task
; Test IR was hand-modified by replacing QUAL.OMP.DEPARRAY with QUAL.OMP.AFFARRAY and by deleting the code that populates DepArray.

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
; }

; CHECK:  %.task.alloc = call ptr @__kmpc_omp_task_alloc(ptr @{{.*}}, i32 %{{.*}}, i32 1, i64 72, i64 0, ptr @foo.{{.*}})
; CHECK:  call void @__kmpc_omp_reg_task_with_affinity(ptr @{{.*}}, i32 %{{.*}}, ptr %.task.alloc, i32 2, ptr %{{.*}})
; CHECK:  call void @__kmpc_omp_task(ptr @{{.*}}, i32 %{{.*}}, ptr %.task.alloc)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.kmp_depend_info = type { i64, i64, i8 }

@aaa = dso_local global i32 0, align 4
@bbb = dso_local global i32 0, align 4

define dso_local void @foo() {
entry:
  %.dep.arr.addr = alloca [2 x %struct.kmp_depend_info], align 8
  %0 = getelementptr inbounds [2 x %struct.kmp_depend_info], ptr %.dep.arr.addr, i64 0, i64 0
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(),
    "QUAL.OMP.AFFARRAY"(i32 2, ptr %0) ]

  call void (...) @bar()
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TASK"() ]
  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

declare dso_local void @bar(...)
