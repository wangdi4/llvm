; This test verifies that GlobalOpt shouldn't crash when types of
; store and loads instructions of a global variable are not same.
; Also verifies that GlobalOpt is not triggered.

; RUN: opt < %s -S -globalopt | FileCheck %s
; RUN: opt < %s -S -passes='globalopt' | FileCheck %s

; CHECK-NOT: %call1 = tail call i32 @foo64(i64 12345)

@GV = internal dso_local local_unnamed_addr global i64 0, align 8

define dso_local i32 @main() {
entry:
  store i64 12345, i64* @GV, align 8
  %0 = load i64, i64* @GV, align 8
  %call1 = tail call i32 @foo64(i64 %0)
  %1 = load i32, i32* bitcast (i64* @GV to i32*), align 8
  %call2 = tail call i32 @foo32(i32 %1)
  ret i32 0
}

declare dso_local i32 @foo32(i32)
declare dso_local i32 @foo64(i64)
