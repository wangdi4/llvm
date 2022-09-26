; RUN: opt -enable-new-pm=0 -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -aa-pipeline=basic-aa -passes='function(vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src:
;
; #include <stdio.h>
; int main() {
;   int n = 2;
;   int x[n];
;
; #pragma omp target firstprivate(x)
;     printf("%p\n", x);
; }

; The test IR is a version of target_fp_map_vla_host.ll with these changes:
; * the size of the vla/typed clause was replaced with a constant.
; * a VARLEN modifier was added to the clause(s).

; Check that the map-type for the VLA (first one) is 161 (PARAM|PRIVATE|TO)
; CHECK: @.offload_maptypes = private unnamed_addr constant [3 x i64] [i64 161, i64 288, i64 288]

; Check that we capture vla size (once for the typed size, and once for the base) and emit a
; map for the firstprivate VLA on target.
; CHECK: define dso_local i32 @main()
; CHECK:   [[SIZE_ADDR1:%.*addr.*]] = alloca i64, align 8
; CHECK:   [[SIZE_ADDR:%.*addr.*]] = alloca i64, align 8

; CHECK:   [[VLA_SIZE_BYTES:%.+]] = mul i64 2, 4
; CHECK:   store i64 2, ptr [[SIZE_ADDR]], align 8
; CHECK:   store i64 2, ptr [[SIZE_ADDR1]], align 8

; CHECK:   [[SIZE_ARG:%.+]] = load i64, ptr [[SIZE_ADDR]], align 8
; CHECK:   [[SIZE_ARG1:%.+]] = load i64, ptr [[SIZE_ADDR1]], align 8

; CHECK:   [[BASE_GEP:%.+]] = getelementptr inbounds [3 x ptr], ptr %.offload_baseptrs, i32 0, i32 0
; CHECK:   store ptr %vla, ptr [[BASE_GEP]], align 8
; CHECK:   [[START_GEP:%.+]] = getelementptr inbounds [3 x ptr], ptr %.offload_ptrs, i32 0, i32 0
; CHECK:   store ptr %vla, ptr [[START_GEP]], align 8
; CHECK:   [[SIZE_GEP:%.+]] = getelementptr inbounds [3 x i64], ptr %.offload_sizes, i32 0, i32 0
; CHECK:   store i64 [[VLA_SIZE_BYTES]], ptr [[SIZE_GEP]], align 8

; CHECK:   [[SIZE_ARG_CAST:%.+]] = inttoptr i64 [[SIZE_ARG]] to ptr
; CHECK:   [[BASE_GEP1:%.+]] = getelementptr inbounds [3 x ptr], ptr %.offload_baseptrs, i32 0, i32 1
; CHECK:   store ptr [[SIZE_ARG_CAST]], ptr [[BASE_GEP1]], align 8
; CHECK:   [[START_GEP1:%.+]] = getelementptr inbounds [3 x ptr], ptr %.offload_ptrs, i32 0, i32 1
; CHECK:   [[SIZE_ARG_CAST1:%.+]] = inttoptr i64 [[SIZE_ARG]] to ptr
; CHECK:   store ptr [[SIZE_ARG_CAST1]], ptr [[START_GEP1]], align 8
; CHECK:   [[SIZE_GEP1:%.+]] = getelementptr inbounds [3 x i64], ptr %.offload_sizes, i32 0, i32 1
; CHECK:   store i64 0, ptr [[SIZE_GEP1]], align 8

; CHECK:   [[SIZE_ARG_CAST2:%.+]] = inttoptr i64 [[SIZE_ARG1]] to ptr
; CHECK:   [[BASE_GEP2:%.+]] = getelementptr inbounds [3 x ptr], ptr %.offload_baseptrs, i32 0, i32 2
; CHECK:   store ptr [[SIZE_ARG_CAST2]], ptr [[BASE_GEP2]], align 8
; CHECK:   [[START_GEP2:%.+]] = getelementptr inbounds [3 x ptr], ptr %.offload_ptrs, i32 0, i32 2
; CHECK:   [[SIZE_ARG_CAST3:%.+]] = inttoptr i64 [[SIZE_ARG1]] to ptr
; CHECK:   store ptr [[SIZE_ARG_CAST3]], ptr [[START_GEP2]], align 8
; CHECK:   [[SIZE_GEP2:%.+]] = getelementptr inbounds [3 x i64], ptr %.offload_sizes, i32 0, i32 2
; CHECK:   store i64 0, ptr [[SIZE_GEP2]], align 8

; Check that the VLA and the captured sizes are passed in to the outlined function for the target region.
; CHECK:   call void @__omp_offloading{{.*}}main{{.*}}(ptr %vla, i64 [[SIZE_ARG]], i64 [[SIZE_ARG1]])

; Check that the captured VLA size is used in the target region for allocation of the firstprivate VLA.
; CHECK: define internal void @__omp_offloading{{.*}}main{{.*}}(ptr %vla, i64 [[SIZE_ARG]], i64 [[SIZE_ARG1]])
; CHECK:   [[SIZE_FP:%.*addr.fpriv]] = alloca i64, align 8
; CHECK:   %vla.fpriv = alloca [2 x i32], align 16
; CHECK:   store i64 [[SIZE_ARG]], ptr [[SIZE_FP]], align 8
; CHECK:   [[SIZE_FP_VAL:%.*]] = load i64, ptr [[SIZE_FP]], align 8

; Check that the firstprivate copy is used inside the region
; CHECK:   %vla.fpriv.gep = getelementptr inbounds [2 x i32], ptr %vla.fpriv, i32 0, i32 0
; CHECK:   %call = call i32 (ptr, ...) @printf({{.*}}, ptr noundef %vla.fpriv.gep)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

@.str = private unnamed_addr constant [4 x i8] c"%p\0A\00", align 1

define dso_local i32 @main() {
entry:
  %n = alloca i64, align 8
  store i64 2, ptr %n, align 8

  %n.val = load i64, ptr %n, align 8
  %n.val1 = load i64, ptr %n, align 8

  %vla = alloca i32, i64 2, align 16

  %map.size = mul i64 2, 4
  %dir = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED.VARLEN"(ptr %vla, i32 0, i64 2),
    "QUAL.OMP.MAP.TO:VARLEN"(ptr %vla, ptr %vla, i64 %map.size, i64 161, i8* null, i8* null) ]

  %call = call i32 (ptr, ...) @printf(ptr noundef @.str, ptr noundef %vla)

  call void @llvm.directive.region.exit(token %dir) [ "DIR.OMP.END.TARGET"() ]

  ret i32 0
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare dso_local i32 @printf(ptr noundef, ...)

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 66313, i32 135942768, !"_Z4main", i32 6, i32 0, i32 0}
