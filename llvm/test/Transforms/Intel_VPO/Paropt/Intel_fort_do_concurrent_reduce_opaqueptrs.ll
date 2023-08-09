; INTEL_CUSTOMIZATION
; RUN: opt -vpo-paropt-loop-mapping-scheme=1 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-loop-collapse -S %s | FileCheck %s
; RUN: opt -vpo-paropt-loop-mapping-scheme=1 -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse)' -S %s | FileCheck %s
; RUN: opt -vpo-paropt-loop-mapping-scheme=0 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s
; RUN: opt -vpo-paropt-loop-mapping-scheme=0 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck %s

; This test checks that "do concurrent" construct, in which FE emits GENERICLOOP directive and implicitly binds
; to TEAMS region, is mapped to DISTRIBUTE.PARLOOP and permits "reduction" clauses after prepare pass

; Test src:
;
; program main
;    integer:: i
;    integer:: a(10)
;
;    i = 1
;    a(:) = 11
;    a(5) = 22
;
;    do concurrent(i=1:10) reduce(+:a)
;      a(i)=a(i)+11
;    end do
; end program

; Verify that DIR.OMP.GENERICLOOP is mapped to DIR.OMP.DISTRIBUTE.PARLOOP for DO_CONCURRENT
; CHECK-NOT: call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(), {{.*}}
; CHECK: call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(), "QUAL.EXT.DO.CONCURRENT"(), {{.*}}, "QUAL.OMP.REDUCTION.ADD:TYPED"

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@0 = internal unnamed_addr constant i32 2, align 4

define void @MAIN__() {
alloca_0:
  %"$io_ctx" = alloca [8 x i64], align 8
  %"main_$A" = alloca [10 x i32], align 16
  %"main_$I" = alloca i32, align 4
  %"main_$I$_1" = alloca i32, align 4
  %"$loop_ctr" = alloca i64, align 8
  store i32 1, ptr %"main_$I", align 4
  store i64 1, ptr %"$loop_ctr", align 8
  br label %loop_test6

loop_test6:                                       ; preds = %loop_body7, %alloca_0
  %"$loop_ctr_fetch.2" = load i64, ptr %"$loop_ctr", align 8
  %rel.1 = icmp sle i64 %"$loop_ctr_fetch.2", 10
  br i1 %rel.1, label %loop_body7, label %loop_exit8

loop_body7:                                       ; preds = %loop_test6
  %"$loop_ctr_fetch.1" = load i64, ptr %"$loop_ctr", align 8
  %"main_$A[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %"main_$A", i64 %"$loop_ctr_fetch.1")
  store i32 11, ptr %"main_$A[]", align 4
  %"$loop_ctr_fetch.3" = load i64, ptr %"$loop_ctr", align 8
  %add.1 = add nsw i64 %"$loop_ctr_fetch.3", 1
  store i64 %add.1, ptr %"$loop_ctr", align 8
  br label %loop_test6

loop_exit8:                                       ; preds = %loop_test6
  %"main_$A[]1" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %"main_$A", i64 5)
  store i32 22, ptr %"main_$A[]1", align 4
  %do.norm.lb = alloca i32, align 4
  store i32 0, ptr %do.norm.lb, align 4
  %do.norm.ub = alloca i32, align 4
  store i32 9, ptr %do.norm.ub, align 4
  %do.norm.iv = alloca i32, align 4
  br label %bb_new10

do.cond14:                                        ; preds = %do.body15, %bb_new12
  %do.norm.iv_fetch.5 = load i32, ptr %do.norm.iv, align 4
  %do.norm.ub_fetch.6 = load i32, ptr %do.norm.ub, align 4
  %rel.2 = icmp sle i32 %do.norm.iv_fetch.5, %do.norm.ub_fetch.6
  br i1 %rel.2, label %do.body15, label %do.epilog16

do.body15:                                        ; preds = %do.cond14
  %do.norm.iv_fetch.7 = load i32, ptr %do.norm.iv, align 4
  %add.3 = add nsw i32 %do.norm.iv_fetch.7, 1
  store i32 %add.3, ptr %"main_$I$_1", align 4
  %"main_$I$_1_fetch.8" = load i32, ptr %"main_$I$_1", align 4
  %int_sext = sext i32 %"main_$I$_1_fetch.8" to i64
  %"main_$A[]2" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %"main_$A", i64 %int_sext)
  %"main_$A[]_fetch.9" = load i32, ptr %"main_$A[]2", align 4
  %add.4 = add nsw i32 %"main_$A[]_fetch.9", 11
  %"main_$I$_1_fetch.10" = load i32, ptr %"main_$I$_1", align 4
  %int_sext3 = sext i32 %"main_$I$_1_fetch.10" to i64
  %"main_$A[]4" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %"main_$A", i64 %int_sext3)
  store i32 %add.4, ptr %"main_$A[]4", align 4
  %do.norm.iv_fetch.11 = load i32, ptr %do.norm.iv, align 4
  %add.5 = add nsw i32 %do.norm.iv_fetch.11, 1
  store i32 %add.5, ptr %do.norm.iv, align 4
  br label %do.cond14

do.epilog16:                                      ; preds = %do.cond14
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.GENERICLOOP"() ]

  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TEAMS"() ]

  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET"() ]

  ret void

bb_new12:                                         ; preds = %bb_new11
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(),
    "QUAL.EXT.DO.CONCURRENT"(),
    "QUAL.OMP.COLLAPSE"(i32 1),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr %"main_$A", i32 0, i32 10),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %"main_$I$_1", i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %do.norm.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %do.norm.iv, i32 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %do.norm.ub, i32 0) ]

  %do.norm.lb_fetch.4 = load i32, ptr %do.norm.lb, align 4
  store i32 %do.norm.lb_fetch.4, ptr %do.norm.iv, align 4
  br label %do.cond14

bb_new10:                                         ; preds = %loop_exit8
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.EXT.DO.CONCURRENT"(),
    "QUAL.OMP.MAP.TOFROM"(ptr %"main_$A", ptr %"main_$A", i64 40, i64 551, ptr null, ptr null), ; MAP type: 551 = 0x227 = IMPLICIT (0x200) | TARGET_PARAM (0x20) | ALWAYS (0x4) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.PRIVATE:TYPED"(ptr %do.norm.iv, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %"main_$I$_1", i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %do.norm.ub, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %do.norm.lb, i32 0, i32 1) ]

  br label %bb_new11

bb_new11:                                         ; preds = %bb_new10
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.EXT.DO.CONCURRENT"(),
    "QUAL.OMP.SHARED:TYPED"(ptr %do.norm.ub, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %do.norm.lb, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %do.norm.iv, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %"main_$I$_1", i32 0, i32 1) ]

  br label %bb_new12
}

declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) 

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token) 

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 64773, i32 7868225, !"MAIN__", i32 9, i32 0, i32 0}
; end INTEL_CUSTOMIZATION
