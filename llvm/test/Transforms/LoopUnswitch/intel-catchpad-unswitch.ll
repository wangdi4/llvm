; RUN: opt -enable-new-pm=0 -loop-unswitch -S %s | FileCheck %s
; (pass not registered with new PM yet)

; CMPLRLLVM-33498:
; Loop exit has a catchswitch, which cannot be split from its catchpads.
; Unswitching should not try to transform this case.
;
; CHECK-LABEL: bb6:
; CHECK-NEXT: catchswitch{{.*}}bb8{{.*}}bb10{{.*}}bb12{{.*}}bb14

target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc19.29.30133"

@global = external global i32
@global.1 = external global i32
@global.2 = external global i32

; Function Attrs: nofree
declare dso_local i32 @__CxxFrameHandler3(...)

declare dso_local void @ham() local_unnamed_addr

define dso_local void @bar(i8** %arg) local_unnamed_addr personality i8* bitcast (i32 (...)* @__CxxFrameHandler3 to i8*) {
bb:
  %tmp = icmp eq i8** %arg, null
  br label %bb1

bb1:                                              ; preds = %bb5, %bb
  br label %bb2

bb2:                                              ; preds = %bb1
  store float undef, float* undef, align 8
  br i1 %tmp, label %bb4, label %bb3

bb3:                                              ; preds = %bb2
  unreachable

bb4:                                              ; preds = %bb2
  invoke void @ham()
          to label %bb5 unwind label %bb6

bb5:                                              ; preds = %bb4
  br label %bb1

bb6:                                              ; preds = %bb4
  %tmp7 = catchswitch within none [label %bb8, label %bb10, label %bb12, label %bb14] unwind to caller

bb8:                                              ; preds = %bb6
  %tmp9 = catchpad within %tmp7 [i32* @global, i32 8, i8* null]
  unreachable

bb10:                                             ; preds = %bb6
  %tmp11 = catchpad within %tmp7 [i32* @global.1, i32 8, i32** undef]
  unreachable

bb12:                                             ; preds = %bb6
  %tmp13 = catchpad within %tmp7 [i32* @global.2, i32 8, i32** undef]
  unreachable

bb14:                                             ; preds = %bb6
  %tmp15 = catchpad within %tmp7 [i8* null, i32 64, i8* null]
  unreachable
}

