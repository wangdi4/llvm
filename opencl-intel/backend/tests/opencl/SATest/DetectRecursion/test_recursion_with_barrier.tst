; RUN: SATest -BUILD --config=%s.cfg 2>&1 | FileCheck %s
; CHECK: Error: recursive call in function(s)
