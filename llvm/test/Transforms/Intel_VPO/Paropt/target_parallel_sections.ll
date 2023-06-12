; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-loop-collapse -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-use-mapper-api=false -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse,vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-use-mapper-api=false -S %s | FileCheck %s
;
; It does a verification whether the "omp target parallel sections" construct is supported in the Paropt codegen for offloading.
;
; void test_square (const int n, double *d) {
;   #pragma omp target map(tofrom: d[0:n*n])
;   {
;     #pragma omp parallel sections
;     {
;       #pragma omp section
;       {
;         d[0] = d[0] + 1.0;
;       }
;       #pragma omp section
;       {
;         d[1] = d[1] + 1.0;
;       }
;       #pragma omp section
;       {
;         d[2] = d[2] + 1.0;
;       }
;       #pragma omp section
;       {
;         d[3] = d[3] + 1.0;
;       }
;     }
;   }
; }
;
; CHECK:  call i32 @__tgt_target({{.*}})

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

define dso_local void @test_square(i32 noundef %n, ptr noundef %d) {
entry:
  %n.addr = alloca i32, align 4
  %d.addr = alloca ptr, align 8
  %d.map.ptr.tmp = alloca ptr, align 8
  store i32 %n, ptr %n.addr, align 4
  store ptr %d, ptr %d.addr, align 8
  %0 = load ptr, ptr %d.addr, align 8
  %1 = load ptr, ptr %d.addr, align 8
  %2 = load ptr, ptr %d.addr, align 8
  %arrayidx = getelementptr inbounds double, ptr %2, i64 0
  %3 = load i32, ptr %n.addr, align 4
  %4 = load i32, ptr %n.addr, align 4
  %mul = mul nsw i32 %3, %4
  %conv = sext i32 %mul to i64
  %5 = mul nuw i64 %conv, 8
  %6 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(ptr %1, ptr %arrayidx, i64 %5, i64 35, ptr null, ptr null), ; MAP type: 35 = 0x23 = TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.PRIVATE:TYPED"(ptr %d.map.ptr.tmp, ptr null, i32 1) ]

  store ptr %1, ptr %d.map.ptr.tmp, align 8
  %7 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.SECTIONS"(),
    "QUAL.OMP.SHARED:TYPED"(ptr %d.map.ptr.tmp, ptr null, i32 1) ]

  %8 = call token @llvm.directive.region.entry() [ "DIR.OMP.SECTION"() ]

  %9 = load ptr, ptr %d.map.ptr.tmp, align 8
  %arrayidx1 = getelementptr inbounds double, ptr %9, i64 0
  %10 = load double, ptr %arrayidx1, align 8
  %add = fadd fast double %10, 1.000000e+00
  %11 = load ptr, ptr %d.map.ptr.tmp, align 8
  %arrayidx2 = getelementptr inbounds double, ptr %11, i64 0
  store double %add, ptr %arrayidx2, align 8
  call void @llvm.directive.region.exit(token %8) [ "DIR.OMP.END.SECTION"() ]

  %12 = call token @llvm.directive.region.entry() [ "DIR.OMP.SECTION"() ]

  %13 = load ptr, ptr %d.map.ptr.tmp, align 8
  %arrayidx3 = getelementptr inbounds double, ptr %13, i64 1
  %14 = load double, ptr %arrayidx3, align 8
  %add4 = fadd fast double %14, 1.000000e+00
  %15 = load ptr, ptr %d.map.ptr.tmp, align 8
  %arrayidx5 = getelementptr inbounds double, ptr %15, i64 1
  store double %add4, ptr %arrayidx5, align 8
  call void @llvm.directive.region.exit(token %12) [ "DIR.OMP.END.SECTION"() ]

  %16 = call token @llvm.directive.region.entry() [ "DIR.OMP.SECTION"() ]

  %17 = load ptr, ptr %d.map.ptr.tmp, align 8
  %arrayidx6 = getelementptr inbounds double, ptr %17, i64 2
  %18 = load double, ptr %arrayidx6, align 8
  %add7 = fadd fast double %18, 1.000000e+00
  %19 = load ptr, ptr %d.map.ptr.tmp, align 8
  %arrayidx8 = getelementptr inbounds double, ptr %19, i64 2
  store double %add7, ptr %arrayidx8, align 8
  call void @llvm.directive.region.exit(token %16) [ "DIR.OMP.END.SECTION"() ]

  %20 = call token @llvm.directive.region.entry() [ "DIR.OMP.SECTION"() ]

  %21 = load ptr, ptr %d.map.ptr.tmp, align 8
  %arrayidx9 = getelementptr inbounds double, ptr %21, i64 3
  %22 = load double, ptr %arrayidx9, align 8
  %add10 = fadd fast double %22, 1.000000e+00
  %23 = load ptr, ptr %d.map.ptr.tmp, align 8
  %arrayidx11 = getelementptr inbounds double, ptr %23, i64 3
  store double %add10, ptr %arrayidx11, align 8
  call void @llvm.directive.region.exit(token %20) [ "DIR.OMP.END.SECTION"() ]

  call void @llvm.directive.region.exit(token %7) [ "DIR.OMP.END.PARALLEL.SECTIONS"() ]

  call void @llvm.directive.region.exit(token %6) [ "DIR.OMP.END.TARGET"() ]

  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

!omp_offload.info = !{!0}

!0 = !{i32 0, i32 64773, i32 3826068, !"_Z11test_square", i32 2, i32 0, i32 0, i32 0}
