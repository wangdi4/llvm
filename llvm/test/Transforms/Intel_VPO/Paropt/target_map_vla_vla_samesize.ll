; REQUIRES: asserts
; RUN: opt -enable-new-pm=0 -switch-to-offload -vpo-cfg-restructuring -vpo-paropt -debug-only=WRegionUtils,vpo-paropt-transform -S %s 2>&1 | FileCheck %s
; RUN: opt -switch-to-offload -aa-pipeline=basic-aa -passes='function(vpo-cfg-restructuring),vpo-paropt' -debug-only=WRegionUtils,vpo-paropt-transform -S %s 2>&1 | FileCheck %s

; Test src:
;
; #include <stdio.h>
; int main() {
;   int n = 2;
;   int x[n];
;
; #pragma omp target map(to:x, y)
;   printf("%p %p\n", x, y);
; }

; Check that the size of both VLAs is captured, even though it's the same value.
; CHECK:     collectNonPointerValuesToBeUsedInOutlinedRegion: Non-pointer values to be passed into the outlined region: 'i64 %n.val i64 %n.val '
; CHECK:     captureAndAddCollectedNonPointerValuesToSharedClause: Added implicit shared/map(to) clause for: 'ptr [[SIZE_ADDR1:%n.val.addr.*]]'
; CHECK:     captureAndAddCollectedNonPointerValuesToSharedClause: Added implicit shared/map(to) clause for: 'ptr [[SIZE_ADDR2:%n.val.addr.*]]'

; Check that the kernel function has arguments for the mapped VLAs and the captured VLA sizes.
; CHECK: define {{.*}} void @__omp_offloading_{{.*}}main{{.*}}(ptr noalias %vla1, ptr noalias %vla2, ptr noalias [[SIZE_ADDR1]], ptr noalias [[SIZE_ADDR2]])

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

@.str = private unnamed_addr constant [7 x i8] c"%p %p\0A\00", align 1

define dso_local i32 @main() {
entry:
  %n = alloca i64, align 8
  store i64 2, ptr %n, align 8

  %n.val = load i64, ptr %n, align 8
  %n.val1 = load i64, ptr %n, align 8

  %vla1 = alloca i32, i64 %n.val, align 16
  %vla2 = alloca i32, i64 %n.val, align 16

  %map.size1 = mul i64 %n.val, 4
  %map.size2 = mul i64 %n.val, 4
  %dir = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TO"(ptr %vla1, ptr %vla1, i64 %map.size1, i64 33, i8* null, i8* null),
    "QUAL.OMP.MAP.TO"(ptr %vla2, ptr %vla2, i64 %map.size2, i64 33, i8* null, i8* null) ]

  %call = call i32 (ptr, ...) @printf(ptr noundef @.str, ptr noundef %vla1, ptr noundef %vla2)

  call void @llvm.directive.region.exit(token %dir) [ "DIR.OMP.END.TARGET"() ]

  ret i32 0
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare dso_local i32 @printf(ptr noundef, ...)

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 66313, i32 135942768, !"_Z4main", i32 6, i32 0, i32 0}
