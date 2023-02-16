; RUN: opt --codegenprepare -S %s | FileCheck %s

target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc19.28.29334"

%struct.widget = type { ptr, ptr, [8 x i8] }

@global = external global %struct.widget

; CHECK-LABEL: hoist
; CHECK-NEXT: bb
; CHECK-NEXT: %tmp = alloca i32
; CHECK-NEXT: br i1{{.*}}%bb1
define dso_local void @hoist() local_unnamed_addr #0 personality ptr @__CxxFrameHandler3 {
bb:
  br i1 poison, label %bb1, label %bb7

bb1:                                              ; preds = %bb
  %tmp = alloca i32, align 4
  invoke void @snork()
          to label %bb6 unwind label %bb2

bb2:                                              ; preds = %bb1
  %tmp3 = catchswitch within none [label %bb4] unwind label %bb8

bb4:                                              ; preds = %bb2
  %tmp5 = catchpad within %tmp3 [ptr @global, i32 0, ptr %tmp]
  unreachable

bb6:                                              ; preds = %bb1
  unreachable

bb7:                                              ; preds = %bb
  ret void

bb8:                                              ; preds = %bb2
  %tmp9 = cleanuppad within none []
  unreachable
}

; tmp2/tmp are in entry block, should not be moved.
; check that ordering is not changed.

; CHECK-LABEL: nohoist
; CHECK: bb:
; CHECK-NEXT: %tmp2 = alloca i32
; CHECK-NEXT: %tmp = alloca i32
define dso_local void @nohoist() local_unnamed_addr #0 personality ptr @__CxxFrameHandler3 {
bb:
  %tmp2 = alloca i32
  %tmp = alloca i32, align 4
  br i1 poison, label %bb1, label %bb7

bb1:                                              ; preds = %bb
  invoke void @snork()
          to label %bb6 unwind label %bb2

bb2:                                              ; preds = %bb1
  %tmp3 = catchswitch within none [label %bb4] unwind label %bb8

bb4:                                              ; preds = %bb2
  %tmp5 = catchpad within %tmp3 [ptr @global, i32 0, ptr %tmp]
  unreachable

bb6:                                              ; preds = %bb1
  unreachable

bb7:                                              ; preds = %bb
  ret void

bb8:                                              ; preds = %bb2
  %tmp9 = cleanuppad within none []
  unreachable
}


; Function Attrs: nofree
declare dso_local i32 @__CxxFrameHandler3(...) #0

declare dso_local void @snork() local_unnamed_addr #0

attributes #0 = { nofree }

!nvvm.annotations = !{}

