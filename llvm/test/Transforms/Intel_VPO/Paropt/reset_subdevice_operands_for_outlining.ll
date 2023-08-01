; REQUIRES: asserts
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -debug -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -debug -S %s 2>&1 | FileCheck %s
;
; Test src:
;
; void foo(int* p)
; {
; int n;
; #pragma omp target device(0) subdevice(1,n:1:1) map(p[0:20])
;  {
;   p[2] = 3;
;  }
; }
;
; This test checks that we are setting non-constant Subdevice clause operands to null before outlining.
; CHECK: %{{.*}} = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.DEVICE"(i32 0), "QUAL.OMP.SUBDEVICE"(i32 1, ptr null, i32 1, i32 1)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

define dso_local void @foo(ptr %p) {
entry:
  %p.addr = alloca ptr, align 8
  %n = alloca i32, align 4
  %.capture_expr.0 = alloca i32, align 4
  %p.map.ptr.tmp = alloca ptr, align 8
  store ptr %p, ptr %p.addr, align 8
  %0 = load i32, ptr %n, align 4
  store i32 %0, ptr %.capture_expr.0, align 4
  %1 = load i32, ptr %.capture_expr.0, align 4
  %2 = load ptr, ptr %p.addr, align 8
  %3 = load ptr, ptr %p.addr, align 8
  %4 = load ptr, ptr %p.addr, align 8

  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.DEVICE"(i32 0),
    "QUAL.OMP.SUBDEVICE"(i32 1, i32 %1, i32 1, i32 1),
    "QUAL.OMP.MAP.TOFROM"(ptr %3, ptr %4, i64 80, i64 35),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %p.map.ptr.tmp, ptr null, i32 1) ]

  store ptr %3, ptr %p.map.ptr.tmp, align 8
  %6 = load ptr, ptr %p.map.ptr.tmp, align 8
  %ptridx = getelementptr inbounds i32, ptr %6, i64 2
  store i32 3, ptr %ptridx, align 4
  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 2054, i32 61031815, !"_Z3foo", i32 5, i32 0, i32 0}
