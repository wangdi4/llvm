; RUN: opt -bugpoint-enable-legacy-pm -vpo-paropt-dispatch-codegen-version=0 -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s -check-prefixes=ALL,VERSION0
; RUN: opt -vpo-paropt-dispatch-codegen-version=0 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s -check-prefixes=ALL,VERSION0
;
; RUN: opt -bugpoint-enable-legacy-pm -vpo-paropt-dispatch-codegen-version=1 -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s -check-prefixes=ALL,VERSION1
; RUN: opt -vpo-paropt-dispatch-codegen-version=1 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s -check-prefixes=ALL,VERSION1
;
; Test Src:
;
;  #include <omp.h>
;  void foo(omp_interop_t obj1, omp_interop_t obj2, omp_interop_t obj3) {
;  #pragma omp interop init(prefer_type("opencl", "level_zero"), targetsync:obj1) \
;                      use(obj2) \
;                      destroy(obj3) \
;                      depend(in:obj2, obj3) depend(out:obj1)
;  }

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

;check code generated for depend clause
;ALL:      @[[PREFER_LIST:[^ ]+]] = private unnamed_addr constant [2 x i32] [i32 3, i32 6]
;ALL:      %[[TASK_ALLOC:.+]] = call ptr @__kmpc_omp_task_alloc(ptr @{{.+}}, i32 %{{.+}}, i32 0, i64 0, i64 0, ptr null)
;ALL:      call void @__kmpc_omp_wait_deps(ptr @{{[^ ,]+}}, i32 %{{[^ ,]+}}, i32 3, ptr %dep.array, i32 0, ptr null)

;check output IR for interop creation, use and release
;ALL:      call void @__kmpc_omp_task_begin_if0(ptr @{{[^ ,]+}}, i32 %{{[^ ,]+}}, ptr %[[TASK_ALLOC]])
;ALL-NEXT: %[[INTEROP_OBJ1:.+]] = call ptr @__tgt_create_interop(i64 %{{[^ ,]+}}, i32 1, i32 2, ptr @[[PREFER_LIST]])
;ALL-NEXT: store ptr %[[INTEROP_OBJ1]], ptr %obj1.addr, align 8
;ALL-NEXT: %[[INTEROP_OBJ2:[^ ]+]] = load ptr, ptr %obj2.addr, align 8

;VERSION0:   %{{[^ ,]+}} = call i32 @__tgt_use_interop(ptr %[[INTEROP_OBJ2]])
; With -vpo-paropt-dispatch-codegen-version=1, just check that __tgt_interop_use_async is called instead of __tgt_use_interop
;VERSION1:   call void @__tgt_interop_use_async(ptr @.kmpc_loc{{.*}}, i32 %my.tid{{.*}}, ptr %[[INTEROP_OBJ2]], i8 0, ptr null)

;ALL-NEXT: %[[INTEROP_OBJ3:[^ ]+]] = load ptr, ptr %obj3.addr, align 8
;ALL-NEXT: %{{[^ ,]+}} = call i32 @__tgt_release_interop(ptr %[[INTEROP_OBJ3]])
;ALL-NEXT: store ptr null, ptr %obj3.addr, align 8
;ALL:      call void @__kmpc_omp_task_complete_if0(ptr @{{[^ ,]+}}, i32 %{{[^ ,]+}}, ptr %[[TASK_ALLOC]])

define dso_local void @foo(ptr %obj1, ptr %obj2, ptr %obj3, ptr %dep.array) {
entry:
  %obj1.addr = alloca ptr, align 8
  %obj2.addr = alloca ptr, align 8
  %obj3.addr = alloca ptr, align 8
  store ptr %obj1, ptr %obj1.addr, align 8
  store ptr %obj2, ptr %obj2.addr, align 8
  store ptr %obj3, ptr %obj3.addr, align 8

  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.INTEROP"(),
    "QUAL.OMP.INIT:TARGETSYNC.PREFER"(ptr %obj1.addr, i64 3, i64 6),
    "QUAL.OMP.USE"(ptr %obj2.addr),
    "QUAL.OMP.DESTROY"(ptr %obj3.addr),
    "QUAL.OMP.DEPARRAY"(i32 3, ptr %dep.array) ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.INTEROP"() ]

  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
