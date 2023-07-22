; RUN: opt -passes=sycl-kernel-barrier -S < %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-barrier -S < %s | FileCheck %s

; This test is to check if the function argument is correctly marked as special
; one (loaded from special buffer). That will happen if the functions with
; synchronization instructions are not sorted by depth-first travering call
; graph.


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

%struct.custom_type = type { %struct.custom_type_nested, i64 }
%struct.custom_type_nested = type { i32, float }
%"class.sycl::_V1::group" = type { %"class.sycl::_V1::range", %"class.sycl::_V1::range", %"class.sycl::_V1::range", %"class.sycl::_V1::range" }
%"class.sycl::_V1::range" = type { %"class.sycl::_V1::detail::array" }
%"class.sycl::_V1::detail::array" = type { [1 x i64] }
%class.anon = type <{ %"class.sycl::_V1::local_accessor", i64, %"class.sycl::_V1::accessor", %"class.sycl::_V1::accessor", %struct.UserDefinedSum, [7 x i8] }>
%"class.sycl::_V1::local_accessor" = type { %"class.sycl::_V1::local_accessor_base" }
%"class.sycl::_V1::local_accessor_base" = type { %"class.sycl::_V1::detail::LocalAccessorBaseDevice", ptr addrspace(3) }
%"class.sycl::_V1::detail::LocalAccessorBaseDevice" = type { %"class.sycl::_V1::range", %"class.sycl::_V1::range", %"class.sycl::_V1::range" }
%"class.sycl::_V1::accessor" = type { %"class.sycl::_V1::detail::LocalAccessorBaseDevice", %union.anon }
%union.anon = type { ptr addrspace(1) }
%struct.UserDefinedSum = type { i8 }

define linkonce_odr void @_ZN4sycl3_V13ext6oneapi12experimental6detail22reduce_over_group_implINS3_21group_with_scratchpadINS0_5groupILi1EEELm18446744073709551615EEE11custom_type14UserDefinedSumISA_EEET0_T_SD_mT1_(ptr addrspace(4) %agg.result) {
entry:
  call void @dummy_barrier.()
  call void @_ZN4sycl3_V115group_broadcastINS0_5groupILi1EEE11custom_typeEENSt9enable_ifIXaa10is_group_vINSt5decayIT_E4typeEEoosr3std21is_trivially_copyableIT0_EE5valuesr6detail6is_vecISA_EE5valueESA_E4typeES7_SA_(ptr addrspace(4) noalias sret(%struct.custom_type) align 8 %agg.result)
  ret void
}

define void @_ZTSZZ4testISt5arrayI11custom_typeLm128EES0_IS1_Lm1EE14UserDefinedSumIS1_EEvN4sycl3_V15queueET_T0_T1_mNSA_10value_typeESC_ENKUlRNS7_7handlerEE_clESE_EUlNS7_7nd_itemILi1EEEE_() !no_barrier_path !1 {
entry:
  call void @dummy_barrier.()
  %__SYCLKernel = alloca %class.anon, align 8
  %__SYCLKernel.ascast = addrspacecast ptr %__SYCLKernel to ptr addrspace(4)
  br label %Split.Barrier.BB1

Split.Barrier.BB1:                                ; preds = %entry
  call void @_Z18work_group_barrierj(i32 0)
  call void @_ZZZ4testISt5arrayI11custom_typeLm128EES0_IS1_Lm1EE14UserDefinedSumIS1_EEvN4sycl3_V15queueET_T0_T1_mNSA_10value_typeESC_ENKUlRNS7_7handlerEE_clESE_ENKUlNS7_7nd_itemILi1EEEE_clESH_(ptr addrspace(4) %__SYCLKernel.ascast)
  ret void
}

define linkonce_odr void @_ZZZ4testISt5arrayI11custom_typeLm128EES0_IS1_Lm1EE14UserDefinedSumIS1_EEvN4sycl3_V15queueET_T0_T1_mNSA_10value_typeESC_ENKUlRNS7_7handlerEE_clESE_ENKUlNS7_7nd_itemILi1EEEE_clESH_(ptr addrspace(4) %this) {
entry:
  call void @dummy_barrier.()
  %ref.tmp4 = alloca %struct.custom_type, align 8
  %ref.tmp4.ascast = addrspacecast ptr %ref.tmp4 to ptr addrspace(4)
  br label %Split.Barrier.BB1

Split.Barrier.BB1:                                ; preds = %entry
  call void @_Z18work_group_barrierj(i32 0)
  call void @_ZN4sycl3_V13ext6oneapi12experimental17reduce_over_groupINS3_21group_with_scratchpadINS0_5groupILi1EEELm18446744073709551615EEE11custom_type14UserDefinedSumIS9_EEENSt9enable_ifIX17is_group_helper_vIT_EET0_E4typeESD_SE_T1_(ptr addrspace(4) %ref.tmp4.ascast)
  ret void
}

define linkonce_odr void @_ZN4sycl3_V13ext6oneapi12experimental17reduce_over_groupINS3_21group_with_scratchpadINS0_5groupILi1EEELm18446744073709551615EEE11custom_type14UserDefinedSumIS9_EEENSt9enable_ifIX17is_group_helper_vIT_EET0_E4typeESD_SE_T1_(ptr addrspace(4) %agg.result) {
entry:
  call void @dummy_barrier.()
  br label %Split.Barrier.BB1

Split.Barrier.BB1:                                ; preds = %entry
  call void @_Z18work_group_barrierj(i32 0)
  call void @_ZN4sycl3_V13ext6oneapi12experimental6detail22reduce_over_group_implINS3_21group_with_scratchpadINS0_5groupILi1EEELm18446744073709551615EEE11custom_type14UserDefinedSumISA_EEET0_T_SD_mT1_(ptr addrspace(4) %agg.result)
  ret void
}

define linkonce_odr void @_ZN4sycl3_V115group_broadcastINS0_5groupILi1EEE11custom_typeEENSt9enable_ifIXaa10is_group_vINSt5decayIT_E4typeEEoosr3std21is_trivially_copyableIT0_EE5valuesr6detail6is_vecISA_EE5valueESA_E4typeES7_SA_NS7_14linear_id_typeE(ptr addrspace(4) noalias sret(%struct.custom_type) align 8 %agg.result) {
entry:
  call void @dummy_barrier.()
  br label %Split.Barrier.BB1

Split.Barrier.BB1:                                ; preds = %entry
  call void @_Z18work_group_barrierj(i32 0)
  ret void
}

; CHECK-LABEL: define linkonce_odr void @_ZN4sycl3_V115group_broadcastINS0_5groupILi1EEE11custom_typeEENSt9enable_ifIXaa10is_group_vINSt5decayIT_E4typeEEoosr3std21is_trivially_copyableIT0_EE5valuesr6detail6is_vecISA_EE5valueESA_E4typeES7_SA_(ptr addrspace(4) noalias sret(%struct.custom_type) align 8 %agg.result)
; CHECK-NOT: call void @_ZN4sycl3_V115group_broadcastINS0_5groupILi1EEE11custom_typeEENSt9enable_ifIXaa10is_group_vINSt5decayIT_E4typeEEoosr3std21is_trivially_copyableIT0_EE5valuesr6detail6is_vecISA_EE5valueESA_E4typeES7_SA_NS7_14linear_id_typeE(ptr addrspace(4) noalias sret(%struct.custom_type) align 8 %agg.result)
; CHECK-LABEL: SyncBB0:
; CHECK: [[LOADVAL:%.*]] = load ptr addrspace(4), ptr {{.*}}, align 8
; CHECK-NEXT: call void @_ZN4sycl3_V115group_broadcastINS0_5groupILi1EEE11custom_typeEENSt9enable_ifIXaa10is_group_vINSt5decayIT_E4typeEEoosr3std21is_trivially_copyableIT0_EE5valuesr6detail6is_vecISA_EE5valueESA_E4typeES7_SA_NS7_14linear_id_typeE(ptr addrspace(4) noalias sret(%struct.custom_type) align 8 [[LOADVAL]])
define linkonce_odr void @_ZN4sycl3_V115group_broadcastINS0_5groupILi1EEE11custom_typeEENSt9enable_ifIXaa10is_group_vINSt5decayIT_E4typeEEoosr3std21is_trivially_copyableIT0_EE5valuesr6detail6is_vecISA_EE5valueESA_E4typeES7_SA_(ptr addrspace(4) noalias sret(%struct.custom_type) align 8 %agg.result) {
entry:
  call void @dummy_barrier.()
  br label %Split.Barrier.BB1

Split.Barrier.BB1:                                ; preds = %entry
  call void @_Z18work_group_barrierj(i32 0)
  call void @_ZN4sycl3_V115group_broadcastINS0_5groupILi1EEE11custom_typeEENSt9enable_ifIXaa10is_group_vINSt5decayIT_E4typeEEoosr3std21is_trivially_copyableIT0_EE5valuesr6detail6is_vecISA_EE5valueESA_E4typeES7_SA_NS7_14linear_id_typeE(ptr addrspace(4) noalias sret(%struct.custom_type) align 8 %agg.result)

  ret void
}

declare void @dummy_barrier.()

declare void @_Z18work_group_barrierj(i32)


!sycl.kernels = !{!0}
!0 = !{ptr @_ZTSZZ4testISt5arrayI11custom_typeLm128EES0_IS1_Lm1EE14UserDefinedSumIS1_EEvN4sycl3_V15queueET_T0_T1_mNSA_10value_typeESC_ENKUlRNS7_7handlerEE_clESE_EUlNS7_7nd_itemILi1EEEE_}
!1 = !{i1 false}

; uselistorder directives
uselistorder ptr @dummy_barrier., { 5, 4, 3, 2, 1, 0 }
uselistorder ptr @_Z18work_group_barrierj, { 4, 3, 2, 1, 0 }

; DEBUGIFY-COUNT-7: WARNING: Instruction with empty DebugLoc in function _ZTSZZ4testISt5arrayI11custom_typeLm128EES0_IS1_Lm1EE14UserDefinedSumIS1_EEvN4sycl3_V15queueET_T0_T1_mNSA_10value_typeESC_ENKUlRNS7_7handlerEE_clESE_EUlNS7_7nd_itemILi1EEEE_
; DEBUGIFY-NOT: WARNING
