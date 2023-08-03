; This test verifies that GlobalOpt shouldn't crash when types of
; store and loads instructions of a global variable are not same.
; Also verifies that GlobalOpt is not triggered to replace use of GV in
; @foo32.

; RUN: opt < %s -S -passes='globalopt' | FileCheck %s

; CHECK: %call1 = tail call i32 @foo64(i64 12345)
; CHECK-NOT: %call2 = tail call i32 @foo32(i64 12345)

@GV = internal dso_local local_unnamed_addr global i64 0, align 8

define dso_local i32 @main() {
entry:
  store i64 12345, ptr @GV, align 8
  %0 = load i64, ptr @GV, align 8
  %call1 = tail call i32 @foo64(i64 %0)
  %1 = load i32, ptr bitcast (ptr @GV to ptr), align 8
  %call2 = tail call i32 @foo32(i32 %1)
  ret i32 0
}

declare dso_local i32 @foo32(i32)
declare dso_local i32 @foo64(i64)
