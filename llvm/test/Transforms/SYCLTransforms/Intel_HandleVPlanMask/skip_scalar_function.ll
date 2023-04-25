; Checks that HandleVPlanMask pass doesn't crash in the corner case when
; there're still scalar builtin function calls in a vectorized kernel (this
; could happend under O0 vectorization, where scalar remainder might not be
; eliminated).

; RUN: opt -passes=sycl-kernel-convert-vplan-mask %s -S -disable-output

; Function Attrs: convergent nounwind
declare i64 @_Z32sub_group_non_uniform_reduce_minl(i64 noundef) #0

; Function Attrs: convergent noinline norecurse nounwind optnone memory(readwrite)
define dso_local void @_ZGVeN8uu_test_min_8() #1 !no_barrier_path !1 !kernel_has_sub_groups !1 !intel_reqd_sub_group_size !2 !vectorized_width !2 !recommended_vector_length !2 !vectorization_dimension !3 !scalar_kernel !4 {
entry:
  %call2 = call i64 @_Z32sub_group_non_uniform_reduce_minl(i64 0) #2
  ret void
}

attributes #0 = { convergent nounwind "no-trapping-math"="true" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "stackrealign" }
attributes #1 = { convergent noinline norecurse nounwind optnone memory(readwrite) "has-sub-groups" "may-have-openmp-directive"="true" "no-trapping-math"="true" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="false" }
attributes #2 = { convergent nounwind "has-vplan-mask" "kernel-call-once" "vector-variants"="_ZGVbM8v__Z32sub_group_non_uniform_reduce_minl(_Z32sub_group_non_uniform_reduce_minDv8_lDv8_j)" }

!opencl.ocl.version = !{!0}
!opencl.spir.version = !{!0}

!0 = !{i32 2, i32 0}
!1 = !{i1 true}
!2 = !{i32 8}
!3 = !{i32 0}
!4 = distinct !{null}
