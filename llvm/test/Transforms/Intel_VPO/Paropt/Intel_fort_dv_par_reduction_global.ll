; INTEL_CUSTOMIZATION
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test for reduction codegen on F90_DVs that are globals.

; Test src:
;
;   module mymod
;    integer*2, allocatable :: A(:)
; end
;
; subroutine foo
;   USE mymod
;
;   !$omp parallel reduction(+: A)
;     A(1) = 111
;   !$omp end parallel
; END

; Check for the fast reduction struct type
; CHECK: %struct.fast_red_t = type <{ %"QNCA_a0$i16*$rank1$" }>

; Check that a global is created to store the number of elements in the dv.
; CHECK: [[NUM_ELEMENTS_GV:[^ ]+]] = private thread_local global i64 0

; Check that the fast reduction callback loads the num_elements global for the F90 DV.
; CHECK-LABEL: define internal void @foo{{[^ ]+}}tree_reduce{{[^ ]+}}(ptr %dst, ptr %src)
; CHECK: [[NUM_ELEMENTS_LOAD:[^ ]+]] = load i64, ptr [[NUM_ELEMENTS_GV]], align 8

; Now check the code generated inside the outlined function for the parallel region.
; CHECK-LABEL: define internal void @foo{{[^ ]*}}DIR.OMP.PARALLEL{{[^ ]*}}(ptr %tid, ptr %bid)

; Check for the allocation of local dope vector, and the fast reduction struct.
; CHECK: [[FAST_RED_STR:[^ ]+]] = alloca %struct.fast_red_t, align 16
; CHECK: [[PRIV_DV:[^ ]+]] = alloca %"QNCA_a0$i16*$rank1$", align 16

; Check that the dope vector init call is emitted for PRIV_DV.
; CHECK: [[PRIV_DV_ARR_SIZE:[^ ]+]] = call i64 @_f90_dope_vector_init2(ptr [[PRIV_DV]], ptr @A)
; CHECK: [[NUM_ELEMENTS:[^ ]+]] = udiv i64 [[PRIV_DV_ARR_SIZE]], 2

; Check that local data is allocated and stored to the addr0 field of the dope vector.
; CHECK: [[PRIV_DV_ADDR0:[^ ]+]] = getelementptr inbounds %"QNCA_a0$i16*$rank1$", ptr [[PRIV_DV]], i32 0, i32 0
; CHECK: [[PRIV_DV_DATA:[^ ]+]] = alloca i16, i64 [[NUM_ELEMENTS]], align 16
; CHECK: store ptr [[PRIV_DV_DATA]], ptr [[PRIV_DV_ADDR0]]
; Check that num_elements is stored to a global so that it can be accessed from the reduction callback.
; CHECK: store i64 [[NUM_ELEMENTS]], ptr [[NUM_ELEMENTS_GV]], align 8

; Check that the GEP for the dope vector in the fast-reduction struct is computed:
; CHECK: [[FAST_RED_DV:[^ ]+]] = getelementptr inbounds %struct.fast_red_t, ptr [[FAST_RED_STR]], i32 0, i32 0

; Check that PRIV_DV is used for the code inside the parallel region.
; CHECK: %"A.addr_a0$" = getelementptr inbounds %"QNCA_a0$i16*$rank1$", ptr [[PRIV_DV]], i64 0, i32 0
; CHECK: %"A.addr_a0$_fetch" = load ptr, ptr %"A.addr_a0$", align 1
; CHECK: store i16 111, ptr %"A.addr_a0$_fetch", align 1

; Check that the copy of the dope vector in the fast reduction struct is initialized.
; CHECK: [[FAST_RED_DV_ARR_SIZE:[^ ]+]] = call i64 @_f90_dope_vector_init2(ptr [[FAST_RED_DV]], ptr [[PRIV_DV]])
; CHECK: [[NUM_ELEMENTS_1:[^ ]+]] = udiv i64 [[FAST_RED_DV_ARR_SIZE]], 2
; CHECK: [[FAST_RED_DV_ADDR0:[^ ]+]] = getelementptr inbounds %"QNCA_a0$i16*$rank1$", ptr [[FAST_RED_DV]], i32 0, i32 0
; CHECK: [[FAST_RED_DV_DATA:[^ ]+]] = alloca i16, i64 [[NUM_ELEMENTS_1]], align 16
; CHECK: store ptr [[FAST_RED_DV_DATA]], ptr [[FAST_RED_DV_ADDR0]]

; Check for calls to kmpc_[end_]reduce, and that FAST_RED_DV and Orig (%"foo_$A") are used between those calls, but not PRIV_DV.
; CHECK: {{.+}} = call i32 @__kmpc_reduce(ptr @{{.+}}, i32 %{{.+}}, i32 1, i32 72, ptr {{.+}}, ptr @foo{{[^ ,]*}}tree_reduce{{[^ ,]*}}, ptr {{.+}})
; CHECK-NOT: {{[^ ]+}} = getelementptr inbounds %"QNCA_a0$i16*$rank1$", ptr [[PRIV_DV]], i32 0, i32 0
; CHECK: {{[^ ]+}} = getelementptr inbounds %"QNCA_a0$i16*$rank1$", ptr [[FAST_RED_DV]], i32 0, i32 0
; CHECK: {{[^ ]+}} = load ptr, ptr @A, align 8
; CHECK: call void @__kmpc_end_reduce(ptr {{[^ ,]+}}, i32 {{[^ ,]+}}, ptr {{[^ ,]+}})

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$i16*$rank1$" = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }

@A = global %"QNCA_a0$i16*$rank1$" { ptr null, i64 0, i64 0, i64 1073741952, i64 1, i64 0, [1 x { i64, i64, i64 }] zeroinitializer }

define void @foo_() {
bb1:
  br label %bb2

bb2:                                              ; preds = %bb1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
  "QUAL.OMP.REDUCTION.ADD:F90_DV.TYPED"(ptr @A, %"QNCA_a0$i16*$rank1$" zeroinitializer, i16 0) ]

  %"A.addr_a0$" = getelementptr inbounds %"QNCA_a0$i16*$rank1$", ptr @A, i64 0, i32 0
  %"A.addr_a0$_fetch" = load ptr, ptr %"A.addr_a0$", align 1
  store i16 111, ptr %"A.addr_a0$_fetch", align 1

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]

  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
; end INTEL_CUSTOMIZATION
