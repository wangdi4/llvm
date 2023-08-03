; REQUIRES: asserts
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -debug -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -debug -S %s 2>&1 | FileCheck %s
;
;Test src:
;
;void foo(int* p)
;  {
;  #pragma omp target map(p[0:20]) device(2) subdevice(1,2:3:4)
;    { p[2] = 3; }
;}
;
; check the encoding of Subdevice and the generation of DeviceID
; CHECK: Subdevice encoding : Device before shift: 0x0000000000000002, after shift: 0x0000000000000002
; CHECK-NEXT: Subdevice encoding : Level before shift: 0x0000000000000001, after shift: 0x0100000000000000
; CHECK-NEXT: Subdevice encoding : Start before shift: 0x0000000000000002, after shift: 0x0002000000000000
; CHECK-NEXT: Subdevice encoding : Length before shift: 0x0000000000000003, after shift: 0x0000030000000000
; CHECK-NEXT: Subdevice encoding : Stride before shift: 0x0000000000000004, after shift: 0x0000000400000000
; CHECK-NEXT: DeviceID after Subdevice encoding: 0x8102030400000002

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

define dso_local void @foo(ptr %p) {
entry:
  %p.addr = alloca ptr, align 8
  %p.map.ptr.tmp = alloca ptr, align 8
  store ptr %p, ptr %p.addr, align 8
  %0 = load ptr, ptr %p.addr, align 8
  %1 = load ptr, ptr %p.addr, align 8
  %2 = load ptr, ptr %p.addr, align 8
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.DEVICE"(i32 2),
    "QUAL.OMP.SUBDEVICE"(i32 1, i32 2, i32 3, i32 4),
    "QUAL.OMP.MAP.TOFROM"(ptr %1, ptr %2, i64 80, i64 35),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %p.map.ptr.tmp, ptr null, i32 1) ]

  store ptr %1, ptr %p.map.ptr.tmp, align 8
  %4 = load ptr, ptr %p.map.ptr.tmp, align 8
  %ptridx = getelementptr inbounds i32, ptr %4, i64 2
  store i32 3, ptr %ptridx, align 4
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 2054, i32 29116728, !"_Z3foo", i32 3, i32 0, i32 0}
