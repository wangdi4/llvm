; RUN: opt %s -sycl-enable-direct-function-call-vectorization -passes="sycl-kernel-set-vf,sycl-kernel-vector-variant-lowering,sycl-kernel-sg-size-collector,sycl-kernel-sg-size-collector-indirect,sycl-kernel-vec-clone,sycl-kernel-vector-variant-fillin,sycl-kernel-update-call-attrs" -S | FileCheck %s

; Kernels with calls to indirect functions with byval cannot be vectorized.
; Check to make sure there are no "vector-variants" for the kernel and indirect call.

%"class.sycl::_V1::vec" = type { <2 x i8> }

define void @kernel(ptr byval(%"class.sycl::_V1::vec") %arg) !kernel_has_sub_groups !0 {
entry:
  call void @__intel_indirect_call.6(ptr %arg)
  ret void
}

; CHECK-NOT: "vector-variants"={{.*kernel}}
; CHECK-NOT: "vector-variants"={{.*__intel_indirect_call_XXX}}

!sycl.kernels = !{!1}

!0 = !{i1 true}
!1 = !{void (%"class.sycl::_V1::vec"*)* @kernel}

declare void @__intel_indirect_call.6(ptr byval(%"class.sycl::_V1::vec"))
