; RUN: opt -add-implicit-args -S %s -o %t.ll
; RUN: llc %t.ll

; This test checks that llc won't crash on result of AddImplicitArgs pass (when we have dbg on structure arguments)

; ModuleID = 'Program'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-pc-win32"

%struct.clPathVertexPtr = type { i32 }

define void @PathVertex_ResetStatus(%struct.clPathVertexPtr* byval %pathVertex) nounwind {
entry:
  %CurrWI = call i64* @get_curr_wi.()
  %currWI3 = load i64* %CurrWI
  %newGID4 = call i64 @get_new_global_id.(i32 0, i64 %currWI3)
  %currWI1 = load i64* %CurrWI
  %newGID2 = call i64 @get_new_global_id.(i32 1, i64 %currWI1)
  %currWI = load i64* %CurrWI
  %newGID = call i64 @get_new_global_id.(i32 2, i64 %currWI)
  call void @__opencl_dbg_enter_function(i64 162091280, i64 %newGID4, i64 %newGID2, i64 %newGID)
  call void @__opencl_dbg_stoppoint(i64 162237968, i64 %newGID4, i64 %newGID2, i64 %newGID)
  %0 = bitcast %struct.clPathVertexPtr* %pathVertex to i8*
  call void @__opencl_dbg_declare_local(i8* %0, i64 162103424, i64 %newGID4, i64 %newGID2, i64 %newGID)
  call void @__opencl_dbg_stoppoint(i64 162239616, i64 %newGID4, i64 %newGID2, i64 %newGID)
  %pointer = getelementptr inbounds %struct.clPathVertexPtr* %pathVertex, i32 0, i32 0, !dbg !38
  store i32 0, i32* %pointer, align 4, !dbg !38
  call void @__opencl_dbg_stoppoint(i64 162240080, i64 %newGID4, i64 %newGID2, i64 %newGID)
  call void @__opencl_dbg_exit_function(i64 162091280, i64 %newGID4, i64 %newGID2, i64 %newGID)
  ret void, !dbg !40
}

declare void @llvm.dbg.declare(metadata, metadata) nounwind readnone

define void @test(%struct.clPathVertexPtr* %pathVertex) nounwind {
entry:
  %currBarrier = alloca i32
  %CurrSBIndex = alloca i64
  %CurrWI = call i64* @get_curr_wi.()
  %pSB = call i8* @get_special_buffer.()
  %IterCount = call i64 @get_iter_count.()
  br label %FirstBB

FirstBB:                                          ; preds = %entry
  store i32 1, i32* %currBarrier
  store i64 0, i64* %CurrSBIndex
  store i64 0, i64* %CurrWI
  br label %SyncBB9

SyncBB9:                                          ; preds = %thenBB, %FirstBB
  %currWI13 = load i64* %CurrWI
  %newGID14 = call i64 @get_new_global_id.(i32 0, i64 %currWI13)
  %currWI11 = load i64* %CurrWI
  %newGID12 = call i64 @get_new_global_id.(i32 1, i64 %currWI11)
  %currWI = load i64* %CurrWI
  %newGID = call i64 @get_new_global_id.(i32 2, i64 %currWI)
  call void @__opencl_dbg_enter_function(i64 162097104, i64 %newGID14, i64 %newGID12, i64 %newGID)
  %loadedCurrSB5 = load i64* %CurrSBIndex
  %"&(pSB[currWI].offset)6" = add nuw i64 %loadedCurrSB5, 0
  %"&pSB[currWI].offset7" = getelementptr inbounds i8* %pSB, i64 %"&(pSB[currWI].offset)6"
  %CastToValueType8 = bitcast i8* %"&pSB[currWI].offset7" to %struct.clPathVertexPtr**
  store %struct.clPathVertexPtr* %pathVertex, %struct.clPathVertexPtr** %CastToValueType8, align 8
  call void @__opencl_dbg_stoppoint(i64 162243136, i64 %newGID14, i64 %newGID12, i64 %newGID)
  %loadedCurrSB = load i64* %CurrSBIndex
  %"&(pSB[currWI].offset)" = add nuw i64 %loadedCurrSB, 0
  %"&pSB[currWI].offset" = getelementptr inbounds i8* %pSB, i64 %"&(pSB[currWI].offset)"
  %CastToValueType = bitcast i8* %"&pSB[currWI].offset" to %struct.clPathVertexPtr**
  %0 = bitcast %struct.clPathVertexPtr** %CastToValueType to i8*
  call void @__opencl_dbg_declare_local(i8* %0, i64 162104192, i64 %newGID14, i64 %newGID12, i64 %newGID)
  call void @__opencl_dbg_stoppoint(i64 162244832, i64 %newGID14, i64 %newGID12, i64 %newGID)
  %loadedCurrSB1 = load i64* %CurrSBIndex
  %"&(pSB[currWI].offset)2" = add nuw i64 %loadedCurrSB1, 0
  %"&pSB[currWI].offset3" = getelementptr inbounds i8* %pSB, i64 %"&(pSB[currWI].offset)2"
  %CastToValueType4 = bitcast i8* %"&pSB[currWI].offset3" to %struct.clPathVertexPtr**
  %1 = load %struct.clPathVertexPtr** %CastToValueType4, align 8, !dbg !41
  %arrayidx = getelementptr inbounds %struct.clPathVertexPtr* %1, i64 0, !dbg !41
  call void @PathVertex_ResetStatus(%struct.clPathVertexPtr* byval %arrayidx), !dbg !41
  call void @__opencl_dbg_stoppoint(i64 162245296, i64 %newGID14, i64 %newGID12, i64 %newGID)
  call void @__opencl_dbg_exit_function(i64 162097104, i64 %newGID14, i64 %newGID12, i64 %newGID)
  br label %"Barrier BB"

"Barrier BB":                                     ; preds = %SyncBB9
  %loadedCurrWI = load i64* %CurrWI
  %check.WI.iter = icmp ult i64 %loadedCurrWI, %IterCount
  br i1 %check.WI.iter, label %thenBB, label %elseBB

thenBB:                                           ; preds = %"Barrier BB"
  %"CurrWI++" = add nuw i64 %loadedCurrWI, 1
  store i64 %"CurrWI++", i64* %CurrWI
  %loadedCurrSB10 = load i64* %CurrSBIndex
  %"loadedCurrSB+Stride" = add nuw i64 %loadedCurrSB10, 8
  store i64 %"loadedCurrSB+Stride", i64* %CurrSBIndex
  br label %SyncBB9

elseBB:                                           ; preds = %"Barrier BB"
  store i64 0, i64* %CurrWI
  store i64 0, i64* %CurrSBIndex
  store i32 0, i32* %currBarrier
  br label %SyncBB

SyncBB:                                           ; preds = %elseBB
  ret void, !dbg !43
}

declare i64 @_Z13get_global_idj(i32)

declare void @__opencl_dbg_declare_local(i8*, i64, i64, i64, i64)

declare void @__opencl_dbg_declare_global(i8*, i64, i64, i64, i64)

declare void @__opencl_dbg_enter_function(i64, i64, i64, i64)

declare void @__opencl_dbg_exit_function(i64, i64, i64, i64)

declare void @__opencl_dbg_stoppoint(i64, i64, i64, i64)

declare void @dummybarrier.()

declare void @_Z7barrierj(i32)

declare i64* @get_curr_wi.() nounwind readnone

declare i8* @get_special_buffer.() nounwind readnone

declare i64 @get_iter_count.() nounwind readnone

declare i64 @get_new_global_id.(i32, i64) nounwind readnone

!llvm.dbg.cu = !{!0}
!opencl.kernels = !{!18}
!opencl.enable.FP_CONTRACT = !{}
!opencl.spir.version = !{!24}
!opencl.ocl.version = !{!25}
!opencl.used.extensions = !{!26}
!opencl.used.optional.core.features = !{!26}
!opencl.compiler.options = !{!26}
!opencl.kernel_info = !{!27}
!opencl.module_info_list = !{}
!llvm.functions_info = !{}

!0 = metadata !{i32 786449, i32 0, i32 12, metadata !"3", metadata !"C:\5Cwork\5Caaboud\5CVolcano\5CApple\5CCleanTrunk\5Cinstall\5CWin64\5CDebug\5Cbin", metadata !"clang version 3.2 (tags/RELEASE_32/final 75485)", i1 true, i1 false, metadata !"", i32 0, metadata !1, metadata !1, metadata !3, metadata !1} ; [ DW_TAG_compile_unit ] [C:\work\aaboud\Volcano\Apple\CleanTrunk\install\Win64\Debug\bin/3][DW_LANG_C99]
!1 = metadata !{metadata !2}
!2 = metadata !{i32 0}
!3 = metadata !{metadata !4}
!4 = metadata !{metadata !5, metadata !14}
!5 = metadata !{i32 786478, i32 0, metadata !6, metadata !"PathVertex_ResetStatus", metadata !"PathVertex_ResetStatus", metadata !"", metadata !6, i32 16, metadata !7, i1 false, i1 true, i32 0, i32 0, null, i32 256, i1 false, void (%struct.clPathVertexPtr*)* @PathVertex_ResetStatus, null, null, metadata !1, i32 17} ; [ DW_TAG_subprogram ] [line 16] [def] [scope 17] [PathVertex_ResetStatus]
!6 = metadata !{i32 786473, metadata !"2", metadata !"C:\5Cwork\5Caaboud\5CVolcano\5CApple\5CCleanTrunk\5Cinstall\5CWin64\5CDebug\5Cbin", null} ; [ DW_TAG_file_type ]
!7 = metadata !{i32 786453, i32 0, metadata !"", i32 0, i32 0, i64 0, i64 0, i64 0, i32 0, null, metadata !8, i32 0, i32 0} ; [ DW_TAG_subroutine_type ] [line 0, size 0, align 0, offset 0] [from ]
!8 = metadata !{null, metadata !9}
!9 = metadata !{i32 786454, null, metadata !"clPathVertexPtr", metadata !6, i32 14, i64 0, i64 0, i64 0, i32 0, metadata !10} ; [ DW_TAG_typedef ] [clPathVertexPtr] [line 14, size 0, align 0, offset 0] [from ]
!10 = metadata !{i32 786451, null, metadata !"", metadata !6, i32 12, i64 32, i64 32, i32 0, i32 0, null, metadata !11, i32 0, i32 0, i32 0} ; [ DW_TAG_structure_type ] [line 12, size 32, align 32, offset 0] [from ]
!11 = metadata !{metadata !12}
!12 = metadata !{i32 786445, metadata !10, metadata !"pointer", metadata !6, i32 13, i64 32, i64 32, i64 0, i32 0, metadata !13} ; [ DW_TAG_member ] [pointer] [line 13, size 32, align 32, offset 0] [from int]
!13 = metadata !{i32 786468, null, metadata !"int", null, i32 0, i64 32, i64 32, i64 0, i32 0, i32 5} ; [ DW_TAG_base_type ] [int] [line 0, size 32, align 32, offset 0, enc DW_ATE_signed]
!14 = metadata !{i32 786478, i32 0, metadata !6, metadata !"test", metadata !"test", metadata !"", metadata !6, i32 21, metadata !15, i1 false, i1 true, i32 0, i32 0, null, i32 256, i1 false, void (%struct.clPathVertexPtr*)* @test, null, null, metadata !1, i32 21} ; [ DW_TAG_subprogram ] [line 21] [def] [test]
!15 = metadata !{i32 786453, i32 0, metadata !"", i32 0, i32 0, i64 0, i64 0, i64 0, i32 0, null, metadata !16, i32 0, i32 0} ; [ DW_TAG_subroutine_type ] [line 0, size 0, align 0, offset 0] [from ]
!16 = metadata !{null, metadata !17}
!17 = metadata !{i32 786447, null, metadata !"", null, i32 0, i64 64, i64 64, i64 0, i32 0, metadata !9} ; [ DW_TAG_pointer_type ] [line 0, size 64, align 64, offset 0] [from clPathVertexPtr]
!18 = metadata !{void (%struct.clPathVertexPtr*)* @test, metadata !19, metadata !20, metadata !21, metadata !22, metadata !23}
!19 = metadata !{metadata !"kernel_arg_addr_space", i32 0}
!20 = metadata !{metadata !"kernel_arg_access_qual", metadata !"none"}
!21 = metadata !{metadata !"kernel_arg_type", metadata !"clPathVertexPtr*"}
!22 = metadata !{metadata !"kernel_arg_type_qual", metadata !""}
!23 = metadata !{metadata !"kernel_arg_name", metadata !"pathVertex"}
!24 = metadata !{i32 1, i32 0}
!25 = metadata !{i32 0, i32 0}
!26 = metadata !{}
!27 = metadata !{void (%struct.clPathVertexPtr*)* @test, metadata !28}
!28 = metadata !{metadata !29, metadata !30, metadata !31, metadata !32, metadata !33, metadata !34, metadata !35, metadata !36, metadata !37}
!29 = metadata !{metadata !"local_buffer_size", null}
!30 = metadata !{metadata !"barrier_buffer_size", i32 8}
!31 = metadata !{metadata !"kernel_execution_length", i32 16}
!32 = metadata !{metadata !"kernel_has_barrier", i1 false}
!33 = metadata !{metadata !"no_barrier_path", null}
!34 = metadata !{metadata !"vectorized_kernel", null}
!35 = metadata !{metadata !"vectorized_width", null}
!36 = metadata !{metadata !"kernel_wrapper", null}
!37 = metadata !{metadata !"scalarized_kernel", null}
!38 = metadata !{i32 18, i32 0, metadata !39, null}
!39 = metadata !{i32 786443, metadata !5, i32 17, i32 0, metadata !6, i32 0} ; [ DW_TAG_lexical_block ] [C:\work\aaboud\Volcano\Apple\CleanTrunk\install\Win64\Debug\bin/2]
!40 = metadata !{i32 19, i32 0, metadata !39, null}
!41 = metadata !{i32 22, i32 0, metadata !42, null}
!42 = metadata !{i32 786443, metadata !14, i32 21, i32 0, metadata !6, i32 1} ; [ DW_TAG_lexical_block ] [C:\work\aaboud\Volcano\Apple\CleanTrunk\install\Win64\Debug\bin/2]
!43 = metadata !{i32 23, i32 0, metadata !42, null}
