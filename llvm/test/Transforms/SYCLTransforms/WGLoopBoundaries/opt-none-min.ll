; RUN: opt -passes="sycl-kernel-analysis,sycl-kernel-wg-loop-bound" %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes="sycl-kernel-analysis,sycl-kernel-wg-loop-bound" %s -S -debug -disable-output 2>&1 | FileCheck %s

; CHECK-NOT: found {{.*}} early exit boundaries
; CHECK-NOT: found {{.*}} uniform early exit conditions

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S32"
target triple = "i686-pc-win32"

define void @min_kernel(ptr addrspace(1) nocapture %dst, i32 %width, i32 %height) noinline nounwind optnone {
  %id0 = tail call i32 @_Z13get_global_idj(i32 0) nounwind readnone
  %id1 = tail call i32 @_Z13get_global_idj(i32 1) nounwind readnone
  %add0 = add nsw i32 %width, -1
  %min0 = tail call i32 @_Z3minii(i32 %id0, i32 %add0) nounwind readnone
  %add1 = add nsw i32 %height, -1
  %min1 = tail call i32 @_Z3minjj(i32 %id1, i32 %add1) nounwind readnone
  %mul = mul nsw i32 %min1, %width
  %ind = add nsw i32 %mul, %min0
  %ptr = getelementptr inbounds <4 x i8>, ptr addrspace(1) %dst, i32 %ind
  store <4 x i8> <i8 100, i8 100, i8 100, i8 100>, ptr addrspace(1) %ptr, align 4
  ret void
}

declare i32 @_Z13get_global_idj(i32) nounwind readnone

declare i32 @_Z3minii(i32, i32) nounwind readnone

declare i32 @_Z3minjj(i32, i32) nounwind readnone

!sycl.kernels = !{!0}

!0 = !{ptr @min_kernel}

; DEBUGIFY-NOT: WARNING
