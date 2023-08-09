; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s
;
;Test src:
;
;int r;
;void bar() 
;{
;  #pragma omp taskgroup reduction(+ : r)
;  {
;  }
;}

;CHECK: %__struct.kmp_taskred_input_t = type { ptr, ptr, i32, ptr, ptr, ptr, i32 }

target datalayout = "e-m:e-p:32:32-p270:32:32-p271:32:32-p272:64:64-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

@r = dso_local global i32 0, align 4

define dso_local void @bar() {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKGROUP"(),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr @r, i32 0, i32 1) ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TASKGROUP"() ]

  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
