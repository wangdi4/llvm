; RUN: opt -opaque-pointers < %s -passes=openmp-opt -S 2>&1 | FileCheck %s

; Check that @omp_get_max_threads() is eliminated, and replaced with a value
; of 5.

; CHECK-LABEL: define i32 @foo5_() local_unnamed_addr {
; CHECK-NEXT: %"main_$VALUE_RET" = alloca i32, align 4
; CHECK-NEXT: call void @omp_set_num_threads(i32 5)
; CHECK-NOT:  call i32 @omp_get_max_threads()
; CHECK-NEXT: store i32 5, ptr %"main_$VALUE_RET", align 4

; Check that @omp_get_max_threads() is eliminated, and replaced with a value
; of 1, not 0.

; CHECK-LABEL: define i32 @foo0_() local_unnamed_addr {
; CHECK-NEXT:  %"main_$VALUE_RET" = alloca i32, align 4
; CHECK-NEXT: call void @omp_set_num_threads(i32 0)
; CHECK-NOT:  call i32 @omp_get_max_threads()
; CHECK-NEXT:  store i32 1, ptr %"main_$VALUE_RET", align 4

@0 = internal unnamed_addr constant i32 0
@"@tid.addr" = external global i32
@"@bid.addr" = external global i32

define i32 @foo5_() local_unnamed_addr {
  %"main_$VALUE_RET" = alloca i32, align 4
  call void @omp_set_num_threads(i32 5)
  %func_result = call i32 @omp_get_max_threads()
  store i32 %func_result, ptr %"main_$VALUE_RET", align 4
  %t0 = load i32, ptr %"main_$VALUE_RET", align 4
  ret i32 %t0
}

declare !llfort.type_idx !1 i32 @omp_get_max_threads() local_unnamed_addr

declare !llfort.type_idx !2 void @omp_set_num_threads(i32) local_unnamed_addr

define i32 @foo0_() local_unnamed_addr {
  %"main_$VALUE_RET" = alloca i32, align 4
  call void @omp_set_num_threads(i32 0)
  %func_result = call i32 @omp_get_max_threads()
  store i32 %func_result, ptr %"main_$VALUE_RET", align 4
  %t0 = load i32, ptr %"main_$VALUE_RET", align 4
  ret i32 %t0
}

declare float @omp_get_max_threads_(...) local_unnamed_addr

declare void @omp_set_num_threads_(...) local_unnamed_addr

!omp_offload.info = !{}
!llvm.module.flags = !{!0}

!0 = !{i32 7, !"openmp", i32 50}
!1 = !{i64 84}
!2 = !{i64 89}
