; INTEL_CUSTOMIZATION
; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
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
;                !$omp taskloop lastprivate(a) firstprivate(a)
;                do i = 1,1
;                  a(1,1,1) = a(1,1,1) + 1
; !                 call bar(a)
;                end do
;                !$omp end taskloop
;            end subroutine
; !           subroutine bar(a)
; !               integer(kind=2), dimension(:, :, :) :: a
; !               print *, a
; !           end subroutine
; !
; !       end program
; Check for the space allocated for the private copy. First field is for the
; dope vector struct, followed by two i64s for array size and thunk buffer offset.
; CHECK: %__struct.kmp_privates.t = type { { i16*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, i64, i64, i32, i32, i32 }

; There should be one pointer to the dv struct in the shared struct for
; lastprivate copyout.
; CHECK: %__struct.shared.t = type { { i16*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }* }

; ModuleID = 'Intel_fort_dv_taskloop_fp_lp.ll'
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

; Check for call to f90_dv_size to get array size in bytes.
; CHECK: [[FOO_A_CAST:%[^ ]+]] = bitcast { i16*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }* %"foo_$A" to i8*
; CHECK: [[ARR_SIZE_IN_BYTES:%[^ ]+]] = call i64 @_f90_dope_vector_size(i8* [[FOO_A_CAST]])

; Check that we call _task_alloc with total size of task_t_with_privates + arr_size
; CHECK: [[TOTAL_SIZE:%[^ ]+]] = add i64 224, [[ARR_SIZE_IN_BYTES]]
; CHECK: [[TASK_ALLOC:[^ ]+]] = call i8* @__kmpc_omp_task_alloc({{.*}}i64 [[TOTAL_SIZE]]{{.*}})

; Check that F90 DV's array size and offset are stored in the thunk
; CHECK: [[ARR_SIZE_GEP:%[^ ]+]] = getelementptr inbounds %__struct.kmp_privates.t, %__struct.kmp_privates.t* %{{[^ ]+}}, i32 0, i32 1
; CHECK: store i64 [[ARR_SIZE_IN_BYTES]], i64* [[ARR_SIZE_GEP]]
; CHECK: [[ARR_OFFSET_GEP:%[^ ]+]] = getelementptr inbounds %__struct.kmp_privates.t, %__struct.kmp_privates.t* %{{[^ ]+}}, i32 0, i32 2
; CHECK: store i64 224, i64* [[ARR_OFFSET_GEP]]

; Check that the new DV is initialized using the original DV
; CHECK: [[FOO_A_PRIV:%[^ ]+]] = getelementptr inbounds %__struct.kmp_privates.t, %__struct.kmp_privates.t* %.privates, i32 0, i32 0
; CHECK: [[FOO_A_PRIV_CAST:%[^ ]+]] = bitcast { i16*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }* [[FOO_A_PRIV]] to i8*
; CHECK: [[FOO_A_CAST:%[^ ]+]] = bitcast { i16*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }* %"foo_$A" to i8*
; CHECK: %.dv.init = call i64 @_f90_dope_vector_init(i8* [[FOO_A_PRIV_CAST]], i8* [[FOO_A_CAST]])

; Check that the local buffer space for the local dope vector, is initialized (as it is firstprivate).
; First, the buffer should be linked to the dope vector's base field
; CHECK: [[FOO_A_PRIV2:%[^ ]+]] = getelementptr inbounds %__struct.kmp_privates.t, %__struct.kmp_privates.t* {{[^,]+}}, i32 0, i32 0
; CHECK: [[FOO_A_PRIV1:%[^ ]+]] = getelementptr inbounds %__struct.kmp_privates.t, %__struct.kmp_privates.t* {{[^,]+}}, i32 0, i32 0
; CHECK: [[FOO_A_BUFFER_OFFSET_GEP:%[^ ]+]] = getelementptr inbounds %__struct.kmp_privates.t, %__struct.kmp_privates.t* {{[^,]+}}, i32 0, i32 2
; CHECK: [[FOO_A_BUFFER_OFFSET:%[^ ]+]] = load i64, i64* [[FOO_A_BUFFER_OFFSET_GEP]]
; CHECK: [[FOO_A_BUFFER:%[^ ]+]] = getelementptr i8, i8* [[TASK_ALLOC]], i64 [[FOO_A_BUFFER_OFFSET]]
; CHECK: [[FOO_A_PRIV1_CAST:%[^ ]+]] = bitcast { i16*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }* [[FOO_A_PRIV1]] to i8**
; CHECK: store i8* [[FOO_A_BUFFER]], i8** [[FOO_A_PRIV1_CAST]]

; After the linking is done, there should be a call to f90_firstprivate_copy
; CHECK: [[FOO_A_PRIV2_CAST:%[^ ]+]] = bitcast { i16*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }* [[FOO_A_PRIV2]] to i8*
; CHECK: [[FOO_A_CAST1:%[^ ]+]] = bitcast { i16*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }* %"foo_$A" to i8*
; CHECK: call void @_f90_firstprivate_copy(i8* [[FOO_A_PRIV2_CAST]], i8* [[FOO_A_CAST1]])

  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKLOOP"(), "QUAL.OMP.PRIVATE"(i32* %"foo_$I"), "QUAL.OMP.LASTPRIVATE:F90_DV"({ i16*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }* %"foo_$A"), "QUAL.OMP.FIRSTPRIVATE:F90_DV"({ i16*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }* %"foo_$A"), "QUAL.OMP.NORMALIZED.IV"(i32* %temp2), "QUAL.OMP.NORMALIZED.UB"(i32* %temp4), "QUAL.OMP.FIRSTPRIVATE"(i32* %temp) ]

; CHECK: define internal void @{{.*}}DIR.OMP.TASK{{.*}}

; Inside the outlined function, check that the buffer for the array is linked to the base field of the dope vector
; CHECK: [[FOO_A_PRIV3:%[^ ]+]] = getelementptr inbounds %__struct.kmp_privates.t, %__struct.kmp_privates.t* {{[^,]+}}, i32 0, i32 0
; CHECK: [[FOO_A_BUFFER_OFFSET_GEP1:%[^ ]+]] = getelementptr inbounds %__struct.kmp_privates.t, %__struct.kmp_privates.t* {{[^,]+}}, i32 0, i32 2
; CHECK: [[FOO_A_BUFFER_OFFSET1:%[^ ]+]] = load i64, i64* [[FOO_A_BUFFER_OFFSET_GEP1]]
; CHECK: [[THUNK_BASE_PTR:%[^ ]+]] = bitcast %__struct.kmp_task_t_with_privates* {{[^, ]+}} to i8*
; CHECK: [[FOO_A_BUFFER1:%[^ ]+]] = getelementptr i8, i8* [[THUNK_BASE_PTR]], i64 [[FOO_A_BUFFER_OFFSET1]]
; CHECK: [[FOO_A_PRIV3_CAST:%[^ ]+]] = bitcast { i16*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }* [[FOO_A_PRIV3]] to i8**
; CHECK: store i8* [[FOO_A_BUFFER1]], i8** [[FOO_A_PRIV3_CAST]]


; Check for a call to the lastprivate copyout code
; CHECK: [[FOO_A_PRIV4:%[^ ]+]] = getelementptr inbounds %__struct.kmp_privates.t, %__struct.kmp_privates.t* {{[^,]+}}, i32 0, i32 0
; CHECK: [[FOO_A_SHR_GEP:[^ ]+]] = getelementptr inbounds %__struct.shared.t, %__struct.shared.t* %{{[^ ]+}}, i32 0, i32 0
; CHECK: [[FOO_A_SHR:%[^ ]+]] = load { i16*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }*, { i16*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }** [[FOO_A_SHR_GEP]]
; CHECK: [[FOO_A_PRIV4_CAST:[^ ]+]] = bitcast { i16*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }* [[FOO_A_PRIV4]] to i8*
; CHECK: [[FOO_A_SHR_CAST:[^ ]+]] = bitcast { i16*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }* [[FOO_A_SHR]] to i8*
; CHECK: call void @_f90_lastprivate_copy(i8* [[FOO_A_PRIV4_CAST]], i8* [[FOO_A_SHR_CAST]])

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
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TASKLOOP"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { nounwind }
; end INTEL_CUSTOMIZATION
