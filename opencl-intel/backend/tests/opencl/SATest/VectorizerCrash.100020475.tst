; RUN: SATest -BUILD --config=%s.cfg -tsize=4 2>&1 | FileCheck %s

; CHECK: Test program was successfully built.
