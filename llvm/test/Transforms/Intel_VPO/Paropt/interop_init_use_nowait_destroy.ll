; RUN: opt -opaque-pointers=1 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -pass-remarks-missed=openmp -S %s 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers=1 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -pass-remarks-missed=openmp -S %s 2>&1 | FileCheck %s

; Test src:
;
; #include <omp.h>
; void foo() {
;   omp_interop_t obj;
; #pragma omp interop init(target : obj)
; #pragma omp interop use(obj) nowait
; #pragma omp interop destroy(obj)
; }

; CHECK: remark: <unknown>:0:0: Nowait clause on interop construct was ignored (not yet supported).

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
; CHECK:  %[[TASK1:[^ ]+]] = call ptr @__kmpc_omp_task_alloc(ptr @{{[^ ,]+}}, i32 0, i32 0, i64 0, i64 0, ptr null)
; CHECK:  call void @__kmpc_omp_task_begin_if0(ptr @{{[^ ,]+}}, i32  %{{[^ ,]+}}, ptr %[[TASK1]])
; CHECK-NEXT:  %[[CREATE_INTEROP:[^ ]+]] = call ptr @__tgt_create_interop(i64  %{{[^ ,]+}}, i32 0, i32 0, ptr null)
; CHECK-NEXT:  store ptr %[[CREATE_INTEROP]], ptr %[[OBJ:[^ ]+]], align 8
; CHECK:  call void @__kmpc_omp_task_complete_if0(ptr @{{[^ ,]+}}, i32 %{{[^ ,]+}}, ptr %[[TASK1]])

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.INTEROP"(),
    "QUAL.OMP.USE"(ptr %obj),
    "QUAL.OMP.NOWAIT"() ]
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.INTEROP"() ]

; check for @__tgt_use_interop(ptr %objinterop.obj.val)
; CHECK:  %[[TASK2:[^ ]+]] = call ptr @__kmpc_omp_task_alloc(ptr @{{[^ ,]+}}, i32 0, i32 0, i64 0, i64 0, ptr null)
; CHECK:  call void @__kmpc_omp_task_begin_if0(ptr @{{[^ ,]+}}, i32 %{{[^ ,]+}}, ptr %[[TASK2]])
; CHECK-NEXT:  %[[INTEROP_VAL:[^ ]+]] = load ptr, ptr %[[OBJ]], align 8
; CHECK-NEXT:  %{{[^ ,]+}} = call i32 @__tgt_use_interop(ptr %[[INTEROP_VAL]])
; CHECK:  call void @__kmpc_omp_task_complete_if0(ptr @{{[^ ,]+}}, i32 %{{[^ ,]+}}, ptr %[[TASK2]])

  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.INTEROP"(),
    "QUAL.OMP.DESTROY"(ptr %obj) ]
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.INTEROP"() ]

; check for @__tgt_release_interop
; CHECK:  %[[TASK3:[^ ]+]] = call ptr @__kmpc_omp_task_alloc(ptr @{{[^ ,]+}}, i32 0, i32 0, i64 0, i64 0, ptr null)
; CHECK:  call void @__kmpc_omp_task_begin_if0(ptr @{{[^ ,]+}}, i32 %{{[^ ,]+}}, ptr %[[TASK3]])
; CHECK-NEXT: %[[INTEROP_VAL2:[^ ]+]] = load ptr, ptr %[[OBJ]], align 8
; CHECK-NEXT: %{{[^ ,]+}} = call i32 @__tgt_release_interop(ptr %[[INTEROP_VAL2]])
; CHECK-NEXT: store ptr null, ptr %[[OBJ]], align 8
; CHECK:  call void @__kmpc_omp_task_complete_if0(ptr @{{[^ ,]+}}, i32 %{{[^ ,]+}}, ptr %[[TASK3]])

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
