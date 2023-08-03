; RUN: llvm-as %S/builtin_lib.rtl -o %t.rtl.bc
; RUN: opt -sycl-kernel-builtin-lib=%t.rtl.bc -passes='debugify,sycl-kernel-resolve-sub-group-wi-call,check-debugify' -S %s -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -sycl-kernel-builtin-lib=%t.rtl.bc -passes='sycl-kernel-resolve-sub-group-wi-call' -S %s | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

%struct._ZTS13pixel_block_t.pixel_block_t = type { [16 x i8], [16 x i8], [16 x i8], [16 x i8] }
%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range" = type { %"class._ZTSN2cl4sycl6detail5arrayILi1EEE.cl::sycl::detail::array" }
%"class._ZTSN2cl4sycl6detail5arrayILi1EEE.cl::sycl::detail::array" = type { [1 x i64] }
%"class._ZTSN2cl4sycl5rangeILi2EEE.cl::sycl::range" = type { %"class._ZTSN2cl4sycl6detail5arrayILi2EEE.cl::sycl::detail::array" }
%"class._ZTSN2cl4sycl6detail5arrayILi2EEE.cl::sycl::detail::array" = type { [2 x i64] }
%"class._ZTSN2cl4sycl3vecIhLi4EEE.cl::sycl::vec" = type { <4 x i8> }
; Function Attrs: nounwind
define void @_ZTS17mandelbrot_kernelIdE(ptr addrspace(1) noalias, ptr byval(%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"), ptr byval(%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"), ptr byval(%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"), ptr byval(%"class._ZTSN2cl4sycl5rangeILi2EEE.cl::sycl::range"), ptr byval(%"class._ZTSN2cl4sycl5rangeILi2EEE.cl::sycl::range"), ptr byval(%"class._ZTSN2cl4sycl3vecIhLi4EEE.cl::sycl::vec"), i32, i32, i32, double, double, double, i32) local_unnamed_addr #0 !kernel_arg_addr_space !12 !kernel_arg_access_qual !13 !kernel_arg_type !14 !kernel_arg_type_qual !15 !kernel_arg_base_type !16 !intel_reqd_sub_group_size !17 !vectorized_kernel !18 !no_barrier_path !19 !kernel_has_sub_groups !19 !scalar_kernel !20 !vectorized_width !21 !arg_type_null_val !28 {
entry:
  ret void
}
; Function Attrs: nounwind readnone
declare i32 @_Z22get_sub_group_local_idv() local_unnamed_addr #1
; Function Attrs: nounwind readnone
declare i32 @_Z22get_max_sub_group_sizev() local_unnamed_addr #1
; Function Attrs: nounwind readnone
declare i64 @_Z13get_global_idj(i32) local_unnamed_addr #1
; Function Attrs: nounwind readnone
declare i64 @_Z12get_group_idj(i32) local_unnamed_addr #1
; CHECK-LABEL: @WG.boundaries._ZTS17mandelbrot_kernelIdE
define [7 x i64] @WG.boundaries._ZTS17mandelbrot_kernelIdE(ptr addrspace(1), ptr, ptr, ptr, ptr, ptr, ptr, i32, i32, i32, double, double, double, i32) {
entry:
  %14 = call i64 @_Z14get_local_sizej(i32 0)
  %15 = call i64 @get_base_global_id.(i32 0)
  %16 = call i64 @_Z14get_local_sizej(i32 1)
  %17 = call i64 @get_base_global_id.(i32 1)
  %18 = call i64 @_Z14get_local_sizej(i32 2)
  %19 = call i64 @get_base_global_id.(i32 2)
  %20 = call i64 @_Z12get_group_idj(i32 0) #1
; CHECK-NOT: _Z22get_max_sub_group_sizev
  %21 = call i32 @_Z22get_max_sub_group_sizev() #1
  %22 = call i32 @_Z22get_sub_group_local_idv() #1
  %23 = trunc i64 %20 to i32
; CHECK: mul i32 16, %{{.*}}
  %24 = mul i32 %21, %23
  %25 = add i32 %22, %24
  %26 = icmp slt i32 %25, %7
  %27 = sext i32 %8 to i64
  %28 = add i64 %16, %17
  %29 = icmp slt i64 %28, %27
  %30 = icmp slt i64 %27, 0
  %31 = or i1 %30, %29
  %32 = select i1 %31, i64 %28, i64 %27
  %33 = sub i64 %32, %17
  %34 = and i1 true, %26
  %35 = icmp slt i64 0, %33
  %36 = and i1 %34, %35
  %zext_cast = zext i1 %36 to i64
  %37 = insertvalue [7 x i64] undef, i64 %14, 2
  %38 = insertvalue [7 x i64] %37, i64 %15, 1
  %39 = insertvalue [7 x i64] %38, i64 %33, 4
  %40 = insertvalue [7 x i64] %39, i64 %17, 3
  %41 = insertvalue [7 x i64] %40, i64 %18, 6
  %42 = insertvalue [7 x i64] %41, i64 %19, 5
  %43 = insertvalue [7 x i64] %42, i64 %zext_cast, 0
  ret [7 x i64] %43
}
declare i64 @_Z14get_local_sizej(i32)
declare i64 @get_base_global_id.(i32)
; Function Attrs: nounwind
define void @__Vectorized_._ZTS17mandelbrot_kernelIdE(ptr addrspace(1) noalias, ptr byval(%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"), ptr byval(%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"), ptr byval(%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"), ptr byval(%"class._ZTSN2cl4sycl5rangeILi2EEE.cl::sycl::range"), ptr byval(%"class._ZTSN2cl4sycl5rangeILi2EEE.cl::sycl::range"), ptr byval(%"class._ZTSN2cl4sycl3vecIhLi4EEE.cl::sycl::vec"), i32, i32, i32, double, double, double, i32) local_unnamed_addr #0 !kernel_arg_addr_space !12 !kernel_arg_access_qual !13 !kernel_arg_type !14 !kernel_arg_type_qual !15 !kernel_arg_base_type !16 !intel_reqd_sub_group_size !17 !vectorized_kernel !20 !no_barrier_path !19 !kernel_has_sub_groups !19 !scalar_kernel !25 !vectorized_width !17 !vectorization_dimension !26 !can_unite_workgroups !27 {
entry:
  ret void
}
attributes #0 = { nounwind }
attributes #1 = { nounwind readnone }
!opencl.enable.FP_CONTRACT = !{}
!spirv.Source = !{!0}
!opencl.spir.version = !{!1}
!opencl.ocl.version = !{!2}
!opencl.used.extensions = !{!3}
!opencl.used.optional.core.features = !{!4}
!spirv.Generator = !{!5}
!sycl.kernels = !{!11}
!0 = !{i32 4, i32 100000}
!1 = !{i32 1, i32 2}
!2 = !{i32 1, i32 0}
!3 = !{}
!4 = !{!"cl_doubles"}
!5 = !{i16 6, i16 14}
!11 = !{ptr @_ZTS17mandelbrot_kernelIdE}
!12 = !{i32 1, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0}
!13 = !{!"none", !"none", !"none", !"none", !"none", !"none", !"none", !"none", !"none", !"none", !"none", !"none", !"none", !"none"}
!14 = !{!"pixel_block_t*", !"cl::sycl::range<1>", !"cl::sycl::range<1>", !"cl::sycl::id<1>", !"cl::sycl::range<2>", !"cl::sycl::range<2>", !"cl::sycl::uchar4", !"int", !"int", !"uint32_t", !"double", !"double", !"double", !"int"}
!15 = !{!"", !"", !"", !"", !"", !"", !"", !"", !"", !"", !"", !"", !"", !""}
!16 = !{!"struct _ZTS13pixel_block_t.pixel_block_t*", !"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range", !"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range", !"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range", !"class._ZTSN2cl4sycl5rangeILi2EEE.cl::sycl::range", !"class._ZTSN2cl4sycl5rangeILi2EEE.cl::sycl::range", !"class._ZTSN2cl4sycl3vecIhLi4EEE.cl::sycl::vec", !"int", !"int", !"int", !"double", !"double", !"double", !"int"}
!17 = !{i32 16}
!18 = !{ptr @__Vectorized_._ZTS17mandelbrot_kernelIdE}
!19 = !{i1 true}
!20 = !{null}
!21 = !{i32 1}
!22 = !{!"pixel_block_t*", !"cl::sycl::range<1>", !"cl::sycl::range<1>", !"cl::sycl::id<1>", !"cl::sycl::range<2>", !"cl::sycl::range<2>", !"cl::sycl::uchar4", !"int", !"int", !"uint32_t", !"float", !"float", !"float", !"int"}
!23 = !{!"struct _ZTS13pixel_block_t.pixel_block_t*", !"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range", !"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range", !"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range", !"class._ZTSN2cl4sycl5rangeILi2EEE.cl::sycl::range", !"class._ZTSN2cl4sycl5rangeILi2EEE.cl::sycl::range", !"class._ZTSN2cl4sycl3vecIhLi4EEE.cl::sycl::vec", !"int", !"int", !"int", !"float", !"float", !"float", !"int"}
!25 = !{ptr @_ZTS17mandelbrot_kernelIdE}
!26 = !{i32 0}
!27 = !{i1 false}
!28 = !{ptr addrspace(1) null, ptr null, ptr null, ptr null, ptr null, ptr null, ptr null, i32 0, i32 0, i32 0, double zeroinitializer, double zeroinitializer, double zeroinitializer, i32 0}

; The get_max_sub_group_size is replaced with constant and related debug
; location will be lost as expected.
; DEBUGIFY: Missing line 9
; DEBUGIFY-NOT: WARNING
