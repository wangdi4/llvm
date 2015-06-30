; RUN: opt -module-cleanup -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll

; CHECK-NOT:    @coord_translate_i_callback

%opencl.image2d_t.0 = type opaque

@sampler = constant i32 20, align 4
@coord_translate_i_callback = addrspace(2) constant [64 x <4 x i32> (i8*, <4 x i32>)*] [<4 x i32> (i8*, <4 x i32>)* @_Z25trans_coord_int_UNDEFINEDPvDv4_i, <4 x i32> (i8*, <4 x i32>)* @_Z25trans_coord_int_UNDEFINEDPvDv4_i, <4 x i32> (i8*, <4 x i32>)* @_Z25trans_coord_int_UNDEFINEDPvDv4_i, <4 x i32> (i8*, <4 x i32>)* @_Z25trans_coord_int_UNDEFINEDPvDv4_i, <4 x i32> (i8*, <4 x i32>)* @_Z25trans_coord_int_UNDEFINEDPvDv4_i, <4 x i32> (i8*, <4 x i32>)* @_Z25trans_coord_int_UNDEFINEDPvDv4_i, <4 x i32> (i8*, <4 x i32>)* @_Z25trans_coord_int_UNDEFINEDPvDv4_i, <4 x i32> (i8*, <4 x i32>)* @_Z25trans_coord_int_UNDEFINEDPvDv4_i, <4 x i32> (i8*, <4 x i32>)* @_Z25trans_coord_int_UNDEFINEDPvDv4_i, <4 x i32> (i8*, <4 x i32>)* @_Z25trans_coord_int_UNDEFINEDPvDv4_i, <4 x i32> (i8*, <4 x i32>)* @_Z25trans_coord_int_UNDEFINEDPvDv4_i, <4 x i32> (i8*, <4 x i32>)* @_Z25trans_coord_int_UNDEFINEDPvDv4_i, <4 x i32> (i8*, <4 x i32>)* @_Z25trans_coord_int_UNDEFINEDPvDv4_i, <4 x i32> (i8*, <4 x i32>)* @_Z25trans_coord_int_UNDEFINEDPvDv4_i, <4 x i32> (i8*, <4 x i32>)* @_Z25trans_coord_int_UNDEFINEDPvDv4_i, <4 x i32> (i8*, <4 x i32>)* @_Z25trans_coord_int_UNDEFINEDPvDv4_i, <4 x i32> (i8*, <4 x i32>)* @_Z34trans_coord_int_NONE_FALSE_NEARESTPvDv4_i, <4 x i32> (i8*, <4 x i32>)* @_Z25trans_coord_int_UNDEFINEDPvDv4_i, <4 x i32> (i8*, <4 x i32>)* @_Z41trans_coord_int_CLAMPTOEDGE_FALSE_NEARESTPvDv4_i, <4 x i32> (i8*, <4 x i32>)* @_Z25trans_coord_int_UNDEFINEDPvDv4_i, <4 x i32> (i8*, <4 x i32>)* @_Z34trans_coord_int_NONE_FALSE_NEARESTPvDv4_i, <4 x i32> (i8*, <4 x i32>)* @_Z25trans_coord_int_UNDEFINEDPvDv4_i, <4 x i32> (i8*, <4 x i32>)* @_Z25trans_coord_int_UNDEFINEDPvDv4_i, <4 x i32> (i8*, <4 x i32>)* @_Z25trans_coord_int_UNDEFINEDPvDv4_i, <4 x i32> (i8*, <4 x i32>)* @_Z25trans_coord_int_UNDEFINEDPvDv4_i, <4 x i32> (i8*, <4 x i32>)* @_Z25trans_coord_int_UNDEFINEDPvDv4_i, <4 x i32> (i8*, <4 x i32>)* @_Z25trans_coord_int_UNDEFINEDPvDv4_i, <4 x i32> (i8*, <4 x i32>)* @_Z25trans_coord_int_UNDEFINEDPvDv4_i, <4 x i32> (i8*, <4 x i32>)* @_Z25trans_coord_int_UNDEFINEDPvDv4_i, <4 x i32> (i8*, <4 x i32>)* @_Z25trans_coord_int_UNDEFINEDPvDv4_i, <4 x i32> (i8*, <4 x i32>)* @_Z25trans_coord_int_UNDEFINEDPvDv4_i, <4 x i32> (i8*, <4 x i32>)* @_Z25trans_coord_int_UNDEFINEDPvDv4_i, <4 x i32> (i8*, <4 x i32>)* @_Z25trans_coord_int_UNDEFINEDPvDv4_i, <4 x i32> (i8*, <4 x i32>)* @_Z25trans_coord_int_UNDEFINEDPvDv4_i, <4 x i32> (i8*, <4 x i32>)* @_Z25trans_coord_int_UNDEFINEDPvDv4_i, <4 x i32> (i8*, <4 x i32>)* @_Z25trans_coord_int_UNDEFINEDPvDv4_i, <4 x i32> (i8*, <4 x i32>)* @_Z25trans_coord_int_UNDEFINEDPvDv4_i, <4 x i32> (i8*, <4 x i32>)* @_Z25trans_coord_int_UNDEFINEDPvDv4_i, <4 x i32> (i8*, <4 x i32>)* @_Z25trans_coord_int_UNDEFINEDPvDv4_i, <4 x i32> (i8*, <4 x i32>)* @_Z25trans_coord_int_UNDEFINEDPvDv4_i, <4 x i32> (i8*, <4 x i32>)* @_Z25trans_coord_int_UNDEFINEDPvDv4_i, <4 x i32> (i8*, <4 x i32>)* @_Z25trans_coord_int_UNDEFINEDPvDv4_i, <4 x i32> (i8*, <4 x i32>)* @_Z25trans_coord_int_UNDEFINEDPvDv4_i, <4 x i32> (i8*, <4 x i32>)* @_Z25trans_coord_int_UNDEFINEDPvDv4_i, <4 x i32> (i8*, <4 x i32>)* @_Z25trans_coord_int_UNDEFINEDPvDv4_i, <4 x i32> (i8*, <4 x i32>)* @_Z25trans_coord_int_UNDEFINEDPvDv4_i, <4 x i32> (i8*, <4 x i32>)* @_Z25trans_coord_int_UNDEFINEDPvDv4_i, <4 x i32> (i8*, <4 x i32>)* @_Z25trans_coord_int_UNDEFINEDPvDv4_i, <4 x i32> (i8*, <4 x i32>)* @_Z25trans_coord_int_UNDEFINEDPvDv4_i, <4 x i32> (i8*, <4 x i32>)* @_Z25trans_coord_int_UNDEFINEDPvDv4_i, <4 x i32> (i8*, <4 x i32>)* @_Z25trans_coord_int_UNDEFINEDPvDv4_i, <4 x i32> (i8*, <4 x i32>)* @_Z25trans_coord_int_UNDEFINEDPvDv4_i, <4 x i32> (i8*, <4 x i32>)* @_Z25trans_coord_int_UNDEFINEDPvDv4_i, <4 x i32> (i8*, <4 x i32>)* @_Z25trans_coord_int_UNDEFINEDPvDv4_i, <4 x i32> (i8*, <4 x i32>)* @_Z25trans_coord_int_UNDEFINEDPvDv4_i, <4 x i32> (i8*, <4 x i32>)* @_Z25trans_coord_int_UNDEFINEDPvDv4_i, <4 x i32> (i8*, <4 x i32>)* @_Z25trans_coord_int_UNDEFINEDPvDv4_i, <4 x i32> (i8*, <4 x i32>)* @_Z25trans_coord_int_UNDEFINEDPvDv4_i, <4 x i32> (i8*, <4 x i32>)* @_Z25trans_coord_int_UNDEFINEDPvDv4_i, <4 x i32> (i8*, <4 x i32>)* @_Z25trans_coord_int_UNDEFINEDPvDv4_i, <4 x i32> (i8*, <4 x i32>)* @_Z25trans_coord_int_UNDEFINEDPvDv4_i, <4 x i32> (i8*, <4 x i32>)* @_Z25trans_coord_int_UNDEFINEDPvDv4_i, <4 x i32> (i8*, <4 x i32>)* @_Z25trans_coord_int_UNDEFINEDPvDv4_i, <4 x i32> (i8*, <4 x i32>)* @_Z25trans_coord_int_UNDEFINEDPvDv4_i], align 16

; Function Attrs: alwaysinline nounwind
declare void @__test_const_as_global_separated_args(<4 x float> addrspace(1)* nocapture, %opencl.image2d_t.0 addrspace(1)*, <2 x i32>, i8 addrspace(3)* noalias, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }* noalias, i64* noalias, [4 x i64], i8* noalias, {}* noalias)

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z34trans_coord_int_NONE_FALSE_NEARESTPvDv4_i(i8* nocapture, <4 x i32>)

; Function Attrs: nounwind readonly
declare <4 x i32> @_Z41trans_coord_int_CLAMPTOEDGE_FALSE_NEARESTPvDv4_i(i8* nocapture, <4 x i32>)

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z25trans_coord_int_UNDEFINEDPvDv4_i(i8* nocapture, <4 x i32>)

define void @test_const_as_global(i8* noalias %pUniformArgs, i64* noalias %pWGId, {}* noalias %RuntimeHandle) {
  ret void
}

!opencl.kernels = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.spir.version = !{!7}
!opencl.ocl.version = !{!7}
!opencl.used.extensions = !{!8}
!opencl.used.optional.core.features = !{!9}
!opencl.compiler.options = !{!8}
!opencl.kernel_info = !{!10}
!opencl.module_info_list = !{!27}
!llvm.functions_info = !{}
!opencl.functions_stats = !{}
!opencl.stat_descriptions = !{}
!opencl.module_stat_info = !{}

!0 = !{void (<4 x float> addrspace(1)*, %opencl.image2d_t.0 addrspace(1)*, <2 x i32>, i8 addrspace(3)*, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }*, i64*, [4 x i64], i8*, {}*)* @__test_const_as_global_separated_args, !1, !2, !3, !4, !5, !6}
!1 = !{!"kernel_arg_addr_space", i32 1, i32 1, i32 0}
!2 = !{!"kernel_arg_access_qual", !"none", !"read_only", !"none"}
!3 = !{!"kernel_arg_type", !"float4*", !"image2d_t", !"int2"}
!4 = !{!"kernel_arg_type_qual", !"", !"", !""}
!5 = !{!"kernel_arg_base_type", !"float4*", !"image2d_t", !"int2"}
!6 = !{!"kernel_arg_name", !"out", !"image", !"coord"}
!7 = !{i32 1, i32 2}
!8 = !{}
!9 = !{!"cl_images"}
!10 = !{void (<4 x float> addrspace(1)*, %opencl.image2d_t.0 addrspace(1)*, <2 x i32>, i8 addrspace(3)*, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }*, i64*, [4 x i64], i8*, {}*)* @__test_const_as_global_separated_args, !11}
!11 = !{!12, !13, !14, !15, !16, !17, !18, !19, !20, !21, !22, !23, !24, !25, !26}
!12 = !{!"local_buffer_size", i32 0}
!13 = !{!"barrier_buffer_size", i32 0}
!14 = !{!"kernel_execution_length", i32 3}
!15 = !{!"max_wg_dimensions", i32 0}
!16 = !{!"kernel_has_barrier", i1 false}
!17 = !{!"kernel_has_global_sync", i1 false}
!18 = !{!"no_barrier_path", i1 true}
!19 = !{!"vectorized_kernel", null}
!20 = !{!"vectorized_width", i32 8}
!21 = !{!"kernel_wrapper", void (i8*, i64*, {}*)* @test_const_as_global}
!22 = !{!"scalarized_kernel", null}
!23 = !{!"block_literal_size", null}
!24 = !{!"private_memory_size", i32 0}
!25 = !{!"vectorization_dimension", i32 0}
!26 = !{!"can_unite_workgroups", i1 true}
!27 = !{!28, !29, !30}
!28 = !{!"global_variable_total_size", i64 0}
!29 = !{!"gen_addr_space_pointer_counter", null}
!30 = !{!"gen_addr_space_pointer_warnings"}
!31 = !{!"any pointer", !32}
!32 = !{!"omnipotent char", !33}
!33 = !{!"Simple C/C++ TBAA"}
