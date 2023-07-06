; RUN: opt %s -passes=sycl-kernel-sg-size-collector-indirect,sycl-kernel-vec-clone,sycl-kernel-vector-variant-fillin -S | FileCheck %s
; RUN: opt %s -passes=sycl-kernel-sg-size-collector-indirect,sycl-kernel-vec-clone,sycl-kernel-vector-variant-fillin -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s

; Check that SGSizeCollectorIndirectPass skips if there is no vector length
; and VectorVariantFillInPass skips empty vector variant string for
; vector_function_ptrs attributes.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

%"class.sycl::_V1::id" = type { %"class.sycl::_V1::detail::array" }
%"class.sycl::_V1::detail::array" = type { [1 x i64] }

$_ZTSZZ4mainENKUlRN4sycl3_V17handlerEE_clES2_E23kernel_fp_dereferencing = comdat any

; CHECK: @"_Z1fi$SIMDTable" = weak global [1 x ptr] [ptr @_Z1fi], align 8
@"_Z1fi$SIMDTable" = weak global [1 x ptr] [ptr @_Z1fi], align 8

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none)
define dso_local noundef i32 @_Z1fi(i32 noundef %val) #0 {
entry:
  %mul = shl nsw i32 %val, 1
  ret i32 %mul
}

; Function Attrs: norecurse nounwind
define dso_local void @_ZTSZZ4mainENKUlRN4sycl3_V17handlerEE_clES2_E23kernel_fp_dereferencing(ptr addrspace(1) noalias nocapture noundef writeonly align 4 %_arg_acc, ptr noalias nocapture noundef readonly byval(%"class.sycl::_V1::id") align 8 %_arg_acc3) local_unnamed_addr #1 comdat !kernel_arg_buffer_location !1 !arg_type_null_val !2 !no_barrier_path !3 !kernel_has_sub_groups !4 !kernel_has_global_sync !4 !kernel_execution_length !5 !max_wg_dimensions !6 !srcloc !7 !kernel_arg_runtime_aligned !8 !kernel_arg_exclusive_ptr !8 !sycl_kernel_omit_args !9 !recommended_vector_length !6 {
entry:
  %0 = load i64, ptr %_arg_acc3, align 8
  %add.ptr.i = getelementptr inbounds i32, ptr addrspace(1) %_arg_acc, i64 %0, !intel-tbaa !10
  %1 = tail call i64 @_Z13get_global_idj(i32 0) #4
  %2 = tail call i32 @__intel_indirect_call_0(ptr addrspace(4) addrspacecast (ptr @"_Z1fi$SIMDTable" to ptr addrspace(4)), i32 2) #5
  %cmp.not.i = icmp eq i32 %2, 4
  br i1 %cmp.not.i, label %if.end.i, label %if.then.i

if.then.i:                                        ; preds = %entry
  %sext = shl i64 %1, 32
  %conv2.i = ashr exact i64 %sext, 32
  %arrayidx.i30.i = getelementptr inbounds i32, ptr addrspace(1) %add.ptr.i, i64 %conv2.i
  store i32 1, ptr addrspace(1) %arrayidx.i30.i, align 4, !tbaa !10
  br label %if.end.i

if.end.i:                                         ; preds = %if.then.i, %entry
  %3 = tail call i32 @__intel_indirect_call_0(ptr addrspace(4) addrspacecast (ptr @"_Z1fi$SIMDTable" to ptr addrspace(4)), i32 3) #5
  %cmp4.not.i = icmp eq i32 %3, 6
  br i1 %cmp4.not.i, label %if.end10.i, label %if.then5.i

if.then5.i:                                       ; preds = %if.end.i
  %sext1 = shl i64 %1, 32
  %conv8.i = ashr exact i64 %sext1, 32
  %arrayidx.i33.i = getelementptr inbounds i32, ptr addrspace(1) %add.ptr.i, i64 %conv8.i
  store i32 1, ptr addrspace(1) %arrayidx.i33.i, align 4, !tbaa !10
  br label %if.end10.i

if.end10.i:                                       ; preds = %if.then5.i, %if.end.i
  %4 = tail call i32 @__intel_indirect_call_0(ptr addrspace(4) addrspacecast (ptr @"_Z1fi$SIMDTable" to ptr addrspace(4)), i32 4) #5
  %cmp11.not.i = icmp eq i32 %4, 8
  %sext3 = shl i64 %1, 32
  %.pre.i = ashr exact i64 %sext3, 32
  br i1 %cmp11.not.i, label %_ZZZ4mainENKUlRN4sycl3_V17handlerEE_clES2_ENKUlNS0_7nd_itemILi1EEEE_clES5_.exit, label %if.then12.i

if.then12.i:                                      ; preds = %if.end10.i
  %arrayidx.i37.i = getelementptr inbounds i32, ptr addrspace(1) %add.ptr.i, i64 %.pre.i
  store i32 1, ptr addrspace(1) %arrayidx.i37.i, align 4, !tbaa !10
  br label %_ZZZ4mainENKUlRN4sycl3_V17handlerEE_clES2_ENKUlNS0_7nd_itemILi1EEEE_clES5_.exit

_ZZZ4mainENKUlRN4sycl3_V17handlerEE_clES2_ENKUlNS0_7nd_itemILi1EEEE_clES5_.exit: ; preds = %if.then12.i, %if.end10.i
  %arrayidx.i41.i = getelementptr inbounds i32, ptr addrspace(1) %add.ptr.i, i64 %.pre.i
  store i32 2, ptr addrspace(1) %arrayidx.i41.i, align 4, !tbaa !10
  ret void
}

declare dso_local i32 @__intel_indirect_call_0(ptr addrspace(4), i32) local_unnamed_addr #2

; Function Attrs: mustprogress nofree nosync nounwind willreturn memory(none)
declare i64 @_Z13get_global_idj(i32) local_unnamed_addr #3

; CHECK: "vector_function_ptrs"="_Z1fi$SIMDTable()"
attributes #0 = { mustprogress nofree norecurse nosync nounwind willreturn memory(none) "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="all" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "prefer-vector-width"="512" "referenced-indirectly" "stack-protector-buffer-size"="8" "sycl-module-id"="test.cpp" "sycl-optlevel"="2" "unsafe-fp-math"="true" "vector_function_ptrs"="_Z1fi$SIMDTable()" }
attributes #1 = { norecurse nounwind "approx-func-fp-math"="false" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="all" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "sycl-module-id"="test.cpp" "sycl-optlevel"="2" "uniform-work-group-size"="true" "unsafe-fp-math"="false" }
attributes #2 = { "prefer-vector-width"="512" }
attributes #3 = { mustprogress nofree nosync nounwind willreturn memory(none) "prefer-vector-width"="512" }
attributes #4 = { nounwind willreturn memory(none) }
attributes #5 = { nounwind }

!sycl.kernels = !{!0}

!0 = !{ptr @_ZTSZZ4mainENKUlRN4sycl3_V17handlerEE_clES2_E23kernel_fp_dereferencing}
!1 = !{i32 -1, i32 -1, i32 -1, i32 -1}
!2 = !{ptr addrspace(1) null, ptr null}
!3 = !{i1 true}
!4 = !{i1 false}
!5 = !{i32 30}
!6 = !{i32 1}
!7 = !{i32 637}
!8 = !{i1 true, i1 false, i1 false, i1 false}
!9 = !{i1 false, i1 true, i1 true, i1 false}
!10 = !{!11, !11, i64 0}
!11 = !{!"int", !12, i64 0}
!12 = !{!"omnipotent char", !13, i64 0}
!13 = !{!"Simple C++ TBAA"}

; DEBUGIFY-NOT: WARNING
