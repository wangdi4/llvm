; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-use-mapper-api=false -S %s | FileCheck %s --check-prefixes=HST,ALL
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-use-mapper-api=false -S %s | FileCheck %s --check-prefixes=HST,ALL
;
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -switch-to-offload -S %s | FileCheck %s --check-prefixes=TGT,ALL
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -S %s | FileCheck %s --check-prefixes=TGT,ALL
;
; This tests checks paropt lowering of 'omp target' construct with 'thread_limit' clause. IR was hand modified because front end does not yet support the THREAD_LIMIT clause.
;
; Test src:
;
; #include <omp.h>
; void foo()
; {
;   int local = 2;
;   #pragma omp target thread_limit(local)
;   { }
; }

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "x86_64"

@llvm.global_ctors = appending global [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 0, ptr @.omp_offloading.requires_reg, ptr null }]

; HST: [[ID5:@.+\.region_id]] = weak constant i8 0
; ALL: [[NAME:@.+]] = internal target_declare unnamed_addr constant [{{[0-9]+}} x i8] c"[[OUTLINEDTARGET5:.+]]\00"

; HST: define dso_local void @foo{{.*}}
; TGT: define weak_odr dso_local void @{{.*}}Z3foo{{.*}}
; HST: [[LOCAL:%local]] = alloca i32, align 4
; HST: store i32 2, ptr [[LOCAL]], align 4
; HST: [[LOCALVAL:%local.val]] = load i32, ptr [[LOCAL]], align 4
; HST: [[LOCALVALZEXT:%local.val.zext]] = zext i32 [[LOCALVAL]] to i64
; HST: [[THREADLIMIT:%.+]] = load i32, ptr [[LOCAL]], align 4
; HST: call i32 @__tgt_target_teams(i64 {{.*}}, ptr [[ID5]], i32 1, ptr {{.*}}, ptr {{.*}}, ptr {{.*}}, ptr {{.*}}, i32 -1, i32 [[THREADLIMIT]])
; HST: call void @[[OUTLINEDTARGET5]](i64 [[LOCALVALZEXT]])

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo() #0 {
entry:
  %local = alloca i32, align 4
  store i32 2, ptr %local, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %local, i32 0, i32 1),
    "QUAL.OMP.THREAD_LIMIT:TYPED"(ptr %local, i32 0) ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: noinline nounwind uwtable
define internal void @.omp_offloading.requires_reg() #2 section ".text.startup" {
entry:
  call void @__tgt_register_requires(i64 1)
  ret void
}

; Function Attrs: nounwind
declare void @__tgt_register_requires(i64) #1

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4}

!0 = !{i32 0, i32 66313, i32 107162452, !"_Z3foo", i32 5, i32 0, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 51}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{i32 7, !"frame-pointer", i32 2}
