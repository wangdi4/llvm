;; Test that SOA versions of write only read_imageui built-ins are used for vectorization.
;;
;; LLVM IR was generated with the following command
;; bash$ ./clang -cc1 -x cl -cl-std=CL2.0 -triple spir64-unknown-unknown -include opencl-c.h -emit-llvm soa_read_imageui-wo.cl -o -

; RUN: %oclopt -runtimelib %p/../Full/runtime.bc -scalarize -predicate -packetize -packet-size=4 -verify %s -S -o - | FileCheck -check-prefix=CHECK_4 %s
; RUN: %oclopt -runtimelib %p/../Full/runtime.bc -scalarize -predicate -packetize -packet-size=8 -verify %s -S -o - | FileCheck -check-prefix=CHECK_8 %s

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown"

%opencl.image2d_wo_t = type opaque

; Function Attrs: nounwind
define spir_kernel void @soa_read_imageui(%opencl.image2d_wo_t addrspace(1)* %img, <4 x i32> addrspace(1)* nocapture readonly %in, i32 %randomId) #0 {
entry:
  %call = tail call spir_func i64 @_Z12get_local_idj(i32 0) #3
  %conv = trunc i64 %call to i32
  %call1 = tail call spir_func i64 @_Z12get_local_idj(i32 1) #3
  %conv2 = trunc i64 %call1 to i32
  %conv3 = and i64 %call, 4294967295
  %call4 = tail call spir_func i64 @_Z14get_local_sizej(i32 0) #3
  %conv5 = and i64 %call1, 4294967295
  %mul = mul i64 %call4, %conv5
  %add = add i64 %mul, %conv3
  %arrayidx = getelementptr inbounds <4 x i32>, <4 x i32> addrspace(1)* %in, i64 %add
  %0 = load <4 x i32>, <4 x i32> addrspace(1)* %arrayidx, align 16, !tbaa !9
  %vecinit = insertelement <2 x i32> undef, i32 %conv, i32 0
  %vecinit6 = insertelement <2 x i32> %vecinit, i32 %conv2, i32 1

; CHECK_4-NOT:    call {{.*}} @_Z13write_imageui14ocl_image2d_woDv2_iDv4_j
; CHECK_8-NOT:    call {{.*}} @_Z13write_imageui14ocl_image2d_woDv2_iDv4_j

; CHECK_4:        call void @_Z18soa4_write_imageui14ocl_image2d_woDv4_iS0_Dv4_jS1_S1_S1_
; CHECK_8:        call void @_Z18soa8_write_imageui14ocl_image2d_woDv8_iS0_Dv8_jS1_S1_S1_

; CHECK_4-NOT:    call {{.*}} @_Z13write_imageui14ocl_image2d_woDv2_iDv4_j
; CHECK_8-NOT:    call {{.*}} @_Z13write_imageui14ocl_image2d_woDv2_iDv4_j

  tail call spir_func void @_Z13write_imageui14ocl_image2d_woDv2_iDv4_j(%opencl.image2d_wo_t addrspace(1)* %img, <2 x i32> %vecinit6, <4 x i32> %0) #4
  %rem = urem i32 %conv, %randomId
  %cmp = icmp eq i32 %rem, 0
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %mul11 = shl <4 x i32> %0, <i32 1, i32 1, i32 1, i32 1>

; CHECK_4-NOT:    call {{.*}} @_Z13write_imageui14ocl_image2d_woDv2_iDv4_j
; CHECK_8-NOT:    call {{.*}} @_Z13write_imageui14ocl_image2d_woDv2_iDv4_j

; CHECK_4:        call void @_Z23mask_soa4_write_imageuiDv4_i14ocl_image2d_woS_S_Dv4_jS1_S1_S1_
; CHECK_8:        call void @_Z23mask_soa8_write_imageuiDv8_i14ocl_image2d_woS_S_Dv8_jS1_S1_S1_

; CHECK_4-NOT:    call {{.*}} @_Z13write_imageui14ocl_image2d_woDv2_iDv4_j
; CHECK_8-NOT:    call {{.*}} @_Z13write_imageui14ocl_image2d_woDv2_iDv4_j

  tail call spir_func void @_Z13write_imageui14ocl_image2d_woDv2_iDv4_j(%opencl.image2d_wo_t addrspace(1)* %img, <2 x i32> %vecinit6, <4 x i32> %mul11) #4
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  ret void
}

; Function Attrs: nounwind readnone
declare spir_func i64 @_Z12get_local_idj(i32) #1

; Function Attrs: nounwind readnone
declare spir_func i64 @_Z14get_local_sizej(i32) #1

declare spir_func void @_Z13write_imageui14ocl_image2d_woDv2_iDv4_j(%opencl.image2d_wo_t addrspace(1)*, <2 x i32>, <4 x i32>) #2

attributes #0 = { nounwind "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind readnone }
attributes #4 = { nounwind }

!sycl.kernels = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!6}
!opencl.spir.version = !{!6}
!opencl.used.extensions = !{!7}
!opencl.used.optional.core.features = !{!7}
!opencl.compiler.options = !{!7}
!llvm.ident = !{!8}

!0 = !{void (%opencl.image2d_wo_t addrspace(1)*, <4 x i32> addrspace(1)*, i32)* @soa_read_imageui, !1, !2, !3, !4, !5}
!1 = !{!"kernel_arg_addr_space", i32 1, i32 1, i32 0}
!2 = !{!"kernel_arg_access_qual", !"write_only", !"none", !"none"}
!3 = !{!"kernel_arg_type", !"image2d_t", !"uint4*", !"uint"}
!4 = !{!"kernel_arg_base_type", !"image2d_t", !"uint4*", !"uint"}
!5 = !{!"kernel_arg_type_qual", !"", !"", !""}
!6 = !{i32 2, i32 0}
!7 = !{}
!8 = !{!"clang version 3.8.1 (ssh://nnopencl-git-01.inn.intel.com/home/git/repo/opencl_qa-clang 2ce0c12736eb1ecfe73afb80c01348289e7c5306) (ssh://nnopencl-git-01.inn.intel.com/home/git/repo/opencl_qa-llvm db18480feecc56e231983ff0c6f48bb7ca5d9689)"}
!9 = !{!10, !10, i64 0}
!10 = !{!"omnipotent char", !11, i64 0}
!11 = !{!"Simple C/C++ TBAA"}
