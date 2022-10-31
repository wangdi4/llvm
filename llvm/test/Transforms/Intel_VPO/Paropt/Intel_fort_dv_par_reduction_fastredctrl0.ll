; INTEL_CUSTOMIZATION
; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-fast-reduction-ctrl=0 -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-fast-reduction-ctrl=0 -S %s | FileCheck %s

; This test is identical to Intel_fort_dv_par_reduction.ll, with -vpo-paropt-fast-reduction-ctrl=0
; to disable creation of an extra local copy, outside the fast reduction struct,
; for the reduction operand dope vector.

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

; Check for the fast reduction struct type
; CHECK: %struct.fast_red_t = type <{ %"QNCA_a0$i16*$rank3$" }>

; Check that a global is created to store the number of elements in the dv.
; CHECK: [[NUM_ELEMENTS_GV:[^ ]+]] = common thread_local global i64 0

; Check that the fast reduction callback loads the num_elements global for the F90 DV.
; CHECK-LABEL: define internal void @foo{{[^ ]+}}tree_reduce{{[^ ]+}}(i8* %dst, i8* %src)
; CHECK: [[NUM_ELEMENTS_LOAD:[^ ]+]] = load i64, i64* [[NUM_ELEMENTS_GV]], align 8

; Now check the code generated inside the outlined function for the parallel region.
; CHECK-LABEL: define internal void @foo{{[^ ]*}}DIR.OMP.PARALLEL{{[^ ]*}}(i32* {{[^ ,]+}}, i32* {{[^ ,]+}}, %"QNCA_a0$i16*$rank3$"* %"foo_$A")

; Check for the allocation of the fast reduction struct, but not an extra local dope vector.
; CHECK: [[FAST_RED_STR:[^ ]+]] = alloca %struct.fast_red_t, align 8
; CHECK-NOT: alloca %"QNCA_a0$i16*$rank3$", align 1
; CHECK: [[FAST_RED_DV:[^ ]+]] = getelementptr inbounds %struct.fast_red_t, %struct.fast_red_t* [[FAST_RED_STR]], i32 0, i32 0

; Check that the dope vector init call is emitted for FAST_RED_DV.
; CHECK: [[FAST_RED_DV_CAST:[^ ]+]] = bitcast %"QNCA_a0$i16*$rank3$"* [[FAST_RED_DV]] to i8*
; CHECK: [[ORIG_DV_CAST:[^ ]+]] = bitcast %"QNCA_a0$i16*$rank3$"* %"foo_$A" to i8*
; CHECK: [[FAST_RED_DV_ARR_SIZE:[^ ]+]] = call i64 @_f90_dope_vector_init(i8* [[FAST_RED_DV_CAST]], i8* [[ORIG_DV_CAST]])
; CHECK: [[NUM_ELEMENTS:[^ ]+]] = udiv i64 [[FAST_RED_DV_ARR_SIZE]], 2

; Check that local data is allocated and stored to the addr0 field of the dope vector.
; CHECK: [[FAST_RED_DV_ADDR0:[^ ]+]] = getelementptr inbounds %"QNCA_a0$i16*$rank3$", %"QNCA_a0$i16*$rank3$"* [[FAST_RED_DV]], i32 0, i32 0
; CHECK: [[FAST_RED_DV_DATA:[^ ]+]] = alloca i16, i64 [[NUM_ELEMENTS]], align 2
; CHECK: store i16* [[FAST_RED_DV_DATA]], i16** [[FAST_RED_DV_ADDR0]]
; Check that num_elements is stored to a global so that it can be accessed from the reduction callback.
; CHECK: store i64 [[NUM_ELEMENTS]], i64* [[NUM_ELEMENTS_GV]], align 8

; Check that the GEP for the dope vector in the fast-reduction struct is computed:

; Check that FAST_RED_DV is used for the code inside the parallel region.
; CHECK: %"foo_$A.addr_a0$" = getelementptr inbounds %"QNCA_a0$i16*$rank3$", %"QNCA_a0$i16*$rank3$"* [[FAST_RED_DV]], i64 0, i32 0
; CHECK: %"foo_$A.addr_a0$_fetch" = load i16*, i16** %"foo_$A.addr_a0$", align 1
; CHECK: store i16 1, i16* %"foo_$A.addr_a0$_fetch", align 1

; Check for calls to kmpc_[end_]reduce, and that FAST_RED_DV and Orig (%"foo_$A") are used between those calls.
; CHECK: {{[^ ]+}} = call i32 @__kmpc_reduce(%struct.ident_t* {{[^ ,]+}}, i32 {{[^ ,]+}}, i32 1, i32 120, i8* {{[^ ,]+}}, void (i8*, i8*)* @foo{{[^ ,]*}}tree_reduce{{[^ ,]*}}, [8 x i32]* {{[^ ,]+}})
; CHECK: {{[^ ]+}} = getelementptr inbounds %"QNCA_a0$i16*$rank3$", %"QNCA_a0$i16*$rank3$"* [[FAST_RED_DV]], i32 0, i32 0
; CHECK: {{[^ ]+}} = getelementptr inbounds %"QNCA_a0$i16*$rank3$", %"QNCA_a0$i16*$rank3$"* %"foo_$A", i32 0, i32 0
; CHECK: call void @__kmpc_end_reduce(%struct.ident_t* {{[^ ,]+}}, i32 {{[^ ,]+}}, [8 x i32]* {{[^ ,]+}})


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
