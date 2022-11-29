; RUN: opt %s -passes=dpcpp-kernel-sg-size-collector-indirect -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt %s -passes=dpcpp-kernel-sg-size-collector-indirect -S | FileCheck %s

%"class.cl::sycl::intel::SimdFunction" = type { %"struct.std::array" }
%"struct.std::array" = type { [2 x i32 (i32, float)*] }

define dso_local i32 @_Z3barPii(i32* nocapture readnone %a, i32 %n) !recommended_vector_length !0 {
entry:
  %foo_simd = alloca %"class.cl::sycl::intel::SimdFunction", align 8
  %0 = bitcast %"class.cl::sycl::intel::SimdFunction"* %foo_simd to i8*
  %1 = tail call i32 (i32, float)* @__intel_create_simd_variant_p0f_i32i32f32f_p0f_i32i32f32f(i32 (i32, float)* nonnull @_Z3fooif) #5
  %2 = tail call i32 (i32, float)* @__intel_create_simd_variant_p0f_i32i32f32f_p0f_i32i32f32f(i32 (i32, float)* nonnull @_Z3fooif) #6
  %3 = getelementptr inbounds %"class.cl::sycl::intel::SimdFunction", %"class.cl::sycl::intel::SimdFunction"* %foo_simd, i64 0, i32 0, i32 0, i64 0
  store i32 (i32, float)* %1, i32 (i32, float)** %3, align 8
  %4 = getelementptr inbounds %"class.cl::sycl::intel::SimdFunction", %"class.cl::sycl::intel::SimdFunction"* %foo_simd, i64 0, i32 0, i32 0, i64 1
  store i32 (i32, float)* %2, i32 (i32, float)** %4, align 8
  %cmp = icmp sgt i32 %n, 0
  br i1 %cmp, label %DIR.OMP.SIMD.2, label %omp.precond.end

DIR.OMP.SIMD.2:                                   ; preds = %entry
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %DIR.OMP.SIMD.2
  %ptrs.i = getelementptr inbounds %"class.cl::sycl::intel::SimdFunction", %"class.cl::sycl::intel::SimdFunction"* %foo_simd, i64 0, i32 0 ;, !intel-tbaa !2
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %DIR.OMP.SIMD.1
  %.omp.iv.local.019 = phi i32 [ %add9, %omp.inner.for.body ], [ 0, %DIR.OMP.SIMD.1 ]
  %call.i = call i32 (i32, float)** @_ZNKSt5arrayIPFiifELm2EE4dataEv(%"struct.std::array"* nonnull %ptrs.i)
  call i32 (i32 (i32, float)**, ...) @__intel_indirect_call_i32_p0p0f_i32i32f32f(i32 (i32, float)** %call.i, i32 5, float 2.000000e+00) #7
  call i32 (i32 (i32, float)**, ...) @__intel_indirect_call_i32_p0p0f_i32i32f32f(i32 (i32, float)** %call.i, i32 5, float 2.000000e+00)
; CHECK: call i32 (i32 (i32, float)**, ...) @__intel_indirect_call_i32_p0p0f_i32i32f32f(i32 (i32, float)** %call.i, i32 5, float 2.000000e+00) #[[ATTR1:.*]]
; CHECK: call i32 (i32 (i32, float)**, ...) @__intel_indirect_call_i32_p0p0f_i32i32f32f(i32 (i32, float)** %call.i, i32 5, float 2.000000e+00) #[[ATTR2:.*]]
  %add9 = add nuw nsw i32 %.omp.iv.local.019, 1
  %exitcond = icmp eq i32 %add9, %n
  br i1 %exitcond, label %DIR.OMP.END.SIMD.3, label %omp.inner.for.body

DIR.OMP.END.SIMD.3:                               ; preds = %omp.inner.for.body
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %DIR.OMP.END.SIMD.3, %entry
  ret i32 0
}

declare dso_local i32 @_Z3fooif(i32 %i, float %f)

declare dso_local i32 (i32, float)** @_ZNKSt5arrayIPFiifELm2EE4dataEv(%"struct.std::array"*)

declare i32 (i32, float)* @__intel_create_simd_variant_p0f_i32i32f32f_p0f_i32i32f32f(i32 (i32, float)*)

declare i32 @__intel_indirect_call_i32_p0p0f_i32i32f32f(i32 (i32, float)**, ...)

attributes #5 = { nounwind "vector-variant"="_ZGVxN0lu__Z3fooif" }
attributes #6 = { nounwind "vector-variant"="_ZGVxM0vv__Z3fooif" }
attributes #7 = { nounwind "vector-variants"="_ZGVxN0lu_XXX,_ZGVxM0vv_XXX" }

; CHECK: attributes #[[ATTR1]] = { nounwind "vector-variants"="_ZGVxN0lu_XXX,_ZGVxM0vv_XXX" }
; CHECK: attributes #[[ATTR2]] = { "vector-variants"="_ZGVbM8vv___intel_indirect_call_XXX,_ZGVbN8vv___intel_indirect_call_XXX" }

!0 = !{i32 8}

; DEBUGIFY-NOT: WARNING
