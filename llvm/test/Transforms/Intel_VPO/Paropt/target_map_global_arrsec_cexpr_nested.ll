; REQUIRES: asserts
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s 2>&1 | FileCheck %s -check-prefix=TFORM
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s 2>&1 | FileCheck %s -check-prefix=TFORM
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s -check-prefix=PREPR
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck %s -check-prefix=PREPR

; float za[10];
; void bar(float*);
;
; void foo() {
;   #pragma omp parallel firstprivate(za)
;   #pragma omp target map(to:za[0:1])
;   bar(&za[0]);
; }

; PREPR:      store ptr @za, ptr [[ZA_ADDR1:%za.addr[0-9]*]], align 8
; PREPR:      call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"()
; PREPR-SAME:   "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr @za, float 0.000000e+00, i64 10)
; PREPR-SAME:   "QUAL.OMP.OPERAND.ADDR"(ptr @za, ptr [[ZA_ADDR1]])
; PREPR:      [[ZA2:%za[0-9]*]] = load volatile ptr, ptr [[ZA_ADDR1]], align 8
; PREPR:      store ptr [[ZA2]], ptr %za.addr
; PREPR:      call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"()
; PREPR-SAME:   "QUAL.OMP.MAP.TO"(ptr [[ZA2]], ptr [[ZA2]], i64 4, i64 33, ptr null, ptr null)
; PREPR-SAME:   "QUAL.OMP.OPERAND.ADDR"(ptr [[ZA2]], ptr %za.addr)
; PREPR:      [[ZA2:%za[0-9]*]] = load volatile ptr, ptr %za.addr, align 8
; PREPR:      call void @bar(ptr noundef [[ZA2]])

; TFORM-NOT: CodeExtractor captured out-of-clause argument

; Check that the outlined function for the target region takes a single argument for za
; TFORM: define internal void @__omp_offloading_{{.*}}foo{{.*}}(ptr [[ZA:%za[0-9]*]])
; TFORM: call void @bar(ptr noundef [[ZA]])

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

@za = dso_local global [10 x float] zeroinitializer, align 16
@llvm.global_ctors = appending global [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 0, ptr @.omp_offloading.requires_reg, ptr null }]

define dso_local void @foo() {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr @za, float 0.000000e+00, i64 10) ]

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TO"(ptr @za, ptr @za, i64 4, i64 33, ptr null, ptr null) ] ; MAP type: 33 = 0x21 = TARGET_PARAM (0x20) | TO (0x1)

  call void @bar(ptr noundef @za)
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET"() ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]

  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

declare dso_local void @bar(ptr noundef)

define internal void @.omp_offloading.requires_reg()  section ".text.startup" {
entry:
  call void @__tgt_register_requires(i64 1)
  ret void
}

declare void @__tgt_register_requires(i64)

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4}

!0 = !{i32 0, i32 64773, i32 3828829, !"foo", i32 6, i32 0, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 51}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{i32 7, !"frame-pointer", i32 2}
