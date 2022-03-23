; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s
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

source_filename = "test.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

;check code generated for depend clauses
;CHECK: %[[DEPVEC_TYPE:[^ ]+]] = type { %__struct.kmp_depend_info, %__struct.kmp_depend_info, %__struct.kmp_depend_info }
;CHECK: @[[PREFER_LIST:[^ ]+]] = private unnamed_addr constant [2 x i32] [i32 3, i32 6]
;CHECK: %task.depend.vec = alloca %[[DEPVEC_TYPE]], align 8
;CHECK: %[[OBJ2:[^ ]+]] = ptrtoint i8** %obj2.addr to i64
;CHECK: store i64 %[[OBJ2]], i64* %.dep.base.ptr, align 8
;CHECK: store i64 8, i64* %.dep.num.bytes, align 8
;CHECK: store i8 1, i8* %.dep.flags, align 1
;CHECK: %[[OBJ3:[^ ]+]] = ptrtoint i8** %obj3.addr to i64
;CHECK: store i64 %[[OBJ3]], i64* %.dep.base.ptr{{[^ ,]*}}, align 8
;CHECK: store i64 8, i64* %.dep.num.bytes{{[^ ,]*}}, align 8
;CHECK: store i8 1, i8* %.dep.flags{{[^ ,]*}}, align 1
;CHECK: %[[OBJ1:[^ ]+]] = ptrtoint i8** %obj1.addr to i64
;CHECK: store i64 %[[OBJ1]], i64* %.dep.base.ptr{{[^ ,]*}}, align 8
;CHECK: store i64 8, i64* %.dep.num.bytes{{[^ ,]*}}, align 8
;CHECK: store i8 3, i8* %.dep.flags{{[^ ,]*}}, align 1
;CHECK: %[[BITCAST_TASK_DEP_VEC:[^ ]+]] = bitcast %__struct.kmp_depend_info* %{{[^ ,]+}} to i8*
;CHECK: call void @__kmpc_omp_wait_deps(%struct.ident_t* @{{[^ ,]+}}, i32 %{{[^ ,]+}}, i32 3, i8* %[[BITCAST_TASK_DEP_VEC]], i32 0, i8* null)

;check output IR for interop creation, use and release
;CHECK:  call void @__kmpc_omp_task_begin_if0(%struct.ident_t* @{{[^ ,]+}}, i32 %{{[^ ,]+}}, i8* %{{[^ ,]+}})
;CHECK-NEXT:  %{{[^ ,]+}} = call i8* @__tgt_create_interop(i64 %{{[^ ,]+}}, i32 1, i32 2, i8* bitcast ([2 x i32]* @[[PREFER_LIST]] to i8*))
;CHECK:   %[[INTEROP_OBJ2:[^ ]+]] = load i8*, i8** %{{[^ ,]+}}, align 8
;CHECK-NEXT:   %{{[^ ,]+}} = call i32 @__tgt_use_interop(i8* %[[INTEROP_OBJ2]])
;CHECK-NEXT:  %[[INTEROP_OBJ3:[^ ]+]] = load i8*, i8** %obj3.addr, align 8
;CHECK-NEXT:  %{{[^ ,]+}} = call i32 @__tgt_release_interop(i8* %[[INTEROP_OBJ3]])
;CHECK-NEXT: store i8* null, i8** %obj3.addr, align 8
;CHECK:  call void @__kmpc_omp_task_complete_if0(%struct.ident_t* @{{[^ ,]+}}, i32 %{{[^ ,]+}}, i8* %{{[^ ,]+}})

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo(i8* %obj1, i8* %obj2, i8* %obj3) #0 {
entry:
  %obj1.addr = alloca i8*, align 8
  %obj2.addr = alloca i8*, align 8
  %obj3.addr = alloca i8*, align 8
  store i8* %obj1, i8** %obj1.addr, align 8
  store i8* %obj2, i8** %obj2.addr, align 8
  store i8* %obj3, i8** %obj3.addr, align 8
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.INTEROP"(), "QUAL.OMP.INIT:TARGETSYNC.PREFER"(i8** %obj1.addr, i64 3, i64 6), "QUAL.OMP.USE"(i8** %obj2.addr), "QUAL.OMP.DESTROY"(i8** %obj3.addr), "QUAL.OMP.DEPEND.IN"(i8** %obj2.addr), "QUAL.OMP.DEPEND.IN"(i8** %obj3.addr), "QUAL.OMP.DEPEND.OUT"(i8** %obj1.addr) ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.INTEROP"() ]
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
