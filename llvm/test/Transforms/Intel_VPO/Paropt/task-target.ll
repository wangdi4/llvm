; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; CHECK: define{{.*}}@main.DIR.OMP.TASK.{{[0-9]+}}
; CHECK-DAG: %.offload_baseptrs = alloca
; CHECK-DAG: %.offload_ptrs = alloca [1 x ptr]
; CHECK-DAG: %.run_host_version = alloca i32
; CHECK: call{{.*}}tgt_target

; The local temporaries created by the target outlining, should be
; declared inside the same region as the target offload call, to ensure
; that they are not killed by other OMP code in the program (a task region
; in this case)

target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

@.str = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1

define dso_local i32 @main() {
entry:
  %retval = alloca i32, align 4
  %a = alloca [10 x i32], align 16
  store i32 0, ptr %retval, align 4
  call void @llvm.lifetime.start.p0(i64 40, ptr %a)
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
     "QUAL.OMP.SHARED"(ptr %a) ]

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SINGLE"() ]

  fence acquire
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(),
     "QUAL.OMP.SHARED"(ptr %a) ]

  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
     "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
     "QUAL.OMP.MAP.TOFROM:AGGRHEAD"(ptr %a, ptr %a, i64 4) ]

  store i32 5, ptr %a, align 16
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.TARGET"() ]
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TASK"() ]
  fence release
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.SINGLE"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  %4 = load i32, ptr %a, align 16
  %call = call i32 (ptr, ...) @printf(ptr @.str, i32 %4)
  call void @llvm.lifetime.end.p0(i64 40, ptr %a)
  ret i32 0
}

declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture)

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

declare dso_local i32 @printf(ptr, ...)

declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture)

!omp_offload.info = !{!0}

!0 = !{i32 0, i32 2055, i32 -939495916, !"main", i32 8, i32 0, i32 0}
