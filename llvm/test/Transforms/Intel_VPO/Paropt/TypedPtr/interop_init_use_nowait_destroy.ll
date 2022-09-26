; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -pass-remarks-missed=openmp -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -pass-remarks-missed=openmp -S %s 2>&1 | FileCheck %s
;
;Test SRC:
;
; #include <omp.h>
; void foo() {
;   omp_interop_t obj;
;   #pragma omp interop init(target:obj)
;   #pragma omp interop use(obj) nowait
;   #pragma omp interop destroy(obj)
;}

;CHECK: remark: <unknown>:0:0: Nowait clause on interop construct was ignored (not yet supported).

source_filename = "test2.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo() #0 {
entry:
  %obj = alloca i8*, align 8
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.INTEROP"(), "QUAL.OMP.INIT:TARGET"(i8** %obj) ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.INTEROP"() ]

;check for @__tgt_create_interop creation
;CHECK:  %[[TASK1:[^ ]+]] = call i8* @__kmpc_omp_task_alloc(%struct.ident_t* @{{[^ ,]+}}, i32 0, i32 0, i64 0, i64 0, i8* null)
;CHECK:  call void @__kmpc_omp_task_begin_if0(%struct.ident_t* @{{[^ ,]+}}, i32  %{{[^ ,]+}}, i8* %[[TASK1]])
;CHECK-NEXT:  %[[CREATE_INTEROP:[^ ]+]] = call i8* @__tgt_create_interop(i64  %{{[^ ,]+}}, i32 0, i32 0, i8* null)
;CHECK-NEXT:  store i8* %[[CREATE_INTEROP]], i8** %[[OBJ:[^ ]+]], align 8
;CHECK:  call void @__kmpc_omp_task_complete_if0(%struct.ident_t* @{{[^ ,]+}}, i32 %{{[^ ,]+}}, i8* %[[TASK1]])

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.INTEROP"(), "QUAL.OMP.USE"(i8** %obj), "QUAL.OMP.NOWAIT"() ]
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.INTEROP"() ]

;check for @__tgt_use_interop(i8* %objinterop.obj.val)
;CHECK:  %[[TASK2:[^ ]+]] = call i8* @__kmpc_omp_task_alloc(%struct.ident_t* @{{[^ ,]+}}, i32 0, i32 0, i64 0, i64 0, i8* null)
;CHECK:  call void @__kmpc_omp_task_begin_if0(%struct.ident_t* @{{[^ ,]+}}, i32 %{{[^ ,]+}}, i8* %[[TASK2]])
;CHECK-NEXT:  %[[INTEROP_VAL:[^ ]+]] = load i8*, i8** %[[OBJ]], align 8
;CHECK-NEXT:  %{{[^ ,]+}} = call i32 @__tgt_use_interop(i8* %[[INTEROP_VAL]])
;CHECK:  call void @__kmpc_omp_task_complete_if0(%struct.ident_t* @{{[^ ,]+}}, i32 %{{[^ ,]+}}, i8* %[[TASK2]])

  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.INTEROP"(), "QUAL.OMP.DESTROY"(i8** %obj) ]
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.INTEROP"() ]

;check for @__tgt_release_interop
;CHECK:  %[[TASK3:[^ ]+]] = call i8* @__kmpc_omp_task_alloc(%struct.ident_t* @{{[^ ,]+}}, i32 0, i32 0, i64 0, i64 0, i8* null)
;CHECK:  call void @__kmpc_omp_task_begin_if0(%struct.ident_t* @{{[^ ,]+}}, i32 %{{[^ ,]+}}, i8* %[[TASK3]])
;CHECK-NEXT: %[[INTEROP_VAL2:[^ ]+]] = load i8*, i8** %[[OBJ]], align 8
;CHECK-NEXT: %{{[^ ,]+}} = call i32 @__tgt_release_interop(i8* %[[INTEROP_VAL2]])
;CHECK-NEXT: store i8* null, i8** %[[OBJ]], align 8
;CHECK:  call void @__kmpc_omp_task_complete_if0(%struct.ident_t* @{{[^ ,]+}}, i32 %{{[^ ,]+}}, i8* %[[TASK3]])
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "may-have-openmp-directive"="true" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0"}
