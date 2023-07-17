; RUN: opt %s -sycl-enable-direct-function-call-vectorization -sycl-enable-direct-subgroup-function-call-vectorization -sycl-enable-byval-byref-function-call-vectorization -passes=sycl-kernel-sg-size-collector -S | FileCheck %s
; RUN: opt %s -passes=sycl-kernel-sg-size-collector -S | FileCheck %s -check-prefix CHECK-NO-FLAG
; RUN: opt %s -enable-debugify -disable-output -sycl-enable-direct-function-call-vectorization -sycl-enable-direct-subgroup-function-call-vectorization -sycl-enable-byval-byref-function-call-vectorization -passes=sycl-kernel-sg-size-collector -S 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt %s -enable-debugify -disable-output -passes=sycl-kernel-sg-size-collector -S 2>&1 | FileCheck %s -check-prefix=DEBUGIFY

%struct.A = type { float, i32, double, i64 }

define void @bar(ptr byval(%struct.A) align 8 %arg) #0 !kernel_arg_base_type !3 !arg_type_null_val !4 {
; CHECK: define void @bar(ptr byval(%struct.A) align 8 %arg) #[[ATTR0:[0-9]+]]
entry:
  call void @foo(ptr nonnull byval(%struct.A) align 8 %arg)
  ret void
}

define void @foo(ptr byref(%struct.A) align 8 %arg) !kernel_arg_base_type !3 !arg_type_null_val !4 {
; CHECK: define void @foo(ptr byref(%struct.A) align 8 %arg) #[[ATTR1:[0-9]+]]
entry:
  call void @bar(ptr nonnull byval(%struct.A) align 8 %arg)
  ret void
}

define void @kernel(ptr nocapture readonly %arr) !recommended_vector_length !0 !kernel_arg_base_type !5 !arg_type_null_val !6 {
entry:
  call void @foo(ptr nonnull byval(%struct.A) align 8 %arr)
  ret void
}

define void @kernel2(ptr nocapture readonly %arr) !recommended_vector_length !1 !kernel_arg_base_type !5 !arg_type_null_val !6 {
entry:
  call void @bar(ptr nonnull byval(%struct.A) align 8 %arr)
  ret void
}

attributes #0 = { "vector-variants"="_ZGVeM16v_bar" }

; CHECK: attributes #[[ATTR0]] = { "vector-variants"="_ZGVeM16v_bar,_ZGVbM8v_bar,_ZGVbN8v_bar" }
; CHECK: attributes #[[ATTR1]] = { "vector-variants"="_ZGVbM8v_foo,_ZGVbN8v_foo,_ZGVbM16v_foo,_ZGVbN16v_foo" }
; CHECK-NOT: vector-variants
; CHECK-NO-FLAG: attributes #0 = { "vector-variants"="_ZGVeM16v_bar" }
; CHECK-NO-FLAG-NOT: vector-variants

!sycl.kernels = !{!2}

!0 = !{i32 8}
!1 = !{i32 16}
!2 = !{ptr @kernel, ptr @kernel2}
!3 = !{!"%struct.A"}
!4 = !{ptr null}
!5 = !{!"%struct.A*"}
!6 = !{ptr null}

; DEBUGIFY-NOT: WARNING
