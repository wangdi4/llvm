; RUN: opt %s -passes=sycl-kernel-sg-size-collector -S | FileCheck %s
; RUN: opt %s -enable-debugify -disable-output -passes=sycl-kernel-sg-size-collector -S 2>&1 | FileCheck %s -check-prefix=DEBUGIFY

; CHECK-NOT: vector-variants

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

%"class.sycl::_V1::nd_item" = type { %"class.sycl::_V1::item", %"class.sycl::_V1::item.83", %"class.sycl::_V1::group" }
%"class.sycl::_V1::item" = type { %"class.sycl::_V1::detail::AccessorImplDevice" }
%"class.sycl::_V1::detail::AccessorImplDevice" = type { %"class.sycl::_V1::range", %"class.sycl::_V1::range", %"class.sycl::_V1::range" }
%"class.sycl::_V1::range" = type { %"class.sycl::_V1::detail::array" }
%"class.sycl::_V1::detail::array" = type { [1 x i64] }
%"class.sycl::_V1::item.83" = type { %"struct.sycl::_V1::detail::ItemBase.84" }
%"struct.sycl::_V1::detail::ItemBase.84" = type { %"class.sycl::_V1::range", %"class.sycl::_V1::range" }
%"class.sycl::_V1::group" = type { %"class.sycl::_V1::range", %"class.sycl::_V1::range", %"class.sycl::_V1::range", %"class.sycl::_V1::range" }

; Function Attrs: mustprogress noinline nounwind willreturn
define internal void @foo(ptr addrspace(4) nocapture writeonly sret(%"class.sycl::_V1::nd_item") %agg.result) #0 {
entry:
  store i32 0, ptr addrspace(4) %agg.result, align 4
  ret void
}

; Function Attrs: mustprogress nounwind willreturn
define void @_ZTS6kernelILi4300364EE(ptr noalias nocapture writeonly %agg) #1 !no_barrier_path !3 !kernel_has_sub_groups !4 !recommended_vector_length !5 {
entry:
  %agg.ascast = addrspacecast ptr %agg to ptr addrspace(4)
  call void @foo(ptr addrspace(4) %agg.ascast)
  ret void
}

attributes #0 = { mustprogress noinline nounwind willreturn "prefer-vector-width"="512" }
attributes #1 = { mustprogress nounwind willreturn "prefer-vector-width"="512" }

!spirv.Source = !{!0}
!opencl.ocl.version = !{!1}
!sycl.kernels = !{!2}

!0 = !{i32 4, i32 100000}
!1 = !{i32 2, i32 0}
!2 = !{ptr @_ZTS6kernelILi4300364EE}
!3 = !{i1 true}
!4 = !{i1 false}
!5 = !{i32 16}

; DEBUGIFY-NOT: WARNING
