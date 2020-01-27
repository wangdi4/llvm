; INTEL_CUSTOMIZATION
; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S < %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,loop-simplify,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S < %s  | FileCheck %s

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
; ModuleID = 'Intel_fort_dv_par_do_lastprivate.ll'
source_filename = "par_dope_vector.f90"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo_({ i16*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }* %"foo_$A") #0 {
alloca:
  %"foo_$I" = alloca i32, align 8
  store i32 1, i32* %"foo_$I"
  %temp = alloca i32
  %temp2 = alloca i32
  %temp4 = alloca i32
  store i32 0, i32* %temp
  store i32 0, i32* %temp2
  store i32 0, i32* %temp4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.PRIVATE"(i32* %"foo_$I"), "QUAL.OMP.LASTPRIVATE:F90_DV"({ i16*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }* %"foo_$A"), "QUAL.OMP.FIRSTPRIVATE:F90_DV"({ i16*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }* %"foo_$A"), "QUAL.OMP.NORMALIZED.IV"(i32* %temp2), "QUAL.OMP.NORMALIZED.UB"(i32* %temp4), "QUAL.OMP.FIRSTPRIVATE"(i32* %temp) ]

; Check for the allocation of local dope vector
; CHECK: [[PRIV_DV:%[^ ]+]] = alloca { i16*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }
; Make sure that the private dope vector is allocated only once
; CHECK-NOT: {{%[^ ]+}} = alloca { i16*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }

; Check that the dope vector init call is emitted
; CHECK: [[SIZE:%[^ ]+]] = call i64 @_f90_dope_vector_init(i8* %{{[^ ]+}}, i8* %{{[^ ]+}})

; Check that local data is allocated and stored to the addr0 field of the dope vector.
; CHECK: [[ADDR0:%[^ ]+]] = getelementptr inbounds { i16*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, { i16*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }* [[PRIV_DV]], i32 0, i32 0
; CHECK: [[DATA:%[^ ]+]] = alloca i16, i64 [[SIZE]]
; CHECK: store i16* [[DATA]], i16** [[ADDR0]]

; Check that we call f90_lastprivate_copy function.
; CHECK: call void @_f90_firstprivate_copy(i8* %{{[^ ]+}}, i8* %{{[^ ]+}})
; CHECK: call void @_f90_lastprivate_copy(i8* %{{[^ ]+}}, i8* %{{[^ ]+}})

  %"foo_$A_$field0$" = getelementptr inbounds { i16*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, { i16*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }* %"foo_$A", i32 0, i32 0
  %"foo_$A_$field6$" = getelementptr inbounds { i16*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, { i16*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }* %"foo_$A", i32 0, i32 6, i32 0
  %"foo_$A_$field6$_$field1$" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"foo_$A_$field6$", i32 0, i32 1
  %"foo_$A_$field0$_fetch" = load i16*, i16** %"foo_$A_$field0$"
  %temp2_fetch64 = load i32, i32* %temp2
  %temp4_fetch65 = load i32, i32* %temp4
  %rel66 = icmp slt i32 %temp4_fetch65, %temp2_fetch64
  br i1 %rel66, label %bb5, label %bb7

bb7:                                              ; preds = %alloca
  store i32 0, i32* %temp2
  store i32 1, i32* %"foo_$I"
  br label %bb8

bb8:                                              ; preds = %bb8, %bb7
  %temp2_fetch6 = load i32, i32* %temp2
  %add8 = add nsw i32 %temp2_fetch6, 1
  store i32 %add8, i32* %"foo_$I"
  %"foo_$A_$field0$_fetch57" = load i16*, i16** %"foo_$A_$field0$"
  %func_result_fetch = load i64, i64* %"foo_$A_$field6$_$field1$"
  %1 = getelementptr inbounds i64, i64* %"foo_$A_$field6$_$field1$", i64 3
  %func_result10_fetch = load i64, i64* %1
  %2 = getelementptr inbounds i64, i64* %"foo_$A_$field6$_$field1$", i64 6
  %func_result12_fetch = load i64, i64* %2
  %add18 = add nsw i64 %func_result_fetch, %func_result10_fetch
  %func_result28_fetch = load i16, i16* %"foo_$A_$field0$_fetch57"
  %int_sext = sext i16 %func_result28_fetch to i32
  %add30 = add nsw i32 %int_sext, 1
  %int_sext32 = trunc i32 %add30 to i16
  store i16 %int_sext32, i16* %"foo_$A_$field0$_fetch57"
  %temp2_fetch59 = load i32, i32* %temp2
  %add61 = add nsw i32 %temp2_fetch59, 1
  store i32 %add61, i32* %temp2
  %temp4_fetch = load i32, i32* %temp4
  %rel = icmp sle i32 %add61, %temp4_fetch
  br i1 %rel, label %bb8, label %bb5

bb5:                                              ; preds = %bb8, %alloca
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { nounwind }
; end INTEL_CUSTOMIZATION
