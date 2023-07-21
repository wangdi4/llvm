; RUN: opt -passes=sycl-kernel-wg-loop-bound %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes=sycl-kernel-wg-loop-bound %s -S | FileCheck %s

; check that
; 1) the pass won't crash and generate WG.boundaries function;
; 2) the replaced tid value is where the orignal get_*_id() is,
;    so that it will domainate all its uses.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

declare void @foo(i64)
declare i64 @_Z13get_global_idj(i32) local_unnamed_addr

define void @constant_kernel(ptr addrspace(1) noalias %out) local_unnamed_addr !no_barrier_path !1 {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0)
  %conv = trunc i64 %call to i32
  %cmp = icmp eq i32 %conv, 0
  br i1 %cmp, label %if.then, label %if.end

if.then:
  %sext = shl i64 %call, 32
  call void @foo(i64 %sext)
  br label %if.end

; CHECK:      entry:
; CHECK-NEXT:  [[CONV:%[^ ]*]] = zext i32 0 to i64
; CHECK:       %sext = shl i64 [[CONV]], 32
; CHECK:       call void @foo(i64 %sext)
; CHECK:       br label %if.end

if.end:
  ret void
}

; CHECK: define {{.*}} @WG.boundaries.constant_kernel(ptr addrspace(1) noalias %{{.*}})

!sycl.kernels = !{!0}

!0 = !{ptr @constant_kernel}
!1 = !{i1 true}

; DEBUGIFY-COUNT-27: Instruction with empty DebugLoc in function WG.boundaries.
; DEBUGIFY-COUNT-2: Missing line
; DEBUGIFY-NOT: WARNING
