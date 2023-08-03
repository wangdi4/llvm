; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -vpo-paropt -S %s | FileCheck %s
; RUN: opt -opaque-pointers=0 -passes='vpo-paropt' -S %s | FileCheck %s

; Verify that offload entry for extern variable is not emitted.
; Original code:
; extern int a;
; #pragma omp declare target to(a)

; CHECK-NOT: @.omp_offloading.entry.a

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "x86_64"

@a = external dso_local target_declare global i32, align 4

!omp_offload.info = !{!0}
!0 = !{i32 1, !"a", i32 0, i32 0, i32* @a}
