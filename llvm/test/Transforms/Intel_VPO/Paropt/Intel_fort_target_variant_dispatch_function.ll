; INTEL_CUSTOMIZATION
;
; RUN: opt -vpo-paropt-prepare -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-paropt-prepare)' -S %s | FileCheck %s
;
; Comple the Fortran source code below with ifx -c -fiopenmp -fopenmp-targets=spir64
;
; module blas
; interface
;    function foo_gpu(arg)         !! variant function
;      integer foo_gpu
;      integer,intent(inout)  :: arg
;    end function foo_gpu
;
;    function foo(arg)             !! base function
;      integer foo
;      integer,intent(inout)  :: arg
;      !$omp  declare variant( foo:foo_gpu ) match( construct={target variant dispatch}, device={arch(gen)} )
;    end function foo
; end interface
; end module blas
;
; program main
; use blas
; use, intrinsic :: ISO_C_BINDING
; integer :: res_gpu, arg
; !$omp target variant dispatch
; res_gpu = foo(arg)
; !$omp end target variant dispatch
; end program
;
; CMPLRLLVM-20306
; Before the fix, compiler asserted when the BB containing the call was not
; the last BB in the VARIANT DISPATCH region. Fortran puts the store instr
; inside the region and triggered the assertion. The fix was to relax the
; restriction and the compiler simply ignores anything in the region after
; the call is found. (This wasn't a problem with subroutine calls because
; there were no store instructions in the region after the subroutine call.)
;
; Verify that the variant call to foo_gpu is emitted
; CHECK: %variant = call i32 @foo_gpu_(i32*

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

@0 = internal unnamed_addr constant i32 2

; Function Attrs: nounwind uwtable
define void @MAIN__() #0 {
alloca_1:
  %"var$1" = alloca [8 x i64], align 16
  %"main_$ARG" = alloca i32, align 8
  %"main_$RES_GPU" = alloca i32, align 8
  br label %bb2

bb2:                                              ; preds = %alloca_1
  br label %bb3

bb3:                                              ; preds = %bb2
  %func_result = call i32 @for_set_reentrancy(i32* @0)
  br label %DIR.OMP.TARGET.VARIANT.DISPATCH.1

DIR.OMP.TARGET.VARIANT.DISPATCH.1:                ; preds = %bb3
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.VARIANT.DISPATCH"() ]
  br label %bb4

bb7:                                              ; preds = %bb8
  br label %bb9

bb5:                                              ; preds = %bb4
  br label %bb8

bb8:                                              ; preds = %bb5
  br label %bb7

bb9:                                              ; preds = %bb7
  %func_result2 = call i32 @foo_(i32* %"main_$ARG")
  br label %bb6

bb6:                                              ; preds = %bb9
  store i32 %func_result2, i32* %"main_$RES_GPU", align 1
  br label %DIR.OMP.END.TARGET.VARIANT.DISPATCH.2

DIR.OMP.END.TARGET.VARIANT.DISPATCH.2:            ; preds = %bb6
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET.VARIANT.DISPATCH"() ]
  br label %bb1

bb4:                                              ; preds = %DIR.OMP.TARGET.VARIANT.DISPATCH.1
  br label %bb5

bb1:                                              ; preds = %DIR.OMP.END.TARGET.VARIANT.DISPATCH.2
  ret void
}

declare i32 @for_set_reentrancy(i32*)
declare token @llvm.directive.region.entry() #1
declare void @llvm.directive.region.exit(token) #1
declare i32 @foo_(i32*) #2

attributes #0 = { nounwind uwtable "intel-lang"="fortran" "min-legal-vector-width"="0" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { nounwind }
attributes #2 = { "openmp-variant"="name:foo_gpu_;construct:target_variant_dispatch;arch:gen" }

!omp_offload.info = !{}

; end INTEL_CUSTOMIZATION
