; RUN: opt -bugpoint-enable-legacy-pm -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='vpo-paropt' -S %s | FileCheck %s
;
;
; Test src:
;
; int foo( ) {
;   static float vx2[100];
;   static float vy2[100];
;   #pragma omp target map(tofrom: vx2[0:30], vy2[0:30])
;   {
;     float dxc;
;   }
;   return 0;
; }

; The test is just intended to check that there is no comp-fail while executing it.
; CHECK: call{{.*}}@__tgt_target

; ModuleID = 'target_map_statics.ll'
source_filename = "target_map_statics.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

@_ZZ3foovE3vx2 = internal global [100 x float] zeroinitializer, align 16
@_ZZ3foovE3vy2 = internal global [100 x float] zeroinitializer, align 16
@llvm.global_ctors = appending global [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 0, ptr @.omp_offloading.requires_reg, ptr null }]

define dso_local noundef i32 @_Z3foov() {
entry:
  %dxc = alloca float, align 4
  br label %DIR.OMP.TARGET.1

DIR.OMP.TARGET.1:                                 ; preds = %entry
  br label %DIR.OMP.TARGET.2

DIR.OMP.TARGET.2:                                 ; preds = %DIR.OMP.TARGET.1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(ptr @_ZZ3foovE3vx2, ptr @_ZZ3foovE3vx2, i64 120, i64 3, ptr null, ptr null), ; MAP type: 3 = 0x3 = FROM (0x2) | TO (0x1)
    "QUAL.OMP.MAP.TOFROM"(ptr @_ZZ3foovE3vy2, ptr @_ZZ3foovE3vy2, i64 120, i64 3, ptr null, ptr null), ; MAP type: 3 = 0x3 = FROM (0x2) | TO (0x1)
    "QUAL.OMP.PRIVATE:TYPED"(ptr %dxc, float 0.000000e+00, i32 1) ]

  br label %DIR.OMP.TARGET.3

DIR.OMP.TARGET.3:                                 ; preds = %DIR.OMP.TARGET.2
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  br label %DIR.OMP.END.TARGET.4

DIR.OMP.END.TARGET.4:                             ; preds = %DIR.OMP.TARGET.3
  ret i32 0
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

define internal void @.omp_offloading.requires_reg() section ".text.startup" {
entry:
  call void @__tgt_register_requires(i64 1)
  ret void
}

declare void @__tgt_register_requires(i64)

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4}

!0 = !{i32 0, i32 64773, i32 3828809, !"_Z3foov", i32 4, i32 0, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 51}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{i32 7, !"frame-pointer", i32 2}

