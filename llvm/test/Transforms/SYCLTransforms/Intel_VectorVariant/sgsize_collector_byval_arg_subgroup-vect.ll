; RUN: opt %s -passes=sycl-kernel-sg-size-collector -sycl-enable-direct-subgroup-function-call-vectorization -sycl-enable-direct-function-call-vectorization -S | FileCheck %s
; RUN: opt %s -enable-debugify -disable-output -passes=sycl-kernel-sg-size-collector -sycl-enable-direct-subgroup-function-call-vectorization -sycl-enable-direct-function-call-vectorization -S 2>&1 | FileCheck %s -check-prefix=DEBUGIFY

;; Have a has-sub-group function calling byref function.
;; We can vectorize with serializartion of byval.

%struct.A = type { float, i32, double, i64 }

define void @f1(ptr byval(%struct.A) align 8 %arg) !kernel_arg_base_type !2 !arg_type_null_val !3 {
entry:
  ret void
}

define void @f2(ptr nocapture readonly %arr) !kernel_arg_base_type !4 !arg_type_null_val !5 {
  call void @f1(ptr nonnull byval(%struct.A) align 8 %arr)
  ret void
}

define void @f3(ptr nocapture readonly %arr) #0 !kernel_arg_base_type !4 !arg_type_null_val !5 {
  call void @f2(ptr nocapture readonly %arr)
  ret void
}

define void @kernel(ptr nocapture readonly %arr) #0 !recommended_vector_length !0 !kernel_arg_base_type !4 !arg_type_null_val !5 {
entry:
  call void @f3(ptr nocapture readonly %arr)
  ret void
}

attributes #0 = { "has-sub-groups" }

;; f1 does not have a vector variant.
; CHECK: attributes #0 = { "vector-variants"="_ZGVbM8v_f2,_ZGVbN8v_f2" }
; CHECK: attributes #1 = { "has-sub-groups" "vector-variants"="_ZGVbM8v_f3,_ZGVbN8v_f3" }
; CHECK: attributes #2 = { "has-sub-groups" }
; CHECK-NOT: attributes

!sycl.kernels = !{!1}

!0 = !{i32 8}
!1 = !{ptr @kernel}
!2 = !{!"%struct.A"}
!3 = !{ptr null}
!4 = !{!"%struct.A*"}
!5 = !{ptr null}

; DEBUGIFY-NOT: WARNING
