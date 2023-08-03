; INTEL_CUSTOMIZATION
; RUN: opt -vpo-paropt-dispatch-codegen-version=0 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -pass-remarks-missed=openmp -S %s 2>&1 | FileCheck %s -check-prefix=OCG -check-prefix=ALL
; RUN: opt -vpo-paropt-dispatch-codegen-version=0 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -pass-remarks-missed=openmp -S %s 2>&1 | FileCheck %s -check-prefix=OCG -check-prefix=ALL
; RUN: opt -vpo-paropt-dispatch-codegen-version=1 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -S <%s | FileCheck %s -check-prefix=NCG -check-prefix=ALL
; RUN: opt -vpo-paropt-dispatch-codegen-version=1 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S <%s | FileCheck %s -check-prefix=NCG -check-prefix=ALL

;
;Test SRC:
;
;  subroutine foo
;    use omp_lib
;    implicit none
;
;  !  integer(kind=omp_interop_kind)::obj ! << This doesn't work currently.
;    integer(kind=8)::obj
;
;    !$omp interop init(prefer_type("level_zero", "opencl"), target:obj)
;    !$omp interop use(obj) nowait
;    !$omp interop destroy(obj)
;
;  end subroutine

;CHECK: remark: <unknown>:0:0: Nowait clause on interop construct was ignored (not yet supported).

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo_() {
alloca_0:
  %"$io_ctx" = alloca [8 x i64], align 8
  %"foo_$OBJ" = alloca i64, align 8
  br label %bb1

bb1:                                              ; preds = %alloca_0
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.INTEROP"(),
    "QUAL.OMP.INIT:TARGET.PREFER"(ptr %"foo_$OBJ", i64 6, i64 3) ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.INTEROP"() ]

;ALL: call void @__kmpc_omp_task_begin_if0(ptr @{{[^ ,]+}}, i32 %{{[^ ,]+}}, ptr %{{[^ ,]+}})
;ALL:  %[[CREATE_INTEROP:[^ ]+]] = call ptr @__tgt_create_interop(i64 %{{[^ ,]+}}, i32 0, i32 2, ptr @.prefer.list)
;ALL-NEXT:  store ptr %[[CREATE_INTEROP]], ptr %"foo_$OBJ", align 8
;ALL:  call void @__kmpc_omp_task_complete_if0(ptr @{{[^ ,]+}}, i32 %{{[^ ,]+}}, ptr %{{[^ ,]+}})

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.INTEROP"(),
    "QUAL.OMP.USE"(ptr %"foo_$OBJ"),
    "QUAL.OMP.NOWAIT"() ]
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.INTEROP"() ]

;ALL: call void @__kmpc_omp_task_begin_if0(ptr @{{[^ ,]+}}, i32 %{{[^ ,]+}}, ptr %{{[^ ,]+}})
;ALL-NEXT: %[[INTEROP_VAL:[^ ]+]] = load ptr, ptr %"foo_$OBJ", align 8
;OCG-NEXT: %{{[^ ,]+}} = call i32 @__tgt_use_interop(ptr %[[INTEROP_VAL]])
;NCG-NEXT:  %my.tid = load i32, ptr @"@tid.addr", align 4
;NCG-NEXT:  call void @__tgt_interop_use_async(ptr @{{.*}}, i32 %my.tid, ptr %[[INTEROP_VAL]], i8 1, ptr null)
;ALL: call void @__kmpc_omp_task_complete_if0(ptr @{{[^ ,]+}}, i32 %{{[^ ,]+}}, ptr %{{[^ ,]+}})

  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.INTEROP"(),
    "QUAL.OMP.DESTROY"(ptr %"foo_$OBJ") ]
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.INTEROP"() ]

;ALL:   call void @__kmpc_omp_task_begin_if0(ptr @{{[^ ,]+}}, i32 %{{[^ ,]+}}, ptr %{{[^ ,]+}})
;ALL-NEXT:    %[[INTEROP_VAL2:[^ ]+]] = load ptr, ptr %"foo_$OBJ", align 8
;ALL-NEXT:    %{{[^ ,]+}} = call i32 @__tgt_release_interop(ptr %[[INTEROP_VAL2]])
;ALL-NEXT:    store ptr null, ptr %"foo_$OBJ", align 8
;ALL:  call void @__kmpc_omp_task_complete_if0(ptr @{{[^ ,]+}}, i32 %{{[^ ,]+}}, ptr %{{[^ ,]+}})


  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token %0)
; end INTEL_CUSTOMIZATION
