; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck %s

; // C++ source:
;#include <omp.h>
;
;int main ()
;{
;   #pragma omp parallel
;   {
;      omp_interop_t iop = 0;
;      #pragma omp interop init(targetsync:iop)
;   }
;   return 0;
;}
; CHECK: [[TID:%[a-zA-Z._0-9]+]] = load i32, ptr @"@tid.addr", align 4
; CHECK: call ptr @__kmpc_omp_task_alloc(ptr @.kmpc_loc.0.0, i32 [[TID]], i32 0, i64 0, i64 0, ptr null)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

define dso_local i32 @main() {
entry:
  %retval = alloca i32, align 4
  %iop = alloca ptr, align 8
  store i32 0, ptr %retval, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %iop, ptr null, i32 1) ]

  store ptr null, ptr %iop, align 8
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.INTEROP"(),
    "QUAL.OMP.INIT:TARGETSYNC"(ptr %iop) ]

  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.INTEROP"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]

  ret i32 0
}

declare token @llvm.directive.region.entry() 

declare void @llvm.directive.region.exit(token) 
