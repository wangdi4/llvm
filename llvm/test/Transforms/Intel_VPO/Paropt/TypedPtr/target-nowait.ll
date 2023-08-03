; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-enable-async-helper-thread -S %s | FileCheck %s
; RUN: opt -opaque-pointers=0 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-enable-async-helper-thread -S %s | FileCheck %s


; #include <omp.h>
; #include <stdio.h>
;
; int main() {
;   int Result[10];
;
; #pragma omp target map(Result) nowait
; {
;    Result[0] = omp_get_num_teams();
; }
;
; #pragma omp parallel num_threads(4)
; {
;   printf("tid = %d\n", omp_get_thread_num());
; }
; #pragma omp taskwait
;   printf("Result = %d .... \n", Result[0]);
; }

; The flag of 3rd argument in __kmpc_omp_task_alloc is expected to be 129.
; CHECK: call i8* @__kmpc_omp_task_alloc(%struct.ident_t* {{.*}}, i32 {{.*}}, i32 129, {{.*}}

target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

@.str = private unnamed_addr constant [10 x i8] c"tid = %d\0A\00", align 1
@.str.1 = private unnamed_addr constant [19 x i8] c"Result = %d .... \0A\00", align 1

define dso_local i32 @main() {
entry:
  %Result = alloca [10 x i32], align 16
  %0 = bitcast [10 x i32]* %Result to i8*
  call void @llvm.lifetime.start.p0i8(i64 40, i8* %0)
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(),
    "QUAL.OMP.TARGET.TASK"(),
    "QUAL.OMP.SHARED"([10 x i32]* %Result) ]

  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.NOWAIT"(),
    "QUAL.OMP.MAP.TOFROM"([10 x i32]* %Result, [10 x i32]* %Result, i64 40, i64 35, i8* null, i8* null) ]

  %call = call i32 @omp_get_num_teams()
  %arrayidx = getelementptr inbounds [10 x i32], [10 x i32]* %Result, i64 0, i64 0
  store i32 %call, i32* %arrayidx, align 16
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TARGET"() ]

  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TASK"() ]

  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.NUM_THREADS"(i32 4) ]

  %call1 = call i32 @omp_get_thread_num()
  %call2 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([10 x i8], [10 x i8]* @.str, i64 0, i64 0), i32 %call1)
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.PARALLEL"() ]

  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKWAIT"() ]

  fence acq_rel
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.TASKWAIT"() ]

  %arrayidx3 = getelementptr inbounds [10 x i32], [10 x i32]* %Result, i64 0, i64 0
  %5 = load i32, i32* %arrayidx3, align 16
  %call4 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([19 x i8], [19 x i8]* @.str.1, i64 0, i64 0), i32 %5)
  %6 = bitcast [10 x i32]* %Result to i8*
  call void @llvm.lifetime.end.p0i8(i64 40, i8* %6)
  ret i32 0
}

declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture)
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare dso_local i32 @omp_get_num_teams()
declare dso_local i32 @printf(i8*, ...)
declare dso_local i32 @omp_get_thread_num()
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture)

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 64770, i32 13746639, !"_Z4main", i32 7, i32 0, i32 0}
