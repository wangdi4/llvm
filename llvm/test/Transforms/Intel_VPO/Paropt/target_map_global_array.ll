; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s -check-prefix=PREPR
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck %s -check-prefix=PREPR
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -early-cse -vpo-restore-operands -S %s | FileCheck %s -check-prefix=RESTR
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,early-cse,vpo-restore-operands)' -S %s | FileCheck %s -check-prefix=RESTR
;
; Check that prepare pass generates a load-of-store for arrS.
; And vpo-paropt-restore undoes it.
;
; int arrS[100];
; void foo()
; {
;    #pragma omp target map(arrS[42:20])
;    {
;      arrS[50] = 3;
;    }
; }
;
; PREPR: store ptr @arrS, ptr [[AADDR:%[a-zA-Z._0-9]+]]
; PREPR: call token @llvm.directive.region.entry()
; PREPR-SAME: "QUAL.OMP.MAP.TOFROM"(ptr @arrS
; PREPR-SAME: "QUAL.OMP.OPERAND.ADDR"(ptr @arrS, ptr [[AADDR]])
; PREPR: [[A_RENAMED:%[a-zA-Z._0-9]+]] = load volatile ptr, ptr [[AADDR]]
; PREPR: [[GEP1:%[a-zA-Z._0-9]+]] = getelementptr inbounds [100 x i32], ptr [[A_RENAMED]], i64 0, i64 50
; PREPR: store i32 3, ptr [[GEP1]], align 8

; Check that after -vpo-restore-operands, the original use of arrS inside the GEP is restored.
; RESTR-NOT: store ptr @arrS, ptr {{%[a-zA-Z._0-9]+}}
; RESTR: call token @llvm.directive.region.entry()
; RESTR-SAME: "QUAL.OMP.MAP.TOFROM"(ptr @arrS{{.*}})
; RESTR-NOT: "QUAL.OMP.OPERAND.ADDR"
; RESTR: [[GEP2:%[a-zA-Z._0-9]+]] = getelementptr inbounds [100 x i32], ptr @arrS, i64 0, i64 50
; RESTR: store i32 3, ptr [[GEP2]], align 8

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

@arrS = dso_local global [100 x i32] zeroinitializer, align 16
@llvm.global_ctors = appending global [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 0, ptr @.omp_offloading.requires_reg, ptr null }]

define dso_local void @_Z3foo()  {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(ptr @arrS, ptr getelementptr inbounds ([100 x i32], ptr @arrS, i64 0, i64 42), i64 80, i64 35, ptr null, ptr null) ] ; MAP type: 35 = 0x23 = TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)

  store i32 3, ptr getelementptr inbounds ([100 x i32], ptr @arrS, i64 0, i64 50), align 8
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

define internal void @.omp_offloading.requires_reg()  section ".text.startup" {
entry:
  call void @__tgt_register_requires(i64 1)
  ret void
}

declare void @__tgt_register_requires(i64)

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4}

!0 = !{i32 0, i32 64773, i32 3828829, !"_Z3foo", i32 4, i32 0, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 51}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{i32 7, !"frame-pointer", i32 2}
