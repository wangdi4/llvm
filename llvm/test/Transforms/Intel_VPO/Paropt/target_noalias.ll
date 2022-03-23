; RUN: opt -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -simplifycfg -switch-to-offload -S %s | FileCheck %s
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
; CHECK: define {{.+}} @__omp_offloading_{{.+}}__Z5test1_{{.+}}(double* noalias %{{.+}}, double* noalias %{{.+}})
; CHECK: define {{.+}} @__omp_offloading_{{.+}}__Z5test2_{{.+}}(double* noalias %{{.+}}, double* noalias %{{.+}})
; CHECK: define {{.+}} @__omp_offloading_{{.+}}__Z5test3_{{.+}}(double* noalias %{{.+}}, double* noalias %{{.+}})

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"
target device_triples = "x86_64-pc-linux-gnu"

define hidden void @test1(double* noalias %dst, double* noalias %src) #0 {
entry:
  %dst.addr = alloca double*, align 8
  %src.addr = alloca double*, align 8
  %dst.map.ptr.tmp = alloca double*, align 8
  %src.map.ptr.tmp = alloca double*, align 8
  store double* %dst, double** %dst.addr, align 8
  store double* %src, double** %src.addr, align 8
  %0 = load double*, double** %dst.addr, align 8
  %1 = load double*, double** %src.addr, align 8
  %2 = load double*, double** %dst.addr, align 8
  %3 = load double*, double** %dst.addr, align 8
  %arrayidx = getelementptr inbounds double, double* %3, i64 0
  %4 = load double*, double** %src.addr, align 8
  %5 = load double*, double** %src.addr, align 8
  %arrayidx1 = getelementptr inbounds double, double* %5, i64 0
  %6 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TOFROM"(double* %2, double* %arrayidx, i64 8, i64 35), "QUAL.OMP.MAP.TOFROM"(double* %4, double* %arrayidx1, i64 8, i64 35), "QUAL.OMP.PRIVATE"(double** %dst.map.ptr.tmp), "QUAL.OMP.PRIVATE"(double** %src.map.ptr.tmp) ]
  store double* %2, double** %dst.map.ptr.tmp, align 8
  store double* %4, double** %src.map.ptr.tmp, align 8
  %7 = load double*, double** %src.map.ptr.tmp, align 8
  %ptridx = getelementptr inbounds double, double* %7, i64 0
  %8 = load double, double* %ptridx, align 8
  %9 = load double*, double** %dst.map.ptr.tmp, align 8
  %ptridx2 = getelementptr inbounds double, double* %9, i64 0
  store double %8, double* %ptridx2, align 8
  call void @llvm.directive.region.exit(token %6) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

define hidden void @test2() #0 {
entry:
  %src = alloca double*, align 8
  %dst = alloca double*, align 8
  %dst.map.ptr.tmp = alloca double*, align 8
  %src.map.ptr.tmp = alloca double*, align 8
  %call = call noalias i8* @malloc(i64 8)
  %0 = bitcast i8* %call to double*
  store double* %0, double** %src, align 8
  %call1 = call noalias i8* @malloc(i64 8)
  %1 = bitcast i8* %call1 to double*
  store double* %1, double** %dst, align 8
  %2 = load double*, double** %dst, align 8
  %3 = load double*, double** %src, align 8
  %4 = load double*, double** %dst, align 8
  %5 = load double*, double** %dst, align 8
  %arrayidx = getelementptr inbounds double, double* %5, i64 0
  %6 = load double*, double** %src, align 8
  %7 = load double*, double** %src, align 8
  %arrayidx2 = getelementptr inbounds double, double* %7, i64 0
  %8 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 1), "QUAL.OMP.MAP.TOFROM"(double* %4, double* %arrayidx, i64 8, i64 35), "QUAL.OMP.MAP.TOFROM"(double* %6, double* %arrayidx2, i64 8, i64 35), "QUAL.OMP.PRIVATE"(double** %dst.map.ptr.tmp), "QUAL.OMP.PRIVATE"(double** %src.map.ptr.tmp) ]
  store double* %4, double** %dst.map.ptr.tmp, align 8
  store double* %6, double** %src.map.ptr.tmp, align 8
  %9 = load double*, double** %src.map.ptr.tmp, align 8
  %ptridx = getelementptr inbounds double, double* %9, i64 0
  %10 = load double, double* %ptridx, align 8
  %11 = load double*, double** %dst.map.ptr.tmp, align 8
  %ptridx3 = getelementptr inbounds double, double* %11, i64 0
  store double %10, double* %ptridx3, align 8
  call void @llvm.directive.region.exit(token %8) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

define hidden void @test3() #0 {
entry:
  %src = alloca double, align 8
  %dst = alloca double, align 8
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 2), "QUAL.OMP.MAP.TOFROM"(double* %dst, double* %dst, i64 8, i64 35), "QUAL.OMP.MAP.TOFROM"(double* %src, double* %src, i64 8, i64 35) ]
  %1 = load double, double* %src, align 8
  store double %1, double* %dst, align 8
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

declare noalias i8* @malloc(i64)

attributes #0 = { "contains-openmp-target"="true" "may-have-openmp-directive"="true" }

!omp_offload.info = !{!0, !1, !2}

!0 = !{i32 0, i32 51, i32 -677946137, !"_Z5test2", i32 11, i32 1, i32 0}
!1 = !{i32 0, i32 51, i32 -677946137, !"_Z5test3", i32 18, i32 2, i32 0}
!2 = !{i32 0, i32 51, i32 -677946137, !"_Z5test1", i32 4, i32 0, i32 0}
