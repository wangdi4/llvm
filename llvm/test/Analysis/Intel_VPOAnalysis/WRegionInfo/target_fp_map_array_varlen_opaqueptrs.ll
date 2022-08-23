; REQUIRES: asserts
; RUN: opt -enable-new-pm=0 -vpo-cfg-restructuring -vpo-wrncollection -analyze -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,print<vpo-wrncollection>)' -disable-output -S %s 2>&1 | FileCheck %s

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

; CHECK: FIRSTPRIVATE clause (size=1): VARLEN(ptr %vla) , TYPED (TYPE: i32, NUM_ELEMENTS: i64 2)
; CHECK: MAP clause (size=1): CHAIN,VARLEN(<ptr %vla, ptr %vla, i64 %map.size, 161 (0x00000000000000A1), null, null> )

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
