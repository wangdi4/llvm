; This test verifies that Andersens Analysis treats "free" call as
; "no side effect" library call.

; RUN: opt < %s -passes='require<anders-aa>' -print-anders-constraints -disable-output 2>&1 | FileCheck %s


; CHECK-NOT: *main:%5 = <universal> (Store)

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@reff = internal unnamed_addr global ptr null, align 8

define dso_local i32 @main() {
entry:
  %0 = tail call noalias ptr @malloc(i64 500)
  store ptr %0, ptr bitcast (ptr @reff to ptr)
  %1 = bitcast ptr %0 to ptr
  br label %L2
L1:
  %2 = load ptr, ptr @reff
  br label %L2
L2:
  %3 = phi ptr [ %1, %entry ], [ %2, %L1 ]
  %4 = tail call noalias ptr @malloc(i64 500)
  store ptr null, ptr @reff
  %5 = bitcast ptr %1 to ptr
  tail call void @free(ptr %5)
  ret i32 0
}

declare dso_local noalias ptr @malloc(i64)
declare dso_local void @free(ptr nocapture)
