; RUN: opt %s -passes=sycl-kernel-sg-size-collector-indirect -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt %s -passes=sycl-kernel-sg-size-collector-indirect -S | FileCheck %s

%"class.cl::sycl::intel::SimdFunction" = type { %"struct.std::array" }
%"struct.std::array" = type { [2 x ptr] }

define dso_local i32 @_Z3barPii(ptr nocapture readnone %a, i32 %n) !recommended_vector_length !0 !kernel_arg_base_type !1 !arg_type_null_val !2 {
entry:
  %foo_simd = alloca %"class.cl::sycl::intel::SimdFunction", align 8
  %0 = tail call ptr @__intel_create_simd_variant_p0f_i32i32f32f_p0f_i32i32f32f(ptr nonnull @_Z3fooif) #5
  %1 = tail call ptr @__intel_create_simd_variant_p0f_i32i32f32f_p0f_i32i32f32f(ptr nonnull @_Z3fooif) #6
  store ptr %0, ptr %foo_simd, align 8
  %2 = getelementptr inbounds %"class.cl::sycl::intel::SimdFunction", ptr %foo_simd, i64 0, i32 0, i32 0, i64 1
  store ptr %1, ptr %2, align 8
  %cmp = icmp sgt i32 %n, 0
  br i1 %cmp, label %DIR.OMP.SIMD.2, label %omp.precond.end

DIR.OMP.SIMD.2:                                   ; preds = %entry
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %DIR.OMP.SIMD.2
  %ptrs.i = getelementptr inbounds %"class.cl::sycl::intel::SimdFunction", ptr %foo_simd, i64 0, i32 0 ;, !intel-tbaa !2
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %DIR.OMP.SIMD.1
  %.omp.iv.local.019 = phi i32 [ %add9, %omp.inner.for.body ], [ 0, %DIR.OMP.SIMD.1 ]
  %call.i = call ptr @_ZNKSt5arrayIPFiifELm2EE4dataEv(ptr nonnull %ptrs.i)
  call i32 (ptr, ...) @__intel_indirect_call_i32_p0p0f_i32i32f32f(ptr %call.i, i32 5, float 2.000000e+00) #7
  call i32 (ptr, ...) @__intel_indirect_call_i32_p0p0f_i32i32f32f(ptr %call.i, i32 5, float 2.000000e+00)
; CHECK: call i32 (ptr, ...) @__intel_indirect_call_i32_p0p0f_i32i32f32f(ptr %call.i, i32 5, float 2.000000e+00) #[[ATTR1:.*]]
; CHECK: call i32 (ptr, ...) @__intel_indirect_call_i32_p0p0f_i32i32f32f(ptr %call.i, i32 5, float 2.000000e+00) #[[ATTR2:.*]]
  %add9 = add nuw nsw i32 %.omp.iv.local.019, 1
  %exitcond = icmp eq i32 %add9, %n
  br i1 %exitcond, label %DIR.OMP.END.SIMD.3, label %omp.inner.for.body

DIR.OMP.END.SIMD.3:                               ; preds = %omp.inner.for.body
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %DIR.OMP.END.SIMD.3, %entry
  ret i32 0
}

declare dso_local i32 @_Z3fooif(i32 %i, float %f)

declare dso_local ptr @_ZNKSt5arrayIPFiifELm2EE4dataEv(ptr)

declare ptr @__intel_create_simd_variant_p0f_i32i32f32f_p0f_i32i32f32f(ptr)

declare i32 @__intel_indirect_call_i32_p0p0f_i32i32f32f(ptr, ...)

attributes #5 = { nounwind "vector-variant"="_ZGV_unknown_N0lu__Z3fooif" }
attributes #6 = { nounwind "vector-variant"="_ZGV_unknown_M0vv__Z3fooif" }
attributes #7 = { nounwind "vector-variants"="_ZGV_unknown_N0lu_XXX,_ZGV_unknown_M0vv_XXX" }

; CHECK: attributes #[[ATTR1]] = { nounwind "vector-variants"="_ZGV_unknown_N0lu_XXX,_ZGV_unknown_M0vv_XXX" }
; CHECK: attributes #[[ATTR2]] = { "vector-variants"="_ZGVbM8vv___intel_indirect_call_XXX,_ZGVbN8vv___intel_indirect_call_XXX" }

!0 = !{i32 8}
!1 = !{!"int*", !"int"}
!2 = !{ptr null, i32 0}

; DEBUGIFY-NOT: WARNING
