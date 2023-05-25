; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-use-mapper-api=false -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-use-mapper-api=false -S %s | FileCheck %s

; This file tests the implementation of omp target with map clause for
; two different target devices. Please note that the device information is
; represented as module level attribute in the form of
; target device_triples = "x86_64-mic,i386-pc-linux-gnu"
;
;  int x = 1, y;
;  int foo()
;  {
;    int i;
;    for (i=0;i<1000;i++)
;    #pragma omp target map(to:x) map(from:y)
;    y = x + 1; // The copy of y on the device has a value of 2.
;    printf("After the target region is executed, y = %d\n", y);
;    return 0;
;  }
;
; CHECK:  @.offload_maptypes = private unnamed_addr constant [2 x i64] [i64 34, i64 33]
; CHECK:  call i32 @__tgt_target({{.*}})

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

@x = dso_local global i32 1, align 4
@y = dso_local global i32 0, align 4
@.str = private unnamed_addr constant [45 x i8] c"After the target region is executed, y = %d\0A\00", align 1
@llvm.global_ctors = appending global [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 0, ptr @.omp_offloading.requires_reg, ptr null }]

define dso_local noundef i32 @foo()  {
entry:
  %i = alloca i32, align 4
  store i32 0, ptr %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %0 = load i32, ptr %i, align 4
  %cmp = icmp slt i32 %0, 1000
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.FROM"(ptr @y, ptr @y, i64 4, i64 34, ptr null, ptr null), ; MAP type: 34 = 0x22 = TARGET_PARAM (0x20) | FROM (0x2)
    "QUAL.OMP.MAP.TO"(ptr @x, ptr @x, i64 4, i64 33, ptr null, ptr null) ] ; MAP type: 33 = 0x21 = TARGET_PARAM (0x20) | TO (0x1)

  %2 = load i32, ptr @x, align 4
  %add = add nsw i32 %2, 1
  store i32 %add, ptr @y, align 4
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET"() ]

  br label %for.inc

for.inc:                                          ; preds = %for.body
  %3 = load i32, ptr %i, align 4
  %inc = add nsw i32 %3, 1
  store i32 %inc, ptr %i, align 4
  br label %for.cond, !llvm.loop !6

for.end:                                          ; preds = %for.cond
  %4 = load i32, ptr @y, align 4
  %call = call i32 (ptr, ...) @printf(ptr noundef @.str, i32 noundef %4)
  ret i32 0
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

declare dso_local i32 @printf(ptr noundef, ...)

define internal void @.omp_offloading.requires_reg() #3 section ".text.startup" {
entry:
  call void @__tgt_register_requires(i64 1)
  ret void
}

declare void @__tgt_register_requires(i64)

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4}

!0 = !{i32 0, i32 64773, i32 3828829, !"foo", i32 8, i32 0, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 51}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{i32 7, !"frame-pointer", i32 2}
!5 = distinct !{!5, !6}
!6 = !{!"llvm.loop.mustprogress"}

