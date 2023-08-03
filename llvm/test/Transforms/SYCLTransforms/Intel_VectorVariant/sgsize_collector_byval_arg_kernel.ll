; RUN: opt %s -sycl-enable-byval-byref-function-call-vectorization -passes=sycl-kernel-sg-size-collector -S | FileCheck %s
; RUN: opt %s -passes=sycl-kernel-sg-size-collector -S | FileCheck %s
; RUN: opt %s -enable-debugify -disable-output -sycl-enable-byval-byref-function-call-vectorization -passes=sycl-kernel-sg-size-collector -S 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt %s -enable-debugify -disable-output -passes=sycl-kernel-sg-size-collector -S 2>&1 | FileCheck %s -check-prefix=DEBUGIFY

%struct.A = type { float, i32, double, i64 }

define void @foo(ptr align 8 %arg) !kernel_arg_base_type !2 !arg_type_null_val !3 {
; CHECK: define void @foo(ptr align 8 %arg) #[[#ATTR:]]
entry:
  ret void
}

define void @kernel(ptr byval(%struct.A) %arr) !recommended_vector_length !0 !kernel_arg_base_type !4 !arg_type_null_val !5 {
entry:
  call void @foo(ptr align 8 %arr)
  ret void
}

; CHECK: attributes #[[#ATTR]] = { "vector-variants"="_ZGVbM8v_foo,_ZGVbN8v_foo" }

!sycl.kernels = !{!1}

!0 = !{i32 8}
!1 = !{ptr @kernel}
!2 = !{!"%struct.A*"}
!3 = !{ptr null}
!4 = !{!"%struct.A"}
!5 = !{ptr null}

; DEBUGIFY-NOT: WARNING
