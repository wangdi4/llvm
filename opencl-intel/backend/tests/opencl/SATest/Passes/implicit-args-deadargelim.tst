; RUN: SATest -BUILD --cpuarch=skx -tsize=4 -config=%s.cfg -dump-llvm-file - 2>&1 | FileCheck %s

; Check that unused arguments, e.g. %pWGId, are eliminated from function
; _ZGVeN4v_foo and foo.

; CHECK-LABEL: define internal fastcc void @_ZGVeN4v_foo({{.*}} %dst, i64 %local.ids.0.val, [4 x i64] %BaseGlbId)
; CHECK-LABEL: define internal fastcc void @foo({{.*}} %dst, i64 %local.ids.0.val, [4 x i64] %BaseGlbId)

; CHECK: Test program was successfully built.
