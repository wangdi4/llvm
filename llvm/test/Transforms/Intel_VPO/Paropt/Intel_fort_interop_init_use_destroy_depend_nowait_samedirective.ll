; INTEL_CUSTOMIZATION
; RUN: opt -vpo-paropt-dispatch-codegen-version=0 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s -check-prefix=OCG -check-prefix=ALL
; RUN: opt -vpo-paropt-dispatch-codegen-version=0 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s -check-prefix=OCG -check-prefix=ALL
; RUN: opt -vpo-paropt-dispatch-codegen-version=1 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -S <%s | FileCheck %s -check-prefix=NCG -check-prefix=ALL
; RUN: opt -vpo-paropt-dispatch-codegen-version=1 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S <%s | FileCheck %s -check-prefix=NCG -check-prefix=ALL

;
;Test SRC:
;
;subroutine foo(obj1, obj2, obj3)
;    use omp_lib
;    implicit none
;    integer(kind=8)::obj1, obj2, obj3
;
;    !$omp interop init(targetsync:obj1) use(obj2) destroy(obj3) nowait &
;    !$omp& depend(in:obj3)
;
;  end subroutine

;ALL: call void @__kmpc_omp_task_begin_if0(ptr @{{[^ ,]+}}, i32 %{{[^ ,]+}}, ptr %{{[^ ,]+}})
;ALL-NEXT:  %[[INTEROP_OBJ:[^ ]+]] = call ptr @__tgt_create_interop(i64 %{{[^ ,]+}}, i32 1, i32 0, ptr null)
;ALL-NEXT:  store ptr %[[INTEROP_OBJ]], ptr %"foo_$OBJ1", align 8
;ALL-NEXT:  %[[INTEROP_OBJ_VAL1:[^ ]+]] = load ptr, ptr %"foo_$OBJ2", align 8
;OCG-NEXT:  %{{[^ ,]+}} = call i32 @__tgt_use_interop(ptr %[[INTEROP_OBJ_VAL1]])
;NCG-NEXT:  %my.tid = load i32, ptr @"@tid.addr", align 4
;NCG-NEXT:  call void @__tgt_interop_use_async(ptr @{{.*}}, i32 %my.tid, ptr %[[INTEROP_OBJ_VAL1]], i8 1, ptr null)
;ALL-NEXT:  %[[INTEROP_OBJ_VAL2:[^ ]+]] = load ptr, ptr %"foo_$OBJ3", align 8
;ALL-NEXT:  %{{[^ ,]+}} = call i32 @__tgt_release_interop(ptr %[[INTEROP_OBJ_VAL2]])
;ALL-NEXT:  store ptr null, ptr %"foo_$OBJ3", align 8
;ALL:  call void @__kmpc_omp_task_complete_if0(ptr @{{[^ ,]+}}, i32 %{{[^ ,]+}}, ptr %{{[^ ,]+}})

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%_TEMPLATE_KMP_DEPEND_INFO_ = type { i64, i64, i8 }

define void @foo_(ptr dereferenceable(8) %"foo_$OBJ1", ptr dereferenceable(8) %"foo_$OBJ2", ptr dereferenceable(8) %"foo_$OBJ3") {
alloca_0:
  %voidptr_deparray = call ptr @__kmpc_alloc(i32 0, i64 24, ptr null)
  br label %bb1

bb1:                                          ; preds = %alloca_0
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.INTEROP"(),
    "QUAL.OMP.INIT:TARGETSYNC"(ptr %"foo_$OBJ1"),
    "QUAL.OMP.USE"(ptr %"foo_$OBJ2"),
    "QUAL.OMP.DESTROY"(ptr %"foo_$OBJ3"),
    "QUAL.OMP.NOWAIT"(),
    "QUAL.OMP.DEPARRAY"(i32 1, ptr %voidptr_deparray) ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.INTEROP"() ]

  call void @__kmpc_free(i32 0, ptr %voidptr_deparray, ptr null)
  ret void
}

declare token @llvm.directive.region.entry()
declare ptr @__kmpc_alloc(i32, i64, ptr)
declare void @__kmpc_free(i32, ptr, ptr)
declare ptr @llvm.intel.subscript.p0.i64.i32.p0.i64(i8, i64, i32, ptr, i64)
declare void @llvm.directive.region.exit(token)
; end INTEL_CUSTOMIZATION
