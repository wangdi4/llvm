; INTEL_CUSTOMIZATION
; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
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

; ModuleID = 'fort_dv_target_private.ll'
source_filename = "fort_dv_target_private.f90"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "x86_64"

%"QNCA_a0$i32*$rank1$.0" = type { i32*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }

@0 = internal unnamed_addr constant [6 x i8] c"%lld\0A\00"

define void @main_IP_foo_(%"QNCA_a0$i32*$rank1$.0"* dereferenceable(72) %A) #0 {
alloca:
  br label %bb31

bb31:                                             ; preds = %alloca
  %addr0 = getelementptr inbounds %"QNCA_a0$i32*$rank1$.0", %"QNCA_a0$i32*$rank1$.0"* %A, i32 0, i32 0
  %addr0.val = load i32*, i32** %addr0, align 8
  %next = getelementptr i32*, i32** %addr0, i64 1

  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TO"(%"QNCA_a0$i32*$rank1$.0"* %A, %"QNCA_a0$i32*$rank1$.0"* %A, i64 72, i64 32), "QUAL.OMP.MAP.TO:CHAIN"(%"QNCA_a0$i32*$rank1$.0"* %A, i32** %next, i64 64, i64 281474976710661), "QUAL.OMP.MAP.TO:CHAIN"(%"QNCA_a0$i32*$rank1$.0"* %A, i32* %addr0.val, i64 8, i64 281474976710677), "QUAL.OMP.FIRSTPRIVATE:F90_DV"(%"QNCA_a0$i32*$rank1$.0"* %A) ]

; Check that a map-types array is created for %A, indicating that map clause was honored.
; CHECK: @.offload_maptypes = private unnamed_addr constant [3 x i64] [i64 32, i64 281474976710661, i64 281474976710677]

; Check that %A is privatized inside the outlined function.
; CHECK: define internal void @__omp_offloading_{{.*}}_main_IP_foo{{.*}}(%"QNCA_a0$i32*$rank1$.0"* %A)

; Check for the allocation of local dope vector
; CHECK: [[PRIV_DV:%[^ ]+]] = alloca %"QNCA_a0$i32*$rank1$.0"

; Check that the dope vector init call is emitted
; CHECK: [[SIZE:%[^ ]+]] = call i64 @_f90_dope_vector_init(i8* %{{[^ ]+}}, i8* %{{[^ ]+}})
; CHECK: [[NUM_ELEMENTS:%[^ ]+]] = udiv i64 [[SIZE]], 4

; Check that local data is allocated and stored to the addr0 field of the dope vector.
; CHECK: [[ADDR0:%[^ ]+]] = getelementptr inbounds %"QNCA_a0$i32*$rank1$.0", %"QNCA_a0$i32*$rank1$.0"* [[PRIV_DV]], i32 0, i32 0
; CHECK: [[DATA:%[^ ]+]] = alloca i32, i64 [[NUM_ELEMENTS]]
; CHECK: store i32* [[DATA]], i32** [[ADDR0]]

; Check that we call f90_firstprivate_copy function.
; CHECK: call void @_f90_firstprivate_copy(i8* %{{[^ ]+}}, i8* %{{[^ ]+}})

  %"A.addr_a0$" = getelementptr inbounds %"QNCA_a0$i32*$rank1$.0", %"QNCA_a0$i32*$rank1$.0"* %A, i32 0, i32 0
  %"A.addr_a0$_fetch11" = load i32*, i32** %"A.addr_a0$"
  %1 = getelementptr inbounds i32, i32* %"A.addr_a0$_fetch11", i64 0
  store i32 20, i32* %1

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { "min-legal-vector-width"="0" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { nounwind }

!omp_offload.info = !{!0}

!0 = !{i32 0, i32 2055, i32 150346101, !"main_IP_foo_", i32 10, i32 0, i32 0}
; end INTEL_CUSTOMIZATION
