; RUN: opt %s -passes=dpcpp-kernel-indirect-call-lowering -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt %s -passes=dpcpp-kernel-indirect-call-lowering -S | FileCheck %s

%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range" = type { %"class._ZTSN2cl4sycl6detail5arrayILi1EEE.cl::sycl::detail::array" }
%"class._ZTSN2cl4sycl6detail5arrayILi1EEE.cl::sycl::detail::array" = type { [1 x i64] }

@"_Z3addii$SIMDTable.new_init" = constant [2 x i32 (i32, i32)*] [i32 (i32, i32)* bitcast (<16 x i32> (<16 x i32>, <16 x i32>, <16 x i32>)* @_ZGVdM16vv_3addii to i32 (i32, i32)*), i32 (i32, i32)* bitcast (<16 x i32> (<16 x i32>, <16 x i32>)* @_ZGVdN16vv_3addii to i32 (i32, i32)*)]
@"_Z3addii$SIMDTable" = constant [1 x i32 (i32, i32)*] [i32 (i32, i32)* bitcast ([2 x i32 (i32, i32)*]* @"_Z3addii$SIMDTable.new_init" to i32 (i32, i32)*)]
@"_Z3subii$SIMDTable.new_init" = constant [2 x i32 (i32, i32)*] [i32 (i32, i32)* bitcast (<16 x i32> (<16 x i32>, <16 x i32>, <16 x i32>)* @_ZGVdM16vv_3subii to i32 (i32, i32)*), i32 (i32, i32)* bitcast (<16 x i32> (<16 x i32>, <16 x i32>)* @_ZGVdN16vv_3subii to i32 (i32, i32)*)]
@"_Z3subii$SIMDTable" = constant [1 x i32 (i32, i32)*] [i32 (i32, i32)* bitcast ([2 x i32 (i32, i32)*]* @"_Z3subii$SIMDTable.new_init" to i32 (i32, i32)*)]

define void @"_ZTSZZ4mainENK3$_0clERN2cl4sycl7handlerEE1K"(i64 addrspace(1)* %_arg_, %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"* byval(%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range") %_arg_1, %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"* byval(%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range") %_arg_2, %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"* byval(%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range") %_arg_3, i64 addrspace(1)* %_arg_4, %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"* byval(%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range") %_arg_6, %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"* byval(%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range") %_arg_7, %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"* byval(%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range") %_arg_8) {
  %early_exit_call = call [7 x i64] @"WG.boundaries._ZTSZZ4mainENK3$_0clERN2cl4sycl7handlerEE1K"(i64 addrspace(1)* %_arg_, %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"* %_arg_1, %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"* %_arg_2, %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"* %_arg_3, i64 addrspace(1)* %_arg_4, %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"* %_arg_6, %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"* %_arg_7, %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"* %_arg_8)
  %1 = extractvalue [7 x i64] %early_exit_call, 0
  %2 = trunc i64 %1 to i1
  br i1 %2, label %WGLoopsEntry, label %37

WGLoopsEntry:                                     ; preds = %0
  %3 = extractvalue [7 x i64] %early_exit_call, 1
  %4 = extractvalue [7 x i64] %early_exit_call, 2
  %5 = extractvalue [7 x i64] %early_exit_call, 3
  %6 = extractvalue [7 x i64] %early_exit_call, 4
  %7 = extractvalue [7 x i64] %early_exit_call, 5
  %8 = extractvalue [7 x i64] %early_exit_call, 6
  %vector.size = ashr i64 %4, 4
  %num.vector.wi = shl i64 %vector.size, 4
  %max.vector.gid = add i64 %num.vector.wi, %3
  %scalar.size = sub i64 %4, %num.vector.wi
  br label %vect_if

vect_if:                                          ; preds = %WGLoopsEntry
  %9 = icmp ne i64 %vector.size, 0
  br i1 %9, label %dim_2_vector_pre_head, label %scalarIf

dim_2_vector_pre_head:                            ; preds = %vect_if
  br label %dim_1_vector_pre_head

dim_1_vector_pre_head:                            ; preds = %dim_1_vector_exit, %dim_2_vector_pre_head
  %dim_2_vector_ind_var = phi i64 [ 0, %dim_2_vector_pre_head ], [ %dim_2_vector_inc_ind_var, %dim_1_vector_exit ]
  br label %dim_0_vector_pre_head

dim_0_vector_pre_head:                            ; preds = %dim_0_vector_exit, %dim_1_vector_pre_head
  %dim_1_vector_ind_var = phi i64 [ 0, %dim_1_vector_pre_head ], [ %dim_1_vector_inc_ind_var, %dim_0_vector_exit ]
  br label %entryvector_func

entryvector_func:                                 ; preds = %indirect.call.loop.exitvector_func, %dim_0_vector_pre_head
  %dim_0_vector_ind_var = phi i64 [ 0, %dim_0_vector_pre_head ], [ %dim_0_vector_inc_ind_var, %indirect.call.loop.exitvector_func ]
  %dim_0_vector_tid = phi i64 [ %3, %dim_0_vector_pre_head ], [ %dim_0_vector_inc_tid, %indirect.call.loop.exitvector_func ]
  %10 = getelementptr inbounds %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range", %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"* %_arg_3, i64 0, i32 0, i32 0, i64 0
  %11 = load i64, i64* %10, align 8
  %add.ptr.i30vector_func = getelementptr inbounds i64, i64 addrspace(1)* %_arg_, i64 %11
  %12 = getelementptr inbounds %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range", %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"* %_arg_8, i64 0, i32 0, i32 0, i64 0
  %13 = load i64, i64* %12, align 8
  %add.ptr.ivector_func = getelementptr inbounds i64, i64 addrspace(1)* %_arg_4, i64 %13
  %broadcast.splatinsertvector_func = insertelement <16 x i64> undef, i64 %dim_0_vector_tid, i32 0
  %broadcast.splatvector_func = shufflevector <16 x i64> %broadcast.splatinsertvector_func, <16 x i64> undef, <16 x i32> zeroinitializer
  %14 = add nuw <16 x i64> %broadcast.splatvector_func, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %15 = and <16 x i64> %14, <i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1>
  %16 = icmp eq <16 x i64> %15, zeroinitializer
  %scalar.gepvector_func = getelementptr inbounds i64, i64 addrspace(1)* %add.ptr.i30vector_func, i64 %dim_0_vector_tid
  %17 = bitcast i64 addrspace(1)* %scalar.gepvector_func to <16 x i64> addrspace(1)*
  %wide.loadvector_func = load <16 x i64>, <16 x i64> addrspace(1)* %17, align 8
  %18 = trunc <16 x i64> %wide.loadvector_func to <16 x i32>
  %scalar.gep2vector_func = getelementptr inbounds i64, i64 addrspace(1)* %add.ptr.ivector_func, i64 %dim_0_vector_tid
  %19 = bitcast i64 addrspace(1)* %scalar.gep2vector_func to <16 x i64> addrspace(1)*
  %wide.load3vector_func = load <16 x i64>, <16 x i64> addrspace(1)* %19, align 8
  %20 = trunc <16 x i64> %wide.load3vector_func to <16 x i32>
  %21 = select <16 x i1> %16, <16 x i32 (i32, i32)**> <i32 (i32, i32)** getelementptr inbounds ([1 x i32 (i32, i32)*], [1 x i32 (i32, i32)*]* @"_Z3addii$SIMDTable", i64 0, i64 0), i32 (i32, i32)** getelementptr inbounds ([1 x i32 (i32, i32)*], [1 x i32 (i32, i32)*]* @"_Z3addii$SIMDTable", i64 0, i64 0), i32 (i32, i32)** getelementptr inbounds ([1 x i32 (i32, i32)*], [1 x i32 (i32, i32)*]* @"_Z3addii$SIMDTable", i64 0, i64 0), i32 (i32, i32)** getelementptr inbounds ([1 x i32 (i32, i32)*], [1 x i32 (i32, i32)*]* @"_Z3addii$SIMDTable", i64 0, i64 0), i32 (i32, i32)** getelementptr inbounds ([1 x i32 (i32, i32)*], [1 x i32 (i32, i32)*]* @"_Z3addii$SIMDTable", i64 0, i64 0), i32 (i32, i32)** getelementptr inbounds ([1 x i32 (i32, i32)*], [1 x i32 (i32, i32)*]* @"_Z3addii$SIMDTable", i64 0, i64 0), i32 (i32, i32)** getelementptr inbounds ([1 x i32 (i32, i32)*], [1 x i32 (i32, i32)*]* @"_Z3addii$SIMDTable", i64 0, i64 0), i32 (i32, i32)** getelementptr inbounds ([1 x i32 (i32, i32)*], [1 x i32 (i32, i32)*]* @"_Z3addii$SIMDTable", i64 0, i64 0), i32 (i32, i32)** getelementptr inbounds ([1 x i32 (i32, i32)*], [1 x i32 (i32, i32)*]* @"_Z3addii$SIMDTable", i64 0, i64 0), i32 (i32, i32)** getelementptr inbounds ([1 x i32 (i32, i32)*], [1 x i32 (i32, i32)*]* @"_Z3addii$SIMDTable", i64 0, i64 0), i32 (i32, i32)** getelementptr inbounds ([1 x i32 (i32, i32)*], [1 x i32 (i32, i32)*]* @"_Z3addii$SIMDTable", i64 0, i64 0), i32 (i32, i32)** getelementptr inbounds ([1 x i32 (i32, i32)*], [1 x i32 (i32, i32)*]* @"_Z3addii$SIMDTable", i64 0, i64 0), i32 (i32, i32)** getelementptr inbounds ([1 x i32 (i32, i32)*], [1 x i32 (i32, i32)*]* @"_Z3addii$SIMDTable", i64 0, i64 0), i32 (i32, i32)** getelementptr inbounds ([1 x i32 (i32, i32)*], [1 x i32 (i32, i32)*]* @"_Z3addii$SIMDTable", i64 0, i64 0), i32 (i32, i32)** getelementptr inbounds ([1 x i32 (i32, i32)*], [1 x i32 (i32, i32)*]* @"_Z3addii$SIMDTable", i64 0, i64 0), i32 (i32, i32)** getelementptr inbounds ([1 x i32 (i32, i32)*], [1 x i32 (i32, i32)*]* @"_Z3addii$SIMDTable", i64 0, i64 0)>, <16 x i32 (i32, i32)**> <i32 (i32, i32)** getelementptr inbounds ([1 x i32 (i32, i32)*], [1 x i32 (i32, i32)*]* @"_Z3subii$SIMDTable", i64 0, i64 0), i32 (i32, i32)** getelementptr inbounds ([1 x i32 (i32, i32)*], [1 x i32 (i32, i32)*]* @"_Z3subii$SIMDTable", i64 0, i64 0), i32 (i32, i32)** getelementptr inbounds ([1 x i32 (i32, i32)*], [1 x i32 (i32, i32)*]* @"_Z3subii$SIMDTable", i64 0, i64 0), i32 (i32, i32)** getelementptr inbounds ([1 x i32 (i32, i32)*], [1 x i32 (i32, i32)*]* @"_Z3subii$SIMDTable", i64 0, i64 0), i32 (i32, i32)** getelementptr inbounds ([1 x i32 (i32, i32)*], [1 x i32 (i32, i32)*]* @"_Z3subii$SIMDTable", i64 0, i64 0), i32 (i32, i32)** getelementptr inbounds ([1 x i32 (i32, i32)*], [1 x i32 (i32, i32)*]* @"_Z3subii$SIMDTable", i64 0, i64 0), i32 (i32, i32)** getelementptr inbounds ([1 x i32 (i32, i32)*], [1 x i32 (i32, i32)*]* @"_Z3subii$SIMDTable", i64 0, i64 0), i32 (i32, i32)** getelementptr inbounds ([1 x i32 (i32, i32)*], [1 x i32 (i32, i32)*]* @"_Z3subii$SIMDTable", i64 0, i64 0), i32 (i32, i32)** getelementptr inbounds ([1 x i32 (i32, i32)*], [1 x i32 (i32, i32)*]* @"_Z3subii$SIMDTable", i64 0, i64 0), i32 (i32, i32)** getelementptr inbounds ([1 x i32 (i32, i32)*], [1 x i32 (i32, i32)*]* @"_Z3subii$SIMDTable", i64 0, i64 0), i32 (i32, i32)** getelementptr inbounds ([1 x i32 (i32, i32)*], [1 x i32 (i32, i32)*]* @"_Z3subii$SIMDTable", i64 0, i64 0), i32 (i32, i32)** getelementptr inbounds ([1 x i32 (i32, i32)*], [1 x i32 (i32, i32)*]* @"_Z3subii$SIMDTable", i64 0, i64 0), i32 (i32, i32)** getelementptr inbounds ([1 x i32 (i32, i32)*], [1 x i32 (i32, i32)*]* @"_Z3subii$SIMDTable", i64 0, i64 0), i32 (i32, i32)** getelementptr inbounds ([1 x i32 (i32, i32)*], [1 x i32 (i32, i32)*]* @"_Z3subii$SIMDTable", i64 0, i64 0), i32 (i32, i32)** getelementptr inbounds ([1 x i32 (i32, i32)*], [1 x i32 (i32, i32)*]* @"_Z3subii$SIMDTable", i64 0, i64 0), i32 (i32, i32)** getelementptr inbounds ([1 x i32 (i32, i32)*], [1 x i32 (i32, i32)*]* @"_Z3subii$SIMDTable", i64 0, i64 0)>
  br label %indirect.call.loop.entryvector_func

indirect.call.loop.entryvector_func:              ; preds = %indirect.call.loop.latchvector_func, %entryvector_func
  %vector_of_func_ptrsvector_func = phi <16 x i32 (i32, i32)**> [ %21, %entryvector_func ], [ %current_vector_of_func_ptrsvector_func, %indirect.call.loop.latchvector_func ]
  %cur_indirect_call_returnvector_func = phi <16 x i32> [ zeroinitializer, %entryvector_func ], [ %final_indirect_call_returnvector_func, %indirect.call.loop.latchvector_func ]
  %indxvector_func = phi i64 [ 0, %entryvector_func ], [ %indx_updatedvector_func, %indirect.call.loop.latchvector_func ]
  %current_fptrvector_func = extractelement <16 x i32 (i32, i32)**> %vector_of_func_ptrsvector_func, i64 %indxvector_func
  %is_visitedvector_func = icmp eq i32 (i32, i32)** %current_fptrvector_func, null
  br i1 %is_visitedvector_func, label %indirect.call.loop.latchvector_func, label %vector.indirect.callvector_func

vector.indirect.callvector_func:                  ; preds = %indirect.call.loop.entryvector_func
  %current.fptr.splatinsertvector_func = insertelement <16 x i32 (i32, i32)**> undef, i32 (i32, i32)** %current_fptrvector_func, i32 0
  %current.fptr.splatvector_func = shufflevector <16 x i32 (i32, i32)**> %current.fptr.splatinsertvector_func, <16 x i32 (i32, i32)**> undef, <16 x i32> zeroinitializer
  %func_ptr_maskvector_func = icmp eq <16 x i32 (i32, i32)**> %current.fptr.splatvector_func, %vector_of_func_ptrsvector_func
  %vector_func_ptr_maskvector_func = sext <16 x i1> %func_ptr_maskvector_func to <16 x i32>
  %22 = getelementptr i32 (i32, i32)*, i32 (i32, i32)** %current_fptrvector_func, i64 1
  %23 = bitcast i32 (i32, i32)** %22 to <16 x i32> (<16 x i32>, <16 x i32>, <16 x i32>)**
  %24 = load <16 x i32> (<16 x i32>, <16 x i32>, <16 x i32>)*, <16 x i32> (<16 x i32>, <16 x i32>, <16 x i32>)** %23, align 8
  %25 = call <16 x i32> %24(<16 x i32> %18, <16 x i32> %20, <16 x i32> %vector_func_ptr_maskvector_func)
  %indirect_call_return_updatedvector_func = select <16 x i1> %func_ptr_maskvector_func, <16 x i32> %25, <16 x i32> %cur_indirect_call_returnvector_func
  %vector_of_func_ptrs_updatedvector_func = select <16 x i1> %func_ptr_maskvector_func, <16 x i32 (i32, i32)**> zeroinitializer, <16 x i32 (i32, i32)**> %vector_of_func_ptrsvector_func
  br label %indirect.call.loop.latchvector_func

indirect.call.loop.latchvector_func:              ; preds = %vector.indirect.callvector_func, %indirect.call.loop.entryvector_func
  %final_indirect_call_returnvector_func = phi <16 x i32> [ %indirect_call_return_updatedvector_func, %vector.indirect.callvector_func ], [ %cur_indirect_call_returnvector_func, %indirect.call.loop.entryvector_func ]
  %current_vector_of_func_ptrsvector_func = phi <16 x i32 (i32, i32)**> [ %vector_of_func_ptrs_updatedvector_func, %vector.indirect.callvector_func ], [ %vector_of_func_ptrsvector_func, %indirect.call.loop.entryvector_func ]
  %indx_updatedvector_func = add i64 %indxvector_func, 1
  %exitcondvector_func = icmp eq i64 %indx_updatedvector_func, 16
  br i1 %exitcondvector_func, label %indirect.call.loop.exitvector_func, label %indirect.call.loop.entryvector_func

indirect.call.loop.exitvector_func:               ; preds = %indirect.call.loop.latchvector_func
  %26 = sext <16 x i32> %final_indirect_call_returnvector_func to <16 x i64>
  store <16 x i64> %26, <16 x i64> addrspace(1)* %17, align 8
  %dim_0_vector_inc_ind_var = add nuw nsw i64 %dim_0_vector_ind_var, 1
  %dim_0_vector_cmp.to.max = icmp eq i64 %dim_0_vector_inc_ind_var, %vector.size
  %dim_0_vector_inc_tid = add nuw nsw i64 %dim_0_vector_tid, 16
  br i1 %dim_0_vector_cmp.to.max, label %dim_0_vector_exit, label %entryvector_func

dim_0_vector_exit:                                ; preds = %indirect.call.loop.exitvector_func
  %dim_1_vector_inc_ind_var = add nuw nsw i64 %dim_1_vector_ind_var, 1
  %dim_1_vector_cmp.to.max = icmp eq i64 %dim_1_vector_inc_ind_var, %6
  br i1 %dim_1_vector_cmp.to.max, label %dim_1_vector_exit, label %dim_0_vector_pre_head

dim_1_vector_exit:                                ; preds = %dim_0_vector_exit
  %dim_2_vector_inc_ind_var = add nuw nsw i64 %dim_2_vector_ind_var, 1
  %dim_2_vector_cmp.to.max = icmp eq i64 %dim_2_vector_inc_ind_var, %8
  br i1 %dim_2_vector_cmp.to.max, label %dim_2_vector_exit, label %dim_1_vector_pre_head

dim_2_vector_exit:                                ; preds = %dim_1_vector_exit
  br label %scalarIf

scalarIf:                                         ; preds = %dim_2_vector_exit, %vect_if
  %27 = icmp ne i64 %scalar.size, 0
  br i1 %27, label %dim_2_pre_head, label %ret

dim_2_pre_head:                                   ; preds = %scalarIf
  br label %dim_1_pre_head

dim_1_pre_head:                                   ; preds = %dim_1_exit, %dim_2_pre_head
  %dim_2_ind_var = phi i64 [ 0, %dim_2_pre_head ], [ %dim_2_inc_ind_var, %dim_1_exit ]
  br label %dim_0_pre_head

dim_0_pre_head:                                   ; preds = %dim_0_exit, %dim_1_pre_head
  %dim_1_ind_var = phi i64 [ 0, %dim_1_pre_head ], [ %dim_1_inc_ind_var, %dim_0_exit ]
  br label %scalar_kernel_entry

scalar_kernel_entry:                              ; preds = %scalar_kernel_entry, %dim_0_pre_head
  %dim_0_ind_var = phi i64 [ 0, %dim_0_pre_head ], [ %dim_0_inc_ind_var, %scalar_kernel_entry ]
  %dim_0_tid = phi i64 [ %max.vector.gid, %dim_0_pre_head ], [ %dim_0_inc_tid, %scalar_kernel_entry ]
  %28 = getelementptr inbounds %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range", %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"* %_arg_3, i64 0, i32 0, i32 0, i64 0
  %29 = load i64, i64* %28, align 8
  %add.ptr.i30 = getelementptr inbounds i64, i64 addrspace(1)* %_arg_, i64 %29
  %30 = getelementptr inbounds %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range", %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"* %_arg_8, i64 0, i32 0, i32 0, i64 0
  %31 = load i64, i64* %30, align 8
  %add.ptr.i = getelementptr inbounds i64, i64 addrspace(1)* %_arg_4, i64 %31
  %rem.i.i = and i64 %dim_0_tid, 1
  %cmp.i.i = icmp eq i64 %rem.i.i, 0
  %32 = select i1 %cmp.i.i, i32 (i32, i32)* bitcast ([1 x i32 (i32, i32)*]* @"_Z3addii$SIMDTable" to i32 (i32, i32)*), i32 (i32, i32)* bitcast ([1 x i32 (i32, i32)*]* @"_Z3subii$SIMDTable" to i32 (i32, i32)*)
  %ptridx.i19.i = getelementptr inbounds i64, i64 addrspace(1)* %add.ptr.i30, i64 %dim_0_tid
  %33 = load i64, i64 addrspace(1)* %ptridx.i19.i, align 8
  %conv.i = trunc i64 %33 to i32
  %ptridx.i15.i = getelementptr inbounds i64, i64 addrspace(1)* %add.ptr.i, i64 %dim_0_tid
  %34 = load i64, i64 addrspace(1)* %ptridx.i15.i, align 8
  %conv7.i = trunc i64 %34 to i32
  %35 = bitcast i32 (i32, i32)* %32 to i32 (i32, i32)**
; CHECK: %36 = insertelement <16 x i32> undef, i32 %conv.i, i32 0
; CHECK-NEXT: %37 = insertelement <16 x i32> undef, i32 %conv7.i, i32 0
; CHECK-NEXT: %38 = bitcast i32 (i32, i32)** %35 to <16 x i32> (<16 x i32>, <16 x i32>, <16 x i32>)**
; CHECK-NEXT: %39 = getelementptr <16 x i32> (<16 x i32>, <16 x i32>, <16 x i32>)*, <16 x i32> (<16 x i32>, <16 x i32>, <16 x i32>)** %38, i32 0
; CHECK-NEXT: %40 = load <16 x i32> (<16 x i32>, <16 x i32>, <16 x i32>)*, <16 x i32> (<16 x i32>, <16 x i32>, <16 x i32>)** %39, align 8
; CHECK-NEXT: %41 = call <16 x i32> %40(<16 x i32> %36, <16 x i32> %37, <16 x i32> <i32 -1, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0>)
; CHECK-NEXT: %42 = extractelement <16 x i32> %41, i32 0
  %36 = tail call i32 (i32 (i32, i32)**, i32, i32, ...) @__intel_indirect_call(i32 (i32, i32)** %35, i32 %conv.i, i32 %conv7.i) #7
; CHECK-NOT: call {{.*}} @__intel_indirect_call
; CHECK-NEXT: %conv8.i = sext i32 %42 to i64
  %conv8.i = sext i32 %36 to i64
  store i64 %conv8.i, i64 addrspace(1)* %ptridx.i19.i, align 8
  %dim_0_inc_ind_var = add nuw nsw i64 %dim_0_ind_var, 1
  %dim_0_cmp.to.max = icmp eq i64 %dim_0_inc_ind_var, %scalar.size
  %dim_0_inc_tid = add nuw nsw i64 %dim_0_tid, 1
  br i1 %dim_0_cmp.to.max, label %dim_0_exit, label %scalar_kernel_entry

dim_0_exit:                                       ; preds = %scalar_kernel_entry
  %dim_1_inc_ind_var = add nuw nsw i64 %dim_1_ind_var, 1
  %dim_1_cmp.to.max = icmp eq i64 %dim_1_inc_ind_var, %6
  br i1 %dim_1_cmp.to.max, label %dim_1_exit, label %dim_0_pre_head

dim_1_exit:                                       ; preds = %dim_0_exit
  %dim_2_inc_ind_var = add nuw nsw i64 %dim_2_ind_var, 1
  %dim_2_cmp.to.max = icmp eq i64 %dim_2_inc_ind_var, %8
  br i1 %dim_2_cmp.to.max, label %dim_2_exit, label %dim_1_pre_head

dim_2_exit:                                       ; preds = %dim_1_exit
  br label %ret

ret:                                              ; preds = %dim_2_exit, %scalarIf
  br label %37

37:                                               ; preds = %0, %ret
  ret void
}

declare i32 @__intel_indirect_call(i32 (i32, i32)**, i32, i32, ...)

declare [7 x i64] @"WG.boundaries._ZTSZZ4mainENK3$_0clERN2cl4sycl7handlerEE1K"(i64 addrspace(1)* %0, %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"* %1, %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"* %2, %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"* %3, i64 addrspace(1)* %4, %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"* %5, %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"* %6, %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"* %7)

declare <16 x i32> @_ZGVdM16vv_3addii(<16 x i32> %A, <16 x i32> %B, <16 x i32> %mask)
declare <16 x i32> @_ZGVdN16vv_3addii(<16 x i32> %A, <16 x i32> %B)
declare <16 x i32> @_ZGVdM16vv_3subii(<16 x i32> %A, <16 x i32> %B, <16 x i32> %mask)
declare <16 x i32> @_ZGVdN16vv_3subii(<16 x i32> %A, <16 x i32> %B)

attributes #7 = { nounwind "vector-variants"="_ZGVdM16vv___intel_indirect_call_XXX,_ZGVdN16vv___intel_indirect_call_XXX" }

; DEBUGIFY-NOT: WARNING
