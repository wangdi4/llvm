; RUN: SATest -BUILD --config=%s.cfg 2>&1 | FileCheck %s

; CHECK: error: Unsupported recursive call in function:
; CHECK:   foo
; CHECK:   bar
; CHECK:   test
; CHECK: CompilerException Optimization error
