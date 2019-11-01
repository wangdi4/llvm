; INTEL_CUSTOMIZATION
; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S < %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,loop-simplify,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S < %s  | FileCheck %s

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

; ModuleID = 'par_array_fixed.f90'
source_filename = "par_array_fixed.f90"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo_(float* %"foo_$A") #0 {
alloca:
  %"var$1" = alloca [8 x i64], align 8
  %foo.A.cast = bitcast float* %"foo_$A" to [10 x float]*
  br label %bb2

bb2:                                              ; preds = %alloca
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.FIRSTPRIVATE"([10 x float]* %foo.A.cast) ]

; Check for the allocation of local copy
; CHECK: [[A_PRIV:%[^ ]+]] = alloca [10 x float]
; Check for initialization of local array

; CHECK: [[A_PRIV_CAST:%[^ ]+]] = bitcast [10 x float]* [[A_PRIV]] to i8*
; CHECK: [[A_CAST:%[^ ]+]] = bitcast [10 x float]* %foo.A.cast to i8*
; CHECK: call void @llvm.memcpy.p0i8.p0i8.i64(i8* [[A_PRIV_CAST]], i8* [[A_CAST]], i64 40, i1 false)

  %foo.A = bitcast [10 x float]* %foo.A.cast to float*
  %1 = getelementptr inbounds float, float* %foo.A, i64 1
  store float 1.000000e+00, float* %1
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { nounwind }
attributes #2 = { nounwind readnone speculatable }
; end INTEL_CUSTOMIZATION
