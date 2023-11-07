; INTEL_CUSTOMIZATION
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-loop-collapse -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring),vpo-paropt-loop-collapse' -S %s | FileCheck %s

; This test checks that in case DO.CONCURRENT is present on a GENERICLOOP with PARALLEL parent, the loop is mapped to a PARALLEL.LOOP
; CHECK: %omp.genericloop = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.EXT.DO.CONCURRENT"()

; Test SRC
;
; subroutine foo
; !$omp parallel num_threads(4)
;  do concurrent(integer :: i = 1 : 10)
;   print*, "hi"
;  end do
; !$omp end parallel
; end subroutine foo

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@strlit = internal unnamed_addr constant [2 x i8] c"hi"

define void @foo_() {
alloca_0:
  %"$io_ctx" = alloca [8 x i64], align 8
  %"foo_$I$_1" = alloca i32, align 4
  %strlit_fetch.5 = load [2 x i8], ptr @strlit, align 1
  br label %bb_new2

do.cond7:                                         ; preds = %do.body8.lr.ph9, %bb_new5
  %do.norm.iv_fetch.2 = load i32, ptr %do.norm.iv, align 4
  %do.norm.ub_fetch.3 = load i32, ptr %do.norm.ub, align 4
  %rel.1 = icmp sle i32 %do.norm.iv_fetch.2, %do.norm.ub_fetch.3
  br i1 %rel.1, label %do.body8.lr.ph9, label %do.epilog10

do.body8.lr.ph9:                                  ; preds = %do.cond7
  %do.norm.iv_fetch.4 = load i32, ptr %do.norm.iv, align 4
  %add.2 = add nsw i32 %do.norm.iv_fetch.4, 1
  store i32 %add.2, ptr %"foo_$I$_1", align 4
  %func_result = call i32 (ptr, ...) @printf(ptr @strlit)
  %func_result2 = call i32 (ptr, ...) @printf(ptr @strlit)
  %do.norm.iv_fetch.6 = load i32, ptr %do.norm.iv, align 4
  %add.3 = add nsw i32 %do.norm.iv_fetch.6, 1
  store i32 %add.3, ptr %do.norm.iv, align 4
  br label %do.cond7

do.epilog10:                                      ; preds = %do.cond7
  call void @llvm.directive.region.exit(token %omp.genericloop) [ "DIR.OMP.END.GENERICLOOP"() ]
  call void @llvm.directive.region.exit(token %omp.parallel) [ "DIR.OMP.END.PARALLEL"() ]
  ret void

bb_new5:                                          ; preds = %bb_new4
  %omp.genericloop = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(),
     "QUAL.EXT.DO.CONCURRENT"(),
     "QUAL.OMP.BIND.TEAMS"(),
     "QUAL.OMP.COLLAPSE"(i32 1),
     "QUAL.OMP.PRIVATE:TYPED"(ptr %"foo_$I$_1", i32 0, i64 1),
     "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %do.norm.lb, i32 0, i64 1),
     "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %do.norm.iv, i32 0),
     "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %do.norm.ub, i32 0) ]

  %do.norm.lb_fetch.1 = load i32, ptr %do.norm.lb, align 4
  store i32 %do.norm.lb_fetch.1, ptr %do.norm.iv, align 4
  br label %do.cond7

bb_new3:                                          ; preds = %bb_new2
  br label %bb_new4

bb_new4:                                          ; preds = %bb_new3
  br label %bb_new5

bb_new2:                                          ; preds = %alloca_0
  %omp.parallel = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
     "QUAL.OMP.NUM_THREADS"(i64 4),
     "QUAL.OMP.SHARED:TYPED"(ptr %"foo_$I$_1", i32 0, i64 1) ]

  %do.norm.lb = alloca i32, align 4
  store i32 0, ptr %do.norm.lb, align 4
  %do.norm.ub = alloca i32, align 4
  store i32 9, ptr %do.norm.ub, align 4
  %do.norm.iv = alloca i32, align 4
  br label %bb_new3
}

declare token @llvm.directive.region.entry()

declare i32 @printf(ptr %0, ...)

declare void @llvm.directive.region.exit(token %0)
; end INTEL_CUSTOMIZATION
