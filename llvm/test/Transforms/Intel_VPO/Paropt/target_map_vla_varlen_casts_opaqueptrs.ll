; REQUIRES: asserts
; RUN: opt -enable-new-pm=0 -switch-to-offload -vpo-cfg-restructuring -vpo-paropt -debug-only=WRegionUtils,vpo-paropt-transform -S %s 2>&1 | FileCheck %s
; RUN: opt -switch-to-offload -aa-pipeline=basic-aa -passes='function(vpo-cfg-restructuring),vpo-paropt' -debug-only=WRegionUtils,vpo-paropt-transform -S %s 2>&1 | FileCheck %s

; Test src:
;
; #include <stdio.h>
; int main() {
;   int n = 2;
;   int x[n], y[n];
;
; #pragma omp target map(to:x, y)
;   printf("%p %p\n", x, y);
; }

; Check that the size of both VLAs is captured, even though the operands are
; zero-offset GEPs/bitcasts on the allocas..
; CHECK:     collectNonPointerValuesToBeUsedInOutlinedRegion: Non-pointer values to be passed into the outlined region: 'i64 %n.val i64 2 '
; CHECK:     captureAndAddCollectedNonPointerValuesToSharedClause: Added implicit shared/map(to)/firstprivate clause for: 'ptr [[SIZE_ADDR1:%n.val.addr.*]]'
; CHECK:     captureAndAddCollectedNonPointerValuesToSharedClause: Added implicit shared/map(to)/firstprivate clause for: 'ptr [[SIZE_ADDR2:%.*addr.*]]'

; Check that the kernel function has arguments for the mapped VLAs and the captured VLA sizes.
; CHECK: define {{.*}} void @__omp_offloading_{{.*}}main{{.*}}(ptr noalias %vla1c, ptr noalias %vla2c, i64 [[SIZE_ADDR1]].val, i64 [[SIZE_ADDR2]].val)
; CHECK: [[SIZE_ADDR2]].fpriv = alloca i64, align 8
; CHECK: [[SIZE_ADDR1]].fpriv = alloca i64, align 8
; CHECK: store i64 [[SIZE_ADDR2]].val, ptr [[SIZE_ADDR2]].fpriv, align 8
; CHECK: store i64 [[SIZE_ADDR1]].val, ptr [[SIZE_ADDR1]].fpriv, align 8

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

@.str = private unnamed_addr constant [7 x i8] c"%p %p\0A\00", align 1

define dso_local i32 @main() {
entry:
  %n = alloca i64, align 8
  store i64 2, ptr %n, align 8

  %n.val = load i64, ptr %n, align 8

  %vla1 = alloca i32, i64 %n.val, align 16
  %vla2 = alloca i32, i64 2, align 16

  %vla1c = bitcast ptr %vla1 to ptr
  %vla2c = getelementptr inbounds i32, ptr %vla2, i32 0

  %map.size1 = mul i64 %n.val, 4
  %map.size2 = mul i64 2, 4
  %dir = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TO"(ptr %vla1c, ptr %vla1c, i64 %map.size1, i64 33, i8* null, i8* null),
    "QUAL.OMP.MAP.TO:VARLEN"(ptr %vla2c, ptr %vla2c, i64 %map.size2, i64 33, i8* null, i8* null) ]

  %call = call i32 (ptr, ...) @printf(ptr noundef @.str, ptr noundef %vla1c, ptr noundef %vla2c)

  call void @llvm.directive.region.exit(token %dir) [ "DIR.OMP.END.TARGET"() ]

  ret i32 0
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare dso_local i32 @printf(ptr noundef, ...)

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 66313, i32 135942768, !"_Z4main", i32 6, i32 0, i32 0}
