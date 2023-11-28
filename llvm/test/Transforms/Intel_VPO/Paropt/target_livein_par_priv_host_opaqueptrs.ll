; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -aa-pipeline=basic-aa -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src:

; int main()
; {
;   int x[2];
; #pragma omp target
; #pragma omp parallel private(x)
;   ;
; }

; Make sure x is not passed in as a parameter for the parallel, target regions.
; CHECK: define internal void @main.DIR.OMP.PARALLEL{{.*}}(ptr %tid, ptr %bid)
; CHECK:   %x.priv = alloca [2 x i32], align 4

; CHECK: define internal void @__omp_offloading{{.*}}Z4main_l4()

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

define dso_local i32 @main() {
entry:
  %x = alloca [2 x i32], align 4
  %array.begin1 = getelementptr inbounds [2 x i32], ptr %x, i32 0, i32 0

  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
 "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
 "QUAL.OMP.LIVEIN"(ptr %x) ]

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
 "QUAL.OMP.PRIVATE:TYPED"(ptr %x, i32 0, i64 2) ]

  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret i32 0
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!0 = !{i32 0, i32 58, i32 -684467853, !"_Z4main", i32 4, i32 0, i32 0}
!1 = !{i32 7, !"openmp", i32 50}
