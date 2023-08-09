; INTEL_CUSTOMIZATION
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-loop-transform  -disable-vpo-paropt-tile=false -S < %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-transform)' -disable-vpo-paropt-tile=false -S < %s | FileCheck %s
; ModuleID = 'tile2+tile2.ll.reordered'
; Test src:
;
; subroutine func1(A)
;     integer :: A(128,100)
;     integer :: i, j
;     !$omp tile sizes(4, 4)
;     !$omp tile sizes(5,16)
;     do i = 1, 100
;     do j = 1, 128
;        A(j,i) = j*1000 + i
;     end do; end do
; end subroutine


; CHECK: alloca_0
; CHECK: floor_iv{{[0-9]*}} = alloca
; CHECK: floor_iv{{[0-9]*}} = alloca
; CHECK: floor_iv{{[0-9]*}} = alloca
; CHECK: floor_iv{{[0-9]*}} = alloca

; CHECK: FLOOR.PREHEAD{{[0-9]*}}:                     ; preds = %bb_new3
; CHECK:   br label %FLOOR.HEAD[[V64:[0-9]*]]

; CHECK: FLOOR.HEAD[[V64]]:
; CHECK:   label %FLOOR.PREHEAD[[V45:[0-9]*]], label %DIR.OMP.END.TILE.1

; CHECK: FLOOR.PREHEAD[[V45]]:
; CHECK:   br label %FLOOR.HEAD[[V41:[0-9]*]]

; CHECK: FLOOR.HEAD[[V41]]:
; CHECK:   label %FLOOR.PREHEAD[[V22:[0-9]*]], label %FLOOR.LATCH[[V70:[0-9]*]]

; CHECK: FLOOR.LATCH[[V70]]:
; CHECK:   br label %FLOOR.HEAD[[V64]]

; CHECK: FLOOR.PREHEAD[[V22]]:
; CHECK:   br label %FLOOR.HEAD[[V18:[0-9]*]]

; CHECK: FLOOR.HEAD[[V18:[0-9]*]]:
; CHECK:   label %FLOOR.PREHEAD[[V0:[0-9]*]], label %FLOOR.LATCH[[V47:[0-9]*]]

; CHECK: FLOOR.LATCH[[V47]]:
; CHECK:   br label %FLOOR.HEAD[[V41]]

; CHECK: FLOOR.PREHEAD[[V0]]:
; CHECK:   br label %FLOOR.HEAD[[V1:[0-9]*]]

; CHECK: FLOOR.HEAD[[V1]]:
; CHECK:   label %DIR.OMP.TILE.3, label %FLOOR.LATCH[[V24:[0-9]*]]

; CHECK: FLOOR.LATCH[[V24]]:
; CHECK:   br label %FLOOR.HEAD[[V18]]

; CHECK: DIR.OMP.TILE.3:

; CHECK: omp.pdo.cond5:
; CHECK:   label %omp.pdo.body6, label %FLOOR.LATCH[[V3:[0-9]*]]

; CHECK: FLOOR.LATCH[[V3]]:
; CHECK:   br label %FLOOR.HEAD[[V1]]

; ModuleID = 't.ll'
source_filename = "tile1+tile1.f90"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define void @func1_(ptr noalias dereferenceable(4) %"func1_$A$argptr") {
alloca_0:
  %"$io_ctx" = alloca [8 x i64], align 8
  %"func1_$A$locptr" = alloca ptr, align 8
  %"func1_$J" = alloca i32, align 4
  %"func1_$I" = alloca i32, align 4
  store ptr %"func1_$A$argptr", ptr %"func1_$A$locptr", align 8
  %"func1_$A.1" = load ptr, ptr %"func1_$A$locptr", align 8
  %do.norm.lb = alloca i32, align 4
  store i32 0, ptr %do.norm.lb, align 4
  %do.norm.ub = alloca i32, align 4
  store i32 127, ptr %do.norm.ub, align 4
  %do.norm.iv = alloca i32, align 4
  %omp.pdo.norm.iv = alloca i32, align 4
  %omp.pdo.norm.lb = alloca i32, align 4
  store i32 0, ptr %omp.pdo.norm.lb, align 4
  %omp.pdo.norm.ub = alloca i32, align 4
  store i32 99, ptr %omp.pdo.norm.ub, align 4
  br label %bb_new2

bb_new2:  ; preds = %alloca_0
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TILE"(),
     "QUAL.OMP.SIZES"(i32 4, i32 4) ]
  br label %bb_new3

bb_new3:  ; preds = %bb_new2
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TILE"(),
     "QUAL.OMP.SIZES"(i32 5, i32 16),
     "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %omp.pdo.norm.iv, i32 0, ptr %do.norm.iv, i32 0),
     "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %omp.pdo.norm.ub, i32 0, ptr %do.norm.ub, i32 0),
     "QUAL.OMP.LIVEIN"(ptr %do.norm.lb),
     "QUAL.OMP.LIVEIN"(ptr %omp.pdo.norm.lb) ]
  br label %DIR.OMP.TILE.3

DIR.OMP.TILE.3:  ; preds = %bb_new3
  %omp.pdo.norm.lb_fetch.2 = load i32, ptr %omp.pdo.norm.lb, align 4
  store i32 %omp.pdo.norm.lb_fetch.2, ptr %omp.pdo.norm.iv, align 4
  br label %omp.pdo.cond5

omp.pdo.cond5:  ; preds = %do.epilog12, %DIR.OMP.TILE.3
  %omp.pdo.norm.iv_fetch.3 = load i32, ptr %omp.pdo.norm.iv, align 4
  %omp.pdo.norm.ub_fetch.4 = load i32, ptr %omp.pdo.norm.ub, align 4
  %rel.1 = icmp sle i32 %omp.pdo.norm.iv_fetch.3, %omp.pdo.norm.ub_fetch.4
  br i1 %rel.1, label %omp.pdo.body6, label %omp.pdo.epilog7

omp.pdo.body6:  ; preds = %omp.pdo.cond5
  %omp.pdo.norm.iv_fetch.5 = load i32, ptr %omp.pdo.norm.iv, align 4
  %add.2 = add nsw i32 %omp.pdo.norm.iv_fetch.5, 1
  store i32 %add.2, ptr %"func1_$I", align 4
  %do.norm.lb_fetch.6 = load i32, ptr %do.norm.lb, align 4
  store i32 %do.norm.lb_fetch.6, ptr %do.norm.iv, align 4
  br label %do.cond10

do.cond10:  ; preds = %omp.pdo.body6, %do.body11
  %do.norm.iv_fetch.7 = load i32, ptr %do.norm.iv, align 4
  %do.norm.ub_fetch.8 = load i32, ptr %do.norm.ub, align 4
  %rel.2 = icmp sle i32 %do.norm.iv_fetch.7, %do.norm.ub_fetch.8
  br i1 %rel.2, label %do.body11, label %do.epilog12

do.body11:  ; preds = %do.cond10
  %do.norm.iv_fetch.9 = load i32, ptr %do.norm.iv, align 4
  %add.4 = add nsw i32 %do.norm.iv_fetch.9, 1
  store i32 %add.4, ptr %"func1_$J", align 4
  %"func1_$J_fetch.10" = load i32, ptr %"func1_$J", align 4
  %mul.1 = mul nsw i32 %"func1_$J_fetch.10", 1000
  %"func1_$I_fetch.11" = load i32, ptr %"func1_$I", align 4
  %add.5 = add nsw i32 %mul.1, %"func1_$I_fetch.11"
  %"func1_$J_fetch.12" = load i32, ptr %"func1_$J", align 4
  %int_sext = sext i32 %"func1_$J_fetch.12" to i64
  %"func1_$I_fetch.13" = load i32, ptr %"func1_$I", align 4
  %int_sext1 = sext i32 %"func1_$I_fetch.13" to i64
  %"func1_$A.1[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 512, ptr elementtype(i32) %"func1_$A.1", i64 %int_sext1)
  %"func1_$A.1[][]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %"func1_$A.1[]", i64 %int_sext)
  store i32 %add.5, ptr %"func1_$A.1[][]", align 4
  %do.norm.iv_fetch.14 = load i32, ptr %do.norm.iv, align 4
  %add.6 = add nsw i32 %do.norm.iv_fetch.14, 1
  store i32 %add.6, ptr %do.norm.iv, align 4
  br label %do.cond10

do.epilog12:  ; preds = %do.cond10
  %"func1_$I_fetch.15" = load i32, ptr %"func1_$I", align 4
  %add.7 = add nsw i32 %"func1_$I_fetch.15", 1
  store i32 %add.7, ptr %"func1_$I", align 4
  %omp.pdo.norm.iv_fetch.16 = load i32, ptr %omp.pdo.norm.iv, align 4
  %add.8 = add nsw i32 %omp.pdo.norm.iv_fetch.16, 1
  store i32 %add.8, ptr %omp.pdo.norm.iv, align 4
  br label %omp.pdo.cond5

omp.pdo.epilog7:  ; preds = %omp.pdo.cond5
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TILE"() ]
  br label %DIR.OMP.END.TILE.1

DIR.OMP.END.TILE.1:  ; preds = %omp.pdo.epilog7
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TILE"() ]
  br label %DIR.OMP.END.TILE.2

DIR.OMP.END.TILE.2:  ; preds = %DIR.OMP.END.TILE.1
  ret void

}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none)
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

; end INTEL_CUSTOMIZATION
