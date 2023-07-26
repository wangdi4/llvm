; INTEL_CUSTOMIZATION
; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -opaque-pointers=0 -passes='function(vpo-cfg-restructuring,loop-simplify,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

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

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo_(float* %"foo_$A") {
alloca:
  %"var$1" = alloca [8 x i64], align 8
  %foo.A.cast = bitcast float* %"foo_$A" to [10 x float]*
  br label %bb2

bb2:                                              ; preds = %alloca
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.FIRSTPRIVATE"([10 x float]* %foo.A.cast) ]

; Check for the allocation of local copy
; CHECK: [[A_PRIV:%[^ ]+]] = alloca [10 x float]
; Check for initialization of local array

; CHECK: [[A_PRIV_CAST:%[^ ]+]] = bitcast [10 x float]* [[A_PRIV]] to i8*
; CHECK: [[A_CAST:%[^ ]+]] = bitcast [10 x float]* %foo.A.cast to i8*
; CHECK: call void @llvm.memcpy.p0i8.p0i8.i64(i8*{{.*}}[[A_PRIV_CAST]], i8*{{.*}}[[A_CAST]], i64 40, i1 false)

  %foo.A = bitcast [10 x float]* %foo.A.cast to float*
  %1 = getelementptr inbounds float, float* %foo.A, i64 1
  store float 1.000000e+00, float* %1
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
; end INTEL_CUSTOMIZATION
