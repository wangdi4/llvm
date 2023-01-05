; This test verifies that load instructions of "nabout" are replaced with
; "@__acrt_iob_func(i32 1)" by globalopt when whole-program-safe is true.

; RUN: opt < %s -passes='require<wholeprogram>,globalopt' -whole-program-assume -S | FileCheck %s

; CHECK: define dso_local void @bar()
; CHECK:  %0 = tail call %struct._iobuf* @__acrt_iob_func(i32 1)
; CHECK:  %call1 = call i32 @fflush(%struct._iobuf* %0)

; CHECK: define dso_local i32 @main()
; CHECK:  tail call void @bar()
; CHECK:  %1 = tail call %struct._iobuf* @__acrt_iob_func(i32 1)
; CHECK:  %call2 = tail call i32 @fflush(%struct._iobuf* %1)

target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc19.16.27035"

%struct._iobuf = type { i8* }

@nabout = internal global %struct._iobuf* null

define dso_local void @bar() {
entry:
  %0 = load %struct._iobuf*, %struct._iobuf** @nabout, align 8
  %call1 = call i32 @fflush(%struct._iobuf* %0)
  ret void
}

define dso_local i32 @main() {
entry:
  %0 = tail call %struct._iobuf* @__acrt_iob_func(i32 1)
  store %struct._iobuf* %0, %struct._iobuf** @nabout, align 8
  tail call void @bar()
  %1 = load %struct._iobuf*, %struct._iobuf** @nabout, align 8
  %call2 = tail call i32 @fflush(%struct._iobuf* %1)
  ret i32 0
}

declare dso_local i32 @fflush(%struct._iobuf*)
declare dso_local %struct._iobuf* @__acrt_iob_func(i32)
