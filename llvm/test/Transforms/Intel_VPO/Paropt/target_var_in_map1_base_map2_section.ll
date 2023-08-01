; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

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
; CHECK-NEXT:    [[DOTOFFLOAD_BASEPTRS:%.*]] = alloca [3 x ptr], align 8
; CHECK-NEXT:    [[DOTOFFLOAD_PTRS:%.*]] = alloca [3 x ptr], align 8
; CHECK-NEXT:    [[DOTOFFLOAD_MAPPERS:%.*]] = alloca [3 x ptr], align 8
; CHECK-NEXT:    [[DOTRUN_HOST_VERSION:%.*]] = alloca i32, align 4
; CHECK-NEXT:    [[X:%.*]] = alloca i32, align 4
; CHECK-NEXT:    [[S:%.*]] = alloca [[STRUCT_S:%.*]], align 8
; CHECK-NEXT:    store ptr [[X]], ptr [[S]], align 8

; Check that X is mapped, and passed as an argument to the kernel function.
; CHECK:         [[TMP9:%.*]] = getelementptr inbounds [3 x ptr], ptr [[DOTOFFLOAD_BASEPTRS]], i32 0, i32 0
; CHECK-NEXT:    store ptr [[X]], ptr [[TMP9]], align 8
; CHECK-NEXT:    [[TMP10:%.*]] = getelementptr inbounds [3 x ptr], ptr [[DOTOFFLOAD_PTRS]], i32 0, i32 0
; CHECK-NEXT:    store ptr [[X]], ptr [[TMP10]], align 8

; CHECK:       omp_offload.failed:
; CHECK-NEXT:    call void @__omp_offloading{{.*}}(ptr [[X]], ptr [[S]])

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64"
target device_triples = "x86_64"

%struct.S = type { ptr }

@.str = private unnamed_addr constant [4 x i8] c"%p\0A\00", align 1

define hidden i32 @main() {
entry:
  %x = alloca i32, align 4
  %s = alloca %struct.S, align 8
  store ptr %x, ptr %s, align 8
  %0 = load ptr, ptr %s, align 8
  %1 = getelementptr ptr, ptr %s, i32 1
  %2 = ptrtoint ptr %1 to i64
  %3 = ptrtoint ptr %s to i64
  %4 = sub i64 %2, %3
  %5 = sdiv exact i64 %4, ptrtoint (ptr getelementptr (i8, ptr null, i32 1) to i64)

  %6 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(ptr %x, ptr %x, i64 4, i64 3, ptr null, ptr null),
    "QUAL.OMP.MAP.TOFROM"(ptr %s, ptr %s, i64 %5, i64 32, ptr null, ptr null),
    "QUAL.OMP.MAP.TOFROM:CHAIN"(ptr %s, ptr %x, i64 0, i64 562949953421328, ptr null, ptr null) ]

  %7 = load ptr, ptr %s, align 8
  %call = call i32 (ptr, ...) @printf(ptr @.str, ptr %7)

  call void @llvm.directive.region.exit(token %6) [ "DIR.OMP.END.TARGET"() ]

  ret i32 0
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare i32 @printf(ptr, ...)

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 58, i32 -671247469, !"_Z4main", i32 12, i32 0, i32 0}
