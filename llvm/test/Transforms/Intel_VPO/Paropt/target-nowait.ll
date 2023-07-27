; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-enable-async-helper-thread -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-enable-async-helper-thread -S %s | FileCheck %s


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
; CHECK: call ptr @__kmpc_omp_task_alloc(ptr {{.*}}, i32 {{.*}}, i32 129, {{.*}}

target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

@.str = private unnamed_addr constant [10 x i8] c"tid = %d\0A\00", align 1
@.str.1 = private unnamed_addr constant [19 x i8] c"Result = %d .... \0A\00", align 1

define dso_local i32 @main() {
entry:
  %Result = alloca [10 x i32], align 16
  call void @llvm.lifetime.start.p0(i64 40, ptr %Result)
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(),
    "QUAL.OMP.TARGET.TASK"(),
    "QUAL.OMP.SHARED:TYPED"(ptr %Result, i32 0, i64 10) ]

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.NOWAIT"(),
    "QUAL.OMP.MAP.TOFROM"(ptr %Result, ptr %Result, i64 40, i64 35, ptr null, ptr null) ] ; MAP type: 35 = 0x23 = TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)

  %call = call i32 @omp_get_num_teams()
  store i32 %call, ptr %Result, align 16
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET"() ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TASK"() ]

  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.NUM_THREADS"(i32 4) ]

  %call1 = call i32 @omp_get_thread_num()
  %call2 = call i32 (ptr, ...) @printf(ptr @.str, i32 %call1)
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.PARALLEL"() ]

  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKWAIT"() ]

  fence acq_rel
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.TASKWAIT"() ]

  %4 = load i32, ptr %Result, align 16
  %call4 = call i32 (ptr, ...) @printf(ptr @.str.1, i32 %4)
  call void @llvm.lifetime.end.p0(i64 40, ptr %Result)
  ret i32 0
}

declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture)
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare dso_local i32 @omp_get_num_teams()
declare dso_local i32 @printf(ptr, ...)
declare dso_local i32 @omp_get_thread_num()
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture)

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 64770, i32 13746639, !"_Z4main", i32 7, i32 0, i32 0}
