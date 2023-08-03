; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src:

; #include <stdio.h>
; int main() {
;   int a;
;   for (long i = 0; i < 10; i++) {
;     #pragma omp target map(a)
;     {
;       int b;
;       printf("&b = %p\n", &b);
;     }
;   }
;   return 0;
; }

; Check that the local offload_baseptr/ptr/mapper arrays are allocated in the
; entry block of the function, and not inside the for loop.
; CHECK: entry:
; CHECK:   %.offload_baseptrs = alloca [1 x ptr], align 8
; CHECK:   %.offload_ptrs = alloca [1 x ptr], align 8
; CHECK:   %.offload_mappers = alloca [1 x ptr], align 8
; CHECK:   %.run_host_version = alloca i32, align 4
; CHECK: br label %for.cond

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

@.str = private unnamed_addr constant [9 x i8] c"&b = %p\0A\00", align 1
@llvm.global_ctors = appending global [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 0, ptr @.omp_offloading.requires_reg, ptr null }]

define dso_local noundef i32 @main() {
entry:
  %retval = alloca i32, align 4
  %a = alloca i32, align 4
  %i = alloca i64, align 8
  %b = alloca i32, align 4
  store i32 0, ptr %retval, align 4
  store i64 0, ptr %i, align 8
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %0 = load i64, ptr %i, align 8
  %cmp = icmp slt i64 %0, 10
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(ptr %a, ptr %a, i64 4, i64 3, ptr null, ptr null), ; MAP type: 3 = 0x3 = FROM (0x2) | TO (0x1)
    "QUAL.OMP.PRIVATE:TYPED"(ptr %b, i32 0, i32 1) ]

  %call = call i32 (ptr, ...) @printf(ptr noundef @.str, ptr noundef %b) #1
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET"() ]

  br label %for.inc

for.inc:                                          ; preds = %for.body
  %2 = load i64, ptr %i, align 8
  %inc = add nsw i64 %2, 1
  store i64 %inc, ptr %i, align 8
  br label %for.cond, !llvm.loop !6

for.end:                                          ; preds = %for.cond
  ret i32 0
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

declare dso_local i32 @printf(ptr noundef, ...)

define internal void @.omp_offloading.requires_reg()  section ".text.startup" {
entry:
  call void @__tgt_register_requires(i64 1)
  ret void
}

declare void @__tgt_register_requires(i64)

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4}

!0 = !{i32 0, i32 64773, i32 3828811, !"_Z4main", i32 5, i32 0, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 51}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{i32 7, !"frame-pointer", i32 2}
!5 = distinct !{!5, !6}
!6 = !{!"llvm.loop.mustprogress"}

