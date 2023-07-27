; INTEL_CUSTOMIZATION
; RUN: opt -opaque-pointers=0 -vpo-paropt-dispatch-codegen-version=0 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s -check-prefix=OCG -check-prefix=ALL
; RUN: opt -opaque-pointers=0 -vpo-paropt-dispatch-codegen-version=0 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s -check-prefix=OCG -check-prefix=ALL
; RUN: opt -opaque-pointers=0 -vpo-paropt-dispatch-codegen-version=1 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -S <%s | FileCheck %s -check-prefix=NCG -check-prefix=ALL
; RUN: opt -opaque-pointers=0 -vpo-paropt-dispatch-codegen-version=1 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S <%s | FileCheck %s -check-prefix=NCG -check-prefix=ALL

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

;ALL: call void @__kmpc_omp_task_begin_if0(%struct.ident_t* @{{[^ ,]+}}, i32 %{{[^ ,]+}}, i8* %{{[^ ,]+}})
;ALL-NEXT:  %[[INTEROP_CAST:[^ ]+]] = bitcast i64* %{{[^ ,]+}} to i8**
;ALL-NEXT:  %[[INTEROP_OBJ:[^ ]+]] = call i8* @__tgt_create_interop(i64 %{{[^ ,]+}}, i32 1, i32 0, i8* null)
;ALL-NEXT:  store i8* %[[INTEROP_OBJ]], i8** %[[INTEROP_CAST]], align 8
;ALL-NEXT:  %[[INTEROP_CAST2:[^ ]+]] = bitcast i64* %{{[^ ,]+}} to i8**
;ALL-NEXT:  %[[INTEROP_OBJ_VAL1:[^ ]+]] = load i8*, i8** %[[INTEROP_CAST2]], align 8
;OCG-NEXT:  %{{[^ ,]+}} = call i32 @__tgt_use_interop(i8* %[[INTEROP_OBJ_VAL1]])
;NCG-NEXT:  %my.tid = load i32, i32* @"@tid.addr", align 4
;NCG-NEXT:  call void @__tgt_interop_use_async(%struct.ident_t* @{{.*}}, i32 %my.tid, i8* %[[INTEROP_OBJ_VAL1]], i8 1, i8* null)
;ALL-NEXT:  %[[INTEROP_CAST3:[^ ]+]] = bitcast i64* %{{[^ ,]+}} to i8**
;ALL-NEXT:  %[[INTEROP_OBJ_VAL2:[^ ]+]] = load i8*, i8** %[[INTEROP_CAST3]], align 8
;ALL-NEXT:  %{{[^ ,]+}} = call i32 @__tgt_release_interop(i8* %[[INTEROP_OBJ_VAL2]])
;ALL-NEXT:  store i8* null, i8** %[[INTEROP_CAST3]], align 8
;ALL:  call void @__kmpc_omp_task_complete_if0(%struct.ident_t* @{{[^ ,]+}}, i32 %{{[^ ,]+}}, i8* %{{[^ ,]+}})

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo_(i64* dereferenceable(8) %"foo_$OBJ1", i64* dereferenceable(8) %"foo_$OBJ2", i64* dereferenceable(8) %"foo_$OBJ3") {
alloca_0:
  %"$io_ctx" = alloca [8 x i64], align 8
  br label %bb1

bb1:                                              ; preds = %alloca_0
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.INTEROP"(),
    "QUAL.OMP.INIT:TARGETSYNC"(i64* %"foo_$OBJ1"),
    "QUAL.OMP.USE"(i64* %"foo_$OBJ2"),
    "QUAL.OMP.DESTROY"(i64* %"foo_$OBJ3"),
    "QUAL.OMP.NOWAIT"(),
    "QUAL.OMP.DEPEND.IN"(i64* %"foo_$OBJ3") ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.INTEROP"() ]

  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token %0)
; end INTEL_CUSTOMIZATION
