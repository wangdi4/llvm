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

!0 = metadata !{void (<4 x float> addrspace(1)*, %opencl.image2d_t.0 addrspace(1)*, <2 x i32>, i8 addrspace(3)*, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }*, i64*, [4 x i64], i8*, {}*)* @__test_const_as_global_separated_args, metadata !1, metadata !2, metadata !3, metadata !4, metadata !5, metadata !6}
!1 = metadata !{metadata !"kernel_arg_addr_space", i32 1, i32 1, i32 0}
!2 = metadata !{metadata !"kernel_arg_access_qual", metadata !"none", metadata !"read_only", metadata !"none"}
!3 = metadata !{metadata !"kernel_arg_type", metadata !"float4*", metadata !"image2d_t", metadata !"int2"}
!4 = metadata !{metadata !"kernel_arg_type_qual", metadata !"", metadata !"", metadata !""}
!5 = metadata !{metadata !"kernel_arg_base_type", metadata !"float4*", metadata !"image2d_t", metadata !"int2"}
!6 = metadata !{metadata !"kernel_arg_name", metadata !"out", metadata !"image", metadata !"coord"}
!7 = metadata !{i32 1, i32 2}
!8 = metadata !{}
!9 = metadata !{metadata !"cl_images"}
!10 = metadata !{void (<4 x float> addrspace(1)*, %opencl.image2d_t.0 addrspace(1)*, <2 x i32>, i8 addrspace(3)*, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }*, i64*, [4 x i64], i8*, {}*)* @__test_const_as_global_separated_args, metadata !11}
!11 = metadata !{metadata !12, metadata !13, metadata !14, metadata !15, metadata !16, metadata !17, metadata !18, metadata !19, metadata !20, metadata !21, metadata !22, metadata !23, metadata !24, metadata !25, metadata !26}
!12 = metadata !{metadata !"local_buffer_size", i32 0}
!13 = metadata !{metadata !"barrier_buffer_size", i32 0}
!14 = metadata !{metadata !"kernel_execution_length", i32 3}
!15 = metadata !{metadata !"max_wg_dimensions", i32 0}
!16 = metadata !{metadata !"kernel_has_barrier", i1 false}
!17 = metadata !{metadata !"kernel_has_global_sync", i1 false}
!18 = metadata !{metadata !"no_barrier_path", i1 true}
!19 = metadata !{metadata !"vectorized_kernel", null}
!20 = metadata !{metadata !"vectorized_width", i32 8}
!21 = metadata !{metadata !"kernel_wrapper", void (i8*, i64*, {}*)* @test_const_as_global}
!22 = metadata !{metadata !"scalarized_kernel", null}
!23 = metadata !{metadata !"block_literal_size", null}
!24 = metadata !{metadata !"private_memory_size", i32 0}
!25 = metadata !{metadata !"vectorization_dimension", i32 0}
!26 = metadata !{metadata !"can_unite_workgroups", i1 true}
!27 = metadata !{metadata !28, metadata !29, metadata !30}
!28 = metadata !{metadata !"global_variable_total_size", i64 0}
!29 = metadata !{metadata !"gen_addr_space_pointer_counter", null}
!30 = metadata !{metadata !"gen_addr_space_pointer_warnings"}
!31 = metadata !{metadata !"any pointer", metadata !32}
!32 = metadata !{metadata !"omnipotent char", metadata !33}
!33 = metadata !{metadata !"Simple C/C++ TBAA"}
