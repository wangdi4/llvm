; RUN: opt -passes=sycl-kernel-resolve-var-tid-call -S < %s | FileCheck %s
; RUN: opt -passes=sycl-kernel-resolve-var-tid-call -enable-debugify -disable-output 2>&1 -S < %s | FileCheck %s -check-prefix=DEBUGIFY

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

declare i64 @_Z13get_global_idj(i32)

; Function Attrs: convergent nounwind
define void @testKernel(i32 %a, ptr %b) !kernel_arg_base_type !4 !arg_type_null_val !5 {
entry:
  %call = call i64 @_Z13get_global_idj(i32 %a)
; CHECK:      [[GID0:%.*]] = call i64 @_Z13get_global_idj(i32 0)
; CHECK-NEXT: [[GID1:%.*]] = call i64 @_Z13get_global_idj(i32 1)
; CHECK-NEXT: [[GID2:%.*]] = call i64 @_Z13get_global_idj(i32 2)
; CHECK-NEXT: [[CMP0:%.*]] = icmp eq i32 %a, 0
; CHECK-NEXT: [[SEL0:%.*]] = select i1 [[CMP0]], i64 [[GID0]], i64 0
; CHECK-NEXT: [[CMP1:%.*]] = icmp eq i32 %a, 1
; CHECK-NEXT: [[SEL1:%.*]] = select i1 [[CMP1]], i64 [[GID1]], i64 [[SEL0]]
; CHECK-NEXT: [[CMP2:%.*]] = icmp eq i32 %a, 2
; CHECK-NEXT: [[CALL:%.*]] = select i1 [[CMP2]], i64 [[GID2]], i64 [[SEL1]]
  store i64 %call, ptr %b
; CHECK: store i64 [[CALL]], ptr %b
  ret void
}

!llvm.linker.options = !{}
!llvm.module.flags = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!1}
!sycl.kernels = !{!3}
!opencl.spir.version = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, i32 2}
!2 = !{}
!3 = !{ptr @testKernel}
!4 = !{!"int", !"long*"}
!5 = !{i32 0, ptr null}

; DEBUGIFY-NOT: WARNING
