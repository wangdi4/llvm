;; Test what SOA versions of read_imageui built-ins are used for vectorization
;;
;; LLVM IR was generated with the following command
;; bash$ ./clang -cc1 -x cl -O2 -cl-std=CL2.0 -triple spir64-unknonw-unknown -include opencl.h -emit-llvm soa_read_imageui.cl -o -

; RUN: opt -runtimelib %p/../Full/runtime.bc -scalarize -predicate -packetize -packet-size=4 -verify %s -S -o - | FileCheck -check-prefix=CHECK_4 %s
; RUN: opt -runtimelib %p/../Full/runtime.bc -scalarize -predicate -packetize -packet-size=8 -verify %s -S -o - | FileCheck -check-prefix=CHECK_8 %s

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknonw-unknown"

%opencl.image2d_t = type opaque

; CHECK_4-DAG-NOT:    @_Z12read_imageui11ocl_image2dDv2_i
; CHECK_4-DAG-NOT:    @_Z12read_imageui11ocl_image2d11ocl_samplerDv2_i
; CHECK_8-DAG-NOT:    @_Z12read_imageui11ocl_image2dDv2_i
; CHECK_8-DAG-NOT:    @_Z12read_imageui11ocl_image2d11ocl_samplerDv2_i

; Function Attrs: nounwind
define spir_kernel void @soa_read_imageui(%opencl.image2d_t addrspace(1)* %img, <4 x i32> addrspace(1)* nocapture %out, i32 %randomId) #0 {
  %1 = tail call spir_func i64 @_Z12get_local_idj(i32 0) #2
  %2 = trunc i64 %1 to i32
  %3 = tail call spir_func i64 @_Z12get_local_idj(i32 1) #2
  %4 = trunc i64 %3 to i32
  %5 = insertelement <2 x i32> undef, i32 %2, i32 0
  %6 = insertelement <2 x i32> %5, i32 %4, i32 1
; CHECK_4:    call void @_Z17soa4_read_imageui11ocl_image2dDv4_iS_PDv4_jS1_S1_S1_
; CHECK_8:    call void @_Z17soa8_read_imageui11ocl_image2dDv8_iS_PDv8_jS1_S1_S1_
  %7 = tail call spir_func <4 x i32> @_Z12read_imageui11ocl_image2dDv2_i(%opencl.image2d_t addrspace(1)* %img, <2 x i32> %6) #2
; CHECK_4:    call void @_Z17soa4_read_imageui11ocl_image2d11ocl_samplerDv4_iS_PDv4_jS1_S1_S1_
; CHECK_8:    call void @_Z17soa8_read_imageui11ocl_image2d11ocl_samplerDv8_iS_PDv8_jS1_S1_S1_
  %8 = tail call spir_func <4 x i32> @_Z12read_imageui11ocl_image2d11ocl_samplerDv2_i(%opencl.image2d_t addrspace(1)* %img, i32 16, <2 x i32> %6) #2
  %9 = add <4 x i32> %7, %8
  %10 = urem i32 %2, %randomId
  %11 = icmp eq i32 %10, 0
  br i1 %11, label %12, label %19

; <label>:12                                      ; preds = %0
  %13 = insertelement <2 x i32> undef, i32 %4, i32 0
  %14 = insertelement <2 x i32> %13, i32 %2, i32 1
; CHECK_4:    call void @_Z22mask_soa4_read_imageuiDv4_i11ocl_image2dS_S_PDv4_jS1_S1_S1_
; CHECK_8:    call void @_Z22mask_soa8_read_imageuiDv8_i11ocl_image2dS_S_PDv8_jS1_S1_S1_
  %15 = tail call spir_func <4 x i32> @_Z12read_imageui11ocl_image2dDv2_i(%opencl.image2d_t addrspace(1)* %img, <2 x i32> %14) #2
  %16 = add <4 x i32> %9, %15
; CHECK_4:    call void @_Z22mask_soa4_read_imageuiDv4_i11ocl_image2d11ocl_samplerS_S_PDv4_jS1_S1_S1_
; CHECK_8:    call void @_Z22mask_soa8_read_imageuiDv8_i11ocl_image2d11ocl_samplerS_S_PDv8_jS1_S1_S1_
  %17 = tail call spir_func <4 x i32> @_Z12read_imageui11ocl_image2d11ocl_samplerDv2_i(%opencl.image2d_t addrspace(1)* %img, i32 16, <2 x i32> %14) #2
  %18 = add <4 x i32> %16, %17
  br label %19

; <label>:19                                      ; preds = %12, %0
  %res.0 = phi <4 x i32> [ %18, %12 ], [ %9, %0 ]
  %20 = and i64 %1, 4294967295
  %21 = tail call spir_func i64 @_Z14get_local_sizej(i32 0) #2
  %22 = and i64 %3, 4294967295
  %23 = mul i64 %21, %22
  %24 = add i64 %23, %20
  %25 = getelementptr inbounds <4 x i32> addrspace(1)* %out, i64 %24
  store <4 x i32> %res.0, <4 x i32> addrspace(1)* %25, align 16, !tbaa !11
  ret void
}

; Function Attrs: nounwind readnone
declare spir_func i64 @_Z12get_local_idj(i32) #1

; Function Attrs: nounwind readnone
declare spir_func <4 x i32> @_Z12read_imageui11ocl_image2dDv2_i(%opencl.image2d_t addrspace(1)*, <2 x i32>) #1

; Function Attrs: nounwind readnone
declare spir_func <4 x i32> @_Z12read_imageui11ocl_image2d11ocl_samplerDv2_i(%opencl.image2d_t addrspace(1)*, i32, <2 x i32>) #1

; Function Attrs: nounwind readnone
declare spir_func i64 @_Z14get_local_sizej(i32) #1

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind readnone }

!opencl.kernels = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.spir.version = !{!6}
!opencl.ocl.version = !{!7}
!opencl.used.extensions = !{!8}
!opencl.used.optional.core.features = !{!9}
!opencl.compiler.options = !{!8}
!llvm.ident = !{!10}

!0 = !{void (%opencl.image2d_t addrspace(1)*, <4 x i32> addrspace(1)*, i32)* @soa_read_imageui, !1, !2, !3, !4, !5}
!1 = !{!"kernel_arg_addr_space", i32 1, i32 1, i32 0}
!2 = !{!"kernel_arg_access_qual", !"read_only", !"none", !"none"}
!3 = !{!"kernel_arg_type", !"image2d_t", !"uint4*", !"uint"}
!4 = !{!"kernel_arg_base_type", !"image2d_t", !"uint4*", !"uint"}
!5 = !{!"kernel_arg_type_qual", !"", !"", !""}
!6 = !{i32 1, i32 2}
!7 = !{i32 2, i32 0}
!8 = !{}
!9 = !{!"cl_images"}
!10 = !{!"clang version 3.6.1 (ssh://nnopencl-git-01.inn.intel.com/home/git/repo/opencl_qa-clang dbd6da456b6bfedded6a7e23b7f77a3a5545b928) (ssh://nnopencl-git-01.inn.intel.com/home/git/repo/opencl_qa-llvm 3989a5c3152680fc6b18775d7940a5379bb0cd57)"}
!11 = !{!12, !12, i64 0}
!12 = !{!"omnipotent char", !13, i64 0}
!13 = !{!"Simple C/C++ TBAA"}
