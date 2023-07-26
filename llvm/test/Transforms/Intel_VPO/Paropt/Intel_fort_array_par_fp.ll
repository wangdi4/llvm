; INTEL_CUSTOMIZATION
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,loop-simplify,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; This file is a simplified version of the expected IR emitted by ifx FE.
; Test src:
;
; !       program main
; !       real, dimension(20) :: a
; !       a(2) = 2
; !       call foo (a)
; !       print *, a(2)
; !
; !       contains
;        subroutine foo(a)
;        real, dimension(10) :: a
;        !$omp parallel firstprivate(a)
;        a(2) = 1
; !       print *, a
;        !$omp end parallel
;        end subroutine
; !
; !       end program

; Check for the allocation of local copy
; CHECK: [[A_PRIV:%[^ ]+]] = alloca [10 x float]

; Check for initialization of local array
; CHECK: [[A_PRIV_GEP0:%.+]] = getelementptr inbounds [10 x float], ptr [[A_PRIV]], i32 0, i32 0
; CHECK: call void @llvm.memcpy.p0.p0.i64(ptr{{.*}}[[A_PRIV_GEP0]], ptr{{.*}}%"foo_$A", i64 40, i1 false)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo_(ptr %"foo_$A") {
alloca:
  %"var$1" = alloca [8 x i64], align 8
  br label %bb2

bb2:                                              ; preds = %alloca
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %"foo_$A", float 0.000000e+00, i64 10) ]

  %1 = getelementptr inbounds float, ptr %"foo_$A", i64 1
  store float 1.000000e+00, ptr %1
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
; end INTEL_CUSTOMIZATION
