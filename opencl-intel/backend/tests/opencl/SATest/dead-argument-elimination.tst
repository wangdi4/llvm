;; The test checks dead implicit args are removed.

; RUN: SATest -BUILD --config=%s.cfg -tsize=1 --dump-llvm-file - | FileCheck %s
; RUN: SATest -BUILD --config=%s.cfg -tsize=1 --dump-llvm-file - -pass-manager-type=lto | FileCheck %s

; CHECK: define internal fastcc i32 @foo(i32 noundef %a)
