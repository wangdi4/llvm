; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -opaque-pointers=0 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src:

; #include <stdio.h>
;
; typedef struct {
;     int *xp;
; } S;
;
; int main() {
;     int x;
;     S s;
;     s.xp = &x;
;
; #pragma omp target map(tofrom:x) map(alloc:s.xp[0:0])
;     printf("%p\n", s.xp);
; }

; The test was hand-modified from the above source.
; When looking at %s as a section pointer, Paropt replaces all its uses in
; the directive with null, so it doesn't get passed into the target region
; as a kernel parameter. This setting of %x to null should not be done here
; because %x is also a base pointer.

; CHECK-LABEL: @main(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[DOTOFFLOAD_SIZES:%.*]] = alloca [3 x i64], align 8
; CHECK-NEXT:    [[DOTOFFLOAD_BASEPTRS:%.*]] = alloca [3 x i8*], align 8
; CHECK-NEXT:    [[DOTOFFLOAD_PTRS:%.*]] = alloca [3 x i8*], align 8
; CHECK-NEXT:    [[DOTOFFLOAD_MAPPERS:%.*]] = alloca [3 x i8*], align 8
; CHECK-NEXT:    [[DOTRUN_HOST_VERSION:%.*]] = alloca i32, align 4
; CHECK-NEXT:    [[X:%.*]] = alloca i32, align 4
; CHECK-NEXT:    [[S:%.*]] = alloca [[STRUCT_S:%.*]], align 8
; CHECK-NEXT:    [[XP:%.*]] = getelementptr inbounds [[STRUCT_S]], %struct.S* [[S]], i32 0, i32 0
; CHECK-NEXT:    store i32* [[X]], i32** [[XP]], align 8

; Check that X is mapped, and passed as an argument to the kernel function.
; CHECK:         [[TMP8:%.*]] = bitcast i32* [[X]] to i8*
; CHECK-NEXT:    [[TMP9:%.*]] = getelementptr inbounds [3 x i8*], [3 x i8*]* [[DOTOFFLOAD_BASEPTRS]], i32 0, i32 0
; CHECK-NEXT:    store i8* [[TMP8]], i8** [[TMP9]], align 8
; CHECK-NEXT:    [[TMP10:%.*]] = getelementptr inbounds [3 x i8*], [3 x i8*]* [[DOTOFFLOAD_PTRS]], i32 0, i32 0
; CHECK-NEXT:    [[TMP11:%.*]] = bitcast i32* [[X]] to i8*
; CHECK-NEXT:    store i8* [[TMP11]], i8** [[TMP10]], align 8

; CHECK:       omp_offload.failed:
; CHECK-NEXT:    call void @__omp_offloading{{.*}}(i32* [[X]], %struct.S* [[S]])

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64"
target device_triples = "x86_64"

%struct.S = type { i32* }

@.str = private unnamed_addr constant [4 x i8] c"%p\0A\00", align 1

define hidden i32 @main() {
entry:
  %x = alloca i32, align 4
  %s = alloca %struct.S, align 8
  %xp = getelementptr inbounds %struct.S, %struct.S* %s, i32 0, i32 0
  store i32* %x, i32** %xp, align 8
  %xp1 = getelementptr inbounds %struct.S, %struct.S* %s, i32 0, i32 0
  %xp2 = getelementptr inbounds %struct.S, %struct.S* %s, i32 0, i32 0
  %0 = load i32*, i32** %xp2, align 8
  %arrayidx = getelementptr inbounds i32, i32* %0, i64 0
  %1 = getelementptr i32*, i32** %xp1, i32 1
  %2 = bitcast i32** %xp1 to i8*
  %3 = bitcast i32** %1 to i8*
  %4 = ptrtoint i8* %3 to i64
  %5 = ptrtoint i8* %2 to i64
  %6 = sub i64 %4, %5
  %7 = sdiv exact i64 %6, ptrtoint (i8* getelementptr (i8, i8* null, i32 1) to i64)

  %8 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(i32* %x, i32* %x, i64 4, i64 3, i8* null, i8* null),
    "QUAL.OMP.MAP.TOFROM"(%struct.S* %s, i32** %xp1, i64 %7, i64 32, i8* null, i8* null),
    "QUAL.OMP.MAP.TOFROM:CHAIN"(i32** %xp1, i32* %x, i64 0, i64 562949953421328, i8* null, i8* null) ]

  %xp3 = getelementptr inbounds %struct.S, %struct.S* %s, i32 0, i32 0
  %9 = load i32*, i32** %xp3, align 8
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i64 0, i64 0), i32* %9)

  call void @llvm.directive.region.exit(token %8) [ "DIR.OMP.END.TARGET"() ]

  ret i32 0
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare i32 @printf(i8*, ...)

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 58, i32 -671247469, !"_Z4main", i32 12, i32 0, i32 0}
