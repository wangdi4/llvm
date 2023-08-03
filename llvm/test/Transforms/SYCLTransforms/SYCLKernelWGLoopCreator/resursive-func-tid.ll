; RUN: opt -passes=sycl-kernel-wgloop-creator %s -S | FileCheck %s
; RUN: opt -passes=sycl-kernel-wgloop-creator %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s

; Check that test won't hang when recursive not-inlined function with tid call
; is called in a kernel.

; CHECK: @foo

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent mustprogress nofree nounwind readnone willreturn
declare i64 @_Z13get_global_idj(i32 noundef) local_unnamed_addr #0

; Function Attrs: convergent noinline norecurse nounwind
define internal fastcc void @foo(ptr addrspace(1) noalias noundef %results) unnamed_addr #1 !recursive_call !1 !kernel_arg_base_type !8 !arg_type_null_val !9 {
entry:
  tail call fastcc void @foo(ptr addrspace(1) noundef %results) #3
  %call.i = tail call i64 @_Z13get_global_idj(i32 noundef 0) #4
  %arrayidx.i = getelementptr inbounds i32, ptr addrspace(1) %results, i64 %call.i
  store i32 0, ptr addrspace(1) %arrayidx.i, align 4, !tbaa !2
  ret void
}

; Function Attrs: convergent norecurse nounwind
define dso_local void @test(ptr addrspace(1) noalias noundef align 4 %results) local_unnamed_addr #2 !recursive_call !1 !no_barrier_path !1 !kernel_has_sub_groups !6 !kernel_has_global_sync !6 !max_wg_dimensions !7 !kernel_arg_base_type !8 !arg_type_null_val !9 {
entry:
  tail call fastcc void @foo(ptr addrspace(1) noundef %results) #3
  ret void
}

attributes #0 = { convergent mustprogress nofree nounwind readnone willreturn }
attributes #1 = { convergent noinline norecurse nounwind }
attributes #2 = { convergent norecurse nounwind }
attributes #3 = { convergent }
attributes #4 = { convergent nounwind readnone willreturn }

!sycl.kernels = !{!0}

!0 = !{ptr @test}
!1 = !{i1 true}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{i1 false}
!7 = !{i32 1}
!8 = !{!"int*"}
!9 = !{ptr addrspace(1) null}

; DEBUGIFY-NOT: WARNING
; DEBUGIFY:      WARNING: Instruction with empty DebugLoc in function test --  %base.gid.dim0 = call i64 @get_base_global_id.(i32 0)
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function test --  %local.size.dim0 = call i64 @_Z14get_local_sizej(i32 0)
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function test --  %max.gid.dim0 = add i64 %base.gid.dim0, %local.size.dim0
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function test --  %dim_0_sub_lid = sub nuw nsw i64 %base.gid.dim0, %base.gid.dim0
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function test --  br label %dim_0_pre_head
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function test --  br label %scalar_kernel_entry
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function test --  %dim_0_inc_ind_var = add nuw nsw i64 %dim_0_ind_var, 1
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function test --  %dim_0_cmp.to.max = icmp eq i64 %dim_0_inc_ind_var, %max.gid.dim0
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function test --  %dim_0_inc_tid = add nuw nsw i64 %dim_0_tid, 1
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function test --  br i1 %dim_0_cmp.to.max, label %dim_0_exit, label %scalar_kernel_entry
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function test --  br label %exit
; DEBUGIFY-NOT: WARNING
