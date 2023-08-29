; RUN: opt -passes=sycl-kernel-add-implicit-args %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-add-implicit-args %s -S | FileCheck %s

; Check that function pointer in global variable is replaced with new function.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

%struct.Base = type { i32 }

; CHECK: @"_ZN7Derived1fEi$SIMDTable" = weak local_unnamed_addr global [2 x ptr] [ptr @_ZGVeM16vv__ZN7Derived1fEi, ptr @_ZGVeN16vv__ZN7Derived1fEi]

@"_ZN7Derived1fEi$SIMDTable" = weak local_unnamed_addr global [2 x ptr] [ptr @_ZGVeM16vv__ZN7Derived1fEi, ptr @_ZGVeN16vv__ZN7Derived1fEi]

; CHECK: define dso_local void @_ZTSZZ4mainENKUlRN4sycl3_V17handlerEE_clES2_E52kernel_fp_to_mf_conversion_in_the_opposite_direction(
; CHECK-SAME: ptr addrspace(3) noalias %pLocalMemBase

define dso_local void @_ZTSZZ4mainENKUlRN4sycl3_V17handlerEE_clES2_E52kernel_fp_to_mf_conversion_in_the_opposite_direction() local_unnamed_addr #0 !arg_type_null_val !4 !no_barrier_path !5 !kernel_has_sub_groups !6 !vectorized_kernel !7 !scalar_kernel !3 !vectorized_width !8 {
entry:
  %0 = load ptr addrspace(4), ptr addrspace(4) getelementptr (i8, ptr addrspace(4) addrspacecast (ptr @"_ZN7Derived1fEi$SIMDTable" to ptr addrspace(4)), i64 8), align 8
  ret void
}

define linkonce_odr dso_local void @_ZGVeM16vv__ZN7Derived1fEi(<16 x ptr addrspace(4)> noundef align 4 dereferenceable_or_null(4) %this, <16 x i32> noundef %n, <16 x i64> %mask) #1 align 2 {
entry:
  %0 = icmp ne <16 x i64> %mask, zeroinitializer
  %1 = shl nsw <16 x i32> %n, <i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1>
  %mm_vectorGEP = getelementptr inbounds %struct.Base, <16 x ptr addrspace(4)> %this, <16 x i64> zeroinitializer, <16 x i32> zeroinitializer
  tail call void @llvm.masked.scatter.v16i32.v16p4(<16 x i32> %1, <16 x ptr addrspace(4)> %mm_vectorGEP, i32 4, <16 x i1> %0)
  ret void
}

define linkonce_odr dso_local void @_ZGVeN16vv__ZN7Derived1fEi(<16 x ptr addrspace(4)> noundef align 4 dereferenceable_or_null(4) %this, <16 x i32> noundef %n) #1 align 2 {
entry:
  %0 = shl nsw <16 x i32> %n, <i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1>
  %mm_vectorGEP = getelementptr inbounds %struct.Base, <16 x ptr addrspace(4)> %this, <16 x i64> zeroinitializer, <16 x i32> zeroinitializer
  tail call void @llvm.masked.scatter.v16i32.v16p4(<16 x i32> %0, <16 x ptr addrspace(4)> %mm_vectorGEP, i32 4, <16 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>)
  ret void
}

declare void @llvm.masked.scatter.v16i32.v16p4(<16 x i32>, <16 x ptr addrspace(4)>, i32 immarg, <16 x i1>) #2

attributes #0 = { convergent norecurse nounwind memory(readwrite) "approx-func-fp-math"="false" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="all" "may-have-openmp-directive"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "sycl-module-id"="fp-to-mf-conversion-in-the-opposite-direction.cpp" "sycl-optlevel"="2" "uniform-work-group-size"="true" "unsafe-fp-math"="false" }
attributes #1 = { mustprogress norecurse nounwind memory(readwrite) "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="all" "may-have-openmp-directive"="false" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "prefer-vector-width"="512" "referenced-indirectly" "stack-protector-buffer-size"="8" "sycl-module-id"="fp-to-mf-conversion-in-the-opposite-direction.cpp" "sycl-optlevel"="2" "unsafe-fp-math"="true" "vector_function_ptrs"="_ZN7Derived1fEi$SIMDTable(_ZGVeM16vv__ZN7Derived1fEi,_ZGVeN16vv__ZN7Derived1fEi)" "widened-size"="16" }
attributes #2 = { nocallback nofree nosync nounwind willreturn memory(write) }

!spirv.Source = !{!0}
!llvm.module.flags = !{!1, !2}
!sycl.kernels = !{!3}

!0 = !{i32 4, i32 100000}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"frame-pointer", i32 2}
!3 = !{ptr @_ZTSZZ4mainENKUlRN4sycl3_V17handlerEE_clES2_E52kernel_fp_to_mf_conversion_in_the_opposite_direction}
!4 = !{ptr addrspace(1) null, ptr null, ptr null, ptr addrspace(1) null, ptr null, ptr addrspace(1) null, ptr null, ptr null, i64 0, ptr addrspace(1) null, ptr null}
!5 = !{i1 true}
!6 = !{i1 false}
!7 = !{null}
!8 = !{i32 16}

; DEBUGIFY-NOT: WARNING
