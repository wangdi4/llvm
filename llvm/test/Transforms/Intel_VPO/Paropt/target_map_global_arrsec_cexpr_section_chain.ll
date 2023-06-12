; REQUIRES: asserts
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt -debug-only=vpo-paropt-target -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring),vpo-paropt' -debug-only=vpo-paropt-target -S %s 2>&1 | FileCheck %s

; Test src:

; typedef struct {
;   int *a;
; } b;
; #pragma omp declare target
; b c[2];
; #pragma omp end declare target
; int d() {
; #pragma omp target map(c[1].a[1])
;   c[1].a[0];
; }

; CHECK: createRenamedValueForV : Renamed 'ptr getelementptr inbounds ([2 x %struct.b], ptr @c, i64 0, i64 1)' (via launder intrinsic) to: 'ptr %{{.*}}'.
; CHECK: createRenamedValueForV : Renamed 'ptr @c' (via launder intrinsic) to: 'ptr %c'.

; CHECK: clearLaunderIntrinBeforeRegion: Number of launder intrinsics for the region is 2.
; CHECK: clearLaunderIntrinBeforeRegion: Replacing launder intrinsic 'ptr %{{.+}}' with its operand.
; CHECK: clearLaunderIntrinBeforeRegion: Replacing launder intrinsic 'ptr %{{.+}}' with its operand.

; CHECK-NOT: call ptr @llvm.launder.invariant.group

; Check that globals @c is not used in the outlined function.
; CHECK: define internal void @__omp_offloading{{.*}}(ptr %c)
; CHECK: %{{[^ ]+}} = getelementptr inbounds [2 x %struct.b], ptr %c, i64 0, i64 1

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64"
target device_triples = "x86_64"

%struct.b = type { ptr }

@c = dso_local target_declare global [2 x %struct.b] zeroinitializer, align 16

define dso_local i32 @d() {
entry:
  %retval = alloca i32, align 4
  %0 = load ptr, ptr getelementptr inbounds ([2 x %struct.b], ptr @c, i64 0, i64 1), align 8
  %arrayidx = getelementptr inbounds i32, ptr %0, i64 1
  %1 = sdiv exact i64 sub (i64 ptrtoint (ptr getelementptr (ptr, ptr getelementptr inbounds ([2 x %struct.b], ptr @c, i64 0, i64 1), i32 1) to i64), i64 ptrtoint (ptr getelementptr inbounds ([2 x %struct.b], ptr @c, i64 0, i64 1) to i64)), ptrtoint (ptr getelementptr (i8, ptr null, i32 1) to i64)
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 1),
    "QUAL.OMP.MAP.TOFROM"(ptr @c, ptr getelementptr inbounds ([2 x %struct.b], ptr @c, i64 0, i64 1), i64 %1, i64 32, ptr null, ptr null), ; MAP type: 32 = 0x20 = TARGET_PARAM (0x20)
    "QUAL.OMP.MAP.TOFROM:CHAIN"(ptr getelementptr inbounds ([2 x %struct.b], ptr @c, i64 0, i64 1), ptr %arrayidx, i64 4, i64 281474976710675, ptr null, ptr null) ] ; MAP type: 281474976710675 = 0x1000000000013 = MEMBER_OF_1 (0x1000000000000) | PTR_AND_OBJ (0x10) | FROM (0x2) | TO (0x1)

  %3 = load ptr, ptr getelementptr inbounds ([2 x %struct.b], ptr @c, i64 0, i64 1), align 8
  %arrayidx1 = getelementptr inbounds i32, ptr %3, i64 0
  %4 = load i32, ptr %arrayidx1, align 4
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TARGET"() ]

  %5 = load i32, ptr %retval, align 4
  ret i32 %5
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

!omp_offload.info = !{!0, !1}

!0 = !{i32 0, i32 64773, i32 3833495, !"_Z1d", i32 8, i32 0, i32 1, i32 0}
!1 = !{i32 1, !"_Z1c", i32 0, i32 0, ptr @c}
