; RUN: %oclopt --ocl-postvect --ocl-vec-clone-isa-encoding-override=AVX512Core -S < %s | FileCheck %s

; Providing two instances of the kernel to OCLPostVect: original and vectorized.
; The vectorized one is artificially over-costed (added an expensive loop).
; As the result the vectorized kernel should be removed.

; CHECK-NOT: define
; CHECK: define void @"_ZTSZZ4mainENK3$_0clERN2cl4sycl7handlerEE4Test"
; CHECK-NOT: define

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

%"class.cl::sycl::range" = type { %"class.cl::sycl::detail::array" }
%"class.cl::sycl::detail::array" = type { [1 x i64] }

; Function Attrs: nounwind
define void @"_ZTSZZ4mainENK3$_0clERN2cl4sycl7handlerEE4Test"(i32 addrspace(1)*, %"class.cl::sycl::range"* byval, %"class.cl::sycl::range"* byval, %"class.cl::sycl::range"* byval, i32 addrspace(1)*, %"class.cl::sycl::range"* byval, %"class.cl::sycl::range"* byval, %"class.cl::sycl::range"* byval, i32 addrspace(1)*, %"class.cl::sycl::range"* byval, %"class.cl::sycl::range"* byval, %"class.cl::sycl::range"* byval, i32 addrspace(1)*, %"class.cl::sycl::range"* byval, %"class.cl::sycl::range"* byval, %"class.cl::sycl::range"* byval) local_unnamed_addr #0 !kernel_arg_addr_space !8 !kernel_arg_access_qual !9 !kernel_arg_type !10 !kernel_arg_type_qual !11 !kernel_arg_base_type !12 !no_barrier_path !13 !kernel_has_sub_groups !13 !vectorized_kernel !14 !vectorized_width !15 !scalarized_kernel !16 {
  %gid = call i64 @_Z13get_global_idj(i32 0) #1
  %gep1 = getelementptr inbounds i32, i32 addrspace(1)* %0, i64 %gid
  %ld = load i32, i32 addrspace(1)* %gep1, align 4
  %add = add i32 %ld, 20
  store i32 %add, i32 addrspace(1)* %gep1, align 4
  ret void
}

; Function Attrs: nounwind readnone
declare i64 @_Z13get_global_idj(i32) local_unnamed_addr #1

; Function Attrs: nounwind readnone
declare i32 @_Z22get_sub_group_local_idv() #1

; Function Attrs: nounwind
define void @"_ZGVeN16uuuuuuuuuuuuuuuu_TSZZ4mainENK3$_0clERN2cl4sycl7handlerEE4Test"(i32 addrspace(1)*, %"class.cl::sycl::range"* byval, %"class.cl::sycl::range"* byval, %"class.cl::sycl::range"* byval, i32 addrspace(1)*, %"class.cl::sycl::range"* byval, %"class.cl::sycl::range"* byval, %"class.cl::sycl::range"* byval, i32 addrspace(1)*, %"class.cl::sycl::range"* byval, %"class.cl::sycl::range"* byval, %"class.cl::sycl::range"* byval, i32 addrspace(1)*, %"class.cl::sycl::range"* byval, %"class.cl::sycl::range"* byval, %"class.cl::sycl::range"* byval) local_unnamed_addr #0 !kernel_arg_addr_space !8 !kernel_arg_access_qual !9 !kernel_arg_type !10 !kernel_arg_type_qual !11 !kernel_arg_base_type !12 !no_barrier_path !13 !kernel_has_sub_groups !13 !ocl_recommended_vector_length !17 !vectorized_kernel !16 !vectorized_width !17 !vectorization_dimension !7 !scalarized_kernel !6 !can_unite_workgroups !18 {
  %gid = call i64 @_Z13get_global_idj(i32 0) #1
  br label %simd.begin.region

simd.begin.region:                                ; preds = %16
  br i1 false, label %scalar.ph, label %min.iters.checked

min.iters.checked:                                ; preds = %simd.begin.region
  br i1 false, label %scalar.ph, label %vector.ph

vector.ph:                                        ; preds = %min.iters.checked
  br label %vector.body

vector.body:                                      ; preds = %vector.body, %vector.ph
  %index2 = phi i32 [ 0, %vector.ph ], [ %index.next, %loop.exit ]
  %vec.ind = phi <16 x i32> [ <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>, %vector.ph ], [ %vec.ind.next, %loop.exit ]
  %17 = sext <16 x i32> %vec.ind to <16 x i64>
  %18 = extractelement <16 x i64> %17, i32 0
  %19 = add nuw i64 %18, %gid
  %20 = getelementptr inbounds i32, i32 addrspace(1)* %0, i64 %19
  %21 = bitcast i32 addrspace(1)* %20 to <16 x i32> addrspace(1)*
  %wide.load = load <16 x i32>, <16 x i32> addrspace(1)* %21, align 4
  %22 = add <16 x i32> %wide.load, <i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1>
  %23 = getelementptr inbounds i32, i32 addrspace(1)* %0, i64 %19
  %24 = bitcast i32 addrspace(1)* %23 to <16 x i32> addrspace(1)*
  br label %loop.next

  ; Let's make the vectorized kernel VERY expensive with a loop of 50 store commands
loop.next:
  %counter = phi i32 [ 50, %vector.body ], [ %counter.next, %loop.next ]
  store <16 x i32> %22, <16 x i32> addrspace(1)* %24, align 4
  %counter.next = sub i32 %counter, 1

  %loop.done = icmp eq i32 %counter.next, 0
  br i1 %loop.done, label %loop.exit, label %loop.next

loop.exit:
  %25 = add nuw i32 %index2, 1
  %broadcast.splatinsert = insertelement <16 x i32> undef, i32 %25, i32 0
  %broadcast.splat = shufflevector <16 x i32> %broadcast.splatinsert, <16 x i32> undef, <16 x i32> zeroinitializer
  %26 = icmp ult <16 x i32> %broadcast.splat, <i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16>
  %27 = extractelement <16 x i1> %26, i32 0
  %index.next = add i32 %index2, 16
  %vec.ind.next = add <16 x i32> %vec.ind, <i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16>
  br i1 true, label %VPlannedBB, label %vector.body

VPlannedBB:                                       ; preds = %vector.body
  br label %middle.block

middle.block:                                     ; preds = %VPlannedBB
  %cmp.n = icmp eq i32 16, 16
  br i1 %cmp.n, label %simd.end.region, label %scalar.ph

scalar.ph:                                        ; preds = %middle.block, %min.iters.checked, %simd.begin.region
  %bc.resume.val = phi i32 [ 16, %middle.block ], [ 0, %simd.begin.region ], [ 0, %min.iters.checked ]
  br label %simd.loop

simd.loop:                                        ; preds = %simd.loop.exit, %scalar.ph
  %index = phi i32 [ %bc.resume.val, %scalar.ph ], [ %indvar, %simd.loop.exit ]
  %28 = sext i32 %index to i64
  %add1 = add nuw i64 %28, %gid
  %gep1 = getelementptr inbounds i32, i32 addrspace(1)* %0, i64 %add1
  %ld = load i32, i32 addrspace(1)* %gep1, align 4
  %add = add i32 %ld, 20
  store i32 %add, i32 addrspace(1)* %gep1, align 4
  br label %simd.loop.exit

simd.loop.exit:                                   ; preds = %simd.loop
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 16
  br i1 %vl.cond, label %simd.loop, label %simd.end.region, !llvm.loop !19

simd.end.region:                                  ; preds = %middle.block, %simd.loop.exit
  br label %return

return:                                           ; preds = %simd.end.region
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

attributes #0 = { nounwind "vector-variants"="_ZGVeN16uuuuuuuuuuuuuuuu__ZTSZZ4mainENK3$_0clERN2cl4sycl7handlerEE4Test" }
attributes #1 = { nounwind readnone }
attributes #2 = { nounwind }

!opencl.enable.FP_CONTRACT = !{}
!spirv.Source = !{!0}
!opencl.spir.version = !{!1}
!opencl.ocl.version = !{!2}
!opencl.used.extensions = !{!3}
!opencl.used.optional.core.features = !{!4}
!spirv.Generator = !{!5}
!opencl.kernels = !{!6}
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
!14 = !{void (i32 addrspace(1)*, %"class.cl::sycl::range"*, %"class.cl::sycl::range"*, %"class.cl::sycl::range"*, i32 addrspace(1)*, %"class.cl::sycl::range"*, %"class.cl::sycl::range"*, %"class.cl::sycl::range"*, i32 addrspace(1)*, %"class.cl::sycl::range"*, %"class.cl::sycl::range"*, %"class.cl::sycl::range"*, i32 addrspace(1)*, %"class.cl::sycl::range"*, %"class.cl::sycl::range"*, %"class.cl::sycl::range"*)* @"_ZGVeN16uuuuuuuuuuuuuuuu_TSZZ4mainENK3$_0clERN2cl4sycl7handlerEE4Test"}
!15 = !{i32 1}
!16 = !{null}
!17 = !{i32 16}
!18 = !{i1 false}
!19 = distinct !{!19, !20}
!20 = !{!"llvm.loop.unroll.disable"}
