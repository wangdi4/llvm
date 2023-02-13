; RUN: opt -passes=sycl-kernel-coerce-types -mtriple x86_64-pc-linux -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-coerce-types -mtriple x86_64-pc-linux -S %s -o - | FileCheck %s
; RUN: opt -passes=sycl-kernel-coerce-win64-types -mtriple x86_64-w64-mingw32 -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-coerce-win64-types -mtriple x86_64-w64-mingw32 -S %s -o - | FileCheck %s
; RUN: opt -passes=sycl-kernel-coerce-types -opaque-pointers -mtriple x86_64-pc-linux -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-coerce-types -opaque-pointers -mtriple x86_64-pc-linux -S %s -o - | FileCheck %s --check-prefix=OPAQUE
; RUN: opt -passes=sycl-kernel-coerce-win64-types -opaque-pointers -mtriple x86_64-w64-mingw32 -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-coerce-win64-types -opaque-pointers -mtriple x86_64-w64-mingw32 -S %s -o - | FileCheck %s --check-prefix=OPAQUE

; The test is to check the function pointer with byval argument is not
; transformed by CoerceTypes/CoerceWin64Types pass. The test may be used when
; function pointer support is enabled.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

%class.FixedVect = type { [2 x float] }
%"class.sycl::_V1::ext::intel::experimental::task_sequence" = type { i32, i64 }

declare void @_Z14function1ArrayIN4sycl3_V13ext5intel4pipeI8my_pipe2fLi12EEEEv9FixedVectIfLi2EE(%class.FixedVect* byval(%class.FixedVect))

; CHECK: declare void @_Z14function1ArrayIN4sycl3_V13ext5intel4pipeI8my_pipe2fLi12EEEEv9FixedVectIfLi2EE(%class.FixedVect* byval(%class.FixedVect))
; OPAQUE: declare void @_Z14function1ArrayIN4sycl3_V13ext5intel4pipeI8my_pipe2fLi12EEEEv9FixedVectIfLi2EE(ptr byval(%class.FixedVect))

define linkonce_odr void @_ZN4sycl3_V13ext5intel12experimental13task_sequenceIL_Z14function1ArrayINS2_4pipeI8my_pipe2fLi12EEEEv9FixedVectIfLi2EEELj12ELj12EEC2Ev() {
entry:
  %call = call i64 @"_Z31__spirv_TaskSequenceCreateINTELPU3AS456class.sycl::_V1::ext::intel::experimental::task_sequenceU13block_pointerFvvEi"(%"class.sycl::_V1::ext::intel::experimental::task_sequence" addrspace(4)* null, void (%class.FixedVect*)* @_Z14function1ArrayIN4sycl3_V13ext5intel4pipeI8my_pipe2fLi12EEEEv9FixedVectIfLi2EE, i32 0)
  ret void
}

declare i64 @"_Z31__spirv_TaskSequenceCreateINTELPU3AS456class.sycl::_V1::ext::intel::experimental::task_sequenceU13block_pointerFvvEi"(%"class.sycl::_V1::ext::intel::experimental::task_sequence" addrspace(4)*, void (%class.FixedVect*)*, i32)

; DEBUGIFY-NOT: WARNING
