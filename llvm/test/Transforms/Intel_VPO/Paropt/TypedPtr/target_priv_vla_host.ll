; RUN: opt -opaque-pointers=0 -enable-new-pm=0 -vpo-paropt-target-capture-non-pointers-using-map-to=true -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -opaque-pointers=0 -vpo-paropt-target-capture-non-pointers-using-map-to=true -aa-pipeline=basic-aa -passes='function(vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src:
;
; #include <stdio.h>
; int main() {
;   int n = 2;
;   int x[n];
;
; #pragma omp target private(x)
;     printf("%p\n", x);
; }

; The test IR is a reduced version of the IR for the above src.

; Check that the map-type for the VLA (first one) is 160 (PARAM|PRIVATE)
; CHECK: @.offload_maptypes = private unnamed_addr constant [2 x i64] [i64 160, i64 33]

; Check that we capture vla size and emit a map for the private VLA on target.
; CHECK: define dso_local i32 @main()
; CHECK:   [[SIZE_ADDR:%n.val.addr]] = alloca i64, align 8
; CHECK:   [[VLA_SIZE_BYTES:%.+]] = mul i64 %n.val, 4
; CHECK:   store i64 %n.val, i64* [[SIZE_ADDR]], align 8

; CHECK:   [[VLA_CAST:%.+]] = bitcast i32* %vla to i8*
; CHECK:   [[BASE_GEP:%.+]] = getelementptr inbounds [2 x i8*], [2 x i8*]* %.offload_baseptrs, i32 0, i32 0
; CHECK:   store i8* [[VLA_CAST]], i8** [[BASE_GEP]], align 8
; CHECK:   [[START_GEP:%.+]] = getelementptr inbounds [2 x i8*], [2 x i8*]* %.offload_ptrs, i32 0, i32 0
; CHECK:   [[VLA_CAST1:%.+]] = bitcast i32* %vla to i8*
; CHECK:   store i8* [[VLA_CAST1]], i8** [[START_GEP]], align 8
; CHECK:   [[SIZE_GEP:%.+]] = getelementptr inbounds [2 x i64], [2 x i64]* %.offload_sizes, i32 0, i32 0
; CHECK:   store i64 [[VLA_SIZE_BYTES]], i64* [[SIZE_GEP]], align 8

; CHECK:   [[SIZE_ADDR_CAST:%.+]] = bitcast i64* [[SIZE_ADDR]] to i8*
; CHECK:   [[BASE_GEP1:%.+]] = getelementptr inbounds [2 x i8*], [2 x i8*]* %.offload_baseptrs, i32 0, i32 1
; CHECK:   store i8* [[SIZE_ADDR_CAST]], i8** [[BASE_GEP1]], align 8
; CHECK:   [[START_GEP1:%.+]] = getelementptr inbounds [2 x i8*], [2 x i8*]* %.offload_ptrs, i32 0, i32 1
; CHECK:   [[SIZE_ADDR_CAST1:%.+]] = bitcast i64* [[SIZE_ADDR]] to i8*
; CHECK:   store i8* [[SIZE_ADDR_CAST1]], i8** [[START_GEP1]], align 8
; CHECK:   [[SIZE_GEP1:%.+]] = getelementptr inbounds [2 x i64], [2 x i64]* %.offload_sizes, i32 0, i32 1
; CHECK:   store i64 8, i64* [[SIZE_GEP1]], align 8

; Check that the VLA and the captured size is passed in to the outlined function for the target region.
; CHECK:   call void @__omp_offloading{{.*}}main{{.*}}(i32* %vla, i64* [[SIZE_ADDR]])

; Check that the captured VLA size is used in the target region for allocation of the private VLA.
; CHECK: define internal void @__omp_offloading{{.*}}main{{.*}}(i32* noalias %vla, i64* noalias [[SIZE_ADDR]])
; CHECK:   [[SIZE_VAL:%[^ ]+]] = load i64, i64* [[SIZE_ADDR]], align 8
; CHECK:   %vla.priv = alloca i32, i64 [[SIZE_VAL]], align 16

; Check that the private copy is used inside the region
; CHECK:   %call = call i32 (i8*, ...) @printf({{.*}}, i32* noundef %vla.priv)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

@.str = private unnamed_addr constant [4 x i8] c"%p\0A\00", align 1

define dso_local i32 @main() {
entry:
  %n = alloca i64, align 8
  store i64 2, i64* %n, align 8

  %n.val = load i64, i64* %n, align 8
  %vla = alloca i32, i64 %n.val, align 16

  %dir = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.PRIVATE"(i32* %vla) ]

  %call = call i32 (i8*, ...) @printf(i8* noundef getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i64 0, i64 0), i32* noundef %vla)

  call void @llvm.directive.region.exit(token %dir) [ "DIR.OMP.END.TARGET"() ]

  ret i32 0
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare dso_local i32 @printf(i8* noundef, ...)

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 66313, i32 135942768, !"_Z4main", i32 6, i32 0, i32 0}
