; REQUIRES: asserts
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S -vpo-paropt-force-member-of-to-point-to-base=true %s | FileCheck %s -check-prefix=TFORM
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S -vpo-paropt-force-member-of-to-point-to-base=true %s | FileCheck %s -check-prefix=TFORM
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -debug-only=vpo-paropt-target -S -vpo-paropt-force-member-of-to-point-to-base=true %s 2>&1 | FileCheck %s -check-prefix=DBG
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -debug-only=vpo-paropt-target -S -vpo-paropt-force-member-of-to-point-to-base=true %s 2>&1 | FileCheck %s -check-prefix=DBG

; Test src:
;
; #include <stdio.h>
;
; typedef struct {
;   int a;
;   float b;
; } S;
;
; __attribute__((noinline)) void foo() {
;   S x, y;
;   x.a = 0; y.a = 2;
;
; #pragma omp target map(tofrom:x.a, y.b)
;   {
;     x.a = 10;
;     y.b = 20;
;   }
;   printf("x = %d, y = %f\n", x.a, y.b);
; }
;
; int main() {
;   foo();
; }

; Check that while processing MapTypes from the directive intrinsics,
; the MemberOf fields are updated based on the index of the starting
; link of the map-chain.
; DBG: Updated MemberOf Flag from '3' to '1'
; DBG: MapType changed from '844424930131971 (0x0003000000000003)' to '281474976710659 (0x0001000000000003)'
; DBG: Updated MemberOf Flag from '1' to '3'
; DBG: MapType changed from '281474976710659 (0x0001000000000003)' to '844424930131971 (0x0003000000000003)'


; Check that the maptypes struct has the value in the correct order. If the memberof
; fields were not updated, the 2nd and 4th elements would be interchanged.
; TFORM: @{{[^ ]+}} = private unnamed_addr constant [4 x i64] [i64 32, i64 281474976710659, i64 32, i64 844424930131971]

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

%struct.S = type { i32, float }

@.str = private unnamed_addr constant [16 x i8] c"x = %d, y = %f\0A\00", align 1
define dso_local void @foo() {
entry:
  %x = alloca %struct.S, align 4
  %y = alloca %struct.S, align 4
  store i32 0, ptr %x, align 4
  store i32 2, ptr %y, align 4
  %0 = getelementptr i32, ptr %x, i32 1
  %1 = ptrtoint ptr %0 to i64
  %2 = ptrtoint ptr %x to i64
  %3 = sub i64 %1, %2
  %4 = sdiv exact i64 %3, ptrtoint (ptr getelementptr (i8, ptr null, i32 1) to i64)
  %b = getelementptr inbounds %struct.S, ptr %y, i32 0, i32 1
  %5 = getelementptr float, ptr %b, i32 1
  %6 = ptrtoint ptr %5 to i64
  %7 = ptrtoint ptr %b to i64
  %8 = sub i64 %6, %7
  %9 = sdiv exact i64 %8, ptrtoint (ptr getelementptr (i8, ptr null, i32 1) to i64)

  %10 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(ptr %y, ptr %b, i64 %9, i64 32),
    "QUAL.OMP.MAP.TOFROM:CHAIN"(ptr %y, ptr %b, i64 4, i64 844424930131971),
    "QUAL.OMP.MAP.TOFROM"(ptr %x, ptr %x, i64 %4, i64 32),
    "QUAL.OMP.MAP.TOFROM:CHAIN"(ptr %x, ptr %x, i64 4, i64 281474976710659) ]


  store i32 10, ptr %x, align 4
  %b4 = getelementptr inbounds %struct.S, ptr %y, i32 0, i32 1
  store float 2.000000e+01, ptr %b4, align 4

  call void @llvm.directive.region.exit(token %10) [ "DIR.OMP.END.TARGET"() ]

  %11 = load i32, ptr %x, align 4
  %b6 = getelementptr inbounds %struct.S, ptr %y, i32 0, i32 1
  %12 = load float, ptr %b6, align 4
  %conv = fpext float %12 to double
  %call = call i32 (ptr, ...) @printf(ptr @.str, i32 %11, double %conv)
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare dso_local i32 @printf(ptr, ...)

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 2055, i32 156315161, !"foo", i32 12, i32 0, i32 0}
