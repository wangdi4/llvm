; INTEL_CUSTOMIZATION
; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-fast-reduction=false -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-fast-reduction=false -S %s | FileCheck %s

; This test is identical to Intel_fort_dv_par_reduction.ll, with fast reduction disabled.

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
;               !$omp parallel reduction(+:a)
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

; Check that there is no fast reduction struct type.
; CHECK-NOT: %struct.fast_red_t = type <{ %"QNCA_a0$i16*$rank3$" }>

; Check that no global is created to store the number of elements in the dv.
; CHECK-NOT: @{{[^ ]+}} = common thread_local global i64 0

; Check the code generated inside the outlined function for the parallel region.
; CHECK-LABEL: define internal void @foo{{[^ ]*}}DIR.OMP.PARALLEL{{[^ ]*}}(i32* {{[^ ,]+}}, i32* {{[^ ,]+}}, %"QNCA_a0$i16*$rank3$"* %"foo_$A")

; Check for the allocation of the local copy of the dope vector.
; CHECK: [[PRIV_DV:[^ ]+]] = alloca %"QNCA_a0$i16*$rank3$", align 1

; Check that the dope vector init call is emitted for PRIV_DV.
; CHECK: [[PRIV_DV_CAST:[^ ]+]] = bitcast %"QNCA_a0$i16*$rank3$"* [[PRIV_DV]] to i8*
; CHECK: [[ORIG_DV_CAST:[^ ]+]] = bitcast %"QNCA_a0$i16*$rank3$"* %"foo_$A" to i8*
; CHECK: [[PRIV_DV_ARR_SIZE:[^ ]+]] = call i64 @_f90_dope_vector_init(i8* [[PRIV_DV_CAST]], i8* [[ORIG_DV_CAST]])
; CHECK: [[NUM_ELEMENTS_1:[^ ]+]] = udiv i64 [[PRIV_DV_ARR_SIZE]], 2

; Check that local data is allocated and stored to the addr0 field of the dope vector.
; CHECK: [[PRIV_DV_ADDR0:[^ ]+]] = getelementptr inbounds %"QNCA_a0$i16*$rank3$", %"QNCA_a0$i16*$rank3$"* [[PRIV_DV]], i32 0, i32 0
; CHECK: [[PRIV_DV_DATA:[^ ]+]] = alloca i16, i64 [[NUM_ELEMENTS_1]], align 1
; CHECK: store i16* [[PRIV_DV_DATA]], i16** [[PRIV_DV_ADDR0]]

; Check that the GEP for the dope vector in the fast-reduction struct is computed:

; Check that PRIV_DV is used for the code inside the parallel region.
; CHECK: %"foo_$A.addr_a0$" = getelementptr inbounds %"QNCA_a0$i16*$rank3$", %"QNCA_a0$i16*$rank3$"* [[PRIV_DV]], i64 0, i32 0
; CHECK: %"foo_$A.addr_a0$_fetch" = load i16*, i16** %"foo_$A.addr_a0$", align 1
; CHECK: store i16 1, i16* %"foo_$A.addr_a0$_fetch", align 1

; Check for calls to kmpc_[end_]critical, and that PRIV_DV and Orig (%"foo_$A") are used between those calls.
; CHECK: call void @__kmpc_critical(%struct.ident_t* {{[^ ,]+}}, i32 {{[^ ,]+}}, [8 x i32]* {{[^ ,]+}})
; CHECK: {{[^ ]+}} = getelementptr inbounds %"QNCA_a0$i16*$rank3$", %"QNCA_a0$i16*$rank3$"* [[PRIV_DV]], i32 0, i32 0
; CHECK: {{[^ ]+}} = getelementptr inbounds %"QNCA_a0$i16*$rank3$", %"QNCA_a0$i16*$rank3$"* %"foo_$A", i32 0, i32 0
; CHECK: call void @__kmpc_end_critical(%struct.ident_t* {{[^ ,]+}}, i32 {{[^ ,]+}}, [8 x i32]* {{[^ ,]+}})


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
%"QNCA_a0$i16*$rank3$" = type { i16*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }

; Function Attrs: nounwind uwtable
define void @foo_(%"QNCA_a0$i16*$rank3$"* dereferenceable(120) "assumed_shape" "ptrnoalias" %"foo_$A") #0 {
alloca:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.REDUCTION.ADD:F90_DV"(%"QNCA_a0$i16*$rank3$"* %"foo_$A") ]

  %"foo_$A.addr_a0$" = getelementptr inbounds %"QNCA_a0$i16*$rank3$", %"QNCA_a0$i16*$rank3$"* %"foo_$A", i64 0, i32 0
  %"foo_$A.addr_a0$_fetch" = load i16*, i16** %"foo_$A.addr_a0$", align 1
  store i16 1, i16* %"foo_$A.addr_a0$_fetch", align 1

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { nounwind uwtable "intel-lang"="fortran" "min-legal-vector-width"="0" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { nounwind }

; end INTEL_CUSTOMIZATION
