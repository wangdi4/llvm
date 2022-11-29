; RUN: opt %s -dpcpp-enable-vector-variant-passes -dpcpp-kernel-create-simd-variant-propagation -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt %s -dpcpp-enable-vector-variant-passes -dpcpp-kernel-create-simd-variant-propagation -S | FileCheck %s
; RUN: opt %s -dpcpp-enable-vector-variant-passes -passes=dpcpp-kernel-create-simd-variant-propagation -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt %s -dpcpp-enable-vector-variant-passes -passes=dpcpp-kernel-create-simd-variant-propagation -S | FileCheck %s

%"class._ZTSN2cl4sycl5INTEL18function_ref_tunedIFiiiENS1_8int_listIJLi16ELi16EEEEJFNS1_6maskedENS1_7varyingENS1_6linearEEFNS1_8unmaskedENS1_7uniformESB_EEEE.cl::sycl::INTEL::function_ref_tuned" = type { %"struct._ZTSSt5arrayIPFiiiELm4EE.std::array" }
%"struct._ZTSSt5arrayIPFiiiELm4EE.std::array" = type { [4 x i32 (i32, i32)*] }
%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range" = type { %"class._ZTSN2cl4sycl6detail5arrayILi1EEE.cl::sycl::detail::array" }
%"class._ZTSN2cl4sycl6detail5arrayILi1EEE.cl::sycl::detail::array" = type { [1 x i64] }

define i32 @_Z3addii(i32 %A, i32 %B) #0 !recommended_vector_length !7 {
entry:
  %add = add nsw i32 %B, %A
  ret i32 %add
}

define i32 @_Z3subii(i32 %A, i32 %B) #1 !recommended_vector_length !7 {
entry:
  %sub = sub nsw i32 %A, %B
  ret i32 %sub
}

; CHECK: define i32 @_Z3addii(i32 %A, i32 %B) #[[ATTRS1:.*]] !recommended_vector_length
; CHECK: define i32 @_Z3subii(i32 %A, i32 %B) #[[ATTRS2:.*]] !recommended_vector_length

define void @"_ZTSZZ4mainENK3$_0clERN2cl4sycl7handlerEE4Init"(%"class._ZTSN2cl4sycl5INTEL18function_ref_tunedIFiiiENS1_8int_listIJLi16ELi16EEEEJFNS1_6maskedENS1_7varyingENS1_6linearEEFNS1_8unmaskedENS1_7uniformESB_EEEE.cl::sycl::INTEL::function_ref_tuned" addrspace(1)* noalias %_arg_, %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"* byval(%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range") %_arg_1, %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"* byval(%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range") %_arg_2, %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"* byval(%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range") %_arg_3) {
entry:
  %0 = getelementptr inbounds %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range", %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"* %_arg_3, i64 0, i32 0, i32 0, i64 0
  %1 = load i64, i64* %0, align 8
  %2 = tail call i32 (i32, i32)* @__intel_create_simd_variant(i32 (i32, i32)* nonnull @_Z3addii) #3
  %3 = tail call i32 (i32, i32)* @__intel_create_simd_variant.1(i32 (i32, i32)* nonnull @_Z3addii) #4
  %4 = tail call i32 (i32, i32)* @__intel_create_simd_variant.2(i32 (i32, i32)* nonnull @_Z3addii) #3
  %5 = tail call i32 (i32, i32)* @__intel_create_simd_variant.3(i32 (i32, i32)* nonnull @_Z3addii) #4
; CHECK: %2 = tail call i32 (i32, i32)* @__intel_create_simd_variant(i32 (i32, i32)* nonnull @_Z3addii) #[[ATTRS3:.*]]
; CHECK-NEXT: %3 = tail call i32 (i32, i32)* @__intel_create_simd_variant.1(i32 (i32, i32)* nonnull @_Z3addii) #[[ATTRS4:.*]]
; CHECK-NEXT: %4 = tail call i32 (i32, i32)* @__intel_create_simd_variant.2(i32 (i32, i32)* nonnull @_Z3addii) #[[ATTRS3:.*]]
; CHECK-NEXT: %5 = tail call i32 (i32, i32)* @__intel_create_simd_variant.3(i32 (i32, i32)* nonnull @_Z3addii) #[[ATTRS4:.*]]
  %add.ptr.i = getelementptr inbounds %"class._ZTSN2cl4sycl5INTEL18function_ref_tunedIFiiiENS1_8int_listIJLi16ELi16EEEEJFNS1_6maskedENS1_7varyingENS1_6linearEEFNS1_8unmaskedENS1_7uniformESB_EEEE.cl::sycl::INTEL::function_ref_tuned", %"class._ZTSN2cl4sycl5INTEL18function_ref_tunedIFiiiENS1_8int_listIJLi16ELi16EEEEJFNS1_6maskedENS1_7varyingENS1_6linearEEFNS1_8unmaskedENS1_7uniformESB_EEEE.cl::sycl::INTEL::function_ref_tuned" addrspace(1)* %_arg_, i64 %1
  %ref.tmp.sroa.0.i.sroa.0.0..sroa_idx = getelementptr inbounds %"class._ZTSN2cl4sycl5INTEL18function_ref_tunedIFiiiENS1_8int_listIJLi16ELi16EEEEJFNS1_6maskedENS1_7varyingENS1_6linearEEFNS1_8unmaskedENS1_7uniformESB_EEEE.cl::sycl::INTEL::function_ref_tuned", %"class._ZTSN2cl4sycl5INTEL18function_ref_tunedIFiiiENS1_8int_listIJLi16ELi16EEEEJFNS1_6maskedENS1_7varyingENS1_6linearEEFNS1_8unmaskedENS1_7uniformESB_EEEE.cl::sycl::INTEL::function_ref_tuned" addrspace(1)* %add.ptr.i, i64 0, i32 0, i32 0, i64 0
  store i32 (i32, i32)* %2, i32 (i32, i32)* addrspace(1)* %ref.tmp.sroa.0.i.sroa.0.0..sroa_idx, align 8
  %ref.tmp.sroa.0.i.sroa.4.0..sroa_idx63 = getelementptr inbounds %"class._ZTSN2cl4sycl5INTEL18function_ref_tunedIFiiiENS1_8int_listIJLi16ELi16EEEEJFNS1_6maskedENS1_7varyingENS1_6linearEEFNS1_8unmaskedENS1_7uniformESB_EEEE.cl::sycl::INTEL::function_ref_tuned", %"class._ZTSN2cl4sycl5INTEL18function_ref_tunedIFiiiENS1_8int_listIJLi16ELi16EEEEJFNS1_6maskedENS1_7varyingENS1_6linearEEFNS1_8unmaskedENS1_7uniformESB_EEEE.cl::sycl::INTEL::function_ref_tuned" addrspace(1)* %_arg_, i64 %1, i32 0, i32 0, i64 1
  store i32 (i32, i32)* %3, i32 (i32, i32)* addrspace(1)* %ref.tmp.sroa.0.i.sroa.4.0..sroa_idx63, align 8
  %ref.tmp.sroa.0.i.sroa.5.0..sroa_idx65 = getelementptr inbounds %"class._ZTSN2cl4sycl5INTEL18function_ref_tunedIFiiiENS1_8int_listIJLi16ELi16EEEEJFNS1_6maskedENS1_7varyingENS1_6linearEEFNS1_8unmaskedENS1_7uniformESB_EEEE.cl::sycl::INTEL::function_ref_tuned", %"class._ZTSN2cl4sycl5INTEL18function_ref_tunedIFiiiENS1_8int_listIJLi16ELi16EEEEJFNS1_6maskedENS1_7varyingENS1_6linearEEFNS1_8unmaskedENS1_7uniformESB_EEEE.cl::sycl::INTEL::function_ref_tuned" addrspace(1)* %_arg_, i64 %1, i32 0, i32 0, i64 2
  store i32 (i32, i32)* %4, i32 (i32, i32)* addrspace(1)* %ref.tmp.sroa.0.i.sroa.5.0..sroa_idx65, align 8
  %ref.tmp.sroa.0.i.sroa.6.0..sroa_idx67 = getelementptr inbounds %"class._ZTSN2cl4sycl5INTEL18function_ref_tunedIFiiiENS1_8int_listIJLi16ELi16EEEEJFNS1_6maskedENS1_7varyingENS1_6linearEEFNS1_8unmaskedENS1_7uniformESB_EEEE.cl::sycl::INTEL::function_ref_tuned", %"class._ZTSN2cl4sycl5INTEL18function_ref_tunedIFiiiENS1_8int_listIJLi16ELi16EEEEJFNS1_6maskedENS1_7varyingENS1_6linearEEFNS1_8unmaskedENS1_7uniformESB_EEEE.cl::sycl::INTEL::function_ref_tuned" addrspace(1)* %_arg_, i64 %1, i32 0, i32 0, i64 3
  store i32 (i32, i32)* %5, i32 (i32, i32)* addrspace(1)* %ref.tmp.sroa.0.i.sroa.6.0..sroa_idx67, align 8
  %6 = call i32 (i32, i32)* @__intel_create_simd_variant.4(i32 (i32, i32)* nonnull @_Z3subii) #5
  %7 = call i32 (i32, i32)* @__intel_create_simd_variant.5(i32 (i32, i32)* nonnull @_Z3subii) #6
  %8 = call i32 (i32, i32)* @__intel_create_simd_variant.6(i32 (i32, i32)* nonnull @_Z3subii) #5
  %9 = call i32 (i32, i32)* @__intel_create_simd_variant.7(i32 (i32, i32)* nonnull @_Z3subii) #6
; CHECK: %6 = call i32 (i32, i32)* @__intel_create_simd_variant.4(i32 (i32, i32)* nonnull @_Z3subii) #[[ATTRS5:.*]]
; CHECK-NEXT: %7 = call i32 (i32, i32)* @__intel_create_simd_variant.5(i32 (i32, i32)* nonnull @_Z3subii) #[[ATTRS6:.*]]
; CHECK-NEXT: %8 = call i32 (i32, i32)* @__intel_create_simd_variant.6(i32 (i32, i32)* nonnull @_Z3subii) #[[ATTRS5:.*]]
; CHECK-NEXT: %9 = call i32 (i32, i32)* @__intel_create_simd_variant.7(i32 (i32, i32)* nonnull @_Z3subii) #[[ATTRS6:.*]]
  %ref.tmp2.sroa.0.i.sroa.0.0..sroa_idx = getelementptr inbounds %"class._ZTSN2cl4sycl5INTEL18function_ref_tunedIFiiiENS1_8int_listIJLi16ELi16EEEEJFNS1_6maskedENS1_7varyingENS1_6linearEEFNS1_8unmaskedENS1_7uniformESB_EEEE.cl::sycl::INTEL::function_ref_tuned", %"class._ZTSN2cl4sycl5INTEL18function_ref_tunedIFiiiENS1_8int_listIJLi16ELi16EEEEJFNS1_6maskedENS1_7varyingENS1_6linearEEFNS1_8unmaskedENS1_7uniformESB_EEEE.cl::sycl::INTEL::function_ref_tuned" addrspace(1)* %add.ptr.i, i64 1, i32 0, i32 0, i64 0
  store i32 (i32, i32)* %6, i32 (i32, i32)* addrspace(1)* %ref.tmp2.sroa.0.i.sroa.0.0..sroa_idx, align 8
  %ref.tmp2.sroa.0.i.sroa.4.0..sroa_idx70 = getelementptr inbounds %"class._ZTSN2cl4sycl5INTEL18function_ref_tunedIFiiiENS1_8int_listIJLi16ELi16EEEEJFNS1_6maskedENS1_7varyingENS1_6linearEEFNS1_8unmaskedENS1_7uniformESB_EEEE.cl::sycl::INTEL::function_ref_tuned", %"class._ZTSN2cl4sycl5INTEL18function_ref_tunedIFiiiENS1_8int_listIJLi16ELi16EEEEJFNS1_6maskedENS1_7varyingENS1_6linearEEFNS1_8unmaskedENS1_7uniformESB_EEEE.cl::sycl::INTEL::function_ref_tuned" addrspace(1)* %add.ptr.i, i64 1, i32 0, i32 0, i64 1
  store i32 (i32, i32)* %7, i32 (i32, i32)* addrspace(1)* %ref.tmp2.sroa.0.i.sroa.4.0..sroa_idx70, align 8
  %ref.tmp2.sroa.0.i.sroa.5.0..sroa_idx72 = getelementptr inbounds %"class._ZTSN2cl4sycl5INTEL18function_ref_tunedIFiiiENS1_8int_listIJLi16ELi16EEEEJFNS1_6maskedENS1_7varyingENS1_6linearEEFNS1_8unmaskedENS1_7uniformESB_EEEE.cl::sycl::INTEL::function_ref_tuned", %"class._ZTSN2cl4sycl5INTEL18function_ref_tunedIFiiiENS1_8int_listIJLi16ELi16EEEEJFNS1_6maskedENS1_7varyingENS1_6linearEEFNS1_8unmaskedENS1_7uniformESB_EEEE.cl::sycl::INTEL::function_ref_tuned" addrspace(1)* %add.ptr.i, i64 1, i32 0, i32 0, i64 2
  store i32 (i32, i32)* %8, i32 (i32, i32)* addrspace(1)* %ref.tmp2.sroa.0.i.sroa.5.0..sroa_idx72, align 8
  %ref.tmp2.sroa.0.i.sroa.6.0..sroa_idx74 = getelementptr inbounds %"class._ZTSN2cl4sycl5INTEL18function_ref_tunedIFiiiENS1_8int_listIJLi16ELi16EEEEJFNS1_6maskedENS1_7varyingENS1_6linearEEFNS1_8unmaskedENS1_7uniformESB_EEEE.cl::sycl::INTEL::function_ref_tuned", %"class._ZTSN2cl4sycl5INTEL18function_ref_tunedIFiiiENS1_8int_listIJLi16ELi16EEEEJFNS1_6maskedENS1_7varyingENS1_6linearEEFNS1_8unmaskedENS1_7uniformESB_EEEE.cl::sycl::INTEL::function_ref_tuned" addrspace(1)* %add.ptr.i, i64 1, i32 0, i32 0, i64 3
  store i32 (i32, i32)* %9, i32 (i32, i32)* addrspace(1)* %ref.tmp2.sroa.0.i.sroa.6.0..sroa_idx74, align 8
  ret void
}

declare i32 (i32, i32)* @__intel_create_simd_variant(i32 (i32, i32)*)
declare i32 (i32, i32)* @__intel_create_simd_variant.1(i32 (i32, i32)*)
declare i32 (i32, i32)* @__intel_create_simd_variant.2(i32 (i32, i32)*)
declare i32 (i32, i32)* @__intel_create_simd_variant.3(i32 (i32, i32)*)
declare i32 (i32, i32)* @__intel_create_simd_variant.4(i32 (i32, i32)*)
declare i32 (i32, i32)* @__intel_create_simd_variant.5(i32 (i32, i32)*)
declare i32 (i32, i32)* @__intel_create_simd_variant.6(i32 (i32, i32)*)
declare i32 (i32, i32)* @__intel_create_simd_variant.7(i32 (i32, i32)*)

attributes #0 = { nounwind memory(none) "referenced-indirectly" }
; NOTE: This attribute is artificially added to check we don't miss them during update
attributes #1 = { "vector-variants"="_ZGVbN4vv__Z3subii" }
attributes #3 = { nounwind "vector-variants"="_ZGVxM16vl__Z3addii" }
attributes #4 = { nounwind "vector-variants"="_ZGVxN16uu__Z3addii" }
attributes #5 = { nounwind "vector-variants"="_ZGVxM16vl__Z3subii" }
attributes #6 = { nounwind "vector-variants"="_ZGVxN16uu__Z3subii" }

; CHECK:      attributes #[[ATTRS1]] = { nounwind memory(none) "referenced-indirectly" "vector-variants"="_ZGVxM16vl__Z3addii,_ZGVxN16uu__Z3addii" }
; CHECK-NEXT: attributes #[[ATTRS2]] = { "vector-variants"="_ZGVbN4vv__Z3subii,_ZGVxM16vl__Z3subii,_ZGVxN16uu__Z3subii" }
; CHECK-NEXT: attributes #[[ATTRS3]] = { nounwind "vector-variants"="_ZGVxM16vl__Z3addii" }
; CHECK-NEXT: attributes #[[ATTRS4]] = { nounwind "vector-variants"="_ZGVxN16uu__Z3addii" }
; CHECK-NEXT: attributes #[[ATTRS5]] = { nounwind "vector-variants"="_ZGVxM16vl__Z3subii" }
; CHECK-NEXT: attributes #[[ATTRS6]] = { nounwind "vector-variants"="_ZGVxN16uu__Z3subii" }

!7 = !{i32 16}

; DEBUGIFY-NOT: WARNING
