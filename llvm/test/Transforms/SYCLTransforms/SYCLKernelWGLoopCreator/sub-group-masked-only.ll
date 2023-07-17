; RUN: opt -passes=sycl-kernel-wgloop-creator %s -S | FileCheck %s
; RUN: opt -passes=sycl-kernel-wgloop-creator %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s

; CHECK-NOT: define void @_ZGVcM4u_test

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent nounwind
define void @test(ptr addrspace(1) noalias %a) local_unnamed_addr #0 !vectorized_masked_kernel !1 !no_barrier_path !2 !kernel_has_sub_groups !2 !vectorized_width !3 !scalar_kernel !4 {
entry:
; CHECK: masked_kernel_entry:
; CHECK-NEXT: %dim_0_vector_ind_var = phi i64 [ %init.gid.dim0, %dim_0_vector_pre_head ], [ %dim_0_vector_inc_ind_var, %pred.call.continue10 ]
; CHECK-NEXT: %ind.var.splatinsert = insertelement <4 x i64> poison, i64 %dim_0_vector_ind_var, i64 0
; CHECK-NEXT: %ind.var.splat = shufflevector <4 x i64> %ind.var.splatinsert, <4 x i64> poison, <4 x i32> zeroinitializer
; CHECK-NEXT: %ind.var.vec = add nuw <4 x i64> %ind.var.splat, <i64 0, i64 1, i64 2, i64 3>
; CHECK-NEXT: %max.gid.splatinsert = insertelement <4 x i64> poison, i64 %max.gid.dim0, i64 0
; CHECK-NEXT: %max.gid.splat = shufflevector <4 x i64> %max.gid.splatinsert, <4 x i64> poison, <4 x i32> zeroinitializer
; CHECK-NEXT: %ind.var.mask.i1 = icmp ult <4 x i64> %ind.var.vec, %max.gid.splat
; CHECK-NEXT: %ind.var.mask = zext <4 x i1> %ind.var.mask.i1 to <4 x i32>

  store i32 1, ptr addrspace(1) %a, align 4, !tbaa !5
  ret void
}

define [7 x i64] @WG.boundaries.test(ptr addrspace(1) %0) !recommended_vector_length !9 {
entry:
  %1 = call i64 @_Z14get_local_sizej(i32 0)
  %2 = call i64 @get_base_global_id.(i32 0)
  %3 = call i64 @_Z14get_local_sizej(i32 1)
  %4 = call i64 @get_base_global_id.(i32 1)
  %5 = call i64 @_Z14get_local_sizej(i32 2)
  %6 = call i64 @get_base_global_id.(i32 2)
  %7 = insertvalue [7 x i64] undef, i64 %1, 2
  %8 = insertvalue [7 x i64] %7, i64 %2, 1
  %9 = insertvalue [7 x i64] %8, i64 %3, 4
  %10 = insertvalue [7 x i64] %9, i64 %4, 3
  %11 = insertvalue [7 x i64] %10, i64 %5, 6
  %12 = insertvalue [7 x i64] %11, i64 %6, 5
  %13 = insertvalue [7 x i64] %12, i64 1, 0
  ret [7 x i64] %13
}

declare i64 @_Z14get_local_sizej(i32)

declare i64 @get_base_global_id.(i32)

; Function Attrs: convergent nounwind
define void @_ZGVcM4u_test(ptr addrspace(1) noalias %a, <4 x i32> %mask) local_unnamed_addr #0 !no_barrier_path !2 !kernel_has_sub_groups !2 !vectorized_width !9 !scalar_kernel !0 !recommended_vector_length !9 !vectorized_kernel !4 !vectorization_dimension !10 !can_unite_workgroups !11 {
entry:
  %0 = icmp ne <4 x i32> %mask, zeroinitializer
  %Predicate = extractelement <4 x i1> %0, i64 0
  br i1 %Predicate, label %pred.call.if, label %pred.call.continue

pred.call.if:                                     ; preds = %entry
  br label %pred.call.continue

pred.call.continue:                               ; preds = %pred.call.if, %entry
  %Predicate2 = extractelement <4 x i1> %0, i64 1
  br i1 %Predicate2, label %pred.call.if5, label %pred.call.continue6

pred.call.if5:                                    ; preds = %pred.call.continue
  br label %pred.call.continue6

pred.call.continue6:                              ; preds = %pred.call.if5, %pred.call.continue
  %1 = phi <4 x i32> [ <i32 4, i32 undef, i32 undef, i32 undef>, %pred.call.continue ], [ <i32 4, i32 4, i32 undef, i32 undef>, %pred.call.if5 ]
  %Predicate3 = extractelement <4 x i1> %0, i64 2
  br i1 %Predicate3, label %pred.call.if7, label %pred.call.continue8

pred.call.if7:                                    ; preds = %pred.call.continue6
  %2 = insertelement <4 x i32> %1, i32 4, i64 2
  br label %pred.call.continue8

pred.call.continue8:                              ; preds = %pred.call.if7, %pred.call.continue6
  %3 = phi <4 x i32> [ %1, %pred.call.continue6 ], [ %2, %pred.call.if7 ]
  %Predicate4 = extractelement <4 x i1> %0, i64 3
  br i1 %Predicate4, label %pred.call.if9, label %pred.call.continue10

pred.call.if9:                                    ; preds = %pred.call.continue8
  %4 = insertelement <4 x i32> %3, i32 4, i64 3
  br label %pred.call.continue10

pred.call.continue10:                             ; preds = %pred.call.if9, %pred.call.continue8
  %5 = phi <4 x i32> [ %3, %pred.call.continue8 ], [ %4, %pred.call.if9 ]
  %6 = getelementptr i32, ptr addrspace(1) %a, <4 x i64> zeroinitializer
  call void @llvm.masked.scatter.v4i32.v4p1(<4 x i32> %5, <4 x ptr addrspace(1)> %6, i32 4, <4 x i1> %0)
  ret void
}

; Function Attrs: nocallback nofree nosync nounwind willreturn writeonly
declare void @llvm.masked.scatter.v4i32.v4p1(<4 x i32>, <4 x ptr addrspace(1)>, i32 immarg, <4 x i1>) #1

attributes #0 = { convergent nounwind "vector-variants"="_ZGVcM4u_test" }
attributes #1 = { nocallback nofree nosync nounwind willreturn writeonly }

!sycl.kernels = !{!0}

!0 = !{ptr @test}
!1 = !{ptr @_ZGVcM4u_test}
!2 = !{i1 true}
!3 = !{i32 1}
!4 = !{null}
!5 = !{!6, !6, i64 0}
!6 = !{!"int", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C/C++ TBAA"}
!9 = !{i32 4}
!10 = !{i32 0}
!11 = !{i1 false}

; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} %uniform.early.exit = extractvalue [7 x i64] %early_exit_call, 0
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} trunc i64 %uniform.early.exit to i1
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} br i1 {{.*}}, label %WGLoopsEntry, label %exit
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} %init.gid.dim0 = extractvalue [7 x i64] %early_exit_call, 1
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} %loop.size.dim0 = extractvalue [7 x i64] %early_exit_call, 2
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} %max.gid.dim0 = add i64 %init.gid.dim0, %loop.size.dim0
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} %init.gid.dim1 = extractvalue [7 x i64] %early_exit_call, 3
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} %loop.size.dim1 = extractvalue [7 x i64] %early_exit_call, 4
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} %max.gid.dim1 = add i64 %init.gid.dim1, %loop.size.dim1
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} %init.gid.dim2 = extractvalue [7 x i64] %early_exit_call, 5
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} %loop.size.dim2 = extractvalue [7 x i64] %early_exit_call, 6
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} %max.gid.dim2 = add i64 %init.gid.dim2, %loop.size.dim2
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} br label %dim_2_vector_pre_head
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} br label %dim_1_vector_pre_head
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} br label %dim_0_vector_pre_head
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} br label %masked_kernel_entry
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} %dim_0_vector_inc_ind_var = add nuw nsw i64 %dim_0_vector_ind_var, 4
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} %dim_0_vector_cmp.to.max = icmp sge i64 %dim_0_vector_inc_ind_var, %max.gid.dim0
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} br i1 %dim_0_vector_cmp.to.max, label %dim_0_vector_exit, label %masked_kernel_entry
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} %dim_1_vector_inc_ind_var = add nuw nsw i64 %dim_1_vector_ind_var, 1
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} %dim_1_vector_cmp.to.max = icmp sge i64 %dim_1_vector_inc_ind_var, %max.gid.dim1
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} br i1 %dim_1_vector_cmp.to.max, label %dim_1_vector_exit, label %dim_0_vector_pre_head
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} %dim_2_vector_inc_ind_var = add nuw nsw i64 %dim_2_vector_ind_var, 1
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} %dim_2_vector_cmp.to.max = icmp sge i64 %dim_2_vector_inc_ind_var, %max.gid.dim2
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} br i1 %dim_2_vector_cmp.to.max, label %dim_2_vector_exit, label %dim_1_vector_pre_head
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} br label %exit
; DEBUGIFY: WARNING: Missing line 1
; DEBUGIFY: WARNING: Missing line 2
; DEBUGIFY: WARNING: Missing variable 1
; DEBUGIFY-NOT: WARNING
