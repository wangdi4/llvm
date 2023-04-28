; RUN: opt -passes=sycl-kernel-postvec %s -S | FileCheck %s
; RUN: opt -passes=sycl-kernel-postvec %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; CHECK: define void @__omp_offloading_804_2303bd4__Z4main_l30()
; CHECK-NOT: !vectorized_width

define void @__omp_offloading_804_2303bd4__Z4main_l30() !recommended_vector_length !1 !sg_emu_size !2 {
entry:
  ret void
}

!sycl.kernels = !{!0}

!0 = !{ptr @__omp_offloading_804_2303bd4__Z4main_l30}
!1 = !{i32 1}
!2 = !{i32 16}

; DEBUGIFY-NOT: WARNING
