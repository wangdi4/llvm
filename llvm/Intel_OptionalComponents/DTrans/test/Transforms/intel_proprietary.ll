; RUN: not opt -passes="set-intel-prop" -f -o - < %s 2>&1 | FileCheck %s

; This test verifies that an error is reported if any attempt is made to write
; IR or bitcode when the "Intel Proprietary" module flag is set.

; CHECK: LLVM ERROR: Bitcode output disabled because proprietary optimizations have been performed.

; Function Attrs: nounwind uwtable
define i32 @main(i32 %argc, ptr nocapture readnone %argv) {
entry:
  %call = tail call zeroext i1 @"\01?check_argc@@YA_NH@Z"(i32 %argc)
  %t0 = zext i1 %call to i32
  %t1 = xor i32 %t0, 1
  ret i32 %t1
}

declare zeroext i1 @"\01?check_argc@@YA_NH@Z"(i32)
