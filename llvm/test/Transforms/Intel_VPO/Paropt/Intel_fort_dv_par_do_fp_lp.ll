; INTEL_CUSTOMIZATION
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,loop-simplify,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

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
;                !$omp parallel do lastprivate(a) firstprivate(a)
;                do i = 1,1
;                  a(1,1,1) = a(1,1,1) + 1
; !                 call bar(a)
;                end do
;                !$omp end parallel do
;            end subroutine
; !           subroutine bar(a)
; !               integer(kind=2), dimension(:, :, :) :: a
; !               print *, a
; !           end subroutine
; !
; !       end program

; Check for the allocation of local dope vector
; CHECK: [[PRIV_DV:%[^ ]+]] = alloca { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }
; Make sure that the private dope vector is allocated only once
; CHECK-NOT: {{%[^ ]+}} = alloca { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }

; Check that the dope vector init call is emitted
; CHECK: [[SIZE:%[^ ]+]] = call i64 @_f90_dope_vector_init2(ptr [[PRIV_DV]], ptr %{{[^ ]+}})
; CHECK: [[IS_ALLOCATED:%[^ ]+]] = icmp sgt i64 [[SIZE]], 0
; CHECK: br i1 [[IS_ALLOCATED]], label %[[IF_THEN:[^ ]+]], label %[[IF_CONTINUE:[^, ]+]]

; CHECK: [[IF_THEN]]:
; CHECK: [[NUM_ELEMENTS:%[^ ]+]] = udiv i64 [[SIZE]], 2
; Check that local data is allocated and stored to the addr0 field of the dope vector.
; CHECK: [[ADDR0:%[^ ]+]] = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, ptr [[PRIV_DV]], i32 0, i32 0
; CHECK: [[DATA:%[^ ]+]] = alloca i16, i64 [[NUM_ELEMENTS]]
; CHECK: store ptr [[DATA]], ptr [[ADDR0]]
; CHECK: br label %[[IF_CONTINUE]]

; CHECK: [[IF_CONTINUE]]:
; CHECK: call void @_f90_firstprivate_copy(ptr [[PRIV_DV]], ptr %{{[^ ]+}})

; CHECK: call void @_f90_lastprivate_copy(ptr %{{[^ ]+}}, ptr %{{[^ ]+}})

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo_(ptr %"foo_$A") {
alloca:
  %"foo_$I" = alloca i32, align 8
  store i32 1, ptr %"foo_$I"
  %temp = alloca i32
  %temp2 = alloca i32
  %temp4 = alloca i32
  store i32 0, ptr %temp
  store i32 0, ptr %temp2
  store i32 0, ptr %temp4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %"foo_$I", i32 0, i64 1),
    "QUAL.OMP.LASTPRIVATE:F90_DV.TYPED"(ptr %"foo_$A", { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] } zeroinitializer, i16 0),
    "QUAL.OMP.FIRSTPRIVATE:F90_DV.TYPED"(ptr %"foo_$A", { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] } zeroinitializer, i16 0),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %temp2, i32 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %temp4, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %temp, i32 0, i64 1) ]

  %"foo_$A_$field0$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, ptr %"foo_$A", i32 0, i32 0
  %"foo_$A_$field6$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, ptr %"foo_$A", i32 0, i32 6, i32 0
  %"foo_$A_$field6$_$field1$" = getelementptr inbounds { i64, i64, i64 }, ptr %"foo_$A_$field6$", i32 0, i32 1
  %"foo_$A_$field0$_fetch" = load ptr, ptr %"foo_$A_$field0$"
  %temp2_fetch64 = load i32, ptr %temp2
  %temp4_fetch65 = load i32, ptr %temp4
  %rel66 = icmp slt i32 %temp4_fetch65, %temp2_fetch64
  br i1 %rel66, label %bb5, label %bb7

bb7:                                              ; preds = %alloca
  store i32 0, ptr %temp2
  store i32 1, ptr %"foo_$I"
  br label %bb8

bb8:                                              ; preds = %bb8, %bb7
  %temp2_fetch6 = load i32, ptr %temp2
  %add8 = add nsw i32 %temp2_fetch6, 1
  store i32 %add8, ptr %"foo_$I"
  %"foo_$A_$field0$_fetch57" = load ptr, ptr %"foo_$A_$field0$"
  %func_result_fetch = load i64, ptr %"foo_$A_$field6$_$field1$"
  %1 = getelementptr inbounds i64, ptr %"foo_$A_$field6$_$field1$", i64 3
  %func_result10_fetch = load i64, ptr %1
  %2 = getelementptr inbounds i64, ptr %"foo_$A_$field6$_$field1$", i64 6
  %func_result12_fetch = load i64, ptr %2
  %add18 = add nsw i64 %func_result_fetch, %func_result10_fetch
  %func_result28_fetch = load i16, ptr %"foo_$A_$field0$_fetch57"
  %int_sext = sext i16 %func_result28_fetch to i32
  %add30 = add nsw i32 %int_sext, 1
  %int_sext32 = trunc i32 %add30 to i16
  store i16 %int_sext32, ptr %"foo_$A_$field0$_fetch57"
  %temp2_fetch59 = load i32, ptr %temp2
  %add61 = add nsw i32 %temp2_fetch59, 1
  store i32 %add61, ptr %temp2
  %temp4_fetch = load i32, ptr %temp4
  %rel = icmp sle i32 %add61, %temp4_fetch
  br i1 %rel, label %bb8, label %bb5

bb5:                                              ; preds = %bb8, %alloca
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
; end INTEL_CUSTOMIZATION
