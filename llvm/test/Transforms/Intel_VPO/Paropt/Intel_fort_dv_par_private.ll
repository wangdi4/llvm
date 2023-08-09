; INTEL_CUSTOMIZATION
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

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
;               !$omp parallel private(a)
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


; Check for the allocation of local dope vector
; CHECK: [[PRIV_DV:%[^ ]+]] = alloca %"QNCA_a0$i16*$rank3$", align 8

; Check that the dope vector init call is emitted
; CHECK: [[SIZE:%[^ ]+]] = call i64 @_f90_dope_vector_init2(ptr [[PRIV_DV]], ptr %"foo_$A")

; Check that local data is allocated and stored to the addr0 field of the dope vector.
; CHECK: [[IS_ALLOCATED:%[^ ]+]] = icmp sgt i64 [[SIZE]], 0
; CHECK: br i1 [[IS_ALLOCATED]], label %allocated.then, label %{{.*}}
; CHECK: allocated.then:
; CHECK: [[NUM_ELEMENTS:%[^ ]+]] = udiv i64 [[SIZE]], 2
; CHECK: [[ADDR0:%[^ ]+]] = getelementptr inbounds %"QNCA_a0$i16*$rank3$", ptr [[PRIV_DV]], i32 0, i32 0
; CHECK: [[DATA:%[^ ]+]] = alloca i16, i64 [[NUM_ELEMENTS]], align 2
; CHECK: store ptr [[DATA]], ptr [[ADDR0]]

; Check that the private DV is used inside the region.
; CHECK: %"foo_$A.addr_a0$_fetch" = load ptr, ptr [[PRIV_DV]], align 1
; CHECK: store i16 1, ptr %"foo_$A.addr_a0$_fetch", align 1

%"QNCA_a0$i16*$rank3$" = type { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }

define void @foo_(ptr noalias dereferenceable(120) "assumed_shape" "ptrnoalias" %"foo_$A") {
bb_new2:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.PRIVATE:F90_DV.TYPED"(ptr %"foo_$A", %"QNCA_a0$i16*$rank3$" zeroinitializer, i16 0) ]

  %"foo_$A.addr_a0$_fetch" = load ptr, ptr %"foo_$A", align 1
  store i16 1, ptr %"foo_$A.addr_a0$_fetch", align 1

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

; end INTEL_CUSTOMIZATION
