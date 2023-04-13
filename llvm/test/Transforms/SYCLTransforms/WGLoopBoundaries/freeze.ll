; RUN: opt -passes=sycl-kernel-wg-loop-bound %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes=sycl-kernel-wg-loop-bound %s -S | FileCheck %s

; Check that freeze instruction is removed and early exit boundary is computed.

; CHECK-NOT: freeze i64

; CHECK-LABEL: @WG.boundaries.test
; CHECK-NEXT: entry:
; CHECK-NEXT:  %local.size0 = call i64 @_Z14get_local_sizej(i32 0)
; CHECK-NEXT:  %base.gid0 = call i64 @get_base_global_id.(i32 0)
; CHECK-NEXT:  %local.size1 = call i64 @_Z14get_local_sizej(i32 1)
; CHECK-NEXT:  %base.gid1 = call i64 @get_base_global_id.(i32 1)
; CHECK-NEXT:  %local.size2 = call i64 @_Z14get_local_sizej(i32 2)
; CHECK-NEXT:  %base.gid2 = call i64 @get_base_global_id.(i32 2)
; CHECK-NEXT:  %to_tid_type = zext i32 17 to i64
; CHECK-NEXT:  %upper.bound0.exclusive = add i64 %to_tid_type, 1
; CHECK-NEXT:  [[CMP1:%[0-9]+]] = icmp ugt i64 %to_tid_type, %upper.bound0.exclusive
; CHECK-NEXT:  %upper.bound0.correct = select i1 [[CMP1]], i64 %to_tid_type, i64 %upper.bound0.exclusive
; CHECK-NEXT:  %init.upper.bound0 = add i64 %local.size0, %base.gid0
; CHECK-NEXT:  [[CMP2:%[0-9]+]] = icmp ult i64 %init.upper.bound0, %upper.bound0.correct
; CHECK-NEXT:  %upper.bound0 = select i1 [[CMP2]], i64 %init.upper.bound0, i64 %upper.bound0.correct
; CHECK-NEXT:  %loop.size0 = sub i64 %upper.bound0, %base.gid0
; CHECK-NEXT:  [[CMP3:%[0-9]+]] = icmp slt i64 0, %loop.size0
; CHECK-NEXT:  [[AND:%[0-9]+]] = and i1 true, [[CMP3]]
; CHECK-NEXT:  %zext_cast = zext i1 [[AND]] to i64
; CHECK-NEXT:  [[INSERT1:%[0-9]+]] = insertvalue [7 x i64] undef, i64 %loop.size0, 2
; CHECK-NEXT:  [[INSERT2:%[0-9]+]] = insertvalue [7 x i64] [[INSERT1]], i64 %base.gid0, 1
; CHECK-NEXT:  [[INSERT3:%[0-9]+]] = insertvalue [7 x i64] [[INSERT2]], i64 %local.size1, 4
; CHECK-NEXT:  [[INSERT4:%[0-9]+]] = insertvalue [7 x i64] [[INSERT3]], i64 %base.gid1, 3
; CHECK-NEXT:  [[INSERT5:%[0-9]+]] = insertvalue [7 x i64] [[INSERT4]], i64 %local.size2, 6
; CHECK-NEXT:  [[INSERT6:%[0-9]+]] = insertvalue [7 x i64] [[INSERT5]], i64 %base.gid2, 5
; CHECK-NEXT:  [[INSERT7:%[0-9]+]] = insertvalue [7 x i64] [[INSERT6]], i64 %zext_cast, 0
; CHECK-NEXT:  ret [7 x i64] [[INSERT7]]

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define void @test() !no_barrier_path !1 {
entry:
  %0 = tail call i64 @_Z13get_global_idj(i32 0)
  %.fr = freeze i64 %0
  %conv.i.i = trunc i64 %.fr to i32
  %cmp.i.i = icmp ugt i32 %conv.i.i, 17
  br i1 %cmp.i.i, label %exit, label %if.end.i.i

if.end.i.i:                                       ; preds = %entry
  br label %exit

exit: ; preds = %if.end.i.i, %entry
  ret void
}

declare i64 @_Z13get_global_idj(i32)

!sycl.kernels = !{!0}

!0 = !{void ()* @test}
!1 = !{i1 true}

; DEBUGIFY:      WARNING: Instruction with empty DebugLoc in function test --  %to_tid_type = zext i32 17 to i64
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function WG.boundaries.test {{.*}} = call i64 @_Z14get_local_sizej(i32 0)
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function WG.boundaries.test {{.*}} = call i64 @get_base_global_id.(i32 0)
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function WG.boundaries.test {{.*}} = call i64 @_Z14get_local_sizej(i32 1)
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function WG.boundaries.test {{.*}} = call i64 @get_base_global_id.(i32 1)
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function WG.boundaries.test {{.*}} = call i64 @_Z14get_local_sizej(i32 2)
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function WG.boundaries.test {{.*}} = call i64 @get_base_global_id.(i32 2)
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function WG.boundaries.test {{.*}} = zext i32 17 to i64
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function WG.boundaries.test {{.*}} = add i64
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function WG.boundaries.test {{.*}} = icmp ugt i64
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function WG.boundaries.test {{.*}} = select i1
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function WG.boundaries.test {{.*}} = add i64
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function WG.boundaries.test {{.*}} = icmp ult i64
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function WG.boundaries.test {{.*}} = select i1
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function WG.boundaries.test {{.*}} = sub i64
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function WG.boundaries.test {{.*}} = icmp slt
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function WG.boundaries.test {{.*}} = and i1
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function WG.boundaries.test {{.*}} = zext i1
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function WG.boundaries.test {{.*}} = insertvalue [7 x i64]
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function WG.boundaries.test {{.*}} = insertvalue [7 x i64]
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function WG.boundaries.test {{.*}} = insertvalue [7 x i64]
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function WG.boundaries.test {{.*}} = insertvalue [7 x i64]
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function WG.boundaries.test {{.*}} = insertvalue [7 x i64]
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function WG.boundaries.test {{.*}} = insertvalue [7 x i64]
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function WG.boundaries.test {{.*}} = insertvalue [7 x i64]
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function WG.boundaries.test --  ret [7 x i64]
; DEBUGIFY-NEXT: WARNING: Missing line 2
; DEBUGIFY-NEXT: WARNING: Missing line 5
; DEBUGIFY-NOT: WARNING
