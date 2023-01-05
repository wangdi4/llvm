; RUN: opt -enable-new-pm=0 -opaque-pointers -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -opaque-pointers -passes='function(vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src:
;
; void foo() {
;  #pragma omp target
;    ;
;  #pragma omp target
;    ;
;  #pragma omp target
;    ;
;  }

; The test IR was hand-modified to add an extra region number field to the
; offload metadata for the second and third target regions.

; When present, region number is at entry 5, and flags are at entry 7.
; Otherwise, region number is implicitly 0, flags are at entry 6.

; Check that when the region number is non-zero, it gets used in the outlined
; region's name, but not when it is 0 or absent.

; CHECK: @__omp_offloading_10309_b1f36f2__Z3foov_l2.region_id = weak constant i8 0
; CHECK: @__omp_offloading_10309_b1f36f2__Z3foov_l4.region_id = weak constant i8 0
; CHECK: @__omp_offloading_10309_b1f36f2__Z3foov_l4.1.region_id = weak constant i8 0

; CHECK: define{{.*}} void @__omp_offloading_10309_b1f36f2__Z3foov_l2()
; CHECK: define{{.*}} void @__omp_offloading_10309_b1f36f2__Z3foov_l4()
; CHECK: define{{.*}} void @__omp_offloading_10309_b1f36f2__Z3foov_l4.1()

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64"
target device_triples = "x86_64"

define protected void @_Z3foov() {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0) ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 1) ]

  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET"() ]

  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 2) ]

  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TARGET"() ]

  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

!omp_offload.info = !{!0, !1, !2}

!0 = !{i32 0, i32 66313, i32 186595058, !"_Z3foov", i32 2, i32 0, i32 0}
!1 = !{i32 0, i32 66313, i32 186595058, !"_Z3foov", i32 4, i32 0, i32 1, i32 0}
!2 = !{i32 0, i32 66313, i32 186595058, !"_Z3foov", i32 4, i32 1, i32 2, i32 0}
