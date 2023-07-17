; RUN: opt %s -passes=sycl-kernel-vector-variant-fillin -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt %s -passes="function(instnamer),sycl-kernel-vector-variant-fillin" -S | FileCheck %s

; This test is to check the VectorVariantFillIn pass correctly replace operand
; in getelementptr instruction with different type.

%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range" = type { %"class._ZTSN2cl4sycl6detail5arrayILi1EEE.cl::sycl::detail::array" }
%"class._ZTSN2cl4sycl6detail5arrayILi1EEE.cl::sycl::detail::array" = type { [1 x i64] }

@"_Z3addii$SIMDTable" = global [1 x ptr] [ptr @_Z3addii], align 8
@"_Z3subii$SIMDTable" = global [1 x ptr] [ptr @_Z3subii], align 8

; Function Attrs: nounwind readnone
define i32 @_Z3addii(i32 %A, i32 %B) #0 {
entry:
  %add = add nsw i32 %B, %A
  ret i32 %add
}

; Function Attrs: nounwind readnone
define i32 @_Z3subii(i32 %A, i32 %B) #1 {
entry:
  %sub = sub nsw i32 %A, %B
  ret i32 %sub
}

; Function Attrs: nofree nosync nounwind readnone willreturn mustprogress
declare i64 @_Z13get_global_idj(i32) local_unnamed_addr #5

; Function Attrs: nounwind
define void @"_ZTSN2cl4sycl6detail19__pf_kernel_wrapperIZZ4mainENK3$_0clERNS0_7handlerEE1KEE"(ptr byval(%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range") %_arg_, ptr addrspace(1) %_arg_1, ptr byval(%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range") %_arg_2, ptr byval(%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range") %_arg_3, ptr byval(%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range") %_arg_4, ptr addrspace(1) %_arg_5, ptr byval(%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range") %_arg_7, ptr byval(%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range") %_arg_8, ptr byval(%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range") %_arg_9) local_unnamed_addr #2 !kernel_arg_base_type !4 !arg_type_null_val !5 {
entry:
  %0 = tail call i64 @_Z13get_global_idj(i32 0) #7
  %1 = load i64, ptr %_arg_9, align 8
  %add.ptr.i = getelementptr inbounds i64, ptr addrspace(1) %_arg_5, i64 %1
  %2 = load i64, ptr %_arg_4, align 8
  %add.ptr.i34 = getelementptr inbounds i64, ptr addrspace(1) %_arg_1, i64 %2
  %rem.i.i.i = and i64 %0, 1
  %cmp.not.i.i.i = icmp eq i64 %rem.i.i.i, 0
  %ptridx.i19.i.i = getelementptr inbounds i64, ptr addrspace(1) %add.ptr.i34, i64 %0
  %3 = load i64, ptr addrspace(1) %ptridx.i19.i.i, align 8
  %conv.i.i = trunc i64 %3 to i32
  %ptridx.i15.i.i = getelementptr inbounds i64, ptr addrspace(1) %add.ptr.i, i64 %0
  %4 = load i64, ptr addrspace(1) %ptridx.i15.i.i, align 8
  %conv7.i.i = trunc i64 %4 to i32
  ; CHECK: select i1 %cmp.not.i.i.i, ptr addrspace(4) addrspacecast (ptr @"_Z3addii$SIMDTable" to ptr addrspace(4)), ptr addrspace(4) addrspacecast (ptr @"_Z3subii$SIMDTable" to ptr addrspace(4))
  ; CHECK-NOT: select i1 %cmp.not.i.i.i, ptr addrspace(4) addrspacecast (ptr @"_Z3addii$SIMDTable" to ptr addrspace(4)), ptr addrspace(4) addrspacecast (ptr @"_Z3subii$SIMDTable" to ptr addrspace(4))

  %5 = select i1 %cmp.not.i.i.i, ptr addrspace(4) addrspacecast (ptr @"_Z3addii$SIMDTable" to ptr addrspace(4)), ptr addrspace(4) addrspacecast (ptr @"_Z3subii$SIMDTable" to ptr addrspace(4))
  %6 = tail call i32 (ptr addrspace(4), i32, i32, ...) @__intel_indirect_call(ptr addrspace(4) %5, i32 %conv.i.i, i32 %conv7.i.i) #8
  %conv8.i.i = sext i32 %6 to i64
  store i64 %conv8.i.i, ptr addrspace(1) %ptridx.i19.i.i, align 8
  ret void
}

declare i32 @__intel_indirect_call(ptr addrspace(4), i32, i32, ...) local_unnamed_addr #3

; Function Attrs: nounwind
define void @"_ZTSZZ4mainENK3$_0clERN2cl4sycl7handlerEE1K"(ptr addrspace(1) %_arg_, ptr byval(%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range") %_arg_1, ptr byval(%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range") %_arg_2, ptr byval(%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range") %_arg_3, ptr addrspace(1) %_arg_4, ptr byval(%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range") %_arg_6, ptr byval(%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range") %_arg_7, ptr byval(%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range") %_arg_8) local_unnamed_addr !kernel_arg_base_type !6 !arg_type_null_val !7 {
entry:
  %0 = load i64, ptr %_arg_3, align 8
  %add.ptr.i33 = getelementptr inbounds i64, ptr addrspace(1) %_arg_, i64 %0
  %1 = load i64, ptr %_arg_8, align 8
  %add.ptr.i = getelementptr inbounds i64, ptr addrspace(1) %_arg_4, i64 %1
  %2 = tail call i64 @_Z13get_global_idj(i32 0) #7
  %rem.i.i = and i64 %2, 1
  %cmp.not.i.i = icmp eq i64 %rem.i.i, 0
  %ptridx.i19.i = getelementptr inbounds i64, ptr addrspace(1) %add.ptr.i33, i64 %2
  %3 = load i64, ptr addrspace(1) %ptridx.i19.i, align 8
  %conv.i = trunc i64 %3 to i32
  %ptridx.i15.i = getelementptr inbounds i64, ptr addrspace(1) %add.ptr.i, i64 %2
  %4 = load i64, ptr addrspace(1) %ptridx.i15.i, align 8
  %conv7.i = trunc i64 %4 to i32

  ; CHECK: select i1 %cmp.not.i.i, ptr addrspace(4) addrspacecast (ptr @"_Z3addii$SIMDTable" to ptr addrspace(4)), ptr addrspace(4) addrspacecast (ptr @"_Z3subii$SIMDTable" to ptr addrspace(4))
  ; CHECK-NOT: select i1 %cmp.not.i.i, ptr addrspace(4) addrspacecast (ptr @"_Z3addii$SIMDTable" to ptr addrspace(4)), ptr addrspace(4) addrspacecast (ptr @"_Z3subii$SIMDTable" to ptr addrspace(4))
  %5 = select i1 %cmp.not.i.i, ptr addrspace(4) addrspacecast (ptr @"_Z3addii$SIMDTable" to ptr addrspace(4)), ptr addrspace(4) addrspacecast (ptr @"_Z3subii$SIMDTable" to ptr addrspace(4))
  %6 = tail call i32 (ptr addrspace(4), i32, i32, ...) @__intel_indirect_call.1(ptr addrspace(4) %5, i32 %conv.i, i32 %conv7.i) #8
  %conv8.i = sext i32 %6 to i64
  store i64 %conv8.i, ptr addrspace(1) %ptridx.i19.i, align 8
  ret void
}

declare i32 @__intel_indirect_call.1(ptr addrspace(4), i32, i32, ...) local_unnamed_addr #3

declare <16 x i32> @_ZGVeM16vv__Z3addii(<16 x i32> %A, <16 x i32> %B, <16 x i32> %mask)
declare <16 x i32> @_ZGVeN16vv__Z3addii(<16 x i32> %A, <16 x i32> %B)
declare <16 x i32> @_ZGVeM16vv__Z3subii(<16 x i32> %A, <16 x i32> %B, <16 x i32> %mask)
declare <16 x i32> @_ZGVeN16vv__Z3subii(<16 x i32> %A, <16 x i32> %B)

attributes #0 = { nounwind readnone "prefer-vector-width"="512" "referenced-indirectly" "vector-variants"="_ZGVeM16vv__Z3addii,_ZGVeN16vv__Z3addii" "vector_function_ptrs"="_Z3addii$SIMDTable(_ZGVeM16vv__Z3addii,_ZGVeN16vv__Z3addii)" }
attributes #1 = { nounwind readnone "prefer-vector-width"="512" "referenced-indirectly" "vector-variants"="_ZGVeM16vv__Z3subii,_ZGVeN16vv__Z3subii" "vector_function_ptrs"="_Z3subii$SIMDTable(_ZGVeM16vv__Z3subii,_ZGVeN16vv__Z3subii)" }
attributes #2 = { nounwind "prefer-vector-width"="512" }
attributes #3 = { "prefer-vector-width"="512" }
attributes #5 = { nofree nosync nounwind readnone willreturn mustprogress "prefer-vector-width"="512" }
attributes #7 = { nounwind readnone willreturn }

!spirv.Source = !{!1}
!opencl.spir.version = !{!2}
!opencl.ocl.version = !{!3}
!sycl.kernels = !{!0}

!0 = !{ptr @"_ZTSN2cl4sycl6detail19__pf_kernel_wrapperIZZ4mainENK3$_0clERNS0_7handlerEE1KEE", ptr @"_ZTSZZ4mainENK3$_0clERN2cl4sycl7handlerEE1K"}
!1 = !{i32 4, i32 100000}
!2 = !{i32 1, i32 2}
!3 = !{i32 1, i32 0}
!4 = !{!"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range", !"long*", !"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range", !"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range", !"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range", !"long*", !"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range", !"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range", !"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"}
!5 = !{ptr null, ptr addrspace(1) null, ptr null, ptr null, ptr null, ptr addrspace(1) null, ptr null, ptr null, ptr null}
!6 = !{!"long*", !"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range", !"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range", !"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range", !"long*", !"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range", !"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range", !"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"}
!7 = !{ptr addrspace(1) null, ptr null, ptr null, ptr null, ptr addrspace(1) null, ptr null, ptr null, ptr null} 

; DEBUGIFY-NOT: WARNING
