; RUN: opt -passes=sycl-kernel-wgloop-creator %s -S | FileCheck %s
; RUN: opt -passes=sycl-kernel-wgloop-creator %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s

; Check that wgloop-creator pass don't create redundant/unresolved get_local_id
; call if not-inlined function has gid call and max_wg_dimensions is 1.

; CHECK-NOT: call i64 @_Z12get_local_idj(i32 1)
; CHECK-NOT: call i64 @_Z12get_local_idj(i32 2)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent noinline norecurse nounwind
define internal fastcc void @foo(ptr addrspace(4) noundef %results) unnamed_addr #0 {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 noundef 0) #3
  %arrayidx = getelementptr inbounds i32, ptr addrspace(4) %results, i64 %call
  store i32 1234, ptr addrspace(4) %arrayidx, align 4, !tbaa !2
  ret void
}

; Function Attrs: convergent mustprogress nofree nounwind readnone willreturn
declare i64 @_Z13get_global_idj(i32 noundef) local_unnamed_addr #1

; Function Attrs: convergent norecurse nounwind
define dso_local void @test(ptr addrspace(1) noalias noundef align 4 %results) local_unnamed_addr #2 !no_barrier_path !6 !kernel_has_sub_groups !7 !kernel_has_global_sync !7 !max_wg_dimensions !8 {
entry:
  %0 = addrspacecast ptr addrspace(1) %results to ptr addrspace(4)
  tail call fastcc void @foo(ptr addrspace(4) noundef %0) #4
  ret void
}

; Function Attrs: convergent norecurse nounwind
define dso_local [7 x i64] @WG.boundaries.test(ptr addrspace(1) noalias noundef align 4 %0) local_unnamed_addr #2 {
entry:
  %local.size0 = call i64 @_Z14get_local_sizej(i32 0) #5
  %base.gid0 = call i64 @get_base_global_id.(i32 0) #5
  %local.size1 = call i64 @_Z14get_local_sizej(i32 1) #5
  %base.gid1 = call i64 @get_base_global_id.(i32 1) #5
  %local.size2 = call i64 @_Z14get_local_sizej(i32 2) #5
  %base.gid2 = call i64 @get_base_global_id.(i32 2) #5
  %1 = insertvalue [7 x i64] undef, i64 %local.size0, 2
  %2 = insertvalue [7 x i64] %1, i64 %base.gid0, 1
  %3 = insertvalue [7 x i64] %2, i64 %local.size1, 4
  %4 = insertvalue [7 x i64] %3, i64 %base.gid1, 3
  %5 = insertvalue [7 x i64] %4, i64 %local.size2, 6
  %6 = insertvalue [7 x i64] %5, i64 %base.gid2, 5
  %7 = insertvalue [7 x i64] %6, i64 1, 0
  ret [7 x i64] %7
}

declare i64 @_Z14get_local_sizej(i32)

declare i64 @get_base_global_id.(i32)

attributes #0 = { convergent noinline norecurse nounwind }
attributes #1 = { convergent mustprogress nofree nounwind readnone willreturn }
attributes #2 = { convergent norecurse nounwind }
attributes #3 = { convergent nounwind readnone willreturn }
attributes #4 = { convergent }
attributes #5 = { nounwind }

!opencl.ocl.version = !{!0}
!opencl.spir.version = !{!0}
!sycl.kernels = !{!1}

!0 = !{i32 2, i32 0}
!1 = !{ptr @test}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{i1 true}
!7 = !{i1 false}
!8 = !{i32 1}

; DEBUGIFY-NOT: WARNING
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test --  %uniform.early.exit = extractvalue [7 x i64] %early_exit_call, 0
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test --  {{.*}} = trunc i64 %uniform.early.exit to i1
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test --  br i1 {{.*}}, label %WGLoopsEntry, label %exit
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test --  %init.gid.dim0 = extractvalue [7 x i64] %early_exit_call, 1
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test --  %loop.size.dim0 = extractvalue [7 x i64] %early_exit_call, 2
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test --  %max.gid.dim0 = add i64 %init.gid.dim0, %loop.size.dim0
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test --  %base.gid.dim0 = call i64 @get_base_global_id.(i32 0)
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test --  %dim_0_sub_lid = sub nuw nsw i64 %init.gid.dim0, %base.gid.dim0
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test --  br label %dim_0_pre_head
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test --  br label %scalar_kernel_entry
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test --  %dim_0_inc_ind_var = add nuw nsw i64 %dim_0_ind_var, 1
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test --  %dim_0_cmp.to.max = icmp eq i64 %dim_0_inc_ind_var, %max.gid.dim0
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test --  %dim_0_inc_tid = add nuw nsw i64 %dim_0_tid, 1
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test --  br i1 %dim_0_cmp.to.max, label %dim_0_exit, label %scalar_kernel_entry
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test --  br label %exit
; DEBUGIFY-NOT: WARNING
