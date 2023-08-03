; RUN: opt -bugpoint-enable-legacy-pm -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='vpo-paropt' -S %s | FileCheck %s
;
; Test support for the "CLOSE" map-type modifier.
; C source:
;
; void foo() {
;   int xyz;
;   #pragma omp target map(close,tofrom:xyz)
;     xyz = 456;
; }
;
; The MAP_TYPE flags for the map in the example is expected to be
;              TO  | FROM | PARAM | CLOSE
;           =  0x1 | 0x2  | 0x20  | 0x400
;           =  0x423 = 1059 (decimal)
; CHECK: @.offload_maptypes = private unnamed_addr constant [1 x i64] [i64 1059]

source_filename = "target_map_close.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

@llvm.global_ctors = appending global [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 0, ptr @.omp_offloading.requires_reg, ptr null }]

define dso_local void @foo() {
entry:
  %xyz = alloca i32, align 4
  br label %DIR.OMP.TARGET.1

DIR.OMP.TARGET.1:                                 ; preds = %entry
  br label %DIR.OMP.TARGET.2

DIR.OMP.TARGET.2:                                 ; preds = %DIR.OMP.TARGET.1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM:CLOSE"(ptr %xyz, ptr %xyz, i64 4, i64 1059, ptr null, ptr null) ] ; MAP type: 1059 = 0x423 = CLOSE (0x400) | TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)

  br label %DIR.OMP.TARGET.3

DIR.OMP.TARGET.3:                                 ; preds = %DIR.OMP.TARGET.2
  store i32 456, ptr %xyz, align 4
  br label %DIR.OMP.END.TARGET.4

DIR.OMP.END.TARGET.4:                             ; preds = %DIR.OMP.TARGET.3
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  br label %DIR.OMP.END.TARGET.5

DIR.OMP.END.TARGET.5:                             ; preds = %DIR.OMP.END.TARGET.4
  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

define internal void @.omp_offloading.requires_reg()  section ".text.startup" {
entry:
  call void @__tgt_register_requires(i64 1)
  ret void
}

declare void @__tgt_register_requires(i64)

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4}

!0 = !{i32 0, i32 64773, i32 3828831, !"foo", i32 3, i32 0, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 51}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{i32 7, !"frame-pointer", i32 2}
