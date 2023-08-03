; Check no assertion is raised if UnifyFunctionExitNodes pass changes the
; function with multiple return blocks.

; RUN: opt -passes='function(mergereturn),module(sycl-kernel-wgloop-creator)' %s -S | FileCheck %s
; RUN: opt -passes='function(mergereturn),module(sycl-kernel-wgloop-creator)' %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; CHECK: ret void
; CHECK-NOT: ret void

; Function Attrs: nofree norecurse nosync nounwind readnone
define dso_local void @test(ptr addrspace(3) %arg) local_unnamed_addr #0 !kernel_arg_addr_space !1 !kernel_arg_access_qual !1 !kernel_arg_type !1 !kernel_arg_base_type !1 !kernel_arg_type_qual !1 !kernel_arg_name !1 !kernel_arg_host_accessible !1 !kernel_arg_pipe_depth !1 !kernel_arg_pipe_io !1 !kernel_arg_buffer_location !1 !no_barrier_path !2  {
entry:
 %load = load i32, ptr addrspace(3) %arg, align 8
 %cmp = icmp sge i32 %load, 0
 br i1 %cmp, label %if.then, label %if.end

if.then:
  %cmp.2 = icmp sle i32 %load, 1
  br i1 %cmp.2, label %if.then.2, label %if.end.2

if.then.2:
  unreachable
if.end:
  ret void

if.end.2:
  ret void
}

attributes #0 = { nofree norecurse nosync nounwind readnone }

!sycl.kernels = !{!0}

!0 = !{ptr @test}
!1 = !{}
!2 = !{i1 true}

; DEBUGIFY-NOT: WARNING:
; DEBUGIFY-COUNT-26: WARNING: Instruction with empty DebugLoc in function test
; The instructions emitted by UnifyFunctionExitNodes didn't have debug info now.
; DEBUGIFY-COUNT-2: WARNING: Missing line
; DEBUGIFY-NOT: WARNING:
