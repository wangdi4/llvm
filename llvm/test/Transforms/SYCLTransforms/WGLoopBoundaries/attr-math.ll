; RUN: opt -passes="sycl-kernel-wg-loop-bound" %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes="sycl-kernel-wg-loop-bound" %s -S | FileCheck %s

; CHECK: @WG.boundaries.test(
; CHECK-SAME: #[[ATTR:[0-9]+]]
; CHECK: attributes #[[ATTR]] = {
; CHECK-SAME: "no-signed-zeros-fp-math"="true"

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent mustprogress nofree norecurse nounwind willreturn writeonly
define dso_local void @test(ptr addrspace(1) nocapture noundef writeonly %dst) local_unnamed_addr #0 !kernel_arg_addr_space !2 !kernel_arg_access_qual !3 !kernel_arg_type !4 !kernel_arg_base_type !4 !kernel_arg_type_qual !5 !kernel_arg_name !6 !kernel_arg_host_accessible !7 !kernel_arg_pipe_depth !8 !kernel_arg_pipe_io !5 !kernel_arg_buffer_location !5 !no_barrier_path !9 !kernel_has_sub_groups !7 {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 noundef 0) #2
  %call1 = tail call i64 @_Z12get_local_idj(i32 noundef 0) #2
  %cmp = icmp ugt i64 %call1, 100
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %conv = trunc i64 %call to i32
  %arrayidx = getelementptr inbounds i32, ptr addrspace(1) %dst, i64 %call
  store i32 %conv, ptr addrspace(1) %arrayidx, align 4, !tbaa !10
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  ret void
}

; Function Attrs: convergent mustprogress nofree nounwind readnone willreturn
declare i64 @_Z13get_global_idj(i32 noundef) local_unnamed_addr #1

; Function Attrs: convergent mustprogress nofree nounwind readnone willreturn
declare i64 @_Z12get_local_idj(i32 noundef) local_unnamed_addr #1

attributes #0 = { convergent mustprogress nofree norecurse nounwind willreturn writeonly "no-signed-zeros-fp-math"="true" }
attributes #1 = { convergent mustprogress nofree nounwind readnone willreturn }
attributes #2 = { convergent nounwind readnone willreturn }

!opencl.ocl.version = !{!0}
!opencl.spir.version = !{!0}
!sycl.kernels = !{!1}

!0 = !{i32 2, i32 0}
!1 = !{ptr @test}
!2 = !{i32 1}
!3 = !{!"none"}
!4 = !{!"int*"}
!5 = !{!""}
!6 = !{!"dst"}
!7 = !{i1 false}
!8 = !{i32 0}
!9 = !{i1 true}
!10 = !{!11, !11, i64 0}
!11 = !{!"int", !12, i64 0}
!12 = !{!"omnipotent char", !13, i64 0}
!13 = !{!"Simple C/C++ TBAA"}

; DEBUGIFY-COUNT-25: WARNING: Instruction with empty DebugLoc in function WG.boundaries.test
; DEBUGIFY: WARNING: Missing line 4
; DEBUGIFY-NOT: WARNING
