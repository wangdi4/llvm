; RUN: opt %s -sycl-enable-vector-variant-passes -passes=sycl-kernel-create-simd-variant-propagation -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt %s -sycl-enable-vector-variant-passes -passes=sycl-kernel-create-simd-variant-propagation -S | FileCheck %s

%"class._ZTSN2cl4sycl5INTEL18function_ref_tunedIFiiiENS1_8int_listIJLi16ELi16EEEEJFNS1_6maskedENS1_7varyingENS1_6linearEEFNS1_8unmaskedENS1_7uniformESB_EEEE.cl::sycl::INTEL::function_ref_tuned" = type { %"struct._ZTSSt5arrayIPFiiiELm4EE.std::array" }
%"struct._ZTSSt5arrayIPFiiiELm4EE.std::array" = type { [4 x ptr] }
%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range" = type { %"class._ZTSN2cl4sycl6detail5arrayILi1EEE.cl::sycl::detail::array" }
%"class._ZTSN2cl4sycl6detail5arrayILi1EEE.cl::sycl::detail::array" = type { [1 x i64] }

define i32 @_Z3addii(i32 %A, i32 %B) #0 !recommended_vector_length !0 {
entry:
  %add = add nsw i32 %B, %A
  ret i32 %add
}

define i32 @_Z3subii(i32 %A, i32 %B) #1 !recommended_vector_length !0 {
entry:
  %sub = sub nsw i32 %A, %B
  ret i32 %sub
}

; CHECK: define i32 @_Z3addii(i32 %A, i32 %B) #[[ATTRS1:.*]] !recommended_vector_length
; CHECK: define i32 @_Z3subii(i32 %A, i32 %B) #[[ATTRS2:.*]] !recommended_vector_length

define void @"_ZTSZZ4mainENK3$_0clERN2cl4sycl7handlerEE4Init"(ptr addrspace(1) noalias %_arg_, ptr byval(%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range") %_arg_1, ptr byval(%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range") %_arg_2, ptr byval(%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range") %_arg_3) !kernel_arg_base_type !1 !arg_type_null_val !2 {
entry:
  %0 = load i64, ptr %_arg_3, align 8
  %1 = tail call ptr @__intel_create_simd_variant(ptr nonnull @_Z3addii) #3
  %2 = tail call ptr @__intel_create_simd_variant.1(ptr nonnull @_Z3addii) #4
  %3 = tail call ptr @__intel_create_simd_variant.2(ptr nonnull @_Z3addii) #3
  %4 = tail call ptr @__intel_create_simd_variant.3(ptr nonnull @_Z3addii) #4
; CHECK: %1 = tail call ptr @__intel_create_simd_variant(ptr nonnull @_Z3addii) #[[ATTRS3:.*]]
; CHECK-NEXT: %2 = tail call ptr @__intel_create_simd_variant.1(ptr nonnull @_Z3addii) #[[ATTRS4:.*]]
; CHECK-NEXT: %3 = tail call ptr @__intel_create_simd_variant.2(ptr nonnull @_Z3addii) #[[ATTRS3:.*]]
; CHECK-NEXT: %4 = tail call ptr @__intel_create_simd_variant.3(ptr nonnull @_Z3addii) #[[ATTRS4:.*]]
  %add.ptr.i = getelementptr inbounds %"class._ZTSN2cl4sycl5INTEL18function_ref_tunedIFiiiENS1_8int_listIJLi16ELi16EEEEJFNS1_6maskedENS1_7varyingENS1_6linearEEFNS1_8unmaskedENS1_7uniformESB_EEEE.cl::sycl::INTEL::function_ref_tuned", ptr addrspace(1) %_arg_, i64 %0
  store ptr %1, ptr addrspace(1) %add.ptr.i, align 8
  %ref.tmp.sroa.0.i.sroa.4.0..sroa_idx63 = getelementptr inbounds %"class._ZTSN2cl4sycl5INTEL18function_ref_tunedIFiiiENS1_8int_listIJLi16ELi16EEEEJFNS1_6maskedENS1_7varyingENS1_6linearEEFNS1_8unmaskedENS1_7uniformESB_EEEE.cl::sycl::INTEL::function_ref_tuned", ptr addrspace(1) %_arg_, i64 %0, i32 0, i32 0, i64 1
  store ptr %2, ptr addrspace(1) %ref.tmp.sroa.0.i.sroa.4.0..sroa_idx63, align 8
  %ref.tmp.sroa.0.i.sroa.5.0..sroa_idx65 = getelementptr inbounds %"class._ZTSN2cl4sycl5INTEL18function_ref_tunedIFiiiENS1_8int_listIJLi16ELi16EEEEJFNS1_6maskedENS1_7varyingENS1_6linearEEFNS1_8unmaskedENS1_7uniformESB_EEEE.cl::sycl::INTEL::function_ref_tuned", ptr addrspace(1) %_arg_, i64 %0, i32 0, i32 0, i64 2
  store ptr %3, ptr addrspace(1) %ref.tmp.sroa.0.i.sroa.5.0..sroa_idx65, align 8
  %ref.tmp.sroa.0.i.sroa.6.0..sroa_idx67 = getelementptr inbounds %"class._ZTSN2cl4sycl5INTEL18function_ref_tunedIFiiiENS1_8int_listIJLi16ELi16EEEEJFNS1_6maskedENS1_7varyingENS1_6linearEEFNS1_8unmaskedENS1_7uniformESB_EEEE.cl::sycl::INTEL::function_ref_tuned", ptr addrspace(1) %_arg_, i64 %0, i32 0, i32 0, i64 3
  store ptr %4, ptr addrspace(1) %ref.tmp.sroa.0.i.sroa.6.0..sroa_idx67, align 8
  %5 = call ptr @__intel_create_simd_variant.4(ptr nonnull @_Z3subii) #5
  %6 = call ptr @__intel_create_simd_variant.5(ptr nonnull @_Z3subii) #6
  %7 = call ptr @__intel_create_simd_variant.6(ptr nonnull @_Z3subii) #5
  %8 = call ptr @__intel_create_simd_variant.7(ptr nonnull @_Z3subii) #6
; CHECK: %5 = call ptr @__intel_create_simd_variant.4(ptr nonnull @_Z3subii) #[[ATTRS5:.*]]
; CHECK-NEXT: %6 = call ptr @__intel_create_simd_variant.5(ptr nonnull @_Z3subii) #[[ATTRS6:.*]]
; CHECK-NEXT: %7 = call ptr @__intel_create_simd_variant.6(ptr nonnull @_Z3subii) #[[ATTRS5:.*]]
; CHECK-NEXT: %8 = call ptr @__intel_create_simd_variant.7(ptr nonnull @_Z3subii) #[[ATTRS6:.*]]
  %ref.tmp2.sroa.0.i.sroa.0.0..sroa_idx = getelementptr inbounds %"class._ZTSN2cl4sycl5INTEL18function_ref_tunedIFiiiENS1_8int_listIJLi16ELi16EEEEJFNS1_6maskedENS1_7varyingENS1_6linearEEFNS1_8unmaskedENS1_7uniformESB_EEEE.cl::sycl::INTEL::function_ref_tuned", ptr addrspace(1) %add.ptr.i, i64 1, i32 0, i32 0, i64 0
  store ptr %5, ptr addrspace(1) %ref.tmp2.sroa.0.i.sroa.0.0..sroa_idx, align 8
  %ref.tmp2.sroa.0.i.sroa.4.0..sroa_idx70 = getelementptr inbounds %"class._ZTSN2cl4sycl5INTEL18function_ref_tunedIFiiiENS1_8int_listIJLi16ELi16EEEEJFNS1_6maskedENS1_7varyingENS1_6linearEEFNS1_8unmaskedENS1_7uniformESB_EEEE.cl::sycl::INTEL::function_ref_tuned", ptr addrspace(1) %add.ptr.i, i64 1, i32 0, i32 0, i64 1
  store ptr %6, ptr addrspace(1) %ref.tmp2.sroa.0.i.sroa.4.0..sroa_idx70, align 8
  %ref.tmp2.sroa.0.i.sroa.5.0..sroa_idx72 = getelementptr inbounds %"class._ZTSN2cl4sycl5INTEL18function_ref_tunedIFiiiENS1_8int_listIJLi16ELi16EEEEJFNS1_6maskedENS1_7varyingENS1_6linearEEFNS1_8unmaskedENS1_7uniformESB_EEEE.cl::sycl::INTEL::function_ref_tuned", ptr addrspace(1) %add.ptr.i, i64 1, i32 0, i32 0, i64 2
  store ptr %7, ptr addrspace(1) %ref.tmp2.sroa.0.i.sroa.5.0..sroa_idx72, align 8
  %ref.tmp2.sroa.0.i.sroa.6.0..sroa_idx74 = getelementptr inbounds %"class._ZTSN2cl4sycl5INTEL18function_ref_tunedIFiiiENS1_8int_listIJLi16ELi16EEEEJFNS1_6maskedENS1_7varyingENS1_6linearEEFNS1_8unmaskedENS1_7uniformESB_EEEE.cl::sycl::INTEL::function_ref_tuned", ptr addrspace(1) %add.ptr.i, i64 1, i32 0, i32 0, i64 3
  store ptr %8, ptr addrspace(1) %ref.tmp2.sroa.0.i.sroa.6.0..sroa_idx74, align 8
  ret void
}

declare ptr @__intel_create_simd_variant(ptr)
declare ptr @__intel_create_simd_variant.1(ptr)
declare ptr @__intel_create_simd_variant.2(ptr)
declare ptr @__intel_create_simd_variant.3(ptr)
declare ptr @__intel_create_simd_variant.4(ptr)
declare ptr @__intel_create_simd_variant.5(ptr)
declare ptr @__intel_create_simd_variant.6(ptr)
declare ptr @__intel_create_simd_variant.7(ptr)

attributes #0 = { nounwind memory(none) "referenced-indirectly" }
; NOTE: This attribute is artificially added to check we don't miss them during update
attributes #1 = { "vector-variants"="_ZGVbN4vv__Z3subii" }
attributes #3 = { nounwind "vector-variants"="_ZGV_unknown_M16vl__Z3addii" }
attributes #4 = { nounwind "vector-variants"="_ZGV_unknown_N16uu__Z3addii" }
attributes #5 = { nounwind "vector-variants"="_ZGV_unknown_M16vl__Z3subii" }
attributes #6 = { nounwind "vector-variants"="_ZGV_unknown_N16uu__Z3subii" }

; CHECK:      attributes #[[ATTRS1]] = { nounwind memory(none) "referenced-indirectly" "vector-variants"="_ZGV_unknown_M16vl__Z3addii,_ZGV_unknown_N16uu__Z3addii" }
; CHECK-NEXT: attributes #[[ATTRS2]] = { "vector-variants"="_ZGVbN4vv__Z3subii,_ZGV_unknown_M16vl__Z3subii,_ZGV_unknown_N16uu__Z3subii" }
; CHECK-NEXT: attributes #[[ATTRS3]] = { nounwind "vector-variants"="_ZGV_unknown_M16vl__Z3addii" }
; CHECK-NEXT: attributes #[[ATTRS4]] = { nounwind "vector-variants"="_ZGV_unknown_N16uu__Z3addii" }
; CHECK-NEXT: attributes #[[ATTRS5]] = { nounwind "vector-variants"="_ZGV_unknown_M16vl__Z3subii" }
; CHECK-NEXT: attributes #[[ATTRS6]] = { nounwind "vector-variants"="_ZGV_unknown_N16uu__Z3subii" }

!0 = !{i32 16}
!1 = !{!"class._ZTSN2cl4sycl5INTEL18function_ref_tunedIFiiiENS1_8int_listIJLi16ELi16EEEEJFNS1_6maskedENS1_7varyingENS1_6linearEEFNS1_8unmaskedENS1_7uniformESB_EEEE.cl::sycl::INTEL::function_ref_tuned*", !"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range", !"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range", !"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"}
!2 = !{ptr addrspace(1) null, ptr null, ptr null, ptr null}

; DEBUGIFY-NOT: WARNING
