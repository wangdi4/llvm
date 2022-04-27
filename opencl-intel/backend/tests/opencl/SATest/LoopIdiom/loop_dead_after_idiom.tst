;; The test checks scalar loop is dead after LoopIdiom pass and it is deleted.

; RUN: SATest -BUILD --config=%s.cfg --tsize=1 --dump-llvm-file - | FileCheck %s

; CHECK: call void @llvm.memcpy
; CHECK-NOT: scalar_kernel_entry
