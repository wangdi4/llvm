; RUN: opt -vpo-paropt-dispatch-codegen-version=0 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -pass-remarks-missed=openmp -S %s 2>&1 | FileCheck %s -check-prefix=OCG -check-prefix=ALL
; RUN: opt -vpo-paropt-dispatch-codegen-version=0 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -pass-remarks-missed=openmp -S %s 2>&1 | FileCheck %s -check-prefix=OCG -check-prefix=ALL
; RUN: opt -vpo-paropt-dispatch-codegen-version=1 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -S <%s | FileCheck %s -check-prefix=NCG -check-prefix=ALL
; RUN: opt -vpo-paropt-dispatch-codegen-version=1 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S <%s | FileCheck %s -check-prefix=NCG -check-prefix=ALL

; Test src:
;
; #include <omp.h>
; void foo() {
;   omp_interop_t obj;
; #pragma omp interop init(target : obj)
; #pragma omp interop use(obj) nowait
; #pragma omp interop destroy(obj)
; }

; NCG-NOT: remark: <unknown>:0:0: Nowait clause on interop construct was ignored (not yet supported).

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo() #0 {
entry:
  %obj = alloca ptr, align 8
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.INTEROP"(),
    "QUAL.OMP.INIT:TARGET"(ptr %obj) ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.INTEROP"() ]

; check for @__tgt_create_interop creation
; ALL:  %[[TASK1:[^ ]+]] = call ptr @__kmpc_omp_task_alloc(ptr @{{[^ ,]+}}, i32 %{{.*}}, i32 0, i64 0, i64 0, ptr null)
; ALL:  call void @__kmpc_omp_task_begin_if0(ptr @{{[^ ,]+}}, i32  %{{[^ ,]+}}, ptr %[[TASK1]])
; ALL-NEXT:  %[[CREATE_INTEROP:[^ ]+]] = call ptr @__tgt_create_interop(i64  %{{[^ ,]+}}, i32 0, i32 0, ptr null)
; ALL-NEXT:  store ptr %[[CREATE_INTEROP]], ptr %[[OBJ:[^ ]+]], align 8
; ALL:  call void @__kmpc_omp_task_complete_if0(ptr @{{[^ ,]+}}, i32 %{{[^ ,]+}}, ptr %[[TASK1]])

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.INTEROP"(),
    "QUAL.OMP.USE"(ptr %obj),
    "QUAL.OMP.NOWAIT"() ]
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.INTEROP"() ]

; check for @__tgt_use_interop(ptr %objinterop.obj.val)
; ALL:  %[[TASK2:[^ ]+]] = call ptr @__kmpc_omp_task_alloc(ptr @{{[^ ,]+}}, i32 %{{.*}}, i32 0, i64 0, i64 0, ptr null)
; ALL:  call void @__kmpc_omp_task_begin_if0(ptr @{{[^ ,]+}}, i32 %{{[^ ,]+}}, ptr %[[TASK2]])
; ALL-NEXT:  %[[INTEROP_VAL:[^ ]+]] = load ptr, ptr %[[OBJ]], align 8
; OCG-NEXT:  %{{[^ ,]+}} = call i32 @__tgt_use_interop(ptr %[[INTEROP_VAL]])
; NCG-NEXT:  %my.tid = load i32, ptr @"@tid.addr", align 4
; NCG-NEXT:  call void @__tgt_interop_use_async(ptr @{{.*}}, i32 %my.tid, ptr %[[INTEROP_VAL]], i8 1, ptr null)
; ALL:  call void @__kmpc_omp_task_complete_if0(ptr @{{[^ ,]+}}, i32 %{{[^ ,]+}}, ptr %[[TASK2]])

  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.INTEROP"(),
    "QUAL.OMP.DESTROY"(ptr %obj) ]
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.INTEROP"() ]

; check for @__tgt_release_interop
; ALL:  %[[TASK3:[^ ]+]] = call ptr @__kmpc_omp_task_alloc(ptr @{{[^ ,]+}}, i32 %{{.*}}, i32 0, i64 0, i64 0, ptr null)
; ALL:  call void @__kmpc_omp_task_begin_if0(ptr @{{[^ ,]+}}, i32 %{{[^ ,]+}}, ptr %[[TASK3]])
; ALL-NEXT: %[[INTEROP_VAL2:[^ ]+]] = load ptr, ptr %[[OBJ]], align 8
; ALL-NEXT: %{{[^ ,]+}} = call i32 @__tgt_release_interop(ptr %[[INTEROP_VAL2]])
; ALL-NEXT: store ptr null, ptr %[[OBJ]], align 8
; ALL:  call void @__kmpc_omp_task_complete_if0(ptr @{{[^ ,]+}}, i32 %{{[^ ,]+}}, ptr %[[TASK3]])

  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 7, !"frame-pointer", i32 2}
