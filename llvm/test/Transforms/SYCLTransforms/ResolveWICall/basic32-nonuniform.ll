; RUN: opt -passes='sycl-kernel-add-implicit-args,debugify,sycl-kernel-resolve-wi-call,check-debugify' -S %s -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes='sycl-kernel-add-implicit-args,sycl-kernel-resolve-wi-call' -S %s | FileCheck %s
; TODO: add checks...
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i32:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S32"
target triple = "i386-pc-linux"

define i32 @GLS0() {
; CHECK:      @GLS0
; CHECK:      [[GropuIDPlusOne:%[a-zA-Z0-9_]+]] = add nsw i32 [[GropuID:%[a-zA-Z0-9_]+]], 1
; CHECK-NEXT: [[IsLastGroup:%[a-zA-Z0-9_]+]]    = icmp eq i32 [[NumGroups:%[a-zA-Z0-9_]+]], [[GropuIDPlusOne]]
; CHECK-NEXT: [[LocalSizeIdx:%[a-zA-Z0-9_]+]]   = zext i1 [[IsLastGroup]] to i32
; CHECK-NEXT: [[GEP:%[a-zA-Z0-9]+]]             = getelementptr { i32, [3 x i32], [3 x i32], [2 x [3 x i32]], [3 x i32], ptr, ptr, [3 x i32], [2 x [3 x i32]], [3 x i32] }, ptr %pWorkDim, i32 0, i32 8, i32 [[LocalSizeIdx]], i32 0
; CHECK:      [[LocalSize:%[a-zA-Z0-9_]+]]       = load i32, ptr [[GEP]]
; CHECK:      ret i32 [[LocalSize]]
  %c = tail call i32 @_Z14get_local_sizej(i32 0) nounwind
  ret i32 %c
}
define i32 @GLS1() {
; CHECK:      @GLS1
; CHECK:      [[GropuIDPlusOne:%[a-zA-Z0-9_]+]] = add nsw i32 [[GropuID:%[a-zA-Z0-9_]+]], 1
; CHECK-NEXT: [[IsLastGroup:%[a-zA-Z0-9_]+]]    = icmp eq i32 [[NumGroups:%[a-zA-Z0-9_]+]], [[GropuIDPlusOne]]
; CHECK-NEXT: [[LocalSizeIdx:%[a-zA-Z0-9_]+]]   = zext i1 [[IsLastGroup]] to i32
; CHECK-NEXT: [[GEP:%[a-zA-Z0-9]+]]             = getelementptr { i32, [3 x i32], [3 x i32], [2 x [3 x i32]], [3 x i32], ptr, ptr, [3 x i32], [2 x [3 x i32]], [3 x i32] }, ptr %pWorkDim, i32 0, i32 8, i32 [[LocalSizeIdx]], i32 1
; CHECK:      [[LocalSize:%[a-zA-Z0-9_]+]]       = load i32, ptr [[GEP]]
; CHECK:      ret i32 [[LocalSize]]
  %c = tail call i32 @_Z14get_local_sizej(i32 1) nounwind
  ret i32 %c
}
define i32 @GLS2() {
; CHECK:      @GLS2
; CHECK:      [[GropuIDPlusOne:%[a-zA-Z0-9_]+]] = add nsw i32 [[GropuID:%[a-zA-Z0-9_]+]], 1
; CHECK-NEXT: [[IsLastGroup:%[a-zA-Z0-9_]+]]    = icmp eq i32 [[NumGroups:%[a-zA-Z0-9_]+]], [[GropuIDPlusOne]]
; CHECK-NEXT: [[LocalSizeIdx:%[a-zA-Z0-9_]+]]   = zext i1 [[IsLastGroup]] to i32
; CHECK-NEXT: [[GEP:%[a-zA-Z0-9]+]]             = getelementptr { i32, [3 x i32], [3 x i32], [2 x [3 x i32]], [3 x i32], ptr, ptr, [3 x i32], [2 x [3 x i32]], [3 x i32] }, ptr %pWorkDim, i32 0, i32 8, i32 [[LocalSizeIdx]], i32 2
; CHECK:      [[LocalSize:%[a-zA-Z0-9_]+]]       = load i32, ptr [[GEP]]
; CHECK:      ret i32 [[LocalSize]]
  %c = tail call i32 @_Z14get_local_sizej(i32 2) nounwind
  ret i32 %c
}
define i32 @GLSX(i32 %x) {
; CHECK:      @GLSX
; CHECK:      [[GropuIDPlusOne:%[a-zA-Z0-9_]+]] = add nsw i32 [[GropuID:%[a-zA-Z0-9_]+]], 1
; CHECK-NEXT: [[IsLastGroup:%[a-zA-Z0-9_]+]]    = icmp eq i32 [[NumGroups:%[a-zA-Z0-9_]+]], [[GropuIDPlusOne]]
; CHECK-NEXT: [[LocalSizeIdx:%[a-zA-Z0-9_]+]]   = zext i1 [[IsLastGroup]] to i32
; CHECK-NEXT: [[GEP:%[a-zA-Z0-9]+]]             = getelementptr { i32, [3 x i32], [3 x i32], [2 x [3 x i32]], [3 x i32], ptr, ptr, [3 x i32], [2 x [3 x i32]], [3 x i32] }, ptr %pWorkDim, i32 0, i32 8, i32 [[LocalSizeIdx]], i32 %x
; CHECK:      [[LocalSize:%[a-zA-Z0-9_]+]]       = load i32, ptr [[GEP]]
  %c = tail call i32 @_Z14get_local_sizej(i32 %x) nounwind
  ret i32 %c
}

define i32 @GELS0() {
; CHECK: @GELS0
; CHECK: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i32, [3 x i32], [3 x i32], [2 x [3 x i32]], [3 x i32], ptr, ptr, [3 x i32], [2 x [3 x i32]], [3 x i32] }, ptr %pWorkDim, i32 0, i32 8, i32 0, i32 0
; CHECK: %InternalLocalSize_0 = load i32, ptr [[GEP]]
; CHECK: ret i32 %InternalLocalSize_0
  %c = tail call i32 @_Z23get_enqueued_local_sizej(i32 0) nounwind
  ret i32 %c
}
define i32 @GELS1() {
; CHECK: @GELS1
; CHECK: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i32, [3 x i32], [3 x i32], [2 x [3 x i32]], [3 x i32], ptr, ptr, [3 x i32], [2 x [3 x i32]], [3 x i32] }, ptr %pWorkDim, i32 0, i32 8, i32 0, i32 1
; CHECK: %InternalLocalSize_1 = load i32, ptr [[GEP]]
; CHECK: ret i32 %InternalLocalSize_1
  %c = tail call i32 @_Z23get_enqueued_local_sizej(i32 1) nounwind
  ret i32 %c
}
define i32 @GELS2() {
; CHECK: @GELS2
; CHECK: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i32, [3 x i32], [3 x i32], [2 x [3 x i32]], [3 x i32], ptr, ptr, [3 x i32], [2 x [3 x i32]], [3 x i32] }, ptr %pWorkDim, i32 0, i32 8, i32 0, i32 2
; CHECK: %InternalLocalSize_2 = load i32, ptr [[GEP]]
; CHECK: ret i32 %InternalLocalSize_2
  %c = tail call i32 @_Z23get_enqueued_local_sizej(i32 2) nounwind
  ret i32 %c
}
define i32 @GELSX(i32 %x) {
; CHECK: @GELSX
; CHECK: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i32, [3 x i32], [3 x i32], [2 x [3 x i32]], [3 x i32], ptr, ptr, [3 x i32], [2 x [3 x i32]], [3 x i32] }, ptr %pWorkDim, i32 0, i32 8, i32 0, i32 %x
; CHECK: %InternalLocalSize_var = load i32, ptr [[GEP]]
  %c = tail call i32 @_Z23get_enqueued_local_sizej(i32 %x) nounwind
  ret i32 %c
}

declare i32 @_Z14get_local_sizej(i32)
declare i32 @_Z23get_enqueued_local_sizej(i32)

!opencl.ocl.version = !{!0}
!0 = !{i32 2, i32 0}

; DEBUGIFY-NOT: WARNING
