; RUN: opt -local-buffers -prepare-kernel-args -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll

; The test checks that global variables usages marked with the dbg_declare_inst metadata are ignored by the pass, and do not cause extra local memory allocation.
; The request size is 100 bytes, which we expect to be rounded to 128.
; In the problematic scenario, the call to dbg_declare global in @f1 is considered a usage and causes double memory allocation, which wil result in alloca [256 .
; CHECK-NOT: alloca [
; CHECK: alloca [128
; CHECK-NOT: alloca [

; ModuleID = 'debugInfo.ll'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-f80:32:32-n8:16:32-S32"
target triple = "i686-pc-win32"

%struct.ExtendedExecutionContext = type opaque

@mykernel.x = internal addrspace(3) global [100 x i8] zeroinitializer, align 1

declare void @__f1_original(i8 addrspace(3)* noalias) nounwind

declare void @llvm.dbg.declare(metadata, metadata) nounwind readnone

declare void @__mykernel_original() nounwind

declare i32 @_Z13get_global_idj(i32)

declare void @__opencl_dbg_declare_local(i8*, i64, i64, i64, i64)

declare void @__opencl_dbg_declare_global(i8*, i64, i64, i64, i64)

declare void @__opencl_dbg_enter_function(i64, i64, i64, i64)

declare void @__opencl_dbg_exit_function(i64, i64, i64, i64)

declare void @__opencl_dbg_stoppoint(i64, i64, i64, i64)

define void @f1(i8 addrspace(3)* noalias %a, i8 addrspace(3)* noalias %pLocalMem, { i32, [3 x i32], [3 x i32], [3 x i32], [3 x i32] }* noalias %pWorkDim, i32* noalias %pWGId, <{ [4 x i32] }>* noalias %pBaseGlbId, i32* noalias %contextpointer, <{ [4 x i32] }>* noalias %pLocalIds, i32 %iterCount, i8* noalias %pSpecialBuf, i32* noalias %pCurrWI, %struct.ExtendedExecutionContext* noalias %extExecContextPointer) nounwind {
entry:
  %_Z13get_global_idj0 = call i32 @_Z13get_global_idj(i32 0)
  %gid0_i64 = zext i32 %_Z13get_global_idj0 to i64
  %_Z13get_global_idj1 = call i32 @_Z13get_global_idj(i32 1)
  %gid1_i64 = zext i32 %_Z13get_global_idj1 to i64
  %_Z13get_global_idj2 = call i32 @_Z13get_global_idj(i32 2)
  %gid2_i64 = zext i32 %_Z13get_global_idj2 to i64
  call void @__opencl_dbg_enter_function(i64 6683384, i64 %gid0_i64, i64 %gid1_i64, i64 %gid2_i64)
  %var_addr = bitcast [100 x i8] addrspace(3)* @mykernel.x to i8*, !dbg_declare_inst !30
  call void @__opencl_dbg_declare_global(i8* %var_addr, i64 6721800, i64 %gid0_i64, i64 %gid1_i64, i64 %gid2_i64)
  %a.addr = alloca i8 addrspace(3)*, align 4
  store i8 addrspace(3)* %a, i8 addrspace(3)** %a.addr, align 4
  call void @__opencl_dbg_stoppoint(i64 6722880, i64 %gid0_i64, i64 %gid1_i64, i64 %gid2_i64)
  %0 = bitcast i8 addrspace(3)** %a.addr to i8*
  call void @__opencl_dbg_declare_local(i8* %0, i64 6722672, i64 %gid0_i64, i64 %gid1_i64, i64 %gid2_i64)
  call void @__opencl_dbg_exit_function(i64 6683384, i64 %gid0_i64, i64 %gid1_i64, i64 %gid2_i64)
  ret void, !dbg !31
}

define void @mykernel(i8 addrspace(3)* noalias %pLocalMem, { i32, [3 x i32], [3 x i32], [3 x i32], [3 x i32] }* noalias %pWorkDim, i32* noalias %pWGId, <{ [4 x i32] }>* noalias %pBaseGlbId, i32* noalias %contextpointer, <{ [4 x i32] }>* noalias %pLocalIds, i32 %iterCount, i8* noalias %pSpecialBuf, i32* noalias %pCurrWI, %struct.ExtendedExecutionContext* noalias %extExecContextPointer) nounwind {
entry:
  %_Z13get_global_idj0 = call i32 @_Z13get_global_idj(i32 0)
  %gid0_i64 = zext i32 %_Z13get_global_idj0 to i64
  %_Z13get_global_idj1 = call i32 @_Z13get_global_idj(i32 1)
  %gid1_i64 = zext i32 %_Z13get_global_idj1 to i64
  %_Z13get_global_idj2 = call i32 @_Z13get_global_idj(i32 2)
  %gid2_i64 = zext i32 %_Z13get_global_idj2 to i64
  call void @__opencl_dbg_enter_function(i64 6704936, i64 %gid0_i64, i64 %gid1_i64, i64 %gid2_i64)
  %var_addr = bitcast [100 x i8] addrspace(3)* @mykernel.x to i8*, !dbg_declare_inst !30
  call void @__opencl_dbg_declare_global(i8* %var_addr, i64 6721800, i64 %gid0_i64, i64 %gid1_i64, i64 %gid2_i64)
  call void @__opencl_dbg_stoppoint(i64 6705624, i64 %gid0_i64, i64 %gid1_i64, i64 %gid2_i64)
  %0 = getelementptr i8 addrspace(3)* %pLocalMem, i32 128
  call void @f1(i8 addrspace(3)* getelementptr inbounds ([100 x i8] addrspace(3)* @mykernel.x, i32 0, i32 0), i8 addrspace(3)* %0, { i32, [3 x i32], [3 x i32], [3 x i32], [3 x i32] }* %pWorkDim, i32* %pWGId, <{ [4 x i32] }>* %pBaseGlbId, i32* %contextpointer, <{ [4 x i32] }>* %pLocalIds, i32 %iterCount, i8* %pSpecialBuf, i32* %pCurrWI, %struct.ExtendedExecutionContext* %extExecContextPointer), !dbg !33
  call void @__opencl_dbg_stoppoint(i64 6705768, i64 %gid0_i64, i64 %gid1_i64, i64 %gid2_i64)
  call void @__opencl_dbg_exit_function(i64 6704936, i64 %gid0_i64, i64 %gid1_i64, i64 %gid2_i64)
  ret void, !dbg !35
}

!llvm.dbg.cu = !{!0}
!opencl.kernels = !{!21}
!opencl.enable.FP_CONTRACT = !{}
!opencl.spir.version = !{!27}
!opencl.ocl.version = !{!28}
!opencl.used.extensions = !{!29}
!opencl.used.optional.core.features = !{!29}
!opencl.compiler.options = !{!29}
!opencl.kernel_info = !{}
!opencl.module_info_list = !{}
!llvm.functions_info = !{}

!0 = metadata !{i32 786449, i32 0, i32 12, metadata !"3", metadata !"C:\5Cocl\5Ctmp", metadata !"clang version 3.2 (tags/RELEASE_32/final 78244)", i1 true, i1 false, metadata !"", i32 0, metadata !1, metadata !1, metadata !3, metadata !15} ; [ DW_TAG_compile_unit ] [C:\ocl\tmp/3] [DW_LANG_C99]
!1 = metadata !{metadata !2}
!2 = metadata !{i32 0}
!3 = metadata !{metadata !4}
!4 = metadata !{metadata !5, metadata !12}
!5 = metadata !{i32 786478, i32 0, metadata !6, metadata !"f1", metadata !"f1", metadata !"", metadata !6, i32 2, metadata !7, i1 false, i1 true, i32 0, i32 0, null, i32 256, i1 false, void (i8 addrspace(3)*, i8 addrspace(3)*, { i32, [3 x i32], [3 x i32], [3 x i32], [3 x i32] }*, i32*, <{ [4 x i32] }>*, i32*, <{ [4 x i32] }>*, i32, i8*, i32*, %struct.ExtendedExecutionContext*)* @f1, null, null, metadata !1, i32 2} ; [ DW_TAG_subprogram ] [line 2] [def] [f1]
!6 = metadata !{i32 786473, metadata !"2", metadata !"C:\5Cocl\5Ctmp", null} ; [ DW_TAG_file_type ]
!7 = metadata !{i32 786453, i32 0, metadata !"", i32 0, i32 0, i64 0, i64 0, i64 0, i32 0, null, metadata !8, i32 0, i32 0} ; [ DW_TAG_subroutine_type ] [line 0, size 0, align 0, offset 0] [from ]
!8 = metadata !{null, metadata !9}
!9 = metadata !{i32 786447, null, metadata !"", null, i32 0, i64 32, i64 32, i64 0, i32 0, metadata !10} ; [ DW_TAG_pointer_type ] [line 0, size 32, align 32, offset 0] [from uchar]
!10 = metadata !{i32 786454, null, metadata !"uchar", metadata !6, i32 33, i64 0, i64 0, i64 0, i32 0, metadata !11} ; [ DW_TAG_typedef ] [uchar] [line 33, size 0, align 0, offset 0] [from unsigned char]
!11 = metadata !{i32 786468, null, metadata !"unsigned char", null, i32 0, i64 8, i64 8, i64 0, i32 0, i32 8} ; [ DW_TAG_base_type ] [unsigned char] [line 0, size 8, align 8, offset 0, enc DW_ATE_unsigned_char]
!12 = metadata !{i32 786478, i32 0, metadata !6, metadata !"mykernel", metadata !"mykernel", metadata !"", metadata !6, i32 4, metadata !13, i1 false, i1 true, i32 0, i32 0, null, i32 0, i1 false, void (i8 addrspace(3)*, { i32, [3 x i32], [3 x i32], [3 x i32], [3 x i32] }*, i32*, <{ [4 x i32] }>*, i32*, <{ [4 x i32] }>*, i32, i8*, i32*, %struct.ExtendedExecutionContext*)* @mykernel, null, null, metadata !1, i32 4} ; [ DW_TAG_subprogram ] [line 4] [def] [mykernel]
!13 = metadata !{i32 786453, i32 0, metadata !"", i32 0, i32 0, i64 0, i64 0, i64 0, i32 0, null, metadata !14, i32 0, i32 0} ; [ DW_TAG_subroutine_type ] [line 0, size 0, align 0, offset 0] [from ]
!14 = metadata !{null}
!15 = metadata !{metadata !16}
!16 = metadata !{metadata !17}
!17 = metadata !{i32 786484, i32 0, metadata !12, metadata !"x", metadata !"x", metadata !"", metadata !6, i32 5, metadata !18, i32 1, i32 1, [100 x i8] addrspace(3)* @mykernel.x} ; [ DW_TAG_variable ] [x] [line 5] [local] [def]
!18 = metadata !{i32 786433, null, metadata !"", null, i32 0, i64 800, i64 8, i32 0, i32 0, metadata !10, metadata !19, i32 0, i32 0} ; [ DW_TAG_array_type ] [line 0, size 800, align 8, offset 0] [from uchar]
!19 = metadata !{metadata !20}
!20 = metadata !{i32 786465, i64 0, i64 99}       ; [ DW_TAG_subrange_type ] [0, 99]
!21 = metadata !{void (i8 addrspace(3)*, { i32, [3 x i32], [3 x i32], [3 x i32], [3 x i32] }*, i32*, <{ [4 x i32] }>*, i32*, <{ [4 x i32] }>*, i32, i8*, i32*, %struct.ExtendedExecutionContext*)* @mykernel, metadata !22, metadata !23, metadata !24, metadata !25, metadata !26}
!22 = metadata !{metadata !"kernel_arg_addr_space"}
!23 = metadata !{metadata !"kernel_arg_access_qual"}
!24 = metadata !{metadata !"kernel_arg_type"}
!25 = metadata !{metadata !"kernel_arg_type_qual"}
!26 = metadata !{metadata !"kernel_arg_name"}
!27 = metadata !{i32 1, i32 0}
!28 = metadata !{i32 0, i32 0}
!29 = metadata !{}
!30 = metadata !{i1 true}
!31 = metadata !{i32 2, i32 0, metadata !32, null}
!32 = metadata !{i32 786443, metadata !5, i32 2, i32 0, metadata !6, i32 0} ; [ DW_TAG_lexical_block ] [C:\ocl\tmp/2]
!33 = metadata !{i32 6, i32 0, metadata !34, null}
!34 = metadata !{i32 786443, metadata !12, i32 4, i32 0, metadata !6, i32 1} ; [ DW_TAG_lexical_block ] [C:\ocl\tmp/2]
!35 = metadata !{i32 7, i32 0, metadata !34, null}
