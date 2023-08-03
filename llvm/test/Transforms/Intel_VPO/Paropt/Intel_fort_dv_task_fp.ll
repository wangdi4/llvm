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
;               !$omp task firstprivate(a)
;               a(1,1,1) = 1
; !              call bar(a)
;               !$omp end task
;           end subroutine
; !          subroutine bar(a)
; !              integer(kind=2), dimension(:, :, :) :: a
; !              print *, a
; !          end subroutine
; !
; !      end program

; Check for the space allocated for the private copy. First field is for the
; dope vector struct, followed by two i64s for array size and thunk buffer offset.
; CHECK: %__struct.kmp_privates.t = type { { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, i64, i64 }

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo_(ptr noalias %"foo_$A") {
alloca:

; Check for call to f90_dv_size to get array size in bytes.
; CHECK: [[ARR_SIZE_IF_ALLOCATED:%[^ ]+]] = call i64 @_f90_dope_vector_size(ptr %"foo_$A")
; CHECK: [[IS_ALLOCATED:%[^ ]+]] = icmp slt i64 [[ARR_SIZE_IF_ALLOCATED]], 0
; CHECK: [[ARR_SIZE_IN_BYTES:%[^ ]+]] = select i1 [[IS_ALLOCATED]], i64 0, i64 [[ARR_SIZE_IF_ALLOCATED]]

; Check that we call _task_alloc with total size of task_t_with_privates + arr_size
; CHECK: [[TOTAL_SIZE:%[^ ]+]] = add i64 208, [[ARR_SIZE_IN_BYTES]]
; CHECK: [[TASK_ALLOC:[^ ]+]] = call ptr @__kmpc_omp_task_alloc({{.*}}i64 [[TOTAL_SIZE]]{{.*}})

; Check that F90 DV's array size and offset are stored in the thunk
; CHECK: [[ARR_SIZE_GEP:%[^ ]+]] = getelementptr inbounds %__struct.kmp_privates.t, ptr %{{[^ ]+}}, i32 0, i32 1
; CHECK: store i64 [[ARR_SIZE_IN_BYTES]], ptr [[ARR_SIZE_GEP]]
; CHECK: [[ARR_OFFSET_GEP:%[^ ]+]] = getelementptr inbounds %__struct.kmp_privates.t, ptr %{{[^ ]+}}, i32 0, i32 2
; CHECK: store i64 208, ptr [[ARR_OFFSET_GEP]]

; Check that the new DV is initialized using the original DV
; CHECK: [[FOO_A_PRIV:%[^ ]+]] = getelementptr inbounds %__struct.kmp_privates.t, ptr %.privates, i32 0, i32 0
; CHECK: %.dv.init = call i64 @_f90_dope_vector_init2(ptr [[FOO_A_PRIV]], ptr %"foo_$A")

; Check that the local buffer space for the local dope vector, is initialized (as it is firstprivate).
; First, the buffer should be linked to the dope vector's base field
; CHECK: [[FOO_A_PRIV2:%[^ ]+]] = getelementptr inbounds %__struct.kmp_privates.t, ptr {{[^,]+}}, i32 0, i32 0
; CHECK: [[FOO_A_PRIV1:%[^ ]+]] = getelementptr inbounds %__struct.kmp_privates.t, ptr {{[^,]+}}, i32 0, i32 0

; Check for :"if(size != 0) then link data field to base of dope vector".
; CHECK: [[FOO_A_DATA_SIZE_GEP:[^ ]+]] = getelementptr inbounds %__struct.kmp_privates.t, ptr {{[^,]+}}, i32 0, i32 1
; CHECK: [[FOO_A_DATA_SIZE:[^ ]+]] = load i64, ptr [[FOO_A_DATA_SIZE_GEP]], align 8
; CHECK: [[IS_SIZE_NON_ZERO:[^ ]+]] = icmp ne i64 [[FOO_A_DATA_SIZE]], 0
; CHECK: br i1 [[IS_SIZE_NON_ZERO]], label %[[IF_THEN:[^ ]+]], label %[[IF_CONTINUE:[^, ]]]

; CHECK: [[IF_THEN]]:
; CHECK: [[FOO_A_BUFFER_OFFSET_GEP:%[^ ]+]] = getelementptr inbounds %__struct.kmp_privates.t, ptr {{[^,]+}}, i32 0, i32 2
; CHECK: [[FOO_A_BUFFER_OFFSET:%[^ ]+]] = load i64, ptr [[FOO_A_BUFFER_OFFSET_GEP]]
; CHECK: [[FOO_A_BUFFER:%[^ ]+]] = getelementptr i8, ptr [[TASK_ALLOC]], i64 [[FOO_A_BUFFER_OFFSET]]
; CHECK: store ptr [[FOO_A_BUFFER]], ptr [[FOO_A_PRIV1]]
; CHECK: br label %[[IF_CONTINUE]]

; After the linking is done, there should be a call to f90_firstprivate_copy
; CHECK: [[IF_CONTINUE]]:
; CHECK: call void @_f90_firstprivate_copy(ptr [[FOO_A_PRIV2]], ptr %"foo_$A")

  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(),
    "QUAL.OMP.FIRSTPRIVATE:F90_DV.TYPED"(ptr %"foo_$A", { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] } zeroinitializer, i16 0) ]

; CHECK: define internal void @{{.*}}DIR.OMP.TASK{{.*}}(i32 %tid, ptr %taskt.withprivates)

; Inside the outlined function, check that the buffer for the array is linked to the base field of the dope vector
; CHECK: [[FOO_A_PRIV3:%[^ ]+]] = getelementptr inbounds %__struct.kmp_privates.t, ptr {{[^,]+}}, i32 0, i32 0
; CHECK: [[FOO_A_BUFFER_OFFSET_GEP1:%[^ ]+]] = getelementptr inbounds %__struct.kmp_privates.t, ptr {{[^,]+}}, i32 0, i32 2
; CHECK: [[FOO_A_BUFFER_OFFSET1:%[^ ]+]] = load i64, ptr [[FOO_A_BUFFER_OFFSET_GEP1]]
; CHECK: [[FOO_A_BUFFER1:%[^ ]+]] = getelementptr i8, ptr %taskt.withprivates, i64 [[FOO_A_BUFFER_OFFSET1]]
; CHECK: store ptr [[FOO_A_BUFFER1]], ptr [[FOO_A_PRIV3]]

  %"foo_$A_$field0$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, ptr %"foo_$A", i32 0, i32 0
  %"foo_$A_$field0$5" = load ptr, ptr %"foo_$A_$field0$"
  store i16 1, ptr %"foo_$A_$field0$5"
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TASK"() ]

  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
; end INTEL_CUSTOMIZATION
