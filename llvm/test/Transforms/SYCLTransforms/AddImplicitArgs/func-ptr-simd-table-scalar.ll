; RUN: opt -passes=sycl-kernel-add-implicit-args %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-add-implicit-args %s -S | FileCheck %s

; Check that function pointer in global variable is replaced with new function.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

%struct.Base = type { i32 }

$_ZN7Derived1fEi = comdat any

; CHECK: @"_ZN7Derived1fEi$SIMDTable" = weak local_unnamed_addr global [1 x ptr] [ptr @_ZN7Derived1fEi], align 8

@"_ZN7Derived1fEi$SIMDTable" = weak local_unnamed_addr global [1 x ptr] [ptr @_ZN7Derived1fEi], align 8

; CHECK: define linkonce_odr dso_local void @_ZN7Derived1fEi(
; CHECK-SAME: ptr addrspace(3) noalias %pLocalMemBase

define linkonce_odr dso_local void @_ZN7Derived1fEi(ptr addrspace(4) noundef align 4 dereferenceable_or_null(4) %this, i32 noundef %n) #0 comdat align 2 !srcloc !5 !sycl_fixed_targets !6 {
entry:
  %mul = shl nsw i32 %n, 1
  %res = getelementptr inbounds %struct.Base, ptr addrspace(4) %this, i64 0, i32 0, !intel-tbaa !7
  store i32 %mul, ptr addrspace(4) %res, align 4, !tbaa !7
  ret void
}

define dso_local void @_ZTSZZ4mainENKUlRN4sycl3_V17handlerEE_clES2_E52kernel_fp_to_mf_conversion_in_the_opposite_direction() local_unnamed_addr #1 !arg_type_null_val !12 !no_barrier_path !13 !kernel_has_sub_groups !14 {
entry:
  %0 = load ptr addrspace(4), ptr addrspace(4) addrspacecast (ptr @"_ZN7Derived1fEi$SIMDTable" to ptr addrspace(4)), align 8
  br label %exit

exit:                                             ; preds = %entry
  ret void
}

attributes #0 = { mustprogress norecurse nounwind "referenced-indirectly" "vector_function_ptrs"="_ZN7Derived1fEi$SIMDTable()" }
attributes #1 = { convergent norecurse nounwind }

!spirv.Source = !{!0}
!llvm.module.flags = !{!1, !2}
!opencl.ocl.version = !{!3}
!sycl.kernels = !{!4}

!0 = !{i32 4, i32 100000}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"frame-pointer", i32 2}
!3 = !{i32 2, i32 0}
!4 = !{ptr @_ZTSZZ4mainENKUlRN4sycl3_V17handlerEE_clES2_E52kernel_fp_to_mf_conversion_in_the_opposite_direction}
!5 = !{i32 767}
!6 = !{}
!7 = !{!8, !9, i64 0}
!8 = !{!"struct@_ZTS4Base", !9, i64 0}
!9 = !{!"int", !10, i64 0}
!10 = !{!"omnipotent char", !11, i64 0}
!11 = !{!"Simple C++ TBAA"}
!12 = !{ptr addrspace(1) null, ptr null, ptr null, ptr addrspace(1) null, ptr null, ptr addrspace(1) null, ptr null, ptr null, i64 0, ptr addrspace(1) null, ptr null}
!13 = !{i1 true}
!14 = !{i1 false}

; DEBUGIFY-NOT: WARNING
