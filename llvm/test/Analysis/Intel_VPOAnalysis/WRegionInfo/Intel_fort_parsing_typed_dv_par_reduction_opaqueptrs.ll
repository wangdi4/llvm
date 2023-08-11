; INTEL_CUSTOMIZATION
; REQUIRES: asserts
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-wrncollection -analyze -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,print<vpo-wrncollection>)' -S %s 2>&1 | FileCheck %s

; CHECK: REDUCTION clause (size=1): (ADD: F90_DV(TYPED(ptr %"foo_$A", TYPE: %"QNCA_a0$i16*$rank3$" = type { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, POINTEE_TYPE: i16, NUM_ELEMENTS: i32 1)))

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

%"QNCA_a0$i16*$rank3$" = type { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }

define void @foo_(ptr noalias dereferenceable(120) "assumed_shape" "ptrnoalias" %"foo_$A") {
bb_new2:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.REDUCTION.ADD:F90_DV.TYPED"(ptr %"foo_$A", %"QNCA_a0$i16*$rank3$" zeroinitializer, i16 0) ]

  %"foo_$A.addr_a0$_fetch" = load ptr, ptr %"foo_$A", align 1
  store i16 1, ptr %"foo_$A.addr_a0$_fetch", align 1

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

; end INTEL_CUSTOMIZATION
