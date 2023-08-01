; INTEL_CUSTOMIZATION
; RUN: opt -vpo-paropt-loop-mapping-scheme=1 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-loop-collapse -vpo-paropt-prepare -S <%s | FileCheck %s
; RUN: opt -vpo-paropt-loop-mapping-scheme=1 -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse,vpo-paropt-prepare)' -S <%s | FileCheck %s

; This is based on unittests_2008F/doconc-doconc, which has a doubly nested do concurrent.
; FFE represents it as "TARGET TEAMS LOOP LOOP" in the IR. Under loop mapping scheme 1, the
; outer LOOP becomes DISTRIBUTE and the inner LOOP becomes PARALLEL DO.

; Verify that the TARGET construct is dropped for this cpu-only compilation
; CHECK-NOT: @llvm.directive.region.entry() [ "DIR.OMP.TARGET"()
; CHECK-NOT: @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"()
; CHECK-LABEL: @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"()
; Verify that the outer DO CONCURRENT becomes "DISTRIBUTE"
; CHECK: @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE"(),{{.*}}"QUAL.EXT.DO.CONCURRENT"()
; Verify that the inner DO CONCURRENT becomes "PARALLEL DO"
; CHECK: @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),{{.*}}"QUAL.EXT.DO.CONCURRENT"()

;  !Fortran test source
;  subroutine foo
;    integer aaa(10,20)
;    do concurrent(integer :: i = 1 : 10)
;        do concurrent(integer :: j = 1 : 20)
;           aaa(i,j)=77
;        end do
;    end do
;  end subroutine foo

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo_() {
alloca_0:
  %"$io_ctx" = alloca [8 x i64], align 8
  %"foo_$J$_2" = alloca i32, align 4
  %"foo_$I$_1" = alloca i32, align 4
  %"foo_$AAA" = alloca [20 x [10 x i32]], align 16
  %do.norm.lb = alloca i32, align 4
  store i32 0, ptr %do.norm.lb, align 4
  %do.norm.ub = alloca i32, align 4
  store i32 9, ptr %do.norm.ub, align 4
  %do.norm.iv = alloca i32, align 4
  br label %bb_new2

bb_new2:                                          ; preds = %alloca_0
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.EXT.DO.CONCURRENT"(),
    "QUAL.OMP.MAP.TOFROM"(ptr %"foo_$AAA", ptr %"foo_$AAA", i64 800, i64 39, ptr null, ptr null), ; MAP type: 39 = 0x27 = TARGET_PARAM (0x20) | ALWAYS (0x4) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.PRIVATE:TYPED"(ptr %do.norm.iv, i32 0, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %"foo_$I$_1", i32 0, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %"foo_$J$_2", i32 0, i64 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %do.norm.ub, i32 0, i64 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %do.norm.lb, i32 0, i64 1) ]

  br label %bb_new3

bb_new3:                                          ; preds = %bb_new2
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.EXT.DO.CONCURRENT"(),
    "QUAL.OMP.SHARED:TYPED"(ptr %do.norm.ub, i32 0, i64 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %do.norm.lb, i32 0, i64 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %"foo_$AAA", i32 0, i64 200),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %do.norm.iv, i32 0, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %"foo_$I$_1", i32 0, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %"foo_$J$_2", i32 0, i64 1) ]

  br label %bb_new4

bb_new4:                                          ; preds = %bb_new3
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(),
    "QUAL.EXT.DO.CONCURRENT"(),
    "QUAL.OMP.COLLAPSE"(i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %"foo_$AAA", i32 0, i64 200),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %"foo_$I$_1", i32 0, i64 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %do.norm.lb, i32 0, i64 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %do.norm.iv, i32 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %do.norm.ub, i32 0) ]

  %do.norm.lb_fetch.1 = load i32, ptr %do.norm.lb, align 4
  store i32 %do.norm.lb_fetch.1, ptr %do.norm.iv, align 4
  br label %do.cond6

do.cond6:                                         ; preds = %do.epilog14, %bb_new4
  %do.norm.iv_fetch.2 = load i32, ptr %do.norm.iv, align 4
  %do.norm.ub_fetch.3 = load i32, ptr %do.norm.ub, align 4
  %rel.1 = icmp sle i32 %do.norm.iv_fetch.2, %do.norm.ub_fetch.3
  br i1 %rel.1, label %do.body7, label %do.epilog8

do.body7:                                         ; preds = %do.cond6
  %do.norm.iv_fetch.4 = load i32, ptr %do.norm.iv, align 4
  %add.2 = add nsw i32 %do.norm.iv_fetch.4, 1
  store i32 %add.2, ptr %"foo_$I$_1", align 4
  %do.norm.lb2 = alloca i32, align 4
  store i32 0, ptr %do.norm.lb2, align 4
  %do.norm.ub3 = alloca i32, align 4
  store i32 19, ptr %do.norm.ub3, align 4
  %do.norm.iv4 = alloca i32, align 4
  %"foo_$I$_1$local_init" = alloca i32, align 4
  %"foo_$I$_1_fetch.9" = load i32, ptr %"foo_$I$_1", align 4
  store i32 %"foo_$I$_1_fetch.9", ptr %"foo_$I$_1$local_init", align 4
  br label %bb_new10

bb_new10:                                         ; preds = %do.body7
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(),
    "QUAL.EXT.DO.CONCURRENT"(),
    "QUAL.OMP.COLLAPSE"(i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %"foo_$AAA", i32 0, i64 200),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %"foo_$I$_1", i32 0, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %"foo_$J$_2", i32 0, i64 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %"foo_$I$_1$local_init", i32 0, i64 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %do.norm.lb2, i32 0, i64 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %do.norm.iv4, i32 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %do.norm.ub3, i32 0) ]

  %do.norm.lb_fetch.5 = load i32, ptr %do.norm.lb2, align 4
  store i32 %do.norm.lb_fetch.5, ptr %do.norm.iv4, align 4
  br label %do.cond12

do.cond12:                                        ; preds = %do.body13, %bb_new10
  %do.norm.iv_fetch.6 = load i32, ptr %do.norm.iv4, align 4
  %do.norm.ub_fetch.7 = load i32, ptr %do.norm.ub3, align 4
  %rel.2 = icmp sle i32 %do.norm.iv_fetch.6, %do.norm.ub_fetch.7
  br i1 %rel.2, label %do.body13, label %do.epilog14

do.body13:                                        ; preds = %do.cond12
  %do.norm.iv_fetch.8 = load i32, ptr %do.norm.iv4, align 4
  %add.4 = add nsw i32 %do.norm.iv_fetch.8, 1
  store i32 %add.4, ptr %"foo_$J$_2", align 4
  %"foo_$I$_1$local_init_fetch.10" = load i32, ptr %"foo_$I$_1$local_init", align 4
  store i32 %"foo_$I$_1$local_init_fetch.10", ptr %"foo_$I$_1", align 4
  %"foo_$I$_1_fetch.11" = load i32, ptr %"foo_$I$_1", align 4
  %int_sext = sext i32 %"foo_$I$_1_fetch.11" to i64
  %"foo_$J$_2_fetch.12" = load i32, ptr %"foo_$J$_2", align 4
  %int_sext1 = sext i32 %"foo_$J$_2_fetch.12" to i64
  %"foo_$AAA[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 40, ptr elementtype(i32) %"foo_$AAA", i64 %int_sext1)
  %"foo_$AAA[][]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %"foo_$AAA[]", i64 %int_sext)
  store i32 77, ptr %"foo_$AAA[][]", align 4
  %do.norm.iv_fetch.13 = load i32, ptr %do.norm.iv4, align 4
  %add.5 = add nsw i32 %do.norm.iv_fetch.13, 1
  store i32 %add.5, ptr %do.norm.iv4, align 4
  br label %do.cond12

do.epilog14:                                      ; preds = %do.cond12
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.GENERICLOOP"() ]

  %do.norm.iv_fetch.14 = load i32, ptr %do.norm.iv, align 4
  %add.6 = add nsw i32 %do.norm.iv_fetch.14, 1
  store i32 %add.6, ptr %do.norm.iv, align 4
  br label %do.cond6

do.epilog8:                                       ; preds = %do.cond6
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.GENERICLOOP"() ]

  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TEAMS"() ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  ret void
}


declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64)

!omp_offload.info = !{!0}

!0 = !{i32 0, i32 66306, i32 76482268, !"foo_", i32 4, i32 0, i32 0}
; end INTEL_CUSTOMIZATION
