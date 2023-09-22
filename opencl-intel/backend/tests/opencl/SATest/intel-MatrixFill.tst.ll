target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024"
target triple = "spir64-unknown-unknown"

; Function Attrs: nounwind
define spir_kernel void @_ZTSZZ16fill_and_extractvENKUlRN2cl4sycl7handlerEE_clES2_E10MatrixTest(ptr addrspace(1) %add.ptr.i) #0 !kernel_arg_addr_space !3 !kernel_arg_access_qual !6 !kernel_arg_type !7 !kernel_arg_base_type !7 !kernel_arg_type_qual !8 !intel_reqd_sub_group_size !9 {
entry:
  %i3 = call spir_func i64 @_Z33__spirv_BuiltInGlobalInvocationIdi(i32 0) #4
  %call.i19.i = call <256 x i32> @llvm.experimental.matrix.fill.v256i32.i32(i32 42, i32 16, i32 16, metadata !"matrix.rowmajor", metadata !"scope.subgroup", metadata !"matrix.use.unnecessary")
  %cmp.i.i = icmp ult i64 %i3, 2147483648
  %arrayidx.i.i = getelementptr inbounds i32, ptr addrspace(1) %add.ptr.i, i64 %i3
  %arrayidx.ascast.i.i = addrspacecast ptr addrspace(1) %arrayidx.i.i to ptr addrspace(4)
  br label %for.cond.i

for.cond.i:                                       ; preds = %for.body.i, %entry
  %i.0.i = phi i32 [ 0, %entry ], [ %inc.i, %for.body.i ]
  %conv11.i = zext i32 %i.0.i to i64
  %call.i17.i = call i64 @llvm.experimental.matrix.wi.slice.length.v256i32(<256 x i32> %call.i19.i, i32 16, i32 16, metadata !"matrix.rowmajor", metadata !"scope.subgroup", metadata !"matrix.use.unnecessary")
  %cmp.i = icmp ugt i64 %call.i17.i, %conv11.i
  br i1 %cmp.i, label %for.body.i, label %_ZZZ16fill_and_extractvENKUlRN2cl4sycl7handlerEE_clES2_ENKUlNS0_7nd_itemILi1EEEE_clES5_.exit

for.body.i:                                       ; preds = %for.cond.i
  %conv.i = sext i32 %i.0.i to i64
  %call.i.i = call i32 @llvm.experimental.matrix.wi.slice.extractelement.v256i32.i64(<256 x i32> %call.i19.i, i32 16, i32 16, i64 %conv.i, metadata !"matrix.rowmajor", metadata !"scope.subgroup", metadata !"matrix.use.unnecessary")
  call void @llvm.assume(i1 %cmp.i.i)
  store i32 %call.i.i, ptr addrspace(4) %arrayidx.ascast.i.i, align 4
  %inc.i = add nuw nsw i32 %i.0.i, 1
  br label %for.cond.i

_ZZZ16fill_and_extractvENKUlRN2cl4sycl7handlerEE_clES2_ENKUlNS0_7nd_itemILi1EEEE_clES5_.exit: ; preds = %for.cond.i
  ret void
}

; Function Attrs: nofree nosync nounwind readnone willreturn
declare <256 x i32> @llvm.experimental.matrix.fill.v256i32.i32(i32, i32, i32, metadata, metadata, metadata) #1

; Function Attrs: nofree nosync nounwind willreturn
declare i64 @llvm.experimental.matrix.wi.slice.length.v256i32(<256 x i32>, i32, i32, metadata, metadata, metadata) #2

; Function Attrs: nofree nosync nounwind willreturn
declare i32 @llvm.experimental.matrix.wi.slice.extractelement.v256i32.i64(<256 x i32>, i32, i32, i64, metadata, metadata, metadata) #2

; Function Attrs: inaccessiblememonly nofree nosync nounwind willreturn
declare void @llvm.assume(i1 noundef) #3

; Function Attrs: nounwind readnone willreturn
declare spir_func i64 @_Z33__spirv_BuiltInGlobalInvocationIdi(i32) #4

attributes #0 = { nounwind }
attributes #1 = { nofree nosync nounwind readnone willreturn }
attributes #2 = { nofree nosync nounwind willreturn }
attributes #3 = { inaccessiblememonly nofree nosync nounwind willreturn }
attributes #4 = { nounwind readnone willreturn }

!spirv.MemoryModel = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!spirv.Source = !{!1}
!opencl.spir.version = !{!2}
!opencl.ocl.version = !{!3}
!opencl.used.extensions = !{!4}
!opencl.used.optional.core.features = !{!4}
!spirv.Generator = !{!5}

!0 = !{i32 2, i32 2}
!1 = !{i32 4, i32 100000}
!2 = !{i32 1, i32 2}
!3 = !{i32 1, i32 0}
!4 = !{}
!5 = !{i16 6, i16 14}
!6 = !{!"none", !"none"}
!7 = !{!"int*", !"class.cl::sycl::id"}
!8 = !{!"", !""}
!9 = !{i32 8}
