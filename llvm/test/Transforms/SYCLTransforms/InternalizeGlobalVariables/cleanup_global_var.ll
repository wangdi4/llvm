; RUN: opt -passes=sycl-kernel-internalize-global-variables,globaldce -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-internalize-global-variables,globaldce -S %s | FileCheck %s

; CHECK: @coord_translate_i_callback

@sampler = constant i32 20, align 4
@coord_translate_i_callback = addrspace(2) constant [64 x ptr] [ptr @_Z25trans_coord_int_UNDEFINEDPvDv4_i, ptr @_Z25trans_coord_int_UNDEFINEDPvDv4_i, ptr @_Z25trans_coord_int_UNDEFINEDPvDv4_i, ptr @_Z25trans_coord_int_UNDEFINEDPvDv4_i, ptr @_Z25trans_coord_int_UNDEFINEDPvDv4_i, ptr @_Z25trans_coord_int_UNDEFINEDPvDv4_i, ptr @_Z25trans_coord_int_UNDEFINEDPvDv4_i, ptr @_Z25trans_coord_int_UNDEFINEDPvDv4_i, ptr @_Z25trans_coord_int_UNDEFINEDPvDv4_i, ptr @_Z25trans_coord_int_UNDEFINEDPvDv4_i, ptr @_Z25trans_coord_int_UNDEFINEDPvDv4_i, ptr @_Z25trans_coord_int_UNDEFINEDPvDv4_i, ptr @_Z25trans_coord_int_UNDEFINEDPvDv4_i, ptr @_Z25trans_coord_int_UNDEFINEDPvDv4_i, ptr @_Z25trans_coord_int_UNDEFINEDPvDv4_i, ptr @_Z25trans_coord_int_UNDEFINEDPvDv4_i, ptr @_Z34trans_coord_int_NONE_FALSE_NEARESTPvDv4_i, ptr @_Z25trans_coord_int_UNDEFINEDPvDv4_i, ptr @_Z41trans_coord_int_CLAMPTOEDGE_FALSE_NEARESTPvDv4_i, ptr @_Z25trans_coord_int_UNDEFINEDPvDv4_i, ptr @_Z34trans_coord_int_NONE_FALSE_NEARESTPvDv4_i, ptr @_Z25trans_coord_int_UNDEFINEDPvDv4_i, ptr @_Z25trans_coord_int_UNDEFINEDPvDv4_i, ptr @_Z25trans_coord_int_UNDEFINEDPvDv4_i, ptr @_Z25trans_coord_int_UNDEFINEDPvDv4_i, ptr @_Z25trans_coord_int_UNDEFINEDPvDv4_i, ptr @_Z25trans_coord_int_UNDEFINEDPvDv4_i, ptr @_Z25trans_coord_int_UNDEFINEDPvDv4_i, ptr @_Z25trans_coord_int_UNDEFINEDPvDv4_i, ptr @_Z25trans_coord_int_UNDEFINEDPvDv4_i, ptr @_Z25trans_coord_int_UNDEFINEDPvDv4_i, ptr @_Z25trans_coord_int_UNDEFINEDPvDv4_i, ptr @_Z25trans_coord_int_UNDEFINEDPvDv4_i, ptr @_Z25trans_coord_int_UNDEFINEDPvDv4_i, ptr @_Z25trans_coord_int_UNDEFINEDPvDv4_i, ptr @_Z25trans_coord_int_UNDEFINEDPvDv4_i, ptr @_Z25trans_coord_int_UNDEFINEDPvDv4_i, ptr @_Z25trans_coord_int_UNDEFINEDPvDv4_i, ptr @_Z25trans_coord_int_UNDEFINEDPvDv4_i, ptr @_Z25trans_coord_int_UNDEFINEDPvDv4_i, ptr @_Z25trans_coord_int_UNDEFINEDPvDv4_i, ptr @_Z25trans_coord_int_UNDEFINEDPvDv4_i, ptr @_Z25trans_coord_int_UNDEFINEDPvDv4_i, ptr @_Z25trans_coord_int_UNDEFINEDPvDv4_i, ptr @_Z25trans_coord_int_UNDEFINEDPvDv4_i, ptr @_Z25trans_coord_int_UNDEFINEDPvDv4_i, ptr @_Z25trans_coord_int_UNDEFINEDPvDv4_i, ptr @_Z25trans_coord_int_UNDEFINEDPvDv4_i, ptr @_Z25trans_coord_int_UNDEFINEDPvDv4_i, ptr @_Z25trans_coord_int_UNDEFINEDPvDv4_i, ptr @_Z25trans_coord_int_UNDEFINEDPvDv4_i, ptr @_Z25trans_coord_int_UNDEFINEDPvDv4_i, ptr @_Z25trans_coord_int_UNDEFINEDPvDv4_i, ptr @_Z25trans_coord_int_UNDEFINEDPvDv4_i, ptr @_Z25trans_coord_int_UNDEFINEDPvDv4_i, ptr @_Z25trans_coord_int_UNDEFINEDPvDv4_i, ptr @_Z25trans_coord_int_UNDEFINEDPvDv4_i, ptr @_Z25trans_coord_int_UNDEFINEDPvDv4_i, ptr @_Z25trans_coord_int_UNDEFINEDPvDv4_i, ptr @_Z25trans_coord_int_UNDEFINEDPvDv4_i, ptr @_Z25trans_coord_int_UNDEFINEDPvDv4_i, ptr @_Z25trans_coord_int_UNDEFINEDPvDv4_i, ptr @_Z25trans_coord_int_UNDEFINEDPvDv4_i, ptr @_Z25trans_coord_int_UNDEFINEDPvDv4_i], align 16

; Function Attrs: alwaysinline nounwind
define void @__test_const_as_global_separated_args(ptr addrspace(1) nocapture, ptr addrspace(1), <2 x i32>, ptr addrspace(3) noalias, ptr noalias, ptr noalias, [4 x i64], ptr noalias, ptr noalias) !kernel_arg_addr_space !1 !kernel_arg_access_qual !2 !kernel_arg_type !3 !kernel_arg_base_type !4 !kernel_arg_type_qual !5 !kernel_arg_name !6 !arg_type_null_val !9 {
  ret void
}

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z34trans_coord_int_NONE_FALSE_NEARESTPvDv4_i(ptr nocapture, <4 x i32>)

; Function Attrs: nounwind readonly
declare <4 x i32> @_Z41trans_coord_int_CLAMPTOEDGE_FALSE_NEARESTPvDv4_i(ptr nocapture, <4 x i32>)

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z25trans_coord_int_UNDEFINEDPvDv4_i(ptr nocapture, <4 x i32>)

define void @test_const_as_global(ptr noalias %pUniformArgs, ptr noalias %pWGId, ptr noalias %RuntimeHandle) {
  ret void
}

!opencl.enable.FP_CONTRACT = !{}
!opencl.spir.version = !{!7}
!opencl.ocl.version = !{!7}
!opencl.used.extensions = !{!8}
!opencl.compiler.options = !{!8}
!llvm.functions_info = !{}
!opencl.functions_stats = !{}
!opencl.stat_descriptions = !{}
!opencl.module_stat_info = !{}

!1 = !{i32 1, i32 1, i32 0}
!2 = !{!"none", !"read_only", !"none"}
!3 = !{!"float4*", !"image2d_t", !"int2"}
!4 = !{!"float4*", !"image2d_t", !"int2"}
!5 = !{!"", !"", !""}
!6 = !{!"out", !"image", !"coord"}
!7 = !{i32 1, i32 2}
!8 = !{}
!9 = !{ptr addrspace(1) null, target("spirv.Image", void, 0, 0, 0, 0, 0, 0, 2) zeroinitializer, <2 x i32> <i32 0, i32 0>}

; DEBUGIFY-NOT: WARNING
