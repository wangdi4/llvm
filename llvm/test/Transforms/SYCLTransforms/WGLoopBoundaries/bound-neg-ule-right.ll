; RUN: opt -passes=sycl-kernel-wg-loop-bound %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes=sycl-kernel-wg-loop-bound %s -S -debug -disable-output 2>&1 | FileCheck %s

; The source code for the test is:
; kernel void test(global ulong *dst, uint b) {
;   size_t gid = get_global_id(0);
;   if (2 <= b - gid)
;     dst[gid] = gid;
; }
; icmp inst is manually modified to ule.

; CHECK:  found 0 early exit boundaries
; CHECK:  found 0 uniform early exit conditions

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define dso_local void @test(ptr addrspace(1) noalias noundef align 8 %dst, i32 noundef %b) !no_barrier_path !1 {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 noundef 0)
  %conv = zext i32 %b to i64
  %sub = sub i64 %conv, %call
  %cmp = icmp ule i64 2, %sub
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %arrayidx = getelementptr inbounds i64, ptr addrspace(1) %dst, i64 %call
  store i64 %call, ptr addrspace(1) %arrayidx, align 8, !tbaa !2
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  ret void
}

declare i64 @_Z13get_global_idj(i32 noundef)

!sycl.kernels = !{!0}

!0 = !{ptr @test}
!1 = !{i1 true}
!2 = !{!3, !3, i64 0}
!3 = !{!"long", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}

; DEBUGIFY-COUNT-6: WARNING: Instruction with empty DebugLoc in function test
; DEBUGIFY-NOT: WARNING
