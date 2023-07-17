; RUN: opt %s -passes=sycl-kernel-vector-variant-fillin -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt %s -passes="function(instnamer),sycl-kernel-vector-variant-fillin" -S | FileCheck %s

%"class.cl::sycl::intel::SimdFunction" = type { %"struct.std::array" }
%"struct.std::array" = type { [2 x ptr] }

define dso_local i32 @_Z3barPii(ptr nocapture readnone %a, i32 %n) !kernel_arg_base_type !0 !arg_type_null_val !1 {
entry:
  %foo_simd = alloca %"class.cl::sycl::intel::SimdFunction", align 8
  %0 = bitcast ptr %foo_simd to ptr
  %1 = tail call ptr @__intel_create_simd_variant_p0f_i32i32f32f_p0f_i32i32f32f(ptr nonnull @_Z3fooif) #0
  %2 = tail call ptr @__intel_create_simd_variant_p0f_i32i32f32f_p0f_i32i32f32f(ptr nonnull @_Z3fooif) #1
; CHECK: %0 = bitcast ptr @_ZGVbN8lu__Z3fooif to ptr
; CHECK-NEXT: %1 = bitcast ptr @_ZGVbM8vv__Z3fooif to ptr
  %3 = getelementptr inbounds %"class.cl::sycl::intel::SimdFunction", ptr %foo_simd, i64 0, i32 0, i32 0, i64 0
  store ptr %1, ptr %3, align 8
  %4 = getelementptr inbounds %"class.cl::sycl::intel::SimdFunction", ptr %foo_simd, i64 0, i32 0, i32 0, i64 1
  store ptr %2, ptr %4, align 8
  %cmp = icmp sgt i32 %n, 0
  br i1 %cmp, label %DIR.OMP.SIMD.2, label %omp.precond.end

DIR.OMP.SIMD.2:                                   ; preds = %entry
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %DIR.OMP.SIMD.2
  %ptrs.i = getelementptr inbounds %"class.cl::sycl::intel::SimdFunction", ptr %foo_simd, i64 0, i32 0
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %DIR.OMP.SIMD.1
  %.omp.iv.local.019 = phi i32 [ %add9, %omp.inner.for.body ], [ 0, %DIR.OMP.SIMD.1 ]
  %call.i = call ptr @_ZNKSt5arrayIPFiifELm2EE4dataEv(ptr nonnull %ptrs.i)
  %5 = call i32 (ptr, ...) @__intel_indirect_call_i32_p0p0f_i32i32f32f(ptr %call.i, i32 5, float 2.000000e+00) #2
  %add9 = add nuw nsw i32 %.omp.iv.local.019, 1
  %exitcond = icmp eq i32 %add9, %n
  br i1 %exitcond, label %DIR.OMP.END.SIMD.3, label %omp.inner.for.body

DIR.OMP.END.SIMD.3:                               ; preds = %omp.inner.for.body
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %DIR.OMP.END.SIMD.3, %entry
  ret i32 0
}

declare dso_local i32 @_Z3fooif(i32, float)

declare dso_local <2 x i32> @_ZGVbN8lu__Z3fooif(<2 x i32>, <2 x float>)

declare dso_local <2 x i32> @_ZGVbM8vv__Z3fooif(<2 x i32>, <2 x float>)

declare dso_local ptr @_ZNKSt5arrayIPFiifELm2EE4dataEv(ptr)

declare ptr @__intel_create_simd_variant_p0f_i32i32f32f_p0f_i32i32f32f(ptr)

declare i32 @__intel_indirect_call_i32_p0p0f_i32i32f32f(ptr, ...)

attributes #0 = { nounwind "vector-variants"="_ZGVbN8lu__Z3fooif" }
attributes #1 = { nounwind "vector-variants"="_ZGVbM8vv__Z3fooif" }
attributes #2 = { nounwind "vector-variants"="_ZGVbN8lu_XXX,_ZGVbM8vv_XXX" }

; DEBUGIFY-NOT: WARNING

!0 = !{!"int*", !"int"}
!1 = !{ptr null, i32 0}
