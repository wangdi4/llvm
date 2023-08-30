; Check if VecClone processes OpenCL kernels again.

; RUN: opt -passes="vec-clone" -S < %s | FileCheck %s


; CHECK: _ZGVcN16uuuuuuuuuuuuuuuu_TSZZ4mainENK3$_0clERN2cl4sycl7handlerEE4Test
; CHECK: _ZGVcM16uuuuuuuuuuuuuuuu_TSZZ4mainENK3$_0clERN2cl4sycl7handlerEE4Test
; CHECK-NOT: _ZGVcN16uuuuuuuuuuuuuuuu_TSZZ4mainENK3$_0clERN2cl4sycl7handlerEE4Test.1
; CHECK-NOT: _ZGVcM16uuuuuuuuuuuuuuuu_TSZZ4mainENK3$_0clERN2cl4sycl7handlerEE4Test.2
; CHECK-NOT: _ZGVcN16uuuuuuuuuuuuuuuu_GVcN16uuuuuuuuuuuuuuuu_TSZZ4mainENK3$_0clERN2cl4sycl7handlerEE4Test
; CHECK-NOT: _ZGVcM16uuuuuuuuuuuuuuuu_GVcN16uuuuuuuuuuuuuuuu_TSZZ4mainENK3$_0clERN2cl4sycl7handlerEE4Test
; CHECK-NOT: _ZGVcN16uuuuuuuuuuuuuuuu_GVcM16uuuuuuuuuuuuuuuu_TSZZ4mainENK3$_0clERN2cl4sycl7handlerEE4Test
; CHECK-NOT: _ZGVcM16uuuuuuuuuuuuuuuu_GVcM16uuuuuuuuuuuuuuuu_TSZZ4mainENK3$_0clERN2cl4sycl7handlerEE4Test

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

%"class.cl::sycl::range" = type { %"class.cl::sycl::detail::array" }
%"class.cl::sycl::detail::array" = type { [1 x i64] }

; Function Attrs: nounwind
define void @"_ZTSZZ4mainENK3$_0clERN2cl4sycl7handlerEE4Test"(ptr addrspace(1) %0, ptr byval(%"class.cl::sycl::range") %1, ptr byval(%"class.cl::sycl::range") %2, ptr byval(%"class.cl::sycl::range") %3, ptr addrspace(1) %4, ptr byval(%"class.cl::sycl::range") %5, ptr byval(%"class.cl::sycl::range") %6, ptr byval(%"class.cl::sycl::range") %7, ptr addrspace(1) %8, ptr byval(%"class.cl::sycl::range") %9, ptr byval(%"class.cl::sycl::range") %10, ptr byval(%"class.cl::sycl::range") %11, ptr addrspace(1) %12, ptr byval(%"class.cl::sycl::range") %13, ptr byval(%"class.cl::sycl::range") %14, ptr byval(%"class.cl::sycl::range") %15) local_unnamed_addr #0 !kernel_arg_addr_space !8 !kernel_arg_access_qual !9 !kernel_arg_type !10 !kernel_arg_type_qual !11 !kernel_arg_base_type !12 !no_barrier_path !13 !kernel_has_sub_groups !13 !vectorized_kernel !14 !vectorized_width !15 !scalarized_kernel !16 !vectorized_masked_kernel !17 {
  %gid = call i64 @_Z13get_global_idj(i32 0) #1
  %gep1 = getelementptr inbounds i32, ptr addrspace(1) %0, i64 %gid
  %ld = load i32, ptr addrspace(1) %gep1, align 4
  %gid.trunc = trunc i64 %gid to i32
  store i32 %gid.trunc, ptr addrspace(1) %gep1, align 4
  ret void
}

; Function Attrs: nounwind readnone
declare i64 @_Z13get_global_idj(i32) local_unnamed_addr #1

; Function Attrs: nounwind readnone
declare i32 @_Z22get_sub_group_local_idv() #1

; Function Attrs: nounwind
define void @"_ZGVcN16uuuuuuuuuuuuuuuu_TSZZ4mainENK3$_0clERN2cl4sycl7handlerEE4Test"(ptr addrspace(1) %0, ptr byval(%"class.cl::sycl::range") %1, ptr byval(%"class.cl::sycl::range") %2, ptr byval(%"class.cl::sycl::range") %3, ptr addrspace(1) %4, ptr byval(%"class.cl::sycl::range") %5, ptr byval(%"class.cl::sycl::range") %6, ptr byval(%"class.cl::sycl::range") %7, ptr addrspace(1) %8, ptr byval(%"class.cl::sycl::range") %9, ptr byval(%"class.cl::sycl::range") %10, ptr byval(%"class.cl::sycl::range") %11, ptr addrspace(1) %12, ptr byval(%"class.cl::sycl::range") %13, ptr byval(%"class.cl::sycl::range") %14, ptr byval(%"class.cl::sycl::range") %15) local_unnamed_addr #0 !kernel_arg_addr_space !8 !kernel_arg_access_qual !9 !kernel_arg_type !10 !kernel_arg_type_qual !11 !kernel_arg_base_type !12 !no_barrier_path !13 !kernel_has_sub_groups !13 !ocl_recommended_vector_length !18 !vectorized_kernel !16 !vectorized_width !18 !vectorization_dimension !7 !scalarized_kernel !6 !can_unite_workgroups !19 {
  %alloca. = alloca ptr addrspace(1), align 8
  store ptr addrspace(1) %0, ptr %alloca., align 8
  %alloca.1 = alloca ptr addrspace(1), align 8
  store ptr addrspace(1) %4, ptr %alloca.1, align 8
  %alloca.2 = alloca ptr addrspace(1), align 8
  store ptr addrspace(1) %8, ptr %alloca.2, align 8
  %alloca.3 = alloca ptr addrspace(1), align 8
  store ptr addrspace(1) %12, ptr %alloca.3, align 8
  %gid = call i64 @_Z13get_global_idj(i32 0) #1
  br label %simd.begin.region

simd.begin.region:                                ; preds = %16
  br label %simd.loop.preheader

simd.loop.preheader:                              ; preds = %simd.begin.region
  %load.6 = load ptr addrspace(1), ptr %alloca.3, align 8
  %load.5 = load ptr addrspace(1), ptr %alloca.2, align 8
  %load.4 = load ptr addrspace(1), ptr %alloca.1, align 8
  %load. = load ptr addrspace(1), ptr %alloca., align 8
  br i1 false, label %scalar.ph, label %vector.ph

vector.ph:                                        ; preds = %simd.loop.preheader
  %broadcast.splatinsert = insertelement <16 x i64> undef, i64 %gid, i32 0
  %broadcast.splat = shufflevector <16 x i64> %broadcast.splatinsert, <16 x i64> undef, <16 x i32> zeroinitializer
  br label %vector.body

vector.body:                                      ; preds = %vector.body, %vector.ph
  %uni.phi = phi i32 [ 0, %vector.ph ], [ %24, %vector.body ]
  %uni.phi7 = phi i32 [ 0, %vector.ph ], [ %23, %vector.body ]
  %vec.phi = phi <16 x i32> [ <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>, %vector.ph ], [ %22, %vector.body ]
  %17 = sext <16 x i32> %vec.phi to <16 x i64>
  %18 = add nuw <16 x i64> %17, %broadcast.splat
  %.extract.0. = extractelement <16 x i64> %18, i32 0
  %scalar.gep = getelementptr inbounds i32, ptr addrspace(1) %load., i64 %.extract.0.
  %19 = bitcast ptr addrspace(1) %scalar.gep to ptr addrspace(1)
  %wide.load = load <16 x i32>, ptr addrspace(1) %19, align 4
  %20 = trunc <16 x i64> %18 to <16 x i32>
  %21 = bitcast ptr addrspace(1) %scalar.gep to ptr addrspace(1)
  store <16 x i32> %20, ptr addrspace(1) %21, align 4
  %22 = add nuw <16 x i32> %vec.phi, <i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16>
  %23 = add nuw i32 %uni.phi7, 16
  %24 = add i32 %uni.phi, 16
  %25 = icmp ne i32 %24, 16
  br i1 false, label %vector.body, label %VPlannedBB, !llvm.loop !20

VPlannedBB:                                       ; preds = %vector.body
  br label %middle.block

middle.block:                                     ; preds = %VPlannedBB
  %cmp.n = icmp eq i32 16, 16
  br i1 %cmp.n, label %simd.end.region, label %scalar.ph

scalar.ph:                                        ; preds = %middle.block, %simd.loop.preheader
  %bc.resume.val = phi i32 [ 0, %simd.loop.preheader ], [ 16, %middle.block ]
  br label %simd.loop

simd.loop:                                        ; preds = %simd.loop.exit, %scalar.ph
  %index = phi i32 [ %bc.resume.val, %scalar.ph ], [ %indvar, %simd.loop.exit ]
  %26 = sext i32 %index to i64
  %add = add nuw i64 %26, %gid
  %gep1 = getelementptr inbounds i32, ptr addrspace(1) %load., i64 %add
  %ld = load i32, ptr addrspace(1) %gep1, align 4
  %gid.trunc = trunc i64 %add to i32
  store i32 %gid.trunc, ptr addrspace(1) %gep1, align 4
  br label %simd.loop.exit

simd.loop.exit:                                   ; preds = %simd.loop
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 16
  br i1 %vl.cond, label %simd.loop, label %simd.end.region, !llvm.loop !22

simd.end.region:                                  ; preds = %middle.block, %simd.loop.exit
  br label %return

return:                                           ; preds = %simd.end.region
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: nounwind
define void @"_ZGVcM16uuuuuuuuuuuuuuuu_TSZZ4mainENK3$_0clERN2cl4sycl7handlerEE4Test"(ptr addrspace(1) %0, ptr byval(%"class.cl::sycl::range") %1, ptr byval(%"class.cl::sycl::range") %2, ptr byval(%"class.cl::sycl::range") %3, ptr addrspace(1) %4, ptr byval(%"class.cl::sycl::range") %5, ptr byval(%"class.cl::sycl::range") %6, ptr byval(%"class.cl::sycl::range") %7, ptr addrspace(1) %8, ptr byval(%"class.cl::sycl::range") %9, ptr byval(%"class.cl::sycl::range") %10, ptr byval(%"class.cl::sycl::range") %11, ptr addrspace(1) %12, ptr byval(%"class.cl::sycl::range") %13, ptr byval(%"class.cl::sycl::range") %14, ptr byval(%"class.cl::sycl::range") %15, <16 x i32> %mask) local_unnamed_addr #0 !kernel_arg_addr_space !8 !kernel_arg_access_qual !9 !kernel_arg_type !10 !kernel_arg_type_qual !11 !kernel_arg_base_type !12 !no_barrier_path !13 !kernel_has_sub_groups !13 !ocl_recommended_vector_length !18 !vectorized_kernel !16 !vectorized_width !18 !vectorization_dimension !7 !scalarized_kernel !6 !can_unite_workgroups !19 {
  %alloca. = alloca ptr addrspace(1), align 8
  store ptr addrspace(1) %0, ptr %alloca., align 8
  %alloca.1 = alloca ptr addrspace(1), align 8
  store ptr addrspace(1) %4, ptr %alloca.1, align 8
  %alloca.2 = alloca ptr addrspace(1), align 8
  store ptr addrspace(1) %8, ptr %alloca.2, align 8
  %alloca.3 = alloca ptr addrspace(1), align 8
  store ptr addrspace(1) %12, ptr %alloca.3, align 8
  %vec.mask = alloca <16 x i32>, align 64
  %mask.cast = bitcast ptr %vec.mask to ptr
  store <16 x i32> %mask, ptr %vec.mask, align 64
  %gid = call i64 @_Z13get_global_idj(i32 0) #1
  br label %simd.begin.region

simd.begin.region:                                ; preds = %16
  br label %simd.loop.preheader

simd.loop.preheader:                              ; preds = %simd.begin.region
  %load.6 = load ptr addrspace(1), ptr %alloca.3, align 8
  %load.5 = load ptr addrspace(1), ptr %alloca.2, align 8
  %load.4 = load ptr addrspace(1), ptr %alloca.1, align 8
  %load. = load ptr addrspace(1), ptr %alloca., align 8
  br i1 false, label %scalar.ph, label %vector.ph

vector.ph:                                        ; preds = %simd.loop.preheader
  %broadcast.splatinsert = insertelement <16 x i64> undef, i64 %gid, i32 0
  %broadcast.splat = shufflevector <16 x i64> %broadcast.splatinsert, <16 x i64> undef, <16 x i32> zeroinitializer
  br label %vector.body

vector.body:                                      ; preds = %vector.body, %vector.ph
  %uni.phi = phi i32 [ 0, %vector.ph ], [ %27, %vector.body ]
  %uni.phi7 = phi i32 [ 0, %vector.ph ], [ %26, %vector.body ]
  %vec.phi = phi <16 x i32> [ <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>, %vector.ph ], [ %25, %vector.body ]
  %17 = sext <16 x i32> %vec.phi to <16 x i64>
  %18 = add nuw <16 x i64> %17, %broadcast.splat
  %.extract.0. = extractelement <16 x i64> %18, i32 0
  %scalar.gep = getelementptr i32, ptr %mask.cast, i32 %uni.phi7
  %19 = bitcast ptr %scalar.gep to ptr
  %wide.load = load <16 x i32>, ptr %19, align 4
  %20 = icmp ne <16 x i32> %wide.load, zeroinitializer
  %21 = xor <16 x i1> %20, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  %scalar.gep8 = getelementptr inbounds i32, ptr addrspace(1) %load., i64 %.extract.0.
  %22 = bitcast ptr addrspace(1) %scalar.gep8 to ptr addrspace(1)
  %wide.masked.load = call <16 x i32> @llvm.masked.load.v16i32.p1(ptr addrspace(1) %22, i32 4, <16 x i1> %20, <16 x i32> undef)
  %23 = trunc <16 x i64> %18 to <16 x i32>
  %24 = bitcast ptr addrspace(1) %scalar.gep8 to ptr addrspace(1)
  call void @llvm.masked.store.v16i32.p1(<16 x i32> %23, ptr addrspace(1) %24, i32 4, <16 x i1> %20)
  %25 = add nuw <16 x i32> %vec.phi, <i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16>
  %26 = add nuw i32 %uni.phi7, 16
  %27 = add i32 %uni.phi, 16
  %28 = icmp ne i32 %27, 16
  br i1 false, label %vector.body, label %VPlannedBB, !llvm.loop !25

VPlannedBB:                                       ; preds = %vector.body
  br label %middle.block

middle.block:                                     ; preds = %VPlannedBB
  %cmp.n = icmp eq i32 16, 16
  br i1 %cmp.n, label %simd.end.region, label %scalar.ph

scalar.ph:                                        ; preds = %middle.block, %simd.loop.preheader
  %bc.resume.val = phi i32 [ 0, %simd.loop.preheader ], [ 16, %middle.block ]
  br label %simd.loop

simd.loop:                                        ; preds = %simd.loop.exit, %scalar.ph
  %index = phi i32 [ %bc.resume.val, %scalar.ph ], [ %indvar, %simd.loop.exit ]
  %29 = sext i32 %index to i64
  %add = add nuw i64 %29, %gid
  %mask.gep = getelementptr i32, ptr %mask.cast, i32 %index
  %mask.parm = load i32, ptr %mask.gep, align 4
  %mask.cond = icmp ne i32 %mask.parm, 0
  br i1 %mask.cond, label %simd.loop.then, label %simd.loop.else

simd.loop.then:                                   ; preds = %simd.loop
  %gep1 = getelementptr inbounds i32, ptr addrspace(1) %load., i64 %add
  %ld = load i32, ptr addrspace(1) %gep1, align 4
  %gid.trunc = trunc i64 %add to i32
  store i32 %gid.trunc, ptr addrspace(1) %gep1, align 4
  br label %simd.loop.exit

simd.loop.else:                                   ; preds = %simd.loop
  br label %simd.loop.exit

simd.loop.exit:                                   ; preds = %simd.loop.else, %simd.loop.then
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 16
  br i1 %vl.cond, label %simd.loop, label %simd.end.region, !llvm.loop !26

simd.end.region:                                  ; preds = %middle.block, %simd.loop.exit
  br label %return

return:                                           ; preds = %simd.end.region
  ret void
}

; Function Attrs: argmemonly nounwind readonly willreturn
declare <16 x i32> @llvm.masked.load.v16i32.p1(ptr addrspace(1), i32 immarg, <16 x i1>, <16 x i32>) #3

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.masked.store.v16i32.p1(<16 x i32>, ptr addrspace(1), i32 immarg, <16 x i1>) #4

attributes #0 = { nounwind "vector-variants"="_ZGVcN16uuuuuuuuuuuuuuuu__ZTSZZ4mainENK3$_0clERN2cl4sycl7handlerEE4Test,_ZGVcM16uuuuuuuuuuuuuuuu__ZTSZZ4mainENK3$_0clERN2cl4sycl7handlerEE4Test" }
attributes #1 = { nounwind readnone }
attributes #2 = { nounwind }
attributes #3 = { argmemonly nounwind readonly willreturn }
attributes #4 = { argmemonly nounwind willreturn }

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
!6 = !{ptr @"_ZTSZZ4mainENK3$_0clERN2cl4sycl7handlerEE4Test"}
!7 = !{i32 0}
!8 = !{i32 1, i32 0, i32 0, i32 0, i32 1, i32 0, i32 0, i32 0, i32 1, i32 0, i32 0, i32 0, i32 1, i32 0, i32 0, i32 0}
!9 = !{!"none", !"none", !"none", !"none", !"none", !"none", !"none", !"none", !"none", !"none", !"none", !"none", !"none", !"none", !"none", !"none"}
!10 = !{!"int*", !"range<1>", !"range<1>", !"id<1>", !"int*", !"range<1>", !"range<1>", !"id<1>", !"int*", !"range<1>", !"range<1>", !"id<1>", !"int*", !"range<1>", !"range<1>", !"id<1>"}
!11 = !{!"", !"", !"", !"", !"", !"", !"", !"", !"", !"", !"", !"", !"", !"", !"", !""}
!12 = !{!"int*", !"class.cl::sycl::range", !"class.cl::sycl::range", !"class.cl::sycl::range", !"int*", !"class.cl::sycl::range", !"class.cl::sycl::range", !"class.cl::sycl::range", !"int*", !"class.cl::sycl::range", !"class.cl::sycl::range", !"class.cl::sycl::range", !"int*", !"class.cl::sycl::range", !"class.cl::sycl::range", !"class.cl::sycl::range"}
!13 = !{i1 true}
!14 = !{ptr @"_ZGVcN16uuuuuuuuuuuuuuuu_TSZZ4mainENK3$_0clERN2cl4sycl7handlerEE4Test"}
!15 = !{i32 1}
!16 = !{null}
!17 = !{ptr @"_ZGVcM16uuuuuuuuuuuuuuuu_TSZZ4mainENK3$_0clERN2cl4sycl7handlerEE4Test"}
!18 = !{i32 16}
!19 = !{i1 false}
!20 = distinct !{!20, !21}
!21 = !{!"llvm.loop.isvectorized", i32 1}
!22 = distinct !{!22, !23, !24}
!23 = !{!"llvm.loop.unroll.disable"}
!24 = !{!"llvm.loop.vectorize.enable", i32 1}
!25 = distinct !{!25, !21}
!26 = distinct !{!26, !23, !24}
