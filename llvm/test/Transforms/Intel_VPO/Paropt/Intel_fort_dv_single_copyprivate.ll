; INTEL_CUSTOMIZATION
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; This file is a simplified version of the IR emitted by ifx FE.
; Test src:
;
; !       program main
; !           integer(kind=2), dimension(3, 3, 3) :: a
; !           a(1,1,1) = 2
; !           call foo (a)
; !           call bar(a)
; !
; !       contains
;            subroutine foo(a)
;                integer(kind=2), dimension(:, :, :) :: a
; !               !$omp parallel private(a)
;                !$omp single
;                a(1,1,1) = 1
;                !$omp end single copyprivate(a)
; !               call bar(a)
; !               !$omp end parallel
;            end subroutine
; !           subroutine bar(a)
; !               integer(kind=2), dimension(:, :, :) :: a
; !               print *, a(1,1,1)
; !           end subroutine
; !
; !       end program

; Check for the copyprivate codegen within the single region
; CHECK: %__struct.kmp_copy_privates_t = type { ptr }

; CHECK: %[[CPRIV_STR:[^ ]+]] = alloca %__struct.kmp_copy_privates_t, align 8
; CHECK: %[[A_PTR_GEP:[^ ]+]] = getelementptr inbounds %__struct.kmp_copy_privates_t, ptr %[[CPRIV_STR]], i32 0, i32 0
; CHECK: store ptr %"foo_$A", ptr %[[A_PTR_GEP]], align 8
; CHECK: call void @__kmpc_copyprivate(ptr @{{[^ ,]+}}, i32 %{{[^ ,]+}}, i32 8, ptr %{{[^ ,]+}}, ptr @[[CPRIV_CALLBACK:[^ ]+]], i32 %{{[^ ]+}})

; Check for the handling of dope vector in the copyprivate callback.
; CHECK: define internal void @[[CPRIV_CALLBACK]](ptr %[[DST_STR:[^ ,]+]], ptr %[[SRC_STR:[^ ]+]]) {
; CHECK: %[[A_SRC_GEP:[^ ]+]] = getelementptr inbounds %__struct.kmp_copy_privates_t, ptr %[[SRC_STR]], i32 0, i32 0
; CHECK: %[[A_DST_GEP:[^ ]+]] = getelementptr inbounds %__struct.kmp_copy_privates_t, ptr %[[DST_STR]], i32 0, i32 0
; CHECK: %[[A_SRC:[^ ]+]] = load ptr, ptr %[[A_SRC_GEP]], align 8
; CHECK: %[[A_DST:[^ ]+]] = load ptr, ptr %[[A_DST_GEP]], align 8
; CHECK: call void @_f90_lastprivate_copy(ptr %[[A_SRC]], ptr %[[A_DST]])

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown<F6><F6>-linux-gnu"

%"QNCA_a0$i16*$rank3$" = type { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }

define void @foo_(ptr dereferenceable(120) "assumed_shape" "ptrnoalias" %"foo_$A") {
alloca_0:
  br label %bb1

bb1:                                              ; preds = %alloca_0
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SINGLE"(),
    "QUAL.OMP.COPYPRIVATE:F90_DV.TYPED"(ptr %"foo_$A", %"QNCA_a0$i16*$rank3$" zeroinitializer, i16 0) ]

  fence acquire
  %"foo_$A.addr_a0$_fetch" = load ptr, ptr %"foo_$A", align 1
  store i16 1, ptr %"foo_$A.addr_a0$_fetch", align 1
  fence release

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SINGLE"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
; end INTEL_CUSTOMIZATION
