; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S <%s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S <%s | FileCheck %s
;
; Test support for the QUAL.OMP.DEPARRAY representation for DEPEND clauses.
;
; // C test with depobj construct and depend clauses with depobj, in, out depend-types
; #include <stdio.h>
; #include <omp.h>
;
; double ccc;
; int main() {
;   int aaa;
;   float bbb;
;
;   // #pragma omp parallel num_threads(8)
;   {
;     // #pragma omp single
;     {
;       omp_depend_t obj1;
;       #pragma omp depobj(obj1) depend(out:aaa)
;
;       #pragma omp task depend(depobj:obj1) depend(out:bbb)
;       {
;         // printf( "task1\n");
;       }
;
;       #pragma omp task depend(in:aaa,bbb) depend(out:ccc)
;       {
;         // printf( "task2\n");
;       }
;
;       #pragma omp depobj(obj1) destroy
;     }// single
;   } // parallel
;
;   return 0;
; }

; CHECK: call void @__kmpc_omp_task_with_deps(ptr @.kmpc_loc{{.*}}, i32 %{{.*}}, ptr %.task.alloc, i32 %dep.array.size, ptr %dep.array, i32 0, ptr null)
; CHECK: call void @__kmpc_omp_task_with_deps(ptr @.kmpc_loc{{.*}}, i32 %{{.*}}, ptr %.task.alloc{{.*}}, i32 3, ptr %dep.array.2, i32 0, ptr null)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.ident_t = type { i32, i32, i32, i32, ptr }
%struct.kmp_depend_info = type { i64, i64, i8 }

@ccc = dso_local global double 0.000000e+00, align 8
@0 = private unnamed_addr constant [23 x i8] c";unknown;unknown;0;0;;\00", align 1
@1 = private unnamed_addr constant %struct.ident_t { i32 0, i32 2, i32 0, i32 22, ptr @0 }, align 8

define dso_local i32 @main() {
entry:
  %retval = alloca i32, align 4
  %aaa = alloca i32, align 4
  %bbb = alloca float, align 4
  %obj1 = alloca ptr, align 8
  %depobj.size.addr = alloca i64, align 8
  %saved_stack = alloca ptr, align 8
  %__vla_expr0 = alloca i64, align 8
  %dep.counter.addr = alloca i64, align 8
  %.dep.arr.addr1 = alloca [3 x %struct.kmp_depend_info], align 8
  %dep.counter.addr2 = alloca i64, align 8
  %i = call i32 @__kmpc_global_thread_num(ptr @1)
  store i32 0, ptr %retval, align 4
  %.dep.arr.addr = call ptr @__kmpc_alloc(i32 %i, i64 48, ptr null)
  store i64 1, ptr %.dep.arr.addr, align 8
  %i3 = getelementptr %struct.kmp_depend_info, ptr %.dep.arr.addr, i64 1
  %i5 = ptrtoint ptr %aaa to i64
  store i64 %i5, ptr %i3, align 8
  %i6 = getelementptr inbounds %struct.kmp_depend_info, ptr %i3, i32 0, i32 1
  store i64 4, ptr %i6, align 8
  %i7 = getelementptr inbounds %struct.kmp_depend_info, ptr %i3, i32 0, i32 2
  store i8 3, ptr %i7, align 8
  %i8 = getelementptr %struct.kmp_depend_info, ptr %.dep.arr.addr, i64 1
  store ptr %i8, ptr %obj1, align 8
  %i10 = load ptr, ptr %obj1, align 8
  %i12 = getelementptr %struct.kmp_depend_info, ptr %i10, i64 -1
  %i14 = load i64, ptr %i12, align 8
  store i64 0, ptr %depobj.size.addr, align 8
  %i15 = load i64, ptr %depobj.size.addr, align 8
  %i16 = add nuw i64 %i15, %i14
  store i64 %i16, ptr %depobj.size.addr, align 8
  %i17 = load i64, ptr %depobj.size.addr, align 8
  %i18 = add nuw i64 0, %i17
  %i19 = add nuw i64 %i18, 1
  %i20 = call ptr @llvm.stacksave()
  store ptr %i20, ptr %saved_stack, align 8
  %dep.array = alloca %struct.kmp_depend_info, i64 %i19, align 16
  store i64 %i19, ptr %__vla_expr0, align 8
  %dep.array.size = trunc i64 %i19 to i32
  %i24 = ptrtoint ptr %bbb to i64
  store i64 %i24, ptr %dep.array, align 16
  %i25 = getelementptr inbounds %struct.kmp_depend_info, ptr %dep.array, i32 0, i32 1
  store i64 4, ptr %i25, align 8
  %i26 = getelementptr inbounds %struct.kmp_depend_info, ptr %dep.array, i32 0, i32 2
  store i8 3, ptr %i26, align 16
  store i64 1, ptr %dep.counter.addr, align 8
  %i27 = load ptr, ptr %obj1, align 8
  %i29 = getelementptr %struct.kmp_depend_info, ptr %i27, i64 -1
  %i31 = load i64, ptr %i29, align 8
  %i32 = mul nuw i64 24, %i31
  %i33 = load i64, ptr %dep.counter.addr, align 8
  %i34 = getelementptr %struct.kmp_depend_info, ptr %dep.array, i64 %i33
  call void @llvm.memcpy.p0.p0.i64(ptr align 8 %i34, ptr align 8 %i27, i64 %i32, i1 false)
  %i37 = add nuw i64 %i33, %i31
  store i64 %i37, ptr %dep.counter.addr, align 8
  %i39 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(),
    "QUAL.OMP.DEPARRAY"(i32 %dep.array.size, ptr %dep.array) ]

  call void @llvm.directive.region.exit(token %i39) [ "DIR.OMP.END.TASK"() ]
  %i40 = load ptr, ptr %saved_stack, align 8
  call void @llvm.stackrestore(ptr %i40)
  %i44 = ptrtoint ptr %aaa to i64
  store i64 %i44, ptr %.dep.arr.addr1, align 8
  %i45 = getelementptr inbounds %struct.kmp_depend_info, ptr %.dep.arr.addr1, i32 0, i32 1
  store i64 4, ptr %i45, align 8
  %i46 = getelementptr inbounds %struct.kmp_depend_info, ptr %.dep.arr.addr1, i32 0, i32 2
  store i8 1, ptr %i46, align 8
  %i47 = getelementptr %struct.kmp_depend_info, ptr %.dep.arr.addr1, i64 1
  %i49 = ptrtoint ptr %bbb to i64
  store i64 %i49, ptr %i47, align 8
  %i50 = getelementptr inbounds %struct.kmp_depend_info, ptr %i47, i32 0, i32 1
  store i64 4, ptr %i50, align 8
  %i51 = getelementptr inbounds %struct.kmp_depend_info, ptr %i47, i32 0, i32 2
  store i8 1, ptr %i51, align 8
  %i52 = getelementptr %struct.kmp_depend_info, ptr %.dep.arr.addr1, i64 2
  store i64 ptrtoint (ptr @ccc to i64), ptr %i52, align 8
  %i54 = getelementptr inbounds %struct.kmp_depend_info, ptr %i52, i32 0, i32 1
  store i64 8, ptr %i54, align 8
  %i55 = getelementptr inbounds %struct.kmp_depend_info, ptr %i52, i32 0, i32 2
  store i8 3, ptr %i55, align 8
  store i64 3, ptr %dep.counter.addr2, align 8
  %dep.array.2 = getelementptr inbounds [3 x %struct.kmp_depend_info], ptr %.dep.arr.addr1, i64 0, i64 0
  %i57 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(),
    "QUAL.OMP.DEPARRAY"(i32 3, ptr %dep.array.2) ]

  call void @llvm.directive.region.exit(token %i57) [ "DIR.OMP.END.TASK"() ]
  %i58 = load ptr, ptr %obj1, align 8
  %i60 = getelementptr %struct.kmp_depend_info, ptr %i58, i64 -1
  call void @__kmpc_free(i32 %i, ptr %i60, ptr null)
  ret i32 0
}

declare i32 @__kmpc_global_thread_num(ptr nocapture readonly)
declare ptr @__kmpc_alloc(i32, i64, ptr)
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare ptr @llvm.stacksave()
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg)
declare void @llvm.stackrestore(ptr)
declare void @__kmpc_free(i32, ptr, ptr)
