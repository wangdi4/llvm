; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -opaque-pointers=0 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s
;
; SRC:
; #include <omp.h>
; #include <stdio.h>
;
; int main() {
;  int Result[10];
;  int a;
;  #pragma omp taskwait
;   printf("Result = %d .... \n", Result[0]);
;}
;
;This test checks that we are parsing the nowait clause on taskwait depend as a task depend and that we are deleting the fence instruction
;The input IR was hand-modified because front end doesn't yet handle the nowait and Depend clauses on taskwait
; CHECK: %{{.*}} = tail call i32 @__kmpc_global_thread_num(%struct.ident_t* @{{.*}})
; CHECK: %{{.*}}  = call i8* @__kmpc_omp_task_alloc(%struct.ident_t* @{{.*}}, i32 %{{.*}}, i32 1, i64 72, i64 0, i32 (i32, i8*)* bitcast (void (i32, %{{.*}}*)* @{{.*}} to i32 (i32, i8*)*))
; CHECK: call void @__kmpc_omp_task_with_deps(%struct.ident_t* @{{.*}}, i32 %{{.*}}, i8* %{{.*}}, i32 1, i8* %{{.*}}, i32 0, i8* null)
; CHECK-NOT: fence acq_rel

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

@.str = private unnamed_addr constant [19 x i8] c"Result = %d .... \0A\00", align 1

define dso_local i32 @main() {
entry:
  %Result = alloca [10 x i32], align 16
  %a = alloca i32, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKWAIT"(),
    "QUAL.OMP.DEPEND.IN"(i32* %a),
    "QUAL.OMP.NOWAIT"() ]
  fence acq_rel
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TASKWAIT"() ]
  %arrayidx = getelementptr inbounds [10 x i32], [10 x i32]* %Result, i64 0, i64 0
  %1 = load i32, i32* %arrayidx, align 16
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([19 x i8], [19 x i8]* @.str, i64 0, i64 0), i32 %1)
  ret i32 0
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare dso_local i32 @printf(i8*, ...)
