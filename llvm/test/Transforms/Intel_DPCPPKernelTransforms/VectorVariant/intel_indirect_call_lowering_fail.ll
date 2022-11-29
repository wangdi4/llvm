; RUN: opt %s -passes=dpcpp-kernel-indirect-call-lowering -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt %s -passes=dpcpp-kernel-indirect-call-lowering -S | FileCheck %s

; CHECK: "vector-variant-failure"="failed to find a masked vector variant for an indirect call"

%"class._ZTSN2cl4sycl5INTEL18function_ref_tunedIFiiiENS1_8int_listIJLi4ELi8EEEEJFNS1_8unmaskedENS1_7varyingES7_EEEE.cl::sycl::INTEL::function_ref_tuned" = type { %"struct._ZTSSt5arrayIPFiiiELm2EE.std::array" }
%"struct._ZTSSt5arrayIPFiiiELm2EE.std::array" = type { [2 x i32 (i32, i32)*] }
%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range" = type { %"class._ZTSN2cl4sycl6detail5arrayILi1EEE.cl::sycl::detail::array" }
%"class._ZTSN2cl4sycl6detail5arrayILi1EEE.cl::sycl::detail::array" = type { [1 x i64] }

define void @"_ZTSN2cl4sycl6detail19__pf_kernel_wrapperIZZ4mainENK3$_0clERNS0_7handlerEE1KEE"(%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"* byval(%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range") %_arg_, i64 addrspace(1)* %_arg_1, %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"* byval(%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range") %_arg_2, %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"* byval(%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range") %_arg_3, %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"* byval(%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range") %_arg_4, %"class._ZTSN2cl4sycl5INTEL18function_ref_tunedIFiiiENS1_8int_listIJLi4ELi8EEEEJFNS1_8unmaskedENS1_7varyingES7_EEEE.cl::sycl::INTEL::function_ref_tuned" addrspace(1)* %_arg_5, %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"* byval(%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range") %_arg_7, %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"* byval(%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range") %_arg_8, %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"* byval(%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range") %_arg_9, i64 addrspace(1)* %_arg_10, %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"* byval(%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range") %_arg_12, %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"* byval(%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range") %_arg_13, %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"* byval(%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range") %_arg_14) local_unnamed_addr {
  %early_exit_call = call [7 x i64] @"WG.boundaries._ZTSN2cl4sycl6detail19__pf_kernel_wrapperIZZ4mainENK3$_0clERNS0_7handlerEE1KEE"(%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"* %_arg_, i64 addrspace(1)* %_arg_1, %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"* %_arg_2, %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"* %_arg_3, %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"* %_arg_4, %"class._ZTSN2cl4sycl5INTEL18function_ref_tunedIFiiiENS1_8int_listIJLi4ELi8EEEEJFNS1_8unmaskedENS1_7varyingES7_EEEE.cl::sycl::INTEL::function_ref_tuned" addrspace(1)* %_arg_5, %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"* %_arg_7, %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"* %_arg_8, %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"* %_arg_9, i64 addrspace(1)* %_arg_10, %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"* %_arg_12, %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"* %_arg_13, %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"* %_arg_14)
  %1 = extractvalue [7 x i64] %early_exit_call, 0
  %2 = trunc i64 %1 to i1
  br i1 %2, label %WGLoopsEntry, label %35

WGLoopsEntry:                                     ; preds = %0
  %3 = extractvalue [7 x i64] %early_exit_call, 1
  %4 = extractvalue [7 x i64] %early_exit_call, 2
  %vector.size = ashr i64 %4, 3
  %num.vector.wi = shl i64 %vector.size, 3
  %max.vector.gid = add i64 %num.vector.wi, %3
  %scalar.size = sub i64 %4, %num.vector.wi
  br label %vect_if

vect_if:                                          ; preds = %WGLoopsEntry
  %5 = icmp ne i64 %vector.size, 0
  br i1 %5, label %dim_0_vector_pre_head, label %scalarIf

dim_0_vector_pre_head:                            ; preds = %vect_if
  br label %entryvector_func

entryvector_func:                                 ; preds = %indirect.call.loop.exitvector_func, %dim_0_vector_pre_head
  %dim_0_vector_ind_var = phi i64 [ 0, %dim_0_vector_pre_head ], [ %dim_0_vector_inc_ind_var, %indirect.call.loop.exitvector_func ]
  %dim_0_vector_tid = phi i64 [ %3, %dim_0_vector_pre_head ], [ %dim_0_vector_inc_tid, %indirect.call.loop.exitvector_func ]
  %6 = getelementptr inbounds %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range", %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"* %_arg_14, i64 0, i32 0, i32 0, i64 0
  %7 = load i64, i64* %6, align 8
  %add.ptr.ivector_func = getelementptr inbounds i64, i64 addrspace(1)* %_arg_10, i64 %7
  %8 = getelementptr inbounds %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range", %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"* %_arg_4, i64 0, i32 0, i32 0, i64 0
  %9 = load i64, i64* %8, align 8
  %add.ptr.i50vector_func = getelementptr inbounds i64, i64 addrspace(1)* %_arg_1, i64 %9
  %10 = getelementptr inbounds %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range", %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"* %_arg_9, i64 0, i32 0, i32 0, i64 0
  %11 = load i64, i64* %10, align 8
  %add.ptr.i39vector_func = getelementptr inbounds %"class._ZTSN2cl4sycl5INTEL18function_ref_tunedIFiiiENS1_8int_listIJLi4ELi8EEEEJFNS1_8unmaskedENS1_7varyingES7_EEEE.cl::sycl::INTEL::function_ref_tuned", %"class._ZTSN2cl4sycl5INTEL18function_ref_tunedIFiiiENS1_8int_listIJLi4ELi8EEEEJFNS1_8unmaskedENS1_7varyingES7_EEEE.cl::sycl::INTEL::function_ref_tuned" addrspace(1)* %_arg_5, i64 %11
  %broadcast.splatinsertvector_func = insertelement <8 x i64> poison, i64 %dim_0_vector_tid, i32 0
  %broadcast.splatvector_func = shufflevector <8 x i64> %broadcast.splatinsertvector_func, <8 x i64> poison, <8 x i32> zeroinitializer
  %broadcast.splatinsert2vector_func = insertelement <8 x %"class._ZTSN2cl4sycl5INTEL18function_ref_tunedIFiiiENS1_8int_listIJLi4ELi8EEEEJFNS1_8unmaskedENS1_7varyingES7_EEEE.cl::sycl::INTEL::function_ref_tuned" addrspace(1)*> poison, %"class._ZTSN2cl4sycl5INTEL18function_ref_tunedIFiiiENS1_8int_listIJLi4ELi8EEEEJFNS1_8unmaskedENS1_7varyingES7_EEEE.cl::sycl::INTEL::function_ref_tuned" addrspace(1)* %add.ptr.i39vector_func, i32 0
  %broadcast.splat3vector_func = shufflevector <8 x %"class._ZTSN2cl4sycl5INTEL18function_ref_tunedIFiiiENS1_8int_listIJLi4ELi8EEEEJFNS1_8unmaskedENS1_7varyingES7_EEEE.cl::sycl::INTEL::function_ref_tuned" addrspace(1)*> %broadcast.splatinsert2vector_func, <8 x %"class._ZTSN2cl4sycl5INTEL18function_ref_tunedIFiiiENS1_8int_listIJLi4ELi8EEEEJFNS1_8unmaskedENS1_7varyingES7_EEEE.cl::sycl::INTEL::function_ref_tuned" addrspace(1)*> poison, <8 x i32> zeroinitializer
  %12 = add nuw <8 x i64> %broadcast.splatvector_func, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7>
  %13 = and <8 x i64> %12, <i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1>
  %scalar.gepvector_func = getelementptr inbounds i64, i64 addrspace(1)* %add.ptr.i50vector_func, i64 %dim_0_vector_tid
  %14 = bitcast i64 addrspace(1)* %scalar.gepvector_func to <8 x i64> addrspace(1)*
  %wide.loadvector_func = load <8 x i64>, <8 x i64> addrspace(1)* %14, align 8
  %15 = trunc <8 x i64> %wide.loadvector_func to <8 x i32>
  %scalar.gep4vector_func = getelementptr inbounds i64, i64 addrspace(1)* %add.ptr.ivector_func, i64 %dim_0_vector_tid
  %16 = bitcast i64 addrspace(1)* %scalar.gep4vector_func to <8 x i64> addrspace(1)*
  %wide.load5vector_func = load <8 x i64>, <8 x i64> addrspace(1)* %16, align 8
  %17 = trunc <8 x i64> %wide.load5vector_func to <8 x i32>
  %mm_vectorGEP6vector_func = getelementptr inbounds %"class._ZTSN2cl4sycl5INTEL18function_ref_tunedIFiiiENS1_8int_listIJLi4ELi8EEEEJFNS1_8unmaskedENS1_7varyingES7_EEEE.cl::sycl::INTEL::function_ref_tuned", <8 x %"class._ZTSN2cl4sycl5INTEL18function_ref_tunedIFiiiENS1_8int_listIJLi4ELi8EEEEJFNS1_8unmaskedENS1_7varyingES7_EEEE.cl::sycl::INTEL::function_ref_tuned" addrspace(1)*> %broadcast.splat3vector_func, <8 x i64> %13, <8 x i32> zeroinitializer, <8 x i32> zeroinitializer, <8 x i64> zeroinitializer
  %18 = addrspacecast <8 x i32 (i32, i32)* addrspace(1)*> %mm_vectorGEP6vector_func to <8 x i32 (i32, i32)* addrspace(4)*>
  br label %indirect.call.loop.entryvector_func

indirect.call.loop.entryvector_func:              ; preds = %indirect.call.loop.latchvector_func, %entryvector_func
  %vector_of_func_ptrsvector_func = phi <8 x i32 (i32, i32)* addrspace(4)*> [ %18, %entryvector_func ], [ %current_vector_of_func_ptrsvector_func, %indirect.call.loop.latchvector_func ]
  %cur_indirect_call_returnvector_func = phi <8 x i32> [ zeroinitializer, %entryvector_func ], [ %final_indirect_call_returnvector_func, %indirect.call.loop.latchvector_func ]
  %indxvector_func = phi i64 [ 0, %entryvector_func ], [ %indx_updatedvector_func, %indirect.call.loop.latchvector_func ]
  %current_fptrvector_func = extractelement <8 x i32 (i32, i32)* addrspace(4)*> %vector_of_func_ptrsvector_func, i64 %indxvector_func
  %is_visitedvector_func = icmp eq i32 (i32, i32)* addrspace(4)* %current_fptrvector_func, null
  br i1 %is_visitedvector_func, label %indirect.call.loop.latchvector_func, label %vector.indirect.callvector_func

vector.indirect.callvector_func:                  ; preds = %indirect.call.loop.entryvector_func
  %current.fptr.splatinsertvector_func = insertelement <8 x i32 (i32, i32)* addrspace(4)*> poison, i32 (i32, i32)* addrspace(4)* %current_fptrvector_func, i32 0
  %current.fptr.splatvector_func = shufflevector <8 x i32 (i32, i32)* addrspace(4)*> %current.fptr.splatinsertvector_func, <8 x i32 (i32, i32)* addrspace(4)*> poison, <8 x i32> zeroinitializer
  %func_ptr_maskvector_func = icmp eq <8 x i32 (i32, i32)* addrspace(4)*> %current.fptr.splatvector_func, %vector_of_func_ptrsvector_func
  %vector_func_ptr_maskvector_func = sext <8 x i1> %func_ptr_maskvector_func to <8 x i32>
  %19 = getelementptr i32 (i32, i32)*, i32 (i32, i32)* addrspace(4)* %current_fptrvector_func, i64 1
  %20 = bitcast i32 (i32, i32)* addrspace(4)* %19 to <8 x i32> (<8 x i32>, <8 x i32>, <8 x i32>) addrspace(4)* addrspace(4)*
  %21 = load <8 x i32> (<8 x i32>, <8 x i32>, <8 x i32>) addrspace(4)*, <8 x i32> (<8 x i32>, <8 x i32>, <8 x i32>) addrspace(4)* addrspace(4)* %20, align 8
  %22 = call addrspace(4) <8 x i32> %21(<8 x i32> %15, <8 x i32> %17, <8 x i32> %vector_func_ptr_maskvector_func)
  %indirect_call_return_updatedvector_func = select <8 x i1> %func_ptr_maskvector_func, <8 x i32> %22, <8 x i32> %cur_indirect_call_returnvector_func
  %vector_of_func_ptrs_updatedvector_func = select <8 x i1> %func_ptr_maskvector_func, <8 x i32 (i32, i32)* addrspace(4)*> zeroinitializer, <8 x i32 (i32, i32)* addrspace(4)*> %vector_of_func_ptrsvector_func
  br label %indirect.call.loop.latchvector_func

indirect.call.loop.latchvector_func:              ; preds = %vector.indirect.callvector_func, %indirect.call.loop.entryvector_func
  %final_indirect_call_returnvector_func = phi <8 x i32> [ %indirect_call_return_updatedvector_func, %vector.indirect.callvector_func ], [ %cur_indirect_call_returnvector_func, %indirect.call.loop.entryvector_func ]
  %current_vector_of_func_ptrsvector_func = phi <8 x i32 (i32, i32)* addrspace(4)*> [ %vector_of_func_ptrs_updatedvector_func, %vector.indirect.callvector_func ], [ %vector_of_func_ptrsvector_func, %indirect.call.loop.entryvector_func ]
  %indx_updatedvector_func = add i64 %indxvector_func, 1
  %exitcondvector_func = icmp eq i64 %indx_updatedvector_func, 8
  br i1 %exitcondvector_func, label %indirect.call.loop.exitvector_func, label %indirect.call.loop.entryvector_func

indirect.call.loop.exitvector_func:               ; preds = %indirect.call.loop.latchvector_func
  %23 = sext <8 x i32> %final_indirect_call_returnvector_func to <8 x i64>
  store <8 x i64> %23, <8 x i64> addrspace(1)* %14, align 8
  %dim_0_vector_inc_ind_var = add nuw nsw i64 %dim_0_vector_ind_var, 1
  %dim_0_vector_cmp.to.max = icmp eq i64 %dim_0_vector_inc_ind_var, %vector.size
  %dim_0_vector_inc_tid = add nuw nsw i64 %dim_0_vector_tid, 8
  br i1 %dim_0_vector_cmp.to.max, label %dim_0_vector_exit, label %entryvector_func

dim_0_vector_exit:                                ; preds = %indirect.call.loop.exitvector_func
  br label %scalarIf

scalarIf:                                         ; preds = %dim_0_vector_exit, %vect_if
  %24 = icmp ne i64 %scalar.size, 0
  br i1 %24, label %dim_0_pre_head, label %ret

dim_0_pre_head:                                   ; preds = %scalarIf
  br label %scalar_kernel_entry

scalar_kernel_entry:                              ; preds = %scalar_kernel_entry, %dim_0_pre_head
  %dim_0_ind_var = phi i64 [ 0, %dim_0_pre_head ], [ %dim_0_inc_ind_var, %scalar_kernel_entry ]
  %dim_0_tid = phi i64 [ %max.vector.gid, %dim_0_pre_head ], [ %dim_0_inc_tid, %scalar_kernel_entry ]
  %25 = getelementptr inbounds %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range", %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"* %_arg_14, i64 0, i32 0, i32 0, i64 0
  %26 = load i64, i64* %25, align 8
  %add.ptr.i = getelementptr inbounds i64, i64 addrspace(1)* %_arg_10, i64 %26
  %27 = getelementptr inbounds %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range", %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"* %_arg_9, i64 0, i32 0, i32 0, i64 0
  %28 = load i64, i64* %27, align 8
  %add.ptr.i39 = getelementptr inbounds %"class._ZTSN2cl4sycl5INTEL18function_ref_tunedIFiiiENS1_8int_listIJLi4ELi8EEEEJFNS1_8unmaskedENS1_7varyingES7_EEEE.cl::sycl::INTEL::function_ref_tuned", %"class._ZTSN2cl4sycl5INTEL18function_ref_tunedIFiiiENS1_8int_listIJLi4ELi8EEEEJFNS1_8unmaskedENS1_7varyingES7_EEEE.cl::sycl::INTEL::function_ref_tuned" addrspace(1)* %_arg_5, i64 %28
  %29 = getelementptr inbounds %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range", %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"* %_arg_4, i64 0, i32 0, i32 0, i64 0
  %30 = load i64, i64* %29, align 8
  %add.ptr.i50 = getelementptr inbounds i64, i64 addrspace(1)* %_arg_1, i64 %30
  %rem.i.i.i = and i64 %dim_0_tid, 1
  %ptridx.i16.i.i = getelementptr inbounds %"class._ZTSN2cl4sycl5INTEL18function_ref_tunedIFiiiENS1_8int_listIJLi4ELi8EEEEJFNS1_8unmaskedENS1_7varyingES7_EEEE.cl::sycl::INTEL::function_ref_tuned", %"class._ZTSN2cl4sycl5INTEL18function_ref_tunedIFiiiENS1_8int_listIJLi4ELi8EEEEJFNS1_8unmaskedENS1_7varyingES7_EEEE.cl::sycl::INTEL::function_ref_tuned" addrspace(1)* %add.ptr.i39, i64 %rem.i.i.i
  %ptridx.i24.i.i = getelementptr inbounds i64, i64 addrspace(1)* %add.ptr.i50, i64 %dim_0_tid
  %31 = load i64, i64 addrspace(1)* %ptridx.i24.i.i, align 8
  %conv.i.i = trunc i64 %31 to i32
  %ptridx.i20.i.i = getelementptr inbounds i64, i64 addrspace(1)* %add.ptr.i, i64 %dim_0_tid
  %32 = load i64, i64 addrspace(1)* %ptridx.i20.i.i, align 8
  %conv8.i.i = trunc i64 %32 to i32
  %arraydecay.i.i.i.i.i = getelementptr inbounds %"class._ZTSN2cl4sycl5INTEL18function_ref_tunedIFiiiENS1_8int_listIJLi4ELi8EEEEJFNS1_8unmaskedENS1_7varyingES7_EEEE.cl::sycl::INTEL::function_ref_tuned", %"class._ZTSN2cl4sycl5INTEL18function_ref_tunedIFiiiENS1_8int_listIJLi4ELi8EEEEJFNS1_8unmaskedENS1_7varyingES7_EEEE.cl::sycl::INTEL::function_ref_tuned" addrspace(1)* %ptridx.i16.i.i, i64 0, i32 0, i32 0, i64 0
  %33 = addrspacecast i32 (i32, i32)* addrspace(1)* %arraydecay.i.i.i.i.i to i32 (i32, i32)* addrspace(4)*
  %34 = tail call i32 (i32 (i32, i32)* addrspace(4)*, i32, i32, ...) @__intel_indirect_call(i32 (i32, i32)* addrspace(4)* %33, i32 %conv.i.i, i32 %conv8.i.i) #1
  %conv10.i.i = sext i32 %34 to i64
  store i64 %conv10.i.i, i64 addrspace(1)* %ptridx.i24.i.i, align 8
  %dim_0_inc_ind_var = add nuw nsw i64 %dim_0_ind_var, 1
  %dim_0_cmp.to.max = icmp eq i64 %dim_0_inc_ind_var, %scalar.size
  %dim_0_inc_tid = add nuw nsw i64 %dim_0_tid, 1
  br i1 %dim_0_cmp.to.max, label %dim_0_exit, label %scalar_kernel_entry

dim_0_exit:                                       ; preds = %scalar_kernel_entry
  br label %ret

ret:                                              ; preds = %dim_0_exit, %scalarIf
  br label %35

35:                                               ; preds = %0, %ret
  ret void
}

declare i32 @__intel_indirect_call(i32 (i32, i32)* addrspace(4)*, i32, i32, ...) local_unnamed_addr

declare [7 x i64] @"WG.boundaries._ZTSN2cl4sycl6detail19__pf_kernel_wrapperIZZ4mainENK3$_0clERNS0_7handlerEE1KEE"(%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"* %0, i64 addrspace(1)* %1, %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"* %2, %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"* %3, %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"* %4, %"class._ZTSN2cl4sycl5INTEL18function_ref_tunedIFiiiENS1_8int_listIJLi4ELi8EEEEJFNS1_8unmaskedENS1_7varyingES7_EEEE.cl::sycl::INTEL::function_ref_tuned" addrspace(1)* %5, %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"* %6, %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"* %7, %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"* %8, i64 addrspace(1)* %9, %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"* %10, %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"* %11, %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"* %12)

attributes #1 = { nounwind "vector-variants"="_ZGVdN4vv__$U0,_ZGVdN8vv__$U0" }

; DEBUGIFY-NOT: WARNING
