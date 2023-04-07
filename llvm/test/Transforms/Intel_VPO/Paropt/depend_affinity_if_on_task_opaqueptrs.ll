; RUN: opt -opaque-pointers=1 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -opaque-pointers=1 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s| FileCheck %s

; Test check the code generated in case 'affinity', 'depend' and 'if' are present on 'task'.
; Test IR was hand-modified by adding QUAL.OMP.AFFARRAY and by deleting the code that populates DepArray.

; Test src:
;
; #include <omp.h>
; int aaa, bbb;
; void bar();
; void foo() {
;
; #pragma omp task if(aaa) depend(out : aaa, bbb)
;   bar();
;
; }

; CHECK:  %.task.alloc = call ptr @__kmpc_omp_task_alloc(ptr @{{.*}}, i32 %{{.*}}, i32 1, i64 72, i64 0, ptr @{{.*}})
; CHECK:  call void @__kmpc_omp_reg_task_with_affinity(ptr @{{.*}}, i32 %{{.*}}, ptr %.task.alloc, i32 2, ptr %{{.*}})
; CHECK:  br i1 %{{.*}}, label %if.then, label %if.else

; CHECK:if.then:
; CHECK:  call void @__kmpc_omp_task_with_deps(ptr @{{.*}}, i32 %{{.*}}, ptr %.task.alloc, i32 2, ptr %{{.*}}, i32 0, ptr null)
; CHECK:  br label %if.end

; CHECK:if.else:
; CHECK:  call void @__kmpc_omp_wait_deps(ptr @{{.*}}, i32 %{{.*}}, i32 2, ptr %{{.*}}, i32 0, ptr null)
; CHECK:  call void @__kmpc_omp_task_begin_if0(ptr @{{.*}}, i32 %{{.*}}, ptr %.task.alloc)
; CHECK:  call void @__kmpc_omp_task_complete_if0(ptr @{{.*}}, i32 %{{.*}}, ptr %.task.alloc)
; CHECK:  br label %if.end

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.kmp_depend_info = type { i64, i64, i8 }

@aaa = dso_local global i32 0, align 4
@bbb = dso_local global i32 0, align 4

define dso_local void @foo() {
entry:
  %.dep.arr.addr = alloca [2 x %struct.kmp_depend_info], align 8
  %0 = load i32, ptr @aaa, align 4
  %tobool = icmp ne i32 %0, 0
  %1 = getelementptr inbounds [2 x %struct.kmp_depend_info], ptr %.dep.arr.addr, i64 0, i64 0
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(),
    "QUAL.OMP.IF"(i1 %tobool),
    "QUAL.OMP.DEPARRAY"(i32 2, ptr %1),
    "QUAL.OMP.AFFARRAY"(i32 2, ptr %1) ]

  call void (...) @bar()
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TASK"() ]
  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

declare dso_local void @bar(...)
