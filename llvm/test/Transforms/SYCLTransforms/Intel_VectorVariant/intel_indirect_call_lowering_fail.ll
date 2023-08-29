; RUN: not opt %s -passes=sycl-kernel-indirect-call-lowering -S -disable-output 2>&1 | FileCheck %s

; CHECK: error: Function '_ZTSN2cl4sycl6detail19__pf_kernel_wrapperIZZ4mainENK3$_0clERNS0_7handlerEE1KEE': failed to find a masked vector variant for an indirect call

%"class._ZTSN2cl4sycl5INTEL18function_ref_tunedIFiiiENS1_8int_listIJLi4ELi8EEEEJFNS1_8unmaskedENS1_7varyingES7_EEEE.cl::sycl::INTEL::function_ref_tuned" = type { %"struct._ZTSSt5arrayIPFiiiELm2EE.std::array" }
%"struct._ZTSSt5arrayIPFiiiELm2EE.std::array" = type { [2 x ptr] }
%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range" = type { %"class._ZTSN2cl4sycl6detail5arrayILi1EEE.cl::sycl::detail::array" }
%"class._ZTSN2cl4sycl6detail5arrayILi1EEE.cl::sycl::detail::array" = type { [1 x i64] }

define void @"_ZTSN2cl4sycl6detail19__pf_kernel_wrapperIZZ4mainENK3$_0clERNS0_7handlerEE1KEE"(ptr byval(%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range") %_arg_, ptr addrspace(1) %_arg_1, ptr byval(%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range") %_arg_2, ptr byval(%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range") %_arg_3, ptr byval(%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range") %_arg_4, ptr addrspace(1) %_arg_5, ptr byval(%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range") %_arg_7, ptr byval(%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range") %_arg_8, ptr byval(%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range") %_arg_9, ptr addrspace(1) %_arg_10, ptr byval(%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range") %_arg_12, ptr byval(%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range") %_arg_13, ptr byval(%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range") %_arg_14) local_unnamed_addr !kernel_arg_base_type !0 !arg_type_null_val !1 {
  %early_exit_call = call [7 x i64] @"WG.boundaries._ZTSN2cl4sycl6detail19__pf_kernel_wrapperIZZ4mainENK3$_0clERNS0_7handlerEE1KEE"(ptr %_arg_, ptr addrspace(1) %_arg_1, ptr %_arg_2, ptr %_arg_3, ptr %_arg_4, ptr addrspace(1) %_arg_5, ptr %_arg_7, ptr %_arg_8, ptr %_arg_9, ptr addrspace(1) %_arg_10, ptr %_arg_12, ptr %_arg_13, ptr %_arg_14)
  %1 = extractvalue [7 x i64] %early_exit_call, 0
  %2 = trunc i64 %1 to i1
  br i1 %2, label %WGLoopsEntry, label %ret.1

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
  %6 = load i64, ptr %_arg_14, align 8
  %add.ptr.ivector_func = getelementptr inbounds i64, ptr addrspace(1) %_arg_10, i64 %6
  %7 = load i64, ptr %_arg_4, align 8
  %add.ptr.i50vector_func = getelementptr inbounds i64, ptr addrspace(1) %_arg_1, i64 %7
  %8 = load i64, ptr %_arg_9, align 8
  %add.ptr.i39vector_func = getelementptr inbounds %"class._ZTSN2cl4sycl5INTEL18function_ref_tunedIFiiiENS1_8int_listIJLi4ELi8EEEEJFNS1_8unmaskedENS1_7varyingES7_EEEE.cl::sycl::INTEL::function_ref_tuned", ptr addrspace(1) %_arg_5, i64 %8
  %broadcast.splatinsertvector_func = insertelement <8 x i64> poison, i64 %dim_0_vector_tid, i32 0
  %broadcast.splatvector_func = shufflevector <8 x i64> %broadcast.splatinsertvector_func, <8 x i64> poison, <8 x i32> zeroinitializer
  %broadcast.splatinsert2vector_func = insertelement <8 x ptr addrspace(1)> poison, ptr addrspace(1) %add.ptr.i39vector_func, i32 0
  %broadcast.splat3vector_func = shufflevector <8 x ptr addrspace(1)> %broadcast.splatinsert2vector_func, <8 x ptr addrspace(1)> poison, <8 x i32> zeroinitializer
  %9 = add nuw <8 x i64> %broadcast.splatvector_func, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7>
  %10 = and <8 x i64> %9, <i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1>
  %scalar.gepvector_func = getelementptr inbounds i64, ptr addrspace(1) %add.ptr.i50vector_func, i64 %dim_0_vector_tid
  %wide.loadvector_func = load <8 x i64>, ptr addrspace(1) %scalar.gepvector_func, align 8
  %11 = trunc <8 x i64> %wide.loadvector_func to <8 x i32>
  %scalar.gep4vector_func = getelementptr inbounds i64, ptr addrspace(1) %add.ptr.ivector_func, i64 %dim_0_vector_tid
  %wide.load5vector_func = load <8 x i64>, ptr addrspace(1) %scalar.gep4vector_func, align 8
  %12 = trunc <8 x i64> %wide.load5vector_func to <8 x i32>
  %mm_vectorGEP6vector_func = getelementptr inbounds %"class._ZTSN2cl4sycl5INTEL18function_ref_tunedIFiiiENS1_8int_listIJLi4ELi8EEEEJFNS1_8unmaskedENS1_7varyingES7_EEEE.cl::sycl::INTEL::function_ref_tuned", <8 x ptr addrspace(1)> %broadcast.splat3vector_func, <8 x i64> %10, <8 x i32> zeroinitializer, <8 x i32> zeroinitializer, <8 x i64> zeroinitializer
  %13 = addrspacecast <8 x ptr addrspace(1)> %mm_vectorGEP6vector_func to <8 x ptr addrspace(4)>
  br label %indirect.call.loop.entryvector_func

indirect.call.loop.entryvector_func:              ; preds = %indirect.call.loop.latchvector_func, %entryvector_func
  %vector_of_func_ptrsvector_func = phi <8 x ptr addrspace(4)> [ %13, %entryvector_func ], [ %current_vector_of_func_ptrsvector_func, %indirect.call.loop.latchvector_func ]
  %cur_indirect_call_returnvector_func = phi <8 x i32> [ zeroinitializer, %entryvector_func ], [ %final_indirect_call_returnvector_func, %indirect.call.loop.latchvector_func ]
  %indxvector_func = phi i64 [ 0, %entryvector_func ], [ %indx_updatedvector_func, %indirect.call.loop.latchvector_func ]
  %current_fptrvector_func = extractelement <8 x ptr addrspace(4)> %vector_of_func_ptrsvector_func, i64 %indxvector_func
  %is_visitedvector_func = icmp eq ptr addrspace(4) %current_fptrvector_func, null
  br i1 %is_visitedvector_func, label %indirect.call.loop.latchvector_func, label %vector.indirect.callvector_func

vector.indirect.callvector_func:                  ; preds = %indirect.call.loop.entryvector_func
  %current.fptr.splatinsertvector_func = insertelement <8 x ptr addrspace(4)> poison, ptr addrspace(4) %current_fptrvector_func, i32 0
  %current.fptr.splatvector_func = shufflevector <8 x ptr addrspace(4)> %current.fptr.splatinsertvector_func, <8 x ptr addrspace(4)> poison, <8 x i32> zeroinitializer
  %func_ptr_maskvector_func = icmp eq <8 x ptr addrspace(4)> %current.fptr.splatvector_func, %vector_of_func_ptrsvector_func
  %vector_func_ptr_maskvector_func = sext <8 x i1> %func_ptr_maskvector_func to <8 x i32>
  %14 = getelementptr ptr, ptr addrspace(4) %current_fptrvector_func, i64 1
  %15 = load ptr addrspace(4), ptr addrspace(4) %14, align 8
  %16 = call addrspace(4) <8 x i32> %15(<8 x i32> %11, <8 x i32> %12, <8 x i32> %vector_func_ptr_maskvector_func)
  %indirect_call_return_updatedvector_func = select <8 x i1> %func_ptr_maskvector_func, <8 x i32> %16, <8 x i32> %cur_indirect_call_returnvector_func
  %vector_of_func_ptrs_updatedvector_func = select <8 x i1> %func_ptr_maskvector_func, <8 x ptr addrspace(4)> zeroinitializer, <8 x ptr addrspace(4)> %vector_of_func_ptrsvector_func
  br label %indirect.call.loop.latchvector_func

indirect.call.loop.latchvector_func:              ; preds = %vector.indirect.callvector_func, %indirect.call.loop.entryvector_func
  %final_indirect_call_returnvector_func = phi <8 x i32> [ %indirect_call_return_updatedvector_func, %vector.indirect.callvector_func ], [ %cur_indirect_call_returnvector_func, %indirect.call.loop.entryvector_func ]
  %current_vector_of_func_ptrsvector_func = phi <8 x ptr addrspace(4)> [ %vector_of_func_ptrs_updatedvector_func, %vector.indirect.callvector_func ], [ %vector_of_func_ptrsvector_func, %indirect.call.loop.entryvector_func ]
  %indx_updatedvector_func = add i64 %indxvector_func, 1
  %exitcondvector_func = icmp eq i64 %indx_updatedvector_func, 8
  br i1 %exitcondvector_func, label %indirect.call.loop.exitvector_func, label %indirect.call.loop.entryvector_func

indirect.call.loop.exitvector_func:               ; preds = %indirect.call.loop.latchvector_func
  %17 = sext <8 x i32> %final_indirect_call_returnvector_func to <8 x i64>
  store <8 x i64> %17, ptr addrspace(1) %scalar.gepvector_func, align 8
  %dim_0_vector_inc_ind_var = add nuw nsw i64 %dim_0_vector_ind_var, 1
  %dim_0_vector_cmp.to.max = icmp eq i64 %dim_0_vector_inc_ind_var, %vector.size
  %dim_0_vector_inc_tid = add nuw nsw i64 %dim_0_vector_tid, 8
  br i1 %dim_0_vector_cmp.to.max, label %dim_0_vector_exit, label %entryvector_func

dim_0_vector_exit:                                ; preds = %indirect.call.loop.exitvector_func
  br label %scalarIf

scalarIf:                                         ; preds = %dim_0_vector_exit, %vect_if
  %18 = icmp ne i64 %scalar.size, 0
  br i1 %18, label %dim_0_pre_head, label %ret

dim_0_pre_head:                                   ; preds = %scalarIf
  br label %scalar_kernel_entry

scalar_kernel_entry:                              ; preds = %scalar_kernel_entry, %dim_0_pre_head
  %dim_0_ind_var = phi i64 [ 0, %dim_0_pre_head ], [ %dim_0_inc_ind_var, %scalar_kernel_entry ]
  %dim_0_tid = phi i64 [ %max.vector.gid, %dim_0_pre_head ], [ %dim_0_inc_tid, %scalar_kernel_entry ]
  %19 = load i64, ptr %_arg_14, align 8
  %add.ptr.i = getelementptr inbounds i64, ptr addrspace(1) %_arg_10, i64 %19
  %20 = load i64, ptr %_arg_9, align 8
  %add.ptr.i39 = getelementptr inbounds %"class._ZTSN2cl4sycl5INTEL18function_ref_tunedIFiiiENS1_8int_listIJLi4ELi8EEEEJFNS1_8unmaskedENS1_7varyingES7_EEEE.cl::sycl::INTEL::function_ref_tuned", ptr addrspace(1) %_arg_5, i64 %20
  %21 = load i64, ptr %_arg_4, align 8
  %add.ptr.i50 = getelementptr inbounds i64, ptr addrspace(1) %_arg_1, i64 %21
  %rem.i.i.i = and i64 %dim_0_tid, 1
  %ptridx.i16.i.i = getelementptr inbounds %"class._ZTSN2cl4sycl5INTEL18function_ref_tunedIFiiiENS1_8int_listIJLi4ELi8EEEEJFNS1_8unmaskedENS1_7varyingES7_EEEE.cl::sycl::INTEL::function_ref_tuned", ptr addrspace(1) %add.ptr.i39, i64 %rem.i.i.i
  %ptridx.i24.i.i = getelementptr inbounds i64, ptr addrspace(1) %add.ptr.i50, i64 %dim_0_tid
  %22 = load i64, ptr addrspace(1) %ptridx.i24.i.i, align 8
  %conv.i.i = trunc i64 %22 to i32
  %ptridx.i20.i.i = getelementptr inbounds i64, ptr addrspace(1) %add.ptr.i, i64 %dim_0_tid
  %23 = load i64, ptr addrspace(1) %ptridx.i20.i.i, align 8
  %conv8.i.i = trunc i64 %23 to i32
  %24 = addrspacecast ptr addrspace(1) %ptridx.i16.i.i to ptr addrspace(4)
  %25 = tail call i32 (ptr addrspace(4), i32, i32, ...) @__intel_indirect_call(ptr addrspace(4) %24, i32 %conv.i.i, i32 %conv8.i.i) #1
  %conv10.i.i = sext i32 %25 to i64
  store i64 %conv10.i.i, ptr addrspace(1) %ptridx.i24.i.i, align 8
  %dim_0_inc_ind_var = add nuw nsw i64 %dim_0_ind_var, 1
  %dim_0_cmp.to.max = icmp eq i64 %dim_0_inc_ind_var, %scalar.size
  %dim_0_inc_tid = add nuw nsw i64 %dim_0_tid, 1
  br i1 %dim_0_cmp.to.max, label %dim_0_exit, label %scalar_kernel_entry

dim_0_exit:                                       ; preds = %scalar_kernel_entry
  br label %ret

ret:                                              ; preds = %dim_0_exit, %scalarIf
  br label %ret.1

ret.1:                                               ; preds = %0, %ret
  ret void
}

declare i32 @__intel_indirect_call(ptr addrspace(4), i32, i32, ...) local_unnamed_addr

declare [7 x i64] @"WG.boundaries._ZTSN2cl4sycl6detail19__pf_kernel_wrapperIZZ4mainENK3$_0clERNS0_7handlerEE1KEE"(ptr %0, ptr addrspace(1) %1, ptr %2, ptr %3, ptr %4, ptr addrspace(1) %5, ptr %_arg_14, ptr %6, ptr %_arg_4, ptr addrspace(1) %7, ptr %_arg_9, ptr %8, ptr %9)

attributes #1 = { nounwind "vector-variants"="_ZGVdN4vv__$U0,_ZGVdN8vv__$U0" }

!0 = !{!"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range", !"long*", !"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range", !"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range", !"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range", !"class._ZTSN2cl4sycl5INTEL18function_ref_tunedIFiiiENS1_8int_listIJLi4ELi8EEEEJFNS1_8unmaskedENS1_7varyingES7_EEEE.cl::sycl::INTEL::function_ref_tuned", !"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range", !"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range", !"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range", !"long*", !"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range", !"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range", !"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"}
!1 = !{ptr null, ptr null, ptr null, ptr null, ptr null, ptr addrspace(1) null, ptr null, ptr null, ptr null, ptr addrspace(1) null, ptr null, ptr null, ptr null}
