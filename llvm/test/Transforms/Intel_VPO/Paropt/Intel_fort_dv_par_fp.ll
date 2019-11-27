; INTEL_CUSTOMIZATION
; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S < %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S < %s  | FileCheck %s

; This file is a simplified version of the IR emitted by ifx FE.
; Test src:
;
; !      program main
; !          integer(kind=2), dimension(3, 3, 3) :: a
; !          a(1,1,1) = 2
; !          call foo (a)
; !          call bar(a)
;
; !      contains
;           subroutine foo(a)
;               integer(kind=2), dimension(:, :, :) :: a
;               !$omp parallel firstprivate(a)
;               a(1,1,1) = 1
; !              call bar(a)
;               !$omp end parallel
;           end subroutine
; !          subroutine bar(a)
; !              integer(kind=2), dimension(:, :, :) :: a
; !              print *, a
; !          end subroutine
; !
; !      end program


; ModuleID = 'intel_fort_dv_par_private.ll'
source_filename = "par_dope_vector.f90"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo_({ i16*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }* noalias %"foo_$A") #0 {
alloca:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.FIRSTPRIVATE:F90_DV"({ i16*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }* %"foo_$A") ]

; Check for the allocation of local dope vector
; CHECK: [[PRIV_DV:%[^ ]+]] = alloca { i16*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }

; Check that the dope vector init call is emitted
; CHECK: [[SIZE:%[^ ]+]] = call i64 @_f90_dope_vector_init(i8* %{{[^ ]+}}, i8* %{{[^ ]+}})

; Check that local data is allocated and stored to the addr0 field of the dope vector.
; CHECK: [[ADDR0:%[^ ]+]] = getelementptr inbounds { i16*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, { i16*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }* [[PRIV_DV]], i32 0, i32 0
; CHECK: [[DATA:%[^ ]+]] = alloca i16, i64 [[SIZE]]
; CHECK: store i16* [[DATA]], i16** [[ADDR0]]

; Check that we call f90_firstprivate_copy function.
; CHECK: call void @_f90_firstprivate_copy(i8* %{{[^ ]+}}, i8* %{{[^ ]+}})

  %"foo_$A_$field0$" = getelementptr inbounds { i16*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, { i16*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }* %"foo_$A", i32 0, i32 0
  %"foo_$A_$field0$5" = load i16*, i16** %"foo_$A_$field0$"
  store i16 1, i16* %"foo_$A_$field0$5"

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { nounwind }

; end INTEL_CUSTOMIZATION
