; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-shared-privatization -pass-remarks-analysis=openmp -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-shared-privatization)' -pass-remarks-analysis=openmp -S %s 2>&1 | FileCheck %s

; Test src:
;
; subroutine foo(a)
;     integer*2 :: a(:)
; !$omp parallel firstprivate(a)
; !$omp end parallel
; end

; CHECK: FIRSTPRIVATE clause for variable 'foo_$A' on 'parallel' construct is redundant

; CHECK:      "QUAL.OMP.FIRSTPRIVATE:F90_DV.TYPED"(ptr null, %"QNCA_a0$i16*$rank1$" zeroinitializer, i16 0)
; CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr %"foo_$A", %"QNCA_a0$i16*$rank1$" zeroinitializer, i32 1)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$i16*$rank1$" = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }

define void @foo_(ptr noalias dereferenceable(72) "assumed_shape" "ptrnoalias" %"foo_$A") {
alloca_0:
  br label %bb_new2

bb_new2:                                          ; preds = %alloca_0
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.FIRSTPRIVATE:F90_DV.TYPED"(ptr %"foo_$A", %"QNCA_a0$i16*$rank1$" zeroinitializer, i16 0) ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]

  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
