; INTEL_CUSTOMIZATION
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-loop-collapse -S <%s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse)' -S <%s | FileCheck %s

; ! F90 source
; SUBROUTINE foo(N,M)
;   INTEGER :: N,M,j,k
;   !$omp target teams loop collapse(2)
;   DO k=1,N
;     DO j=1,N
;       call bar()
;     ENDDO
;   ENDDO
; END SUBROUTINE foo
;
; For host-only compilation (ie, when -fopenmp-targets is not used
; and therefore there is no "target device_triples" in the IR) of
; "target teams loop collapse(N)", check that:
;  - Loop collapsing is happening
;  - The computation of collapsed bounds is hoisted immediately before the TARGET construct
;
; CHECK:      store i64 0, ptr %omp.collapsed.lb
; CHECK-NEXT: store i64 %omp.collapsed.ub.value, ptr %omp.collapsed.ub
; CHECK-NEXT: call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"()

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define void @foo_(ptr noalias dereferenceable(4) %"foo_$N$argptr", ptr noalias dereferenceable(4) %"foo_$M$argptr") {
alloca_0:
  %"$io_ctx" = alloca [8 x i64], align 8
  %"foo_$N$locptr" = alloca ptr, align 8
  %"foo_$M$locptr" = alloca ptr, align 8
  %"foo_$K" = alloca i32, align 4
  %"foo_$J" = alloca i32, align 4
  store ptr %"foo_$N$argptr", ptr %"foo_$N$locptr", align 8
  %"foo_$N.1" = load ptr, ptr %"foo_$N$locptr", align 8
  store ptr %"foo_$M$argptr", ptr %"foo_$M$locptr", align 8
  %"foo_$M.2" = load ptr, ptr %"foo_$M$locptr", align 8
  %"foo_$N.1_fetch.9" = load i32, ptr %"foo_$N.1", align 4
  %do.end = alloca i32, align 4
  store i32 %"foo_$N.1_fetch.9", ptr %do.end, align 4
  %do.norm.lb = alloca i32, align 4
  store i32 0, ptr %do.norm.lb, align 4
  %do.norm.ub = alloca i32, align 4
  %do.end_fetch.10 = load i32, ptr %do.end, align 4
  %sub.3 = sub nsw i32 %do.end_fetch.10, 1
  %add.3 = add nsw i32 %sub.3, 1
  %sub.4 = sub nsw i32 %add.3, 1
  store i32 %sub.4, ptr %do.norm.ub, align 4
  %do.norm.iv = alloca i32, align 4
  %"foo_$N.1_fetch.3" = load i32, ptr %"foo_$N.1", align 4
  %omp.pdo.end = alloca i32, align 4
  store i32 %"foo_$N.1_fetch.3", ptr %omp.pdo.end, align 4
  %omp.pdo.norm.iv = alloca i32, align 4
  %omp.pdo.norm.lb = alloca i32, align 4
  store i32 0, ptr %omp.pdo.norm.lb, align 4
  %omp.pdo.norm.ub = alloca i32, align 4
  %omp.pdo.end_fetch.4 = load i32, ptr %omp.pdo.end, align 4
  %sub.1 = sub nsw i32 %omp.pdo.end_fetch.4, 1
  %add.1 = add nsw i32 %sub.1, 1
  %sub.2 = sub nsw i32 %add.1, 1
  store i32 %sub.2, ptr %omp.pdo.norm.ub, align 4
  br label %bb_new2

bb_new2:                                          ; preds = %alloca_0
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %do.norm.iv, i32 0, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %omp.pdo.norm.iv, i32 0, i64 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %do.norm.ub, i32 0, i64 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %do.norm.lb, i32 0, i64 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %omp.pdo.norm.lb, i32 0, i64 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %omp.pdo.norm.ub, i32 0, i64 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %"foo_$N.1", i32 0, i64 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %"foo_$J", i32 0, i64 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %"foo_$K", i32 0, i64 1) ]

  br label %bb_new3

bb_new3:                                          ; preds = %bb_new2
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.SHARED:TYPED"(ptr %do.norm.ub, i32 0, i64 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %do.norm.lb, i32 0, i64 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %omp.pdo.norm.lb, i32 0, i64 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %omp.pdo.norm.ub, i32 0, i64 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %"foo_$N.1", i32 0, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %do.norm.iv, i32 0, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %omp.pdo.norm.iv, i32 0, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %"foo_$J", i32 0, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %"foo_$K", i32 0, i64 1) ]

  br label %bb_new4

bb_new4:                                          ; preds = %bb_new3
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(),
    "QUAL.OMP.COLLAPSE"(i32 2),
    "QUAL.OMP.SHARED:TYPED"(ptr %"foo_$N.1", i32 0, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %"foo_$J", i32 0, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %"foo_$K", i32 0, i64 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %do.norm.lb, i32 0, i64 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %omp.pdo.norm.lb, i32 0, i64 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %omp.pdo.norm.iv, i32 0, ptr %do.norm.iv, i32 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %omp.pdo.norm.ub, i32 0, ptr %do.norm.ub, i32 0) ]

  %omp.pdo.norm.lb_fetch.5 = load i32, ptr %omp.pdo.norm.lb, align 4
  store i32 %omp.pdo.norm.lb_fetch.5, ptr %omp.pdo.norm.iv, align 4
  br label %omp.pdo.cond6

omp.pdo.cond6:                                    ; preds = %do.epilog13, %bb_new4
  %omp.pdo.norm.iv_fetch.6 = load i32, ptr %omp.pdo.norm.iv, align 4
  %omp.pdo.norm.ub_fetch.7 = load i32, ptr %omp.pdo.norm.ub, align 4
  %rel.1 = icmp sle i32 %omp.pdo.norm.iv_fetch.6, %omp.pdo.norm.ub_fetch.7
  br i1 %rel.1, label %omp.pdo.body7, label %omp.pdo.epilog8

omp.pdo.body7:                                    ; preds = %omp.pdo.cond6
  %omp.pdo.norm.iv_fetch.8 = load i32, ptr %omp.pdo.norm.iv, align 4
  %add.2 = add nsw i32 %omp.pdo.norm.iv_fetch.8, 1
  store i32 %add.2, ptr %"foo_$K", align 4
  %do.norm.lb_fetch.11 = load i32, ptr %do.norm.lb, align 4
  store i32 %do.norm.lb_fetch.11, ptr %do.norm.iv, align 4
  br label %do.cond11

do.cond11:                                        ; preds = %do.body12, %omp.pdo.body7
  %do.norm.iv_fetch.12 = load i32, ptr %do.norm.iv, align 4
  %do.norm.ub_fetch.13 = load i32, ptr %do.norm.ub, align 4
  %rel.2 = icmp sle i32 %do.norm.iv_fetch.12, %do.norm.ub_fetch.13
  br i1 %rel.2, label %do.body12, label %do.epilog13

do.body12:                                        ; preds = %do.cond11
  %do.norm.iv_fetch.14 = load i32, ptr %do.norm.iv, align 4
  %add.4 = add nsw i32 %do.norm.iv_fetch.14, 1
  store i32 %add.4, ptr %"foo_$J", align 4
  call void @bar_.void()
  %do.norm.iv_fetch.15 = load i32, ptr %do.norm.iv, align 4
  %add.5 = add nsw i32 %do.norm.iv_fetch.15, 1
  store i32 %add.5, ptr %do.norm.iv, align 4
  br label %do.cond11

do.epilog13:                                      ; preds = %do.cond11
  %omp.pdo.norm.iv_fetch.16 = load i32, ptr %omp.pdo.norm.iv, align 4
  %add.6 = add nsw i32 %omp.pdo.norm.iv_fetch.16, 1
  store i32 %add.6, ptr %omp.pdo.norm.iv, align 4
  br label %omp.pdo.cond6

omp.pdo.epilog8:                                  ; preds = %omp.pdo.cond6
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.GENERICLOOP"() ]

  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TEAMS"() ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

; Function Attrs: noinline nounwind optnone uwtable
define internal void @bar_.void() {
wrap_start20:
  call void (...) @bar_()
  ret void
}

declare void @bar_(...)

!omp_offload.info = !{!0}

!0 = !{i32 0, i32 66306, i32 82185368, !"foo_", i32 3, i32 0, i32 0}
; end INTEL_CUSTOMIZATION
