; REQUIRES: asserts
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt -debug-only=vpo-paropt-target -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring),vpo-paropt' -debug-only=vpo-paropt-target -S %s 2>&1 | FileCheck %s

; Make sure Paropt can handle const-expr map-size/if operands which
; use global vars that are operands on a clause on the region.

; Test src:
;
; #include <stdio.h>
; long N = 100;
; int a[10], b[10];
;
; void foo() {
; #pragma omp target map(a[0:(long)&N]) map(b[1:(long)&N]) if((long)&N)
;   {
;     printf("a[1] = %d, %ld\n", a[1], ((long)&N * 4));
;   }
;   return;
; }
;

; CHECK: renameNonPointerConstExprVInEntryDirective: Expr 'i1 icmp ne (i64 ptrtoint (ptr @N to i64), i64 0)' hoisted to Instruction 'i1 %[[IF:cexpr.inst]]'.
; CHECK: renameNonPointerConstExprVInEntryDirective: Expr 'i64 mul nuw (i64 ptrtoint (ptr @N to i64), i64 4)' hoisted to Instruction 'i64 %[[SIZE:cexpr.inst[^ ']+]]'.
; CHECK: createRenamedValueForV : Renamed 'ptr @a' (via launder intrinsic) to: 'ptr %a'.
; CHECK: createRenamedValueForV : Renamed 'ptr getelementptr inbounds ([10 x i32], ptr @b, i64 0, i64 1)' (via launder intrinsic) to: 'ptr [[B1:%[^ ]+]]'.
; CHECK: createRenamedValueForV : Renamed 'ptr @b' (via launder intrinsic) to: 'ptr %b'.
; CHECK: createRenamedValueForV : Renamed 'ptr @N' (via launder intrinsic) to: 'ptr %N'.

; CHECK: clearLaunderIntrinBeforeRegion: Number of launder intrinsics for the region is 4.
; CHECK: clearLaunderIntrinBeforeRegion: Replacing launder intrinsic 'ptr %a' with its operand.
; CHECK: clearLaunderIntrinBeforeRegion: Replacing launder intrinsic 'ptr [[B1]]' with its operand.
; CHECK: clearLaunderIntrinBeforeRegion: Replacing launder intrinsic 'ptr %b' with its operand.
; CHECK-DAG: clearLaunderIntrinBeforeRegion: Clearing 1 unhandled intrinsics.
; CHECK: clearLaunderIntrinBeforeRegion: Replacing launder intrinsic 'ptr %N' with its operand.

; CHECK-NOT: call ptr @llvm.launder.invariant.group

; Check that the hoisted IF Instruction is used in the target codegen
; CHECK: %[[CHECK:[^ ]+]] = icmp ne i1 %[[IF]], false
; CHECK: br i1 %[[CHECK]], label %{{.+}}, label %{{.+}}

; Check that the same SIZE Instruction is used in maps for a and b, which had
; the same constant-expr size.
; CHECK: %[[SIZE_GEP1:[^ ]+]] = getelementptr inbounds [3 x i64], ptr %.offload_sizes, i32 0, i32 0
; CHECK: store i64 %[[SIZE]], ptr %[[SIZE_GEP1]], align 8
; CHECK: %[[SIZE_GEP2:[^ ]+]] = getelementptr inbounds [3 x i64], ptr %.offload_sizes, i32 0, i32 1
; CHECK: store i64 %[[SIZE]], ptr %[[SIZE_GEP2]], align 8

; Check that globals @a, @b, @N are not used in the outlined function.
; CHECK: define internal void @__omp_offloading_{{.*}}foov{{.*}}(ptr %a, ptr %b, i64 %N.val)
; CHECK: %N.fpriv = alloca i64, align 8
; CHECK: store i64 %N.val, ptr %N.fpriv, align 8
; CHECK: %[[A1:[^ ]+]] = getelementptr inbounds [10 x i32], ptr %a, i64 0, i64 1
; CHECK: %[[A1_LOAD:[^ ]+]] = load i32, ptr %[[A1]], align 4
; CHECK: %[[N1:[^ ]+]] = ptrtoint ptr %N.fpriv to i64
; CHECK: %[[N2:[^ ]+]] = mul nsw i64 %[[N1]], 4
; CHECK: %call = call i32 (ptr, ...) @printf(ptr @.str, i32 %[[A1_LOAD]], i64 %[[N2]])

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64"
target device_triples = "x86_64"

@N = external global i64, align 8
@a = external global [10 x i32], align 16
@b = external global [10 x i32], align 16
@.str = private unnamed_addr constant [16 x i8] c"a[1] = %d, %ld\0A\00", align 1

define hidden void @_Z3foov() {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.IF"(i1 icmp ne (i64 ptrtoint (ptr @N to i64), i64 0)),
    "QUAL.OMP.MAP.TOFROM"(ptr @a, ptr getelementptr inbounds ([10 x i32], ptr @a, i64 0, i64 0), i64 mul nuw (i64 ptrtoint (ptr @N to i64), i64 4), i64 35, ptr null, ptr null),
    "QUAL.OMP.MAP.TOFROM"(ptr @b, ptr getelementptr inbounds ([10 x i32], ptr @b, i64 0, i64 1), i64 mul nuw (i64 ptrtoint (ptr @N to i64), i64 4), i64 3, ptr null, ptr null),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr @N, i64 0, i32 1) ]

  %1 = load i32, ptr getelementptr inbounds ([10 x i32], ptr @a, i64 0, i64 1), align 4
  %call = call i32 (ptr, ...) @printf(ptr @.str, i32 %1, i64 mul nsw (i64 ptrtoint (ptr @N to i64), i64 4))

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare i32 @printf(ptr, ...)

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 66309, i32 64032527, !"_Z3foov", i32 6, i32 0, i32 0}
