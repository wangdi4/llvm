; INTEL_CUSTOMIZATION
; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
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
; CHECK: %__struct.kmp_copy_privates_t = type { %"QNCA_a0$i16*$rank3$"* }

; CHECK: %[[CPRIV_STR:[^ ]+]] = alloca %__struct.kmp_copy_privates_t, align 8
; CHECK: %[[A_PTR_GEP:[^ ]+]] = getelementptr inbounds %__struct.kmp_copy_privates_t, %__struct.kmp_copy_privates_t* %[[CPRIV_STR]], i32 0, i32 0
; CHECK: store %"QNCA_a0$i16*$rank3$"* %"foo_$A", %"QNCA_a0$i16*$rank3$"** %[[A_PTR_GEP]], align 8
; CHECK: call void @__kmpc_copyprivate(%struct.ident_t* @{{[^ ,]+}}, i32 %{{[^ ,]+}}, i32 8, i8* %{{[^ ,]+}}, i8* bitcast (void (%__struct.kmp_copy_privates_t*, %__struct.kmp_copy_privates_t*)* @[[CPRIV_CALLBACK:[^ ]+]] to i8*), i32 %{{[^ ]+}})

; Check for the handling of dope vector in the copyprivate callback.
; CHECK: define internal void @[[CPRIV_CALLBACK]](%__struct.kmp_copy_privates_t* %[[DST_STR:[^ ,]+]], %__struct.kmp_copy_privates_t* %[[SRC_STR:[^ ]+]]) {
; CHECK: %[[A_SRC_GEP:[^ ]+]] = getelementptr inbounds %__struct.kmp_copy_privates_t, %__struct.kmp_copy_privates_t* %[[SRC_STR]], i32 0, i32 0
; CHECK: %[[A_DST_GEP:[^ ]+]] = getelementptr inbounds %__struct.kmp_copy_privates_t, %__struct.kmp_copy_privates_t* %[[DST_STR]], i32 0, i32 0
; CHECK: %[[A_SRC:[^ ]+]] = load %"QNCA_a0$i16*$rank3$"*, %"QNCA_a0$i16*$rank3$"** %[[A_SRC_GEP]], align 8
; CHECK: %[[A_DST:[^ ]+]] = load %"QNCA_a0$i16*$rank3$"*, %"QNCA_a0$i16*$rank3$"** %[[A_DST_GEP]], align 8
; CHECK: %[[A_SRC_CAST:[^ ]+]] = bitcast %"QNCA_a0$i16*$rank3$"* %[[A_SRC]] to i8*
; CHECK: %[[A_DST_CAST:[^ ]+]] = bitcast %"QNCA_a0$i16*$rank3$"* %[[A_DST]] to i8*
; CHECK: call void @_f90_lastprivate_copy(i8* %[[A_SRC_CAST]], i8* %[[A_DST_CAST]])


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown<F6><F6>-linux-gnu"

%"QNCA_a0$i16*$rank3$" = type { i16*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }

; Function Attrs: noinline nounwind uwtable
define void @foo_(%"QNCA_a0$i16*$rank3$"* dereferenceable(120) "assumed_shape" "ptrnoalias" %"foo_$A") #0 {
alloca_0:
  br label %bb1

bb1:                                              ; preds = %alloca_0
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SINGLE"(), "QUAL.OMP.COPYPRIVATE:F90_DV"(%"QNCA_a0$i16*$rank3$"* %"foo_$A") ]

  fence acquire
  %"foo_$A.addr_a0$" = getelementptr inbounds %"QNCA_a0$i16*$rank3$", %"QNCA_a0$i16*$rank3$"* %"foo_$A", i64 0, i32 0
  %"foo_$A.addr_a0$_fetch" = load i16*, i16** %"foo_$A.addr_a0$", align 1
  store i16 1, i16* %"foo_$A.addr_a0$_fetch", align 1
  fence release

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SINGLE"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind uwtable "intel-lang"="fortran" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { nounwind }
; end INTEL_CUSTOMIZATION
