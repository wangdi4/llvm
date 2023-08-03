; REQUIRES: asserts
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt -debug-only=vpo-paropt-target -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring),vpo-paropt' -debug-only=vpo-paropt-target -S %s 2>&1 | FileCheck %s

; Make sure Paropt can handle const-expr section-ptr operands which
; are also used inside the region.

; #include <stdio.h>
; int a[10];
;
; void foo() {
; #pragma omp target map(a[1:5])
;   printf("a[1] = %d\n", a[1]);
;   return;
; }

; CHECK: createRenamedValueForV : Renamed 'ptr getelementptr inbounds ([10 x i32], ptr @a, i64 0, i64 1)' (via launder intrinsic) to: 'ptr %{{.*}}'.
; CHECK: createRenamedValueForV : Renamed 'ptr @a' (via launder intrinsic) to: 'ptr %a'.

; CHECK: clearLaunderIntrinBeforeRegion: Number of launder intrinsics for the region is 2.
; CHECK: clearLaunderIntrinBeforeRegion: Replacing launder intrinsic 'ptr %{{.+}}' with its operand.
; CHECK: clearLaunderIntrinBeforeRegion: Replacing launder intrinsic 'ptr %{{.+}}' with its operand.

; CHECK-NOT: call ptr @llvm.launder.invariant.group

; Check that globals @a is not used in the outlined function.
; CHECK: define internal void @__omp_offloading_{{.*}}foo{{.*}}(ptr %a)
; CHECK: %[[A1:[^ ]+]] = getelementptr inbounds [10 x i32], ptr %a, i64 0, i64 1
; CHECK: %[[A1_LOAD:[^ ]+]] = load i32, ptr %0, align 4
; CHECK: call i32 (ptr, ...) @printf(ptr noundef @.str, i32 noundef %[[A1_LOAD]])

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64"
target device_triples = "x86_64"

@a = external global [10 x i32], align 16
@.str = private unnamed_addr constant [11 x i8] c"a[1] = %d\0A\00", align 1

; Function Attrs: convergent noinline nounwind optnone uwtable
define protected void @foo() {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(ptr @a, ptr getelementptr inbounds ([10 x i32], ptr @a, i64 0, i64 1), i64 20, i64 35, ptr null, ptr null) ] ; MAP type: 35 = 0x23 = TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)

  %1 = load i32, ptr getelementptr inbounds ([10 x i32], ptr @a, i64 0, i64 1), align 4
  %call = call i32 (ptr, ...) @printf(ptr noundef @.str, i32 noundef %1) #3
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

; Function Attrs: convergent
declare i32 @printf(ptr noundef, ...)

!omp_offload.info = !{!0}

!0 = !{i32 0, i32 64773, i32 3826064, !"_Z3foo", i32 5, i32 0, i32 0, i32 0}
