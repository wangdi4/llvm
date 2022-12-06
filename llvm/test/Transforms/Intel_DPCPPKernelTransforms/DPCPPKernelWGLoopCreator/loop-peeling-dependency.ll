; This test checks that dependent instructions of peel target are cloned in the correct order.
;
; RUN: opt -passes=dpcpp-kernel-wgloop-creator %s -S | FileCheck %s
; RUN: opt -passes=dpcpp-kernel-wgloop-creator %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s

%"class.cl::sycl::id" = type { %"class.cl::sycl::detail::array" }
%"class.cl::sycl::detail::array" = type { [1 x i64] }

define void @_ZTS18larger_sad_calc_16(i16 addrspace(1)* %_arg_blk_sad, i64 %_arg_image_width_macroblocks, i64 %_arg_image_height_macroblocks) !no_barrier_path !1 !vectorized_kernel !2 {
; CHECK-LABEL: @_ZTS18larger_sad_calc_16(
; CHECK:         [[MUL_I_IVECTOR_FUNC:%.*]] = mul i32 0, 0
; CHECK-NEXT:    [[MUL3_I_IVECTOR_FUNC:%.*]] = mul i32 [[MUL_I_IVECTOR_FUNC]], 0
; CHECK-NEXT:    [[CONV10_I_IVECTOR_FUNC:%.*]] = sext i32 [[MUL3_I_IVECTOR_FUNC]] to i64
; CHECK-NEXT:    [[ADD_PTR_I156_I_IVECTOR_FUNC:%.*]] = getelementptr inbounds i16, i16 addrspace(1)* [[_ARG_BLK_SAD:%.*]], i64 [[CONV10_I_IVECTOR_FUNC]]
; CHECK-NEXT:    [[CONV11_I_IVECTOR_FUNC:%.*]] = sext i32 [[MUL3_I_IVECTOR_FUNC]] to i64
; CHECK-NEXT:    [[ADD_PTR_I153_I_IVECTOR_FUNC:%.*]] = getelementptr inbounds i16, i16 addrspace(1)* [[ADD_PTR_I156_I_IVECTOR_FUNC]], i64 [[CONV11_I_IVECTOR_FUNC]]
; CHECK-NEXT:    [[TMP1:%.*]] = bitcast i16 addrspace(1)* [[ADD_PTR_I153_I_IVECTOR_FUNC]] to <16 x i16> addrspace(1)*
; CHECK-NEXT:    [[PEEL_PTR2INT:%.*]] = ptrtoint <16 x i16> addrspace(1)* [[TMP1]] to i64
entry:
  ret void
}

define void @_ZGVeN16uuu__ZTS18larger_sad_calc_16(i16 addrspace(1)* %_arg_blk_sad, i64 %_arg_image_width_macroblocks, i64 %_arg_image_height_macroblocks) !no_barrier_path !1 !vectorized_width !3 !vectorization_dimension !5 !can_unite_workgroups !1 {
entry:
  %mul.i.i = mul i32 0, 0
  %mul3.i.i = mul i32 %mul.i.i, 0
  %add8.i.i = add nsw i32 0, 0
  %mul9.i.i = shl nsw i32 0, 0
  %conv10.i.i = sext i32 %mul3.i.i to i64
  %add.ptr.i156.i.i = getelementptr inbounds i16, i16 addrspace(1)* %_arg_blk_sad, i64 %conv10.i.i
  %conv11.i.i = sext i32 %mul3.i.i to i64
  %add.ptr.i153.i.i = getelementptr inbounds i16, i16 addrspace(1)* %add.ptr.i156.i.i, i64 %conv11.i.i
  %scalar.gep = getelementptr inbounds i16, i16 addrspace(1)* %add.ptr.i153.i.i, i64 undef
  %0 = bitcast i16 addrspace(1)* %add.ptr.i153.i.i to <16 x i16> addrspace(1)*
  %wide.load = load <16 x i16>, <16 x i16> addrspace(1)* %0, align 2, !intel.preferred_alignment !4
  ret void
}

!sycl.kernels = !{!0}

!0 = !{void (i16 addrspace(1)*, i64, i64)* @_ZTS18larger_sad_calc_16}
!1 = !{i1 true}
!2 = !{void (i16 addrspace(1)*, i64, i64)* @_ZGVeN16uuu__ZTS18larger_sad_calc_16}
!3 = !{i32 16}
!4 = !{i32 32}
!5 = !{i32 0}

; DEBUGIFY-COUNT-60: WARNING: Instruction with empty DebugLoc in function _ZTS18larger_sad_calc_16
; DEBUGIFY-NEXT: WARNING: Missing line 13
; DEBUGIFY-NEXT: PASS
