; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src:

; #include <omp.h>
; #include <stdio.h>
;
; typedef struct {
;   int a;
; } S;
;
; S s1;
;
; int main() {
;   s1.a = 111;
; //#pragma omp parallel
;   {
;     int tid = omp_get_thread_num();
; #pragma omp target firstprivate(s1) firstprivate(tid)
;     {
;       s1.a = tid;
; //#pragma omp parallel
;       { printf("a = %d, &a = %p\n", s1.a, &s1.a); }
;     }
;   }
; }

; Make sure that a local copy of %s1 is created within the outlined function
; for the target construct, even though there's an explicit map on it.

; CHECK: define internal void @__omp_offloading_{{.*}}_Z4main{{.*}}(ptr %s1{{.*}})
; CHECK: [[S1FPRIV:%s1.fpriv]] = alloca %struct.S, align 8

; CHECK-DAG: call i32 (ptr, ...) @printf({{.*}}, ptr noundef [[S1FPRIV]])
; CHECK-DAG: call void @llvm.memcpy.p0.p0.i64(ptr {{.*}}[[S1FPRIV]], ptr {{.*}}%s1, i64 4, i1 false)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

%struct.S = type { i32 }

@s1 = dso_local global %struct.S zeroinitializer, align 4
@.str = private unnamed_addr constant [17 x i8] c"a = %d, &a = %p\0A\00", align 1

define dso_local i32 @main() {
entry:
  %tid = alloca i32, align 4
  store i32 111, ptr @s1, align 4
  %call = call i32 @omp_get_thread_num() #2
  store i32 %call, ptr %tid, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr @s1, %struct.S zeroinitializer, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %tid, i32 0, i32 1),
    "QUAL.OMP.MAP.TO"(ptr @s1, ptr @s1, i64 4, i64 161, ptr null, ptr null) ] ; MAP type: 161 = 0xa1 = PRIVATE (0x80) | TARGET_PARAM (0x20) | TO (0x1)

  %1 = load i32, ptr %tid, align 4
  store i32 %1, ptr @s1, align 4
  %2 = load i32, ptr @s1, align 4
  %call1 = call i32 (ptr, ...) @printf(ptr noundef @.str, i32 noundef %2, ptr noundef @s1) #2
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  ret i32 0
}

declare dso_local i32 @omp_get_thread_num()
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare dso_local i32 @printf(ptr noundef, ...)

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 57, i32 -698712660, !"_Z4main", i32 14, i32 0, i32 0}
