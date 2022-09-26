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

; The test IR is a version of target_fp_vla_host.ll with these changes:
; * the vla was replaced with a constant size array.
; * a VARLEN modifier was added to the clause(s).

; Check that the map-type for the VLA (first one) is 161 (PARAM|PRIVATE|TO)
; CHECK: @.offload_sizes = private unnamed_addr constant [2 x i64] [i64 8, i64 0]
; CHECK: @.offload_maptypes = private unnamed_addr constant [2 x i64] [i64 161, i64 288]

; Check that we capture vla size and emit a map for the firstprivate VLA on target.
; CHECK: define dso_local i32 @main()
; CHECK:   [[SIZE_ADDR:%.*.addr.*]] = alloca i64, align 8
; CHECK:   store i64 1, i64* [[SIZE_ADDR]], align 8
; CHECK:   [[SIZE_ARG:%.+]] = load i64, i64* [[SIZE_ADDR]], align 8

; CHECK:   [[VLA_CAST:%.+]] = bitcast [2 x i32]* %vla to i8*
; CHECK:   [[BASE_GEP:%.+]] = getelementptr inbounds [2 x i8*], [2 x i8*]* %.offload_baseptrs, i32 0, i32 0
; CHECK:   store i8* [[VLA_CAST]], i8** [[BASE_GEP]], align 8
; CHECK:   [[START_GEP:%.+]] = getelementptr inbounds [2 x i8*], [2 x i8*]* %.offload_ptrs, i32 0, i32 0
; CHECK:   [[VLA_CAST1:%.+]] = bitcast [2 x i32]* %vla to i8*
; CHECK:   store i8* [[VLA_CAST1]], i8** [[START_GEP]], align 8

; CHECK:   [[SIZE_ARG_CAST:%.+]] = inttoptr i64 [[SIZE_ARG]] to i8*
; CHECK:   [[BASE_GEP1:%.+]] = getelementptr inbounds [2 x i8*], [2 x i8*]* %.offload_baseptrs, i32 0, i32 1
; CHECK:   store i8* [[SIZE_ARG_CAST]], i8** [[BASE_GEP1]], align 8
; CHECK:   [[START_GEP1:%.+]] = getelementptr inbounds [2 x i8*], [2 x i8*]* %.offload_ptrs, i32 0, i32 1
; CHECK:   [[SIZE_ARG_CAST1:%.+]] = inttoptr i64 [[SIZE_ARG]] to i8*
; CHECK:   store i8* [[SIZE_ARG_CAST1]], i8** [[START_GEP1]], align 8

; Check that the VLA and the captured size is passed in to the outlined function for the target region.
; CHECK:   call void @__omp_offloading{{.*}}main{{.*}}([2 x i32]* %vla, i64 [[SIZE_ARG]])

; Check that the captured VLA size is used in the target region for allocation of the firstprivate VLA.
; CHECK: define internal void @__omp_offloading{{.*}}main{{.*}}([2 x i32]* %vla, i64 [[SIZE_ARG]])
; CHECK:   [[SIZE_FP:%.*addr.*fpriv]] = alloca i64, align 8
; CHECK:   %vla.fpriv = alloca [2 x i32], align 16
; CHECK:   store i64 [[SIZE_ARG]], i64* [[SIZE_FP]], align 8
; CHECK:   [[SIZE_FP_VAL:%.*]] = load i64, i64* [[SIZE_FP]], align 8

; Check that the private copy is used inside the region
; CHECK:   %call = call i32 (i8*, ...) @printf({{.*}}, [2 x i32]* noundef %vla.fpriv)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

@.str = private unnamed_addr constant [4 x i8] c"%p\0A\00", align 1

define dso_local i32 @main() {
entry:
  %n = alloca i64, align 8
  store i64 2, i64* %n, align 8

  %n.val = load i64, i64* %n, align 8
  %vla = alloca [2 x i32], align 16

  %dir = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.FIRSTPRIVATE:VARLEN"([2 x i32]* %vla) ]

  %call = call i32 (i8*, ...) @printf(i8* noundef getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i64 0, i64 0), [2 x i32]* noundef %vla)

  call void @llvm.directive.region.exit(token %dir) [ "DIR.OMP.END.TARGET"() ]

  ret i32 0
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare dso_local i32 @printf(i8* noundef, ...)

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 66313, i32 135942768, !"_Z4main", i32 6, i32 0, i32 0}
