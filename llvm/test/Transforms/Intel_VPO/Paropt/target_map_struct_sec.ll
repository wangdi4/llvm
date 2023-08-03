; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s 2>&1 | FileCheck %s

; #include <stdio.h>
;
; typedef struct {
;     int* a;
;     float b;
; } S;
;
; __attribute__((noinline)) void foo() {
;     S x;
;     int c[10];
;     x.a = &c[0];
;     x.a[1] = 1;
;     x.b = 2;
;
; #pragma omp target map(tofrom:x.a[:10])
;     {
;         x.a[1] = 10;
;         x.b = 20;
;     }
;     printf("a = %d, b = %f\n", x.a[1], x.b);
; }
;
; int main() {
;     foo();
; }

; Check that CodeExtractor does not emit an error for out-of-clause arguments being caputred.
; CHECK-NOT: CodeExtractor captured out-of-clause argument.

; Check that the outlined function only takes one argument (%x).
; CHECK: call void @__omp_offloading_{{.*}}foo{{.*}}(ptr %x)

source_filename = "target_map_struct_and_sec.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

%struct.S = type { ptr, float }

@.str = private unnamed_addr constant [16 x i8] c"a = %d, b = %f\0A\00", align 1
@llvm.global_ctors = appending global [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 0, ptr @.omp_offloading.requires_reg, ptr null }]

define dso_local void @_Z3foov() {
entry:
  %x = alloca %struct.S, align 8
  %c = alloca [10 x i32], align 16
  %arrayidx = getelementptr inbounds [10 x i32], ptr %c, i64 0, i64 0
  %a = getelementptr inbounds %struct.S, ptr %x, i32 0, i32 0
  store ptr %arrayidx, ptr %a, align 8
  %a1 = getelementptr inbounds %struct.S, ptr %x, i32 0, i32 0
  %0 = load ptr, ptr %a1, align 8
  %arrayidx2 = getelementptr inbounds i32, ptr %0, i64 1
  store i32 1, ptr %arrayidx2, align 4
  %b = getelementptr inbounds %struct.S, ptr %x, i32 0, i32 1
  store float 2.000000e+00, ptr %b, align 8
  %a3 = getelementptr inbounds %struct.S, ptr %x, i32 0, i32 0
  %a4 = getelementptr inbounds %struct.S, ptr %x, i32 0, i32 0
  %1 = load ptr, ptr %a4, align 8
  %arrayidx5 = getelementptr inbounds i32, ptr %1, i64 0
  %2 = getelementptr ptr, ptr %a3, i32 1
  %3 = ptrtoint ptr %2 to i64
  %4 = ptrtoint ptr %a3 to i64
  %5 = sub i64 %3, %4
  %6 = sdiv exact i64 %5, ptrtoint (ptr getelementptr (i8, ptr null, i32 1) to i64)
  %7 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(ptr %x, ptr %a3, i64 %6, i64 32, ptr null, ptr null), ; MAP type: 32 = 0x20 = TARGET_PARAM (0x20)
    "QUAL.OMP.MAP.TOFROM:CHAIN"(ptr %a3, ptr %arrayidx5, i64 40, i64 281474976710675, ptr null, ptr null) ] ; MAP type: 281474976710675 = 0x1000000000013 = MEMBER_OF_1 (0x1000000000000) | PTR_AND_OBJ (0x10) | FROM (0x2) | TO (0x1)

  %a6 = getelementptr inbounds %struct.S, ptr %x, i32 0, i32 0
  %8 = load ptr, ptr %a6, align 8
  %arrayidx7 = getelementptr inbounds i32, ptr %8, i64 1
  store i32 10, ptr %arrayidx7, align 4
  %b8 = getelementptr inbounds %struct.S, ptr %x, i32 0, i32 1
  store float 2.000000e+01, ptr %b8, align 8
  call void @llvm.directive.region.exit(token %7) [ "DIR.OMP.END.TARGET"() ]

  %a9 = getelementptr inbounds %struct.S, ptr %x, i32 0, i32 0
  %9 = load ptr, ptr %a9, align 8
  %arrayidx10 = getelementptr inbounds i32, ptr %9, i64 1
  %10 = load i32, ptr %arrayidx10, align 4
  %b11 = getelementptr inbounds %struct.S, ptr %x, i32 0, i32 1
  %11 = load float, ptr %b11, align 8
  %conv = fpext float %11 to double
  %call = call i32 (ptr, ...) @printf(ptr noundef @.str, i32 noundef %10, double noundef nofpclass(nan inf) %conv)
  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

declare dso_local i32 @printf(ptr noundef, ...)

define dso_local noundef i32 @main() {
entry:
  call void @_Z3foov()
  ret i32 0
}

define internal void @.omp_offloading.requires_reg() section ".text.startup" {
entry:
  call void @__tgt_register_requires(i64 1)
  ret void
}

declare void @__tgt_register_requires(i64)

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4}

!0 = !{i32 0, i32 64773, i32 3828812, !"_Z3foov", i32 14, i32 0, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 51}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{i32 7, !"frame-pointer", i32 2}
