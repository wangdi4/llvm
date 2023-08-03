; RUN: opt -bugpoint-enable-legacy-pm -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -simplifycfg -switch-to-offload -S %s | FileCheck %s
; RUN: opt -aa-pipeline=basic-aa -passes='function(loop(loop-rotate),vpo-cfg-restructuring,vpo-paropt-prepare,loop-simplify,sroa,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt,function(simplifycfg)' -switch-to-offload -S %s | FileCheck %s
;
; This test checks that paropt adds noalias attribute to outlined target
; region's arguments if actuals are known to be noalias and addresses do
; not escape.
;
; void test1(double * __restrict dst, double * __restrict src) {
; #pragma omp target map(src[0:1], dst[0:1])
;   dst[0] = src[0];
; }
;
; void test2() {
;   double *src = (double*) malloc(sizeof(double));
;   double *dst = (double*) malloc(sizeof(double));
; #pragma omp target map(src[0:1], dst[0:1])
;   dst[0] = src[0];
; }
;
; void test3() {
;   double src;
;   double dst;
; #pragma omp target map(src, dst)
;   dst = src;
; }
;
; CHECK: define {{.+}} @__omp_offloading_{{.+}}__Z5test1_{{.+}}(ptr noalias %{{.+}}, ptr noalias %{{.+}})
; CHECK: define {{.+}} @__omp_offloading_{{.+}}__Z5test2_{{.+}}(ptr noalias %{{.+}}, ptr noalias %{{.+}})
; CHECK: define {{.+}} @__omp_offloading_{{.+}}__Z5test3_{{.+}}(ptr noalias %{{.+}}, ptr noalias %{{.+}})

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"
target device_triples = "x86_64-pc-linux-gnu"

define hidden void @test1(ptr noalias %dst, ptr noalias %src) #0 {
entry:
  %dst.addr = alloca ptr, align 8
  %src.addr = alloca ptr, align 8
  %dst.map.ptr.tmp = alloca ptr, align 8
  %src.map.ptr.tmp = alloca ptr, align 8
  store ptr %dst, ptr %dst.addr, align 8
  store ptr %src, ptr %src.addr, align 8
  %0 = load ptr, ptr %dst.addr, align 8
  %1 = load ptr, ptr %src.addr, align 8
  %2 = load ptr, ptr %dst.addr, align 8
  %3 = load ptr, ptr %dst.addr, align 8
  %4 = load ptr, ptr %src.addr, align 8
  %5 = load ptr, ptr %src.addr, align 8
  %6 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(ptr %2, ptr %3, i64 8, i64 35, ptr null, ptr null), ; MAP type: 35 = 0x23 = TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.MAP.TOFROM"(ptr %4, ptr %5, i64 8, i64 35, ptr null, ptr null), ; MAP type: 35 = 0x23 = TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.PRIVATE:TYPED"(ptr %dst.map.ptr.tmp, ptr null, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %src.map.ptr.tmp, ptr null, i32 1) ]

  store ptr %2, ptr %dst.map.ptr.tmp, align 8
  store ptr %4, ptr %src.map.ptr.tmp, align 8
  %7 = load ptr, ptr %src.map.ptr.tmp, align 8
  %8 = load double, ptr %7, align 8
  %9 = load ptr, ptr %dst.map.ptr.tmp, align 8
  store double %8, ptr %9, align 8
  call void @llvm.directive.region.exit(token %6) [ "DIR.OMP.END.TARGET"() ]

  ret void
}

define hidden void @test2() #0 {
entry:
  %src = alloca ptr, align 8
  %dst = alloca ptr, align 8
  %dst.map.ptr.tmp = alloca ptr, align 8
  %src.map.ptr.tmp = alloca ptr, align 8
  %call = call noalias ptr @malloc(i64 8)
  store ptr %call, ptr %src, align 8
  %call1 = call noalias ptr @malloc(i64 8)
  store ptr %call1, ptr %dst, align 8
  %0 = load ptr, ptr %dst, align 8
  %1 = load ptr, ptr %src, align 8
  %2 = load ptr, ptr %dst, align 8
  %3 = load ptr, ptr %dst, align 8
  %4 = load ptr, ptr %src, align 8
  %5 = load ptr, ptr %src, align 8
  %6 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 1),
    "QUAL.OMP.MAP.TOFROM"(ptr %2, ptr %3, i64 8, i64 35, ptr null, ptr null), ; MAP type: 35 = 0x23 = TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.MAP.TOFROM"(ptr %4, ptr %5, i64 8, i64 35, ptr null, ptr null), ; MAP type: 35 = 0x23 = TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.PRIVATE:TYPED"(ptr %dst.map.ptr.tmp, ptr null, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %src.map.ptr.tmp, ptr null, i32 1) ]

  store ptr %2, ptr %dst.map.ptr.tmp, align 8
  store ptr %4, ptr %src.map.ptr.tmp, align 8
  %7 = load ptr, ptr %src.map.ptr.tmp, align 8
  %8 = load double, ptr %7, align 8
  %9 = load ptr, ptr %dst.map.ptr.tmp, align 8
  store double %8, ptr %9, align 8
  call void @llvm.directive.region.exit(token %6) [ "DIR.OMP.END.TARGET"() ]

  ret void
}

define hidden void @test3() #0 {
entry:
  %src = alloca double, align 8
  %dst = alloca double, align 8
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 2),
    "QUAL.OMP.MAP.TOFROM"(ptr %dst, ptr %dst, i64 8, i64 35, ptr null, ptr null), ; MAP type: 35 = 0x23 = TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.MAP.TOFROM"(ptr %src, ptr %src, i64 8, i64 35, ptr null, ptr null) ] ; MAP type: 35 = 0x23 = TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)

  %1 = load double, ptr %src, align 8
  store double %1, ptr %dst, align 8
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare noalias ptr @malloc(i64)
attributes #0 = { "contains-openmp-target"="true" "may-have-openmp-directive"="true" }

!omp_offload.info = !{!0, !1, !2}
!0 = !{i32 0, i32 51, i32 -677946137, !"_Z5test2", i32 11, i32 1, i32 0}
!1 = !{i32 0, i32 51, i32 -677946137, !"_Z5test3", i32 18, i32 2, i32 0}
!2 = !{i32 0, i32 51, i32 -677946137, !"_Z5test1", i32 4, i32 0, i32 0}
