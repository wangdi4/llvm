; RUN: SATest -BUILD --cpuarch=skx -tsize=16 --config=%s.cfg 2>&1 | FileCheck %s

; CHECK: error: Recursive call in function with barrier is unsupported:
; CHECK:   _ZGVeN16v_foo_with_barrier
; CHECK:   _ZGVeM16v_foo_with_barrier
; CHECK:   foo_with_barrier
; CHECK:   test
; CHECK:   _ZGVeN16u_test
; CHECK: CompilerException Optimization error
