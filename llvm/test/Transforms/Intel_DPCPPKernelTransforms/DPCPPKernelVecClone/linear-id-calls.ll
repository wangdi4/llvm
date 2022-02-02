; RUN: opt -dpcpp-kernel-vec-clone -dpcpp-vector-variant-isa-encoding-override=AVX512Core %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=dpcpp-kernel-vec-clone -dpcpp-vector-variant-isa-encoding-override=AVX512Core %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -dpcpp-kernel-vec-clone -dpcpp-vector-variant-isa-encoding-override=AVX512Core %s -S -o - | FileCheck %s
; RUN: opt -passes=dpcpp-kernel-vec-clone -dpcpp-vector-variant-isa-encoding-override=AVX512Core %s -S -o - | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

%"class.cl::sycl::range" = type { %"class.cl::sycl::detail::array" }
%"class.cl::sycl::detail::array" = type { [1 x i64] }

define void @"_ZTSZZ4mainENK3$_0clERN2cl4sycl7handlerEE4Test"(i32 addrspace(1)*, %"class.cl::sycl::range"* byval(%"class.cl::sycl::range"), %"class.cl::sycl::range"* byval(%"class.cl::sycl::range"), %"class.cl::sycl::range"* byval(%"class.cl::sycl::range"), i32 addrspace(1)*, %"class.cl::sycl::range"* byval(%"class.cl::sycl::range"), %"class.cl::sycl::range"* byval(%"class.cl::sycl::range"), %"class.cl::sycl::range"* byval(%"class.cl::sycl::range"), i32 addrspace(1)*, %"class.cl::sycl::range"* byval(%"class.cl::sycl::range"), %"class.cl::sycl::range"* byval(%"class.cl::sycl::range"), %"class.cl::sycl::range"* byval(%"class.cl::sycl::range"), i32 addrspace(1)*, %"class.cl::sycl::range"* byval(%"class.cl::sycl::range"), %"class.cl::sycl::range"* byval(%"class.cl::sycl::range"), %"class.cl::sycl::range"* byval(%"class.cl::sycl::range")) local_unnamed_addr #0 !kernel_arg_addr_space !8 !kernel_arg_access_qual !9 !kernel_arg_type !10 !kernel_arg_type_qual !11 !kernel_arg_base_type !12 !no_barrier_path !13 !kernel_has_sub_groups !13 !recommended_vector_length !14 {
; Remaining non-vectorized function
; CHECK-LABEL: _ZTSZZ4mainENK3$_0clERN2cl4sycl7handlerEE4Test

; Generated vector version
; CHECK-LABEL: _ZGVeN16uuuuuuuuuuuuuuuu__ZTSZZ4mainENK3$_0clERN2cl4sycl7handlerEE4Test

; Checks that we do not hit a label from the WRN region.
; CHECK-NOT: {{^[._a-zA-Z0-9]*}}:
; get_global_id call should be hoisted outside region!
; CHECK: %gid = call i64 @_Z13get_global_idj(i32 0) #1
; CHECK-NOT: %slid = call i32 @_Z22get_sub_group_local_idv() #1
; CHECK-NEXT: label %simd.begin.region

; CHECK-LABEL: simd.begin.region:
; CHECK-NEXT:    %entry.region = call token @llvm.directive.region.entry()
; CHECK-NEXT:    br label %simd.loop.preheader
; CHECK-EMPTY:

; CHECK-LABEL:  simd.loop.header:
; CHECK-NEXT:    [[INDEX:%.*]] = phi i32
; CHECK-NEXT:    [[INDEX_I64:%.*]] = sext i32 [[INDEX]] to i64
; CHECK-NEXT:    [[GID_LINEAR:%.*]] = add nuw i64 [[INDEX_I64]], %gid
; CHECK-NOT: call
; CHECK:         [[GID_LINEAR_I32:%.*]] = trunc i64 [[GID_LINEAR]] to i32
; CHECK-NEXT:    store i32 [[GID_LINEAR_I32]]
; CHECK-NEXT:    store i32 [[INDEX]]
; CHECK-LABEL: simd.end.region:
; CHECK-NEXT: call void @llvm.directive.region.exit(token %entry.region)
  %gid = call i64 @_Z13get_global_idj(i32 0) #1
  %gep1 = getelementptr inbounds i32, i32 addrspace(1)* %0, i64 %gid
  %ld = load i32, i32 addrspace(1)* %gep1, align 4
  %gid.trunc = trunc i64 %gid to i32
  store i32 %gid.trunc, i32 addrspace(1)* %gep1, align 4

  %slid = call i32 @_Z22get_sub_group_local_idv() #1
  store i32 %slid, i32 addrspace(1)* %gep1, align 4

  ret void
}

declare i64 @_Z13get_global_idj(i32) local_unnamed_addr #1

declare i32 @_Z22get_sub_group_local_idv() #1

attributes #0 = { nounwind }
attributes #1 = { nounwind readnone }

!opencl.enable.FP_CONTRACT = !{}
!spirv.Source = !{!0}
!opencl.spir.version = !{!1}
!opencl.ocl.version = !{!2}
!opencl.used.extensions = !{!3}
!opencl.used.optional.core.features = !{!4}
!spirv.Generator = !{!5}
!sycl.kernels = !{!6}
!opencl.gen_addr_space_pointer_counter = !{!7}

!0 = !{i32 4, i32 100000}
!1 = !{i32 1, i32 2}
!2 = !{i32 1, i32 0}
!3 = !{!"cl_khr_subgroups"}
!4 = !{}
!5 = !{i16 6, i16 14}
!6 = !{void (i32 addrspace(1)*, %"class.cl::sycl::range"*, %"class.cl::sycl::range"*, %"class.cl::sycl::range"*, i32 addrspace(1)*, %"class.cl::sycl::range"*, %"class.cl::sycl::range"*, %"class.cl::sycl::range"*, i32 addrspace(1)*, %"class.cl::sycl::range"*, %"class.cl::sycl::range"*, %"class.cl::sycl::range"*, i32 addrspace(1)*, %"class.cl::sycl::range"*, %"class.cl::sycl::range"*, %"class.cl::sycl::range"*)* @"_ZTSZZ4mainENK3$_0clERN2cl4sycl7handlerEE4Test"}
!7 = !{i32 0}
!8 = !{i32 1, i32 0, i32 0, i32 0, i32 1, i32 0, i32 0, i32 0, i32 1, i32 0, i32 0, i32 0, i32 1, i32 0, i32 0, i32 0}
!9 = !{!"none", !"none", !"none", !"none", !"none", !"none", !"none", !"none", !"none", !"none", !"none", !"none", !"none", !"none", !"none", !"none"}
!10 = !{!"int*", !"range<1>", !"range<1>", !"id<1>", !"int*", !"range<1>", !"range<1>", !"id<1>", !"int*", !"range<1>", !"range<1>", !"id<1>", !"int*", !"range<1>", !"range<1>", !"id<1>"}
!11 = !{!"", !"", !"", !"", !"", !"", !"", !"", !"", !"", !"", !"", !"", !"", !"", !""}
!12 = !{!"int*", !"class.cl::sycl::range", !"class.cl::sycl::range", !"class.cl::sycl::range", !"int*", !"class.cl::sycl::range", !"class.cl::sycl::range", !"class.cl::sycl::range", !"int*", !"class.cl::sycl::range", !"class.cl::sycl::range", !"class.cl::sycl::range", !"int*", !"class.cl::sycl::range", !"class.cl::sycl::range", !"class.cl::sycl::range"}
!13 = !{i1 true}
!14 = !{i32 16}

; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uuuuuuuuuuuuuuuu__ZTSZZ4mainENK3$_0clERN2cl4sycl7handlerEE4Test {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uuuuuuuuuuuuuuuu__ZTSZZ4mainENK3$_0clERN2cl4sycl7handlerEE4Test {{.*}} call
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uuuuuuuuuuuuuuuu__ZTSZZ4mainENK3$_0clERN2cl4sycl7handlerEE4Test {{.*}} add
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uuuuuuuuuuuuuuuu__ZTSZZ4mainENK3$_0clERN2cl4sycl7handlerEE4Test {{.*}} add
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uuuuuuuuuuuuuuuu__ZTSZZ4mainENK3$_0clERN2cl4sycl7handlerEE4Test {{.*}} icmp
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uuuuuuuuuuuuuuuu__ZTSZZ4mainENK3$_0clERN2cl4sycl7handlerEE4Test {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uuuuuuuuuuuuuuuu__ZTSZZ4mainENK3$_0clERN2cl4sycl7handlerEE4Test {{.*}} call
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uuuuuuuuuuuuuuuu__ZTSZZ4mainENK3$_0clERN2cl4sycl7handlerEE4Test {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16uuuuuuuuuuuuuuuu__ZTSZZ4mainENK3$_0clERN2cl4sycl7handlerEE4Test {{.*}} alloca
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16uuuuuuuuuuuuuuuu__ZTSZZ4mainENK3$_0clERN2cl4sycl7handlerEE4Test {{.*}} store
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16uuuuuuuuuuuuuuuu__ZTSZZ4mainENK3$_0clERN2cl4sycl7handlerEE4Test {{.*}} alloca
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16uuuuuuuuuuuuuuuu__ZTSZZ4mainENK3$_0clERN2cl4sycl7handlerEE4Test {{.*}} store
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16uuuuuuuuuuuuuuuu__ZTSZZ4mainENK3$_0clERN2cl4sycl7handlerEE4Test {{.*}} alloca
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16uuuuuuuuuuuuuuuu__ZTSZZ4mainENK3$_0clERN2cl4sycl7handlerEE4Test {{.*}} store
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16uuuuuuuuuuuuuuuu__ZTSZZ4mainENK3$_0clERN2cl4sycl7handlerEE4Test {{.*}} alloca
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16uuuuuuuuuuuuuuuu__ZTSZZ4mainENK3$_0clERN2cl4sycl7handlerEE4Test {{.*}} store
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16uuuuuuuuuuuuuuuu__ZTSZZ4mainENK3$_0clERN2cl4sycl7handlerEE4Test {{.*}} alloca
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16uuuuuuuuuuuuuuuu__ZTSZZ4mainENK3$_0clERN2cl4sycl7handlerEE4Test {{.*}} bitcast
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16uuuuuuuuuuuuuuuu__ZTSZZ4mainENK3$_0clERN2cl4sycl7handlerEE4Test {{.*}} store
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16uuuuuuuuuuuuuuuu__ZTSZZ4mainENK3$_0clERN2cl4sycl7handlerEE4Test {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16uuuuuuuuuuuuuuuu__ZTSZZ4mainENK3$_0clERN2cl4sycl7handlerEE4Test {{.*}} call
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16uuuuuuuuuuuuuuuu__ZTSZZ4mainENK3$_0clERN2cl4sycl7handlerEE4Test {{.*}} sext
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16uuuuuuuuuuuuuuuu__ZTSZZ4mainENK3$_0clERN2cl4sycl7handlerEE4Test {{.*}} add
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16uuuuuuuuuuuuuuuu__ZTSZZ4mainENK3$_0clERN2cl4sycl7handlerEE4Test {{.*}} getelementptr
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16uuuuuuuuuuuuuuuu__ZTSZZ4mainENK3$_0clERN2cl4sycl7handlerEE4Test {{.*}} load
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16uuuuuuuuuuuuuuuu__ZTSZZ4mainENK3$_0clERN2cl4sycl7handlerEE4Test {{.*}} icmp
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16uuuuuuuuuuuuuuuu__ZTSZZ4mainENK3$_0clERN2cl4sycl7handlerEE4Test {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16uuuuuuuuuuuuuuuu__ZTSZZ4mainENK3$_0clERN2cl4sycl7handlerEE4Test {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16uuuuuuuuuuuuuuuu__ZTSZZ4mainENK3$_0clERN2cl4sycl7handlerEE4Test {{.*}} add
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16uuuuuuuuuuuuuuuu__ZTSZZ4mainENK3$_0clERN2cl4sycl7handlerEE4Test {{.*}} icmp
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16uuuuuuuuuuuuuuuu__ZTSZZ4mainENK3$_0clERN2cl4sycl7handlerEE4Test {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16uuuuuuuuuuuuuuuu__ZTSZZ4mainENK3$_0clERN2cl4sycl7handlerEE4Test {{.*}} call
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16uuuuuuuuuuuuuuuu__ZTSZZ4mainENK3$_0clERN2cl4sycl7handlerEE4Test {{.*}} br
; DEBUGIFY-NOT: WARNING
