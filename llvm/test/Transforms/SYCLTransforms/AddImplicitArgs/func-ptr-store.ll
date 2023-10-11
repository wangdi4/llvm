; RUN: opt -passes=sycl-kernel-add-implicit-args %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-add-implicit-args %s -S | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

%"class.sycl::_V1::id" = type { %"class.sycl::_V1::detail::array" }
%"class.sycl::_V1::detail::array" = type { [1 x i64] }
%"class.sycl::_V1::INTEL::function_ref_tuned" = type { %"struct.std::array" }
%"struct.std::array" = type { [4 x ptr] }

define double @_Z3addii(i32 %A, i32 %B) #0 {
entry:
  %add = add nsw i32 %B, %A
  %conv = sitofp i32 %add to double
  ret double %conv
}

define double @_Z3subii(i32 %A, i32 %B) #1 {
entry:
  %sub = sub nsw i32 %A, %B
  %conv = sitofp i32 %sub to double
  ret double %conv
}

define void @_ZTSZZ4mainENKUlRN4sycl3_V17handlerEE_clES2_E4Init(ptr addrspace(1) noalias nocapture writeonly align 8 %_arg_AccC, ptr noalias nocapture readonly byval(%"class.sycl::_V1::id") align 8 %_arg_AccC3) local_unnamed_addr #2 !kernel_arg_addr_space !3 !kernel_arg_access_qual !4 !kernel_arg_type !5 !kernel_arg_base_type !5 !kernel_arg_type_qual !6 !arg_type_null_val !7 !no_barrier_path !8 !kernel_has_sub_groups !9 !kernel_has_global_sync !9 !kernel_execution_length !10 !max_wg_dimensions !11 !barrier_buffer_size !11 !private_memory_size !11 !vectorized_width !12 !spirv.ParameterDecorations !13 {
scalar_kernel_entry:
; CHECK-LABEL: define void @_ZTSZZ4mainENKUlRN4sycl3_V17handlerEE_clES2_E4Init(
; CHECK-SAME: ptr addrspace(3) noalias %pLocalMemBase, ptr noalias %pWorkDim, ptr noalias %pWGId, [4 x i64] %BaseGlbId, ptr noalias %pSpecialBuf, ptr noalias %RuntimeHandle)
; CHECK: store ptr @_ZGVeM4vv__Z3addii, ptr addrspace(1) %add.ptr.i, align 8
; CHECK: store ptr @_ZGVeN4uu__Z3addii, ptr addrspace(1)
; CHECK: store ptr @_ZGVeM8vv__Z3addii, ptr addrspace(1)
; CHECK: store ptr @_ZGVeN8uu__Z3addii, ptr addrspace(1)
; CHECK: store ptr @_ZGVeM4vv__Z3subii, ptr addrspace(1)
; CHECK: store ptr @_ZGVeN4uu__Z3subii, ptr addrspace(1)
; CHECK: store ptr @_ZGVeM8vv__Z3subii, ptr addrspace(1)
; CHECK: store ptr @_ZGVeN8uu__Z3subii, ptr addrspace(1)

  %0 = load i64, ptr %_arg_AccC3, align 8
  %add.ptr.i = getelementptr inbounds %"class.sycl::_V1::INTEL::function_ref_tuned", ptr addrspace(1) %_arg_AccC, i64 %0
  store ptr @_ZGVeM4vv__Z3addii, ptr addrspace(1) %add.ptr.i, align 8
  %ref.tmp.sroa.0.i.sroa.4.0.add.ptr.i.sroa_idx = getelementptr inbounds i8, ptr addrspace(1) %add.ptr.i, i64 8
  store ptr @_ZGVeN4uu__Z3addii, ptr addrspace(1) %ref.tmp.sroa.0.i.sroa.4.0.add.ptr.i.sroa_idx, align 8
  %ref.tmp.sroa.0.i.sroa.5.0.add.ptr.i.sroa_idx = getelementptr inbounds i8, ptr addrspace(1) %add.ptr.i, i64 16
  store ptr @_ZGVeM8vv__Z3addii, ptr addrspace(1) %ref.tmp.sroa.0.i.sroa.5.0.add.ptr.i.sroa_idx, align 8
  %ref.tmp.sroa.0.i.sroa.6.0.add.ptr.i.sroa_idx = getelementptr inbounds i8, ptr addrspace(1) %add.ptr.i, i64 24
  store ptr @_ZGVeN8uu__Z3addii, ptr addrspace(1) %ref.tmp.sroa.0.i.sroa.6.0.add.ptr.i.sroa_idx, align 8
  %arrayidx.i27.i = getelementptr inbounds %"class.sycl::_V1::INTEL::function_ref_tuned", ptr addrspace(1) %add.ptr.i, i64 1
  store ptr @_ZGVeM4vv__Z3subii, ptr addrspace(1) %arrayidx.i27.i, align 8
  %ref.tmp2.sroa.0.i.sroa.4.0.arrayidx.i27.i.sroa_idx = getelementptr inbounds i8, ptr addrspace(1) %arrayidx.i27.i, i64 8
  store ptr @_ZGVeN4uu__Z3subii, ptr addrspace(1) %ref.tmp2.sroa.0.i.sroa.4.0.arrayidx.i27.i.sroa_idx, align 8
  %ref.tmp2.sroa.0.i.sroa.5.0.arrayidx.i27.i.sroa_idx = getelementptr inbounds i8, ptr addrspace(1) %arrayidx.i27.i, i64 16
  store ptr @_ZGVeM8vv__Z3subii, ptr addrspace(1) %ref.tmp2.sroa.0.i.sroa.5.0.arrayidx.i27.i.sroa_idx, align 8
  %ref.tmp2.sroa.0.i.sroa.6.0.arrayidx.i27.i.sroa_idx = getelementptr inbounds i8, ptr addrspace(1) %arrayidx.i27.i, i64 24
  store ptr @_ZGVeN8uu__Z3subii, ptr addrspace(1) %ref.tmp2.sroa.0.i.sroa.6.0.arrayidx.i27.i.sroa_idx, align 8
  ret void
}

; CHECK: define <4 x double> @_ZGVeM4vv__Z3addii(
; CHECK-SAME: ptr addrspace(3) noalias %pLocalMemBase, ptr noalias %pWorkDim, ptr noalias %pWGId, [4 x i64] %BaseGlbId, ptr noalias %pSpecialBuf, ptr noalias %RuntimeHandle)

define <4 x double> @_ZGVeM4vv__Z3addii(<4 x i32> %A, <4 x i32> %B, <4 x double> %mask) #3 {
entry:
  %vec.retval = alloca <4 x double>, align 32
  %0 = fcmp une <4 x double> %mask, zeroinitializer
  %1 = add nsw <4 x i32> %B, %A
  %2 = sitofp <4 x i32> %1 to <4 x double>
  call void @llvm.masked.store.v4f64.p0(<4 x double> %2, ptr %vec.retval, i32 32, <4 x i1> %0)
  %vec.ret = load <4 x double>, ptr %vec.retval, align 32
  ret <4 x double> %vec.ret
}

define <4 x double> @_ZGVeN4uu__Z3addii(i32 %A, i32 %B) #3 {
entry:
  %add = add nsw i32 %B, %A
  %conv = sitofp i32 %add to double
  %broadcast.splatinsert = insertelement <4 x double> poison, double %conv, i64 0
  %broadcast.splat = shufflevector <4 x double> %broadcast.splatinsert, <4 x double> poison, <4 x i32> zeroinitializer
  ret <4 x double> %broadcast.splat
}

define <8 x double> @_ZGVeM8vv__Z3addii(<8 x i32> %A, <8 x i32> %B, <8 x double> %mask) #3 {
entry:
  %vec.retval = alloca <8 x double>, align 64
  %0 = fcmp une <8 x double> %mask, zeroinitializer
  %1 = add nsw <8 x i32> %B, %A
  %2 = sitofp <8 x i32> %1 to <8 x double>
  call void @llvm.masked.store.v8f64.p0(<8 x double> %2, ptr %vec.retval, i32 64, <8 x i1> %0)
  %vec.ret = load <8 x double>, ptr %vec.retval, align 64
  ret <8 x double> %vec.ret
}

define <8 x double> @_ZGVeN8uu__Z3addii(i32 %A, i32 %B) #3 {
entry:
  %add = add nsw i32 %B, %A
  %conv = sitofp i32 %add to double
  %broadcast.splatinsert = insertelement <8 x double> poison, double %conv, i64 0
  %broadcast.splat = shufflevector <8 x double> %broadcast.splatinsert, <8 x double> poison, <8 x i32> zeroinitializer
  ret <8 x double> %broadcast.splat
}

define <4 x double> @_ZGVeM4vv__Z3subii(<4 x i32> %A, <4 x i32> %B, <4 x double> %mask) #3 {
entry:
  %vec.retval = alloca <4 x double>, align 32
  %0 = fcmp une <4 x double> %mask, zeroinitializer
  %1 = sub nsw <4 x i32> %A, %B
  %2 = sitofp <4 x i32> %1 to <4 x double>
  call void @llvm.masked.store.v4f64.p0(<4 x double> %2, ptr %vec.retval, i32 32, <4 x i1> %0)
  %vec.ret = load <4 x double>, ptr %vec.retval, align 32
  ret <4 x double> %vec.ret
}

define <4 x double> @_ZGVeN4uu__Z3subii(i32 %A, i32 %B) #3 {
entry:
  %sub = sub nsw i32 %A, %B
  %conv = sitofp i32 %sub to double
  %broadcast.splatinsert = insertelement <4 x double> poison, double %conv, i64 0
  %broadcast.splat = shufflevector <4 x double> %broadcast.splatinsert, <4 x double> poison, <4 x i32> zeroinitializer
  ret <4 x double> %broadcast.splat
}

define <8 x double> @_ZGVeM8vv__Z3subii(<8 x i32> %A, <8 x i32> %B, <8 x double> %mask) #3 {
entry:
  %vec.retval = alloca <8 x double>, align 64
  %0 = fcmp une <8 x double> %mask, zeroinitializer
  %1 = sub nsw <8 x i32> %A, %B
  %2 = sitofp <8 x i32> %1 to <8 x double>
  call void @llvm.masked.store.v8f64.p0(<8 x double> %2, ptr %vec.retval, i32 64, <8 x i1> %0)
  %vec.ret = load <8 x double>, ptr %vec.retval, align 64
  ret <8 x double> %vec.ret
}

define <8 x double> @_ZGVeN8uu__Z3subii(i32 %A, i32 %B) #3 {
entry:
  %sub = sub nsw i32 %A, %B
  %conv = sitofp i32 %sub to double
  %broadcast.splatinsert = insertelement <8 x double> poison, double %conv, i64 0
  %broadcast.splat = shufflevector <8 x double> %broadcast.splatinsert, <8 x double> poison, <8 x i32> zeroinitializer
  ret <8 x double> %broadcast.splat
}

declare void @llvm.masked.store.v4f64.p0(<4 x double>, ptr nocapture, i32 immarg, <4 x i1>) #4

declare void @llvm.masked.store.v8f64.p0(<8 x double>, ptr nocapture, i32 immarg, <8 x i1>) #4

attributes #0 = { mustprogress nofree norecurse nosync nounwind willreturn memory(none) "prefer-vector-width"="512" "referenced-indirectly" "vector-variants"="_ZGVeM4vv__Z3addii,_ZGVeN4uu__Z3addii,_ZGVeM8vv__Z3addii,_ZGVeN8uu__Z3addii" }
attributes #1 = { mustprogress nofree norecurse nosync nounwind willreturn memory(none) "prefer-vector-width"="512" "referenced-indirectly" "vector-variants"="_ZGVeM4vv__Z3subii,_ZGVeN4uu__Z3subii,_ZGVeM8vv__Z3subii,_ZGVeN8uu__Z3subii" }
attributes #2 = { nounwind }
attributes #3 = { mustprogress nofree norecurse nosync nounwind willreturn memory(readwrite) }
attributes #4 = { nocallback nofree nosync nounwind willreturn memory(argmem: write) }

!spirv.Source = !{!0}
!spirv.Generator = !{!1}
!sycl.kernels = !{!2}

!0 = !{i32 4, i32 100000}
!1 = !{i16 6, i16 14}
!2 = !{ptr @_ZTSZZ4mainENKUlRN4sycl3_V17handlerEE_clES2_E4Init}
!3 = !{i32 1, i32 0}
!4 = !{!"none", !"none"}
!5 = !{!"class.sycl::_V1::INTEL::function_ref_tuned*", !"class.sycl::_V1::id"}
!6 = !{!"", !""}
!7 = !{ptr addrspace(1) null, ptr null}
!8 = !{i1 true}
!9 = !{i1 false}
!10 = !{i32 26}
!11 = !{i32 0}
!12 = !{i32 1}
!13 = !{!14, !16}
!14 = !{!15}
!15 = !{i32 44, i32 8}
!16 = !{!17, !15}
!17 = !{i32 38, i32 2}

; DEBUGIFY-NOT: WARNING
