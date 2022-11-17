; RUN: opt -dpcpp-uniform-wg-size -dpcpp-kernel-add-implicit-args -debugify -dpcpp-kernel-resolve-wi-call -check-debugify -S %s -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -dpcpp-uniform-wg-size -dpcpp-kernel-add-implicit-args -dpcpp-kernel-resolve-wi-call -S %s | FileCheck %s
; RUN: opt -dpcpp-uniform-wg-size -passes='dpcpp-kernel-add-implicit-args,debugify,dpcpp-kernel-resolve-wi-call,check-debugify' -S %s -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -dpcpp-uniform-wg-size -passes='dpcpp-kernel-add-implicit-args,dpcpp-kernel-resolve-wi-call' -S %s | FileCheck %s

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define i64 @GLS0() {
; CHECK:      @GLS0
; CHECK:      [[GEP:%[a-zA-Z0-9]+]]             = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }* %pWorkDim, i32 0, i32 8, i32 0, i32 0
; CHECK-NEXT: [[LocalSize:%[a-zA-Z0-9_]+]]      = load i64, i64* [[GEP]]
; CHECK-NEXT: ret i64 [[LocalSize]]
  %c = tail call i64 @_Z14get_local_sizej(i32 0) nounwind
  ret i64 %c
}
define i64 @GLS1() {
; CHECK:      @GLS1
; CHECK:      [[GEP:%[a-zA-Z0-9]+]]             = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }* %pWorkDim, i32 0, i32 8, i32 0, i32 1
; CHECK-NEXT: [[LocalSize:%[a-zA-Z0-9_]+]]      = load i64, i64* [[GEP]]
; CHECK-NEXT: ret i64 [[LocalSize]]
  %c = tail call i64 @_Z14get_local_sizej(i32 1) nounwind
  ret i64 %c
}
define i64 @GLS2() {
; CHECK:      @GLS2
; CHECK:      [[GEP:%[a-zA-Z0-9]+]]             = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }* %pWorkDim, i32 0, i32 8, i32 0, i32 2
; CHECK-NEXT: [[LocalSize:%[a-zA-Z0-9_]+]]      = load i64, i64* [[GEP]]
; CHECK-NEXT: ret i64 [[LocalSize]]
  %c = tail call i64 @_Z14get_local_sizej(i32 2) nounwind
  ret i64 %c
}
define i64 @GLSX(i32 %x) {
; CHECK:      @GLSX
; CHECK:      [[GEP:%[a-zA-Z0-9]+]]             = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }* %pWorkDim, i32 0, i32 8, i32 0, i32 %x 
; CHECK-NEXT: [[LocalSize:%[a-zA-Z0-9_]+]]      = load i64, i64* [[GEP]]
  %c = tail call i64 @_Z14get_local_sizej(i32 %x) nounwind
  ret i64 %c
}

declare i64 @_Z14get_local_sizej(i32)

!opencl.ocl.version = !{!0}
!0 = !{i32 2, i32 0}

; DEBUGIFY-NOT: WARNING
