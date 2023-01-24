; RUN: opt -passes=sycl-kernel-wg-loop-bound %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes=sycl-kernel-wg-loop-bound %s -S | FileCheck %s

; Check that freeze instruction, whose operand depends on kernel arg, is removed.

; CHECK-NOT: freeze i1

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define void @_ZTS17mandelbrot_kernelIfE(i32 %_arg_crunch) !no_barrier_path !1 {
entry:
  %cmp.not1 = icmp eq i32 %_arg_crunch, 0
  br i1 %cmp.not1, label %while.end.thread, label %while.body

while.body:                                     ; preds = %while.body, %entry
  %m = phi i32 [ %dec, %while.body ], [ %_arg_crunch, %entry ]
  %dec = add i32 %m, -1
  %cmp.not = icmp eq i32 %dec, 0
  br i1 false, label %while.body, label %while.end

while.end:                                      ; preds = %while.body
  %cond.fr = freeze i1 %cmp.not
  br i1 %cond.fr, label %while.end.thread, label %exit

while.end.thread:                               ; preds = %while.end, %entry
  br label %exit

exit:
  ret void
}

!sycl.kernels = !{!0}

!0 = !{void (i32)* @_ZTS17mandelbrot_kernelIfE}
!1 = !{i1 true}

; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _ZTS17mandelbrot_kernelIfE {{.*}} br label %while.body
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function WG.boundaries._ZTS17mandelbrot_kernelIfE {{.*}} call i64 @_Z14get_local_sizej(i32 0)
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function WG.boundaries._ZTS17mandelbrot_kernelIfE {{.*}} call i64 @get_base_global_id.(i32 0)
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function WG.boundaries._ZTS17mandelbrot_kernelIfE {{.*}} call i64 @_Z14get_local_sizej(i32 1)
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function WG.boundaries._ZTS17mandelbrot_kernelIfE {{.*}} call i64 @get_base_global_id.(i32 1)
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function WG.boundaries._ZTS17mandelbrot_kernelIfE {{.*}} call i64 @_Z14get_local_sizej(i32 2)
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function WG.boundaries._ZTS17mandelbrot_kernelIfE {{.*}} call i64 @get_base_global_id.(i32 2)
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function WG.boundaries._ZTS17mandelbrot_kernelIfE {{.*}} xor i1 {{.*}}, true
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function WG.boundaries._ZTS17mandelbrot_kernelIfE {{.*}} and i1 true,
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function WG.boundaries._ZTS17mandelbrot_kernelIfE {{.*}} zext i1 {{.*}} to i64
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function WG.boundaries._ZTS17mandelbrot_kernelIfE {{.*}} insertvalue [7 x i64] undef, i64 %local.size0, 2
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function WG.boundaries._ZTS17mandelbrot_kernelIfE {{.*}} insertvalue [7 x i64] {{.*}}, i64 %base.gid0, 1
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function WG.boundaries._ZTS17mandelbrot_kernelIfE {{.*}} insertvalue [7 x i64] {{.*}}, i64 %local.size1, 4
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function WG.boundaries._ZTS17mandelbrot_kernelIfE {{.*}} insertvalue [7 x i64] {{.*}}, i64 %base.gid1, 3
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function WG.boundaries._ZTS17mandelbrot_kernelIfE {{.*}} insertvalue [7 x i64] {{.*}}, i64 %local.size2, 6
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function WG.boundaries._ZTS17mandelbrot_kernelIfE {{.*}} insertvalue [7 x i64] {{.*}}, i64 %base.gid2, 5
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function WG.boundaries._ZTS17mandelbrot_kernelIfE {{.*}} insertvalue [7 x i64] {{.*}}, i64 %zext_cast, 0
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function WG.boundaries._ZTS17mandelbrot_kernelIfE {{.*}} ret [7 x i64]
; DEBUGIFY: WARNING: Missing line 2
; DEBUGIFY: WARNING: Missing line 7
; DEBUGIFY-NOT: WARNING
