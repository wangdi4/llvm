; RUN: opt -passes="dpcpp-kernel-analysis,dpcpp-kernel-wg-loop-bound" %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes="dpcpp-kernel-analysis,dpcpp-kernel-wg-loop-bound" %s -S -debug -disable-output 2>&1 | FileCheck %s
; RUN: opt -dpcpp-kernel-analysis -dpcpp-kernel-wg-loop-bound %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -dpcpp-kernel-analysis -dpcpp-kernel-wg-loop-bound %s -S -debug -disable-output 2>&1 | FileCheck %s

; The source code for the test is:
; #define lid(N) ((int) get_local_id(N))
; __kernel void test(int const n1) {
;   if (-1 * lid(0) + n1 >= 0) printf("local id: %d\n", lid(0));
; }

; CHECK: WGLoopBoundaries
; CHECK: found 2 early exit boundaries
; CHECK: Dim=0, Contains=T, IsGID=F, IsSigned=T, IsUpperBound=T
; CHECK-SAME: %to_tid_type = sext i32 %final_right_bound to i64
; CHECK-NEXT: Dim=0, Contains=T, IsGID=F, IsSigned=T, IsUpperBound=F
; CHECK-SAME: %to_tid_type1 = sext i32 %final_left_bound to i64
; CHECK: found 0 uniform early exit conditions
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

@.str = private unnamed_addr addrspace(2) constant [14 x i8] c"local id: %d\0A\00", align 1

; Function Attrs: convergent norecurse nounwind
define dso_local void @test(i32 noundef %n1) local_unnamed_addr #0 !kernel_arg_addr_space !4 !kernel_arg_access_qual !5 !kernel_arg_type !6 !kernel_arg_base_type !6 !kernel_arg_type_qual !7 !kernel_arg_name !8 !kernel_arg_host_accessible !9 !kernel_arg_pipe_depth !4 !kernel_arg_pipe_io !7 !kernel_arg_buffer_location !7 !no_barrier_path !10 !kernel_has_sub_groups !9 {
entry:
  %call = tail call i64 @_Z12get_local_idj(i32 noundef 0) #3
  %conv = trunc i64 %call to i32
  %add = sub i32 %n1, %conv
  %cmp = icmp sgt i32 %add, -1
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %call4 = tail call i32 (i8 addrspace(2)*, ...) @printf(i8 addrspace(2)* noundef getelementptr inbounds ([14 x i8], [14 x i8] addrspace(2)* @.str, i64 0, i64 0), i32 noundef %conv) #4
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  ret void
}

; Function Attrs: convergent mustprogress nofree nounwind readnone willreturn
declare i64 @_Z12get_local_idj(i32 noundef) local_unnamed_addr #1

; Function Attrs: convergent nofree nounwind
declare noundef i32 @printf(i8 addrspace(2)* nocapture noundef readonly, ...) local_unnamed_addr #2

attributes #0 = { convergent norecurse nounwind "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="true" }
attributes #1 = { convergent mustprogress nofree nounwind readnone willreturn "frame-pointer"="none" "no-trapping-math"="true" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "stackrealign" }
attributes #2 = { convergent nofree nounwind "frame-pointer"="none" "no-trapping-math"="true" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "stackrealign" }
attributes #3 = { convergent nounwind readnone willreturn }
attributes #4 = { convergent nobuiltin nounwind }

!llvm.linker.options = !{}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!0}
!opencl.spir.version = !{!0}
!opencl.used.extensions = !{!1}
!opencl.used.optional.core.features = !{!1}
!opencl.compiler.options = !{!1}
!llvm.ident = !{!2}
!sycl.kernels = !{!3}

!0 = !{i32 1, i32 2}
!1 = !{}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!3 = !{void (i32)* @test}
!4 = !{i32 0}
!5 = !{!"none"}
!6 = !{!"int"}
!7 = !{!""}
!8 = !{!"n1"}
!9 = !{i1 false}
!10 = !{i1 true}

; DEBUGIFY-COUNT-14: Instruction with empty DebugLoc in function test
; DEBUGIFY-COUNT-46: Instruction with empty DebugLoc in function WG.boundaries.test
; DEBUGIFY-COUNT-1: Missing line
; DEBUGIFY-NOT: WARNING
