; INTEL_CUSTOMIZATION
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; This file is a simplified version of the IR expected from ifx FE.
; Test src:
; !  program main
; !      integer::a(2)
; !      a(1) = 10
; !      print *, a(1)
; !      call foo(a)
; !      print *, a(1)
; !  contains
;       subroutine foo(a)
;           integer::a(:)
;           !$omp target firstprivate(a)
; !          print *, a(1)
;           a(1) = 20
;           !$omp end target
;       end subroutine
; !  end program

; Check that a map-types array is created for %A, indicating that map clause was honored.
; CHECK: @.offload_maptypes = private unnamed_addr constant [3 x i64] [i64 32, i64 281474976710661, i64 281474976710677]

; Check that %A is privatized inside the outlined function.
; CHECK: define internal void @__omp_offloading_{{.*}}_main_IP_foo{{.*}}(ptr %A)

; Check for the allocation of local dope vector
; CHECK: [[PRIV_DV:%[^ ]+]] = alloca %"QNCA_a0$i32*$rank1$.0"

; Check that the dope vector init call is emitted
; CHECK: [[SIZE:%[^ ]+]] = call i64 @_f90_dope_vector_init2(ptr %{{[^ ]+}}, ptr %{{[^ ]+}})
; CHECK: [[NUM_ELEMENTS:%[^ ]+]] = udiv i64 [[SIZE]], 4

; Check that local data is allocated and stored to the addr0 field of the dope vector.
; CHECK: [[ADDR0:%[^ ]+]] = getelementptr inbounds %"QNCA_a0$i32*$rank1$.0", ptr [[PRIV_DV]], i32 0, i32 0
; CHECK: [[DATA:%[^ ]+]] = alloca i32, i64 [[NUM_ELEMENTS]]
; CHECK: store ptr [[DATA]], ptr [[PRIV_DV]]

; Check that we call f90_firstprivate_copy function.
; CHECK: call void @_f90_firstprivate_copy(ptr %{{[^ ]+}}, ptr %{{[^ ]+}})

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "x86_64"

%"QNCA_a0$i32*$rank1$.0" = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }

@0 = internal unnamed_addr constant [6 x i8] c"%lld\0A\00"

define void @main_IP_foo_(ptr dereferenceable(72) %A) {
alloca:
  br label %bb31

bb31:                                             ; preds = %alloca
  %addr0 = getelementptr inbounds %"QNCA_a0$i32*$rank1$.0", ptr %A, i32 0, i32 0
  %addr0.val = load ptr, ptr %addr0, align 8
  %next = getelementptr ptr, ptr %addr0, i64 1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TO"(ptr %A, ptr %A, i64 72, i64 32), ; MAP type: 32 = 0x20 = TARGET_PARAM (0x20)
    "QUAL.OMP.MAP.TO:CHAIN"(ptr %A, ptr %next, i64 64, i64 281474976710661), ; MAP type: 281474976710661 = 0x1000000000005 = MEMBER_OF_1 (0x1000000000000) | ALWAYS (0x4) | TO (0x1)
    "QUAL.OMP.MAP.TO:CHAIN"(ptr %A, ptr %addr0.val, i64 8, i64 281474976710677), ; MAP type: 281474976710677 = 0x1000000000015 = MEMBER_OF_1 (0x1000000000000) | PTR_AND_OBJ (0x10) | ALWAYS (0x4) | TO (0x1)
    "QUAL.OMP.FIRSTPRIVATE:F90_DV.TYPED"(ptr %A, %"QNCA_a0$i32*$rank1$.0" zeroinitializer, i32 0) ]

  %"A.addr_a0$_fetch11" = load ptr, ptr %A
  store i32 20, ptr %"A.addr_a0$_fetch11"

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 2055, i32 150346101, !"main_IP_foo_", i32 10, i32 0, i32 0}
; end INTEL_CUSTOMIZATION
