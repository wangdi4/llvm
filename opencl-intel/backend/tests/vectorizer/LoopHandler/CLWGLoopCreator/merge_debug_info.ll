; RUN: opt -cl-loop-creator -verify %s -S -o %t.merged.ll
; RUN: FileCheck %s --input-file=%t.merged.ll
; ModuleID = 'Program'
;
; Test what the vector subprogram DIE is removed along w\ the vectorized kernel
; and only the scalar subprogram DIE remains in the module
;
; CHECK:       define void @test
; CHECK-NOT:   define void @__Vectorized_.test
; CHECK:       @test{{.+}}DW_TAG_subprogram
; CHECK-NOT:   @__Vectorized_.test{{.+}}DW_TAG_subprogram

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: nounwind
define void @test(float addrspace(1)* nocapture %lhs, float addrspace(1)* nocapture %rhs, float addrspace(1)* nocapture %out) #0 {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) #2, !dbg !53
  %arrayidx = getelementptr inbounds float addrspace(1)* %rhs, i64 %call, !dbg !54
  %0 = load float addrspace(1)* %arrayidx, align 1, !dbg !54
  %arrayidx1 = getelementptr inbounds float addrspace(1)* %lhs, i64 %call, !dbg !54
  %1 = load float addrspace(1)* %arrayidx1, align 1, !dbg !54
  %mul = fmul float %0, %1, !dbg !54
  %arrayidx2 = getelementptr inbounds float addrspace(1)* %out, i64 %call, !dbg !54
  store float %mul, float addrspace(1)* %arrayidx2, align 1, !dbg !54
  ret void, !dbg !55
}

; Function Attrs: nounwind readnone
declare i64 @_Z13get_global_idj(i32) #1

; Function Attrs: nounwind readnone
declare void @llvm.dbg.value(metadata, i64, metadata) #2

define [7 x i64] @WG.boundaries.test(float addrspace(1)*, float addrspace(1)*, float addrspace(1)*) {
entry:
  %3 = call i64 @_Z14get_local_sizej(i32 0)
  %4 = call i64 @get_base_global_id.(i32 0)
  %5 = call i64 @_Z14get_local_sizej(i32 1)
  %6 = call i64 @get_base_global_id.(i32 1)
  %7 = call i64 @_Z14get_local_sizej(i32 2)
  %8 = call i64 @get_base_global_id.(i32 2)
  %9 = insertvalue [7 x i64] undef, i64 %3, 2
  %10 = insertvalue [7 x i64] %9, i64 %4, 1
  %11 = insertvalue [7 x i64] %10, i64 %5, 4
  %12 = insertvalue [7 x i64] %11, i64 %6, 3
  %13 = insertvalue [7 x i64] %12, i64 %7, 6
  %14 = insertvalue [7 x i64] %13, i64 %8, 5
  %15 = insertvalue [7 x i64] %14, i64 1, 0
  ret [7 x i64] %15
}

declare i64 @_Z14get_local_sizej(i32)

declare i64 @get_base_global_id.(i32)

; Function Attrs: nounwind
define void @__Vectorized_.test(float addrspace(1)* nocapture %lhs, float addrspace(1)* nocapture %rhs, float addrspace(1)* nocapture %out) #0 {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) #2, !dbg !56
  %0 = getelementptr inbounds float addrspace(1)* %rhs, i64 %call, !dbg !57
  %ptrTypeCast = bitcast float addrspace(1)* %0 to <4 x float> addrspace(1)*
  %1 = load <4 x float> addrspace(1)* %ptrTypeCast, align 1, !dbg !57
  %2 = getelementptr inbounds float addrspace(1)* %lhs, i64 %call, !dbg !57
  %ptrTypeCast4 = bitcast float addrspace(1)* %2 to <4 x float> addrspace(1)*
  %3 = load <4 x float> addrspace(1)* %ptrTypeCast4, align 1, !dbg !57
  %mul5 = fmul <4 x float> %1, %3, !dbg !57
  %4 = getelementptr inbounds float addrspace(1)* %out, i64 %call, !dbg !57
  %ptrTypeCast6 = bitcast float addrspace(1)* %4 to <4 x float> addrspace(1)*
  store <4 x float> %mul5, <4 x float> addrspace(1)* %ptrTypeCast6, align 1, !dbg !57
  ret void, !dbg !58
}

declare i1 @__ocl_allOne(i1)

declare i1 @__ocl_allZero(i1)

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-frame-pointer-elim-non-leaf"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp
-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-frame-pointer-elim-non-leaf"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "
unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind readnone }

!llvm.dbg.cu = !{!0}
!opencl.kernels = !{!14}
!opencl.enable.FP_CONTRACT = !{}
!opencl.spir.version = !{!21}
!opencl.ocl.version = !{!21}
!opencl.used.extensions = !{!22}
!opencl.used.optional.core.features = !{!22}
!opencl.compiler.options = !{!22}
!opencl.kernel_info = !{!23, !40}
!opencl.module_info_list = !{!49}
!llvm.functions_info = !{}
!opencl.functions_stats = !{}
!opencl.stat_descriptions = !{}
!opencl.module_stat_info = !{}

!0 = metadata !{i32 786449, metadata !1, i32 12, metadata !"clang version 3.4 ", i1 false, metadata !"", i32 0, metadata !2, metadata !2, metadata !3, metadata !2, metadata !2, metadata !""}
; [ DW_TAG_compile_unit ] [/home/aelizuno/git/ocl/install/RH64/Debug/bin/1] [DW_LANG_C99]
!1 = metadata !{metadata !"1", metadata !"/home/aelizuno/git/ocl/install/RH64/Debug/bin"}
!2 = metadata !{i32 0}
!3 = metadata !{metadata !4, metadata !10}
!4 = metadata !{i32 786478, metadata !1, metadata !5, metadata !"test", metadata !"test", metadata !"", i32 1, metadata !6, i1 false, i1 true, i32 0, i32 0, null, i32 256, i1 false, void (float addrspace(1)*, float addrspace(1)*, float addrspace(1)*)* @test, null, null, metadata !2, i32 1} ; [ DW_TAG_subprogram ] [line 1] [def] [test]
!5 = metadata !{i32 786473, metadata !1}          ; [ DW_TAG_file_type ] [/home/aelizuno/git/ocl/install/RH64/Debug/bin/1]
!6 = metadata !{i32 786453, i32 0, i32 0, metadata !"", i32 0, i64 0, i64 0, i64 0, i32 0, null, metadata !7, i32 0, i32 0} ; [ DW_TAG_subroutine_type ] [line 0, size 0, align 0, offset 0] [from ]
!7 = metadata !{null, metadata !8, metadata !8, metadata !8}
!8 = metadata !{i32 786447, null, null, metadata !"", i32 0, i64 64, i64 64, i64 0, i32 0, metadata !9} ; [ DW_TAG_pointer_type ] [line 0, size 64, align 64, offset 0] [from float]
!9 = metadata !{i32 786468, null, null, metadata !"float", i32 0, i64 32, i64 32, i64 0, i32 0, i32 4} ; [ DW_TAG_base_type ] [float] [line 0, size 32, align 32, offset 0, enc DW_ATE_float]
!10 = metadata !{i32 786478, metadata !1, metadata !5, metadata !"test", metadata !"test", metadata !"__Vectorized_.test", i32 1, metadata !11, i1 false, i1 true, i32 0, i32 0, null, i32 256, i1 false, void (float addrspace(1)*, float addrspace(1)*, float addrspace(1)*)* @__Vectorized_.test, null, null, metadata !2, i32 1} ; [ DW_TAG_subprogram ] [line 1] [def] [test]
!11 = metadata !{i32 786453, i32 0, i32 0, metadata !"__Vectorized_.test", i32 0, i64 0, i64 0, i64 0, i32 0, null, metadata !12, i32 0, i32 0} ; [ DW_TAG_subroutine_type ] [__Vectorized_.test] [line 0, size 0, align 0, offset 0] [from ]
!12 = metadata !{null, metadata !13, metadata !13, metadata !13}
!13 = metadata !{i32 786447, null, null, metadata !"__Vectorized_.test", i32 0, i64 64, i64 64, i64 0, i32 0, metadata !9} ; [ DW_TAG_pointer_type ] [__Vectorized_.test] [line 0, size 64, align 64, offset 0] [from float]
!14 = metadata !{void (float addrspace(1)*, float addrspace(1)*, float addrspace(1)*)* @test, metadata !15, metadata !16, metadata !17, metadata !18, metadata !19, metadata !20}
!15 = metadata !{metadata !"kernel_arg_addr_space", i32 1, i32 1, i32 1}
!16 = metadata !{metadata !"kernel_arg_access_qual", metadata !"none", metadata !"none", metadata !"none"}
!17 = metadata !{metadata !"kernel_arg_type", metadata !"float*", metadata !"float*", metadata !"float*"}
!18 = metadata !{metadata !"kernel_arg_type_qual", metadata !"", metadata !"", metadata !""}
!19 = metadata !{metadata !"kernel_arg_base_type", metadata !"float*", metadata !"float*", metadata !"float*"}
!20 = metadata !{metadata !"kernel_arg_name", metadata !"lhs", metadata !"rhs", metadata !"out"}
!21 = metadata !{i32 1, i32 2}
!22 = metadata !{}
!23 = metadata !{void (float addrspace(1)*, float addrspace(1)*, float addrspace(1)*)* @test, metadata !24}
!24 = metadata !{metadata !25, metadata !26, metadata !27, metadata !28, metadata !29, metadata !30, metadata !31, metadata !32, metadata !33, metadata !34, metadata !35, metadata !36, metadata !37, metadata !38, metadata !39}
!25 = metadata !{metadata !"local_buffer_size", null}
!26 = metadata !{metadata !"barrier_buffer_size", null}
!27 = metadata !{metadata !"kernel_execution_length", i32 9}
!28 = metadata !{metadata !"max_wg_dimensions", i32 1}
!29 = metadata !{metadata !"kernel_has_barrier", i1 false}
!30 = metadata !{metadata !"kernel_has_global_sync", i1 false}
!31 = metadata !{metadata !"no_barrier_path", i1 true}
!32 = metadata !{metadata !"vectorized_kernel", void (float addrspace(1)*, float addrspace(1)*, float addrspace(1)*)* @__Vectorized_.test}
!33 = metadata !{metadata !"vectorized_width", i32 1}
!34 = metadata !{metadata !"kernel_wrapper", null}
!35 = metadata !{metadata !"scalarized_kernel", null}
!36 = metadata !{metadata !"block_literal_size", null}
!37 = metadata !{metadata !"private_memory_size", null}
!38 = metadata !{metadata !"vectorization_dimension", null}
!39 = metadata !{metadata !"can_unite_workgroups", null}
!40 = metadata !{void (float addrspace(1)*, float addrspace(1)*, float addrspace(1)*)* @__Vectorized_.test, metadata !41}
!41 = metadata !{metadata !25, metadata !26, metadata !42, metadata !28, metadata !29, metadata !30, metadata !43, metadata !44, metadata !45, metadata !34, metadata !46, metadata !36, metadata !37, metadata !47, metadata !48}
!42 = metadata !{metadata !"kernel_execution_length", i32 12}
!43 = metadata !{metadata !"no_barrier_path", null}
!44 = metadata !{metadata !"vectorized_kernel", null}
!45 = metadata !{metadata !"vectorized_width", i32 4}
!46 = metadata !{metadata !"scalarized_kernel", void (float addrspace(1)*, float addrspace(1)*, float addrspace(1)*)* @test}
!47 = metadata !{metadata !"vectorization_dimension", i32 0}
!48 = metadata !{metadata !"can_unite_workgroups", i1 true}
!49 = metadata !{metadata !50, metadata !51, metadata !52}
!50 = metadata !{metadata !"global_variable_total_size", i64 0}
!51 = metadata !{metadata !"gen_addr_space_pointer_counter", null}
!52 = metadata !{metadata !"gen_addr_space_pointer_warnings"}
!53 = metadata !{i32 2, i32 0, metadata !4, null}
!54 = metadata !{i32 3, i32 0, metadata !4, null}
!55 = metadata !{i32 4, i32 0, metadata !4, null}
!56 = metadata !{i32 2, i32 0, metadata !10, null}
!57 = metadata !{i32 3, i32 0, metadata !10, null}
!58 = metadata !{i32 4, i32 0, metadata !10, null}
