; RUN: opt -enable-new-pm=0  -vpo-cfg-restructuring -vpo-paropt-apply-config -vpo-paropt-prepare -vpo-paropt -vpo-paropt-config=%S/Inputs/Intel_paropt_registerallocmode.yaml -switch-to-offload -S %s | FileCheck %s
; RUN: opt -passes='require<vpo-paropt-config-analysis>,function(vpo-cfg-restructuring,vpo-paropt-apply-config,vpo-paropt-prepare),vpo-paropt' -vpo-paropt-config=%S/Inputs/Intel_paropt_registerallocmode.yaml -switch-to-offload -S %s | FileCheck %s

; Original code:
;int main()
;{
;  #pragma omp target
;  {
;  }
;  #pragma omp target
;  {
;  }
;  #pragma omp target
;  {
;  }
;  #pragma omp target
;  {
;  }
;  #pragma omp target
;  {
;  }
;  return 0;
;}
; CHECK-DAG: @__omp_offloading_10309_1260012__Z4main_l3() #{{[0-9]+}} !RegisterAllocMode ![[FLAG:[0-9]+]]
; CHECK-DAG: ![[FLAG]] = !{i32 1}

; CHECK-DAG: @__omp_offloading_10309_1260012__Z4main_l6() #{{[0-9]+}} !RegisterAllocMode ![[FLAG:[0-9]+]]
; CHECK-DAG: ![[FLAG]] = !{i32 2}

; CHECK-DAG: @__omp_offloading_10309_1260012__Z4main_l9() #{{[0-9]+}} !RegisterAllocMode ![[FLAG:[0-9]+]] {
; CHECK-DAG: ![[FLAG]] = !{i32 0}

; CHECK-DAG: @__omp_offloading_10309_1260012__Z4main_l12() #{{[0-9]+}} {

; CHECK-DAG: @__omp_offloading_10309_1260012__Z4main_l15() #{{[0-9]+}} {

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  store i32 0, ptr %retval, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0) ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 1) ]

  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET"() ]

  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 2) ]

  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TARGET"() ]

  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 3) ]

  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.TARGET"() ]

  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 4) ]

  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.TARGET"() ]

  ret i32 0
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }

!omp_offload.info = !{!0, !1, !2, !3, !4}
!llvm.module.flags = !{!5, !6, !7, !8}

!0 = !{i32 0, i32 66313, i32 19267602, !"_Z4main", i32 9, i32 2, i32 0}
!1 = !{i32 0, i32 66313, i32 19267602, !"_Z4main", i32 6, i32 1, i32 0}
!2 = !{i32 0, i32 66313, i32 19267602, !"_Z4main", i32 3, i32 0, i32 0}
!3 = !{i32 0, i32 66313, i32 19267602, !"_Z4main", i32 12, i32 3, i32 0}
!4 = !{i32 0, i32 66313, i32 19267602, !"_Z4main", i32 15, i32 4, i32 0}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{i32 7, !"openmp", i32 50}
!7 = !{i32 7, !"uwtable", i32 2}
!8 = !{i32 7, !"frame-pointer", i32 2}
