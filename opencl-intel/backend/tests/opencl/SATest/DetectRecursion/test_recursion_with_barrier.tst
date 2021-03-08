; RUN: SATest -BUILD --config=%s.cfg |& FileCheck %s
; CHECK: Error: recursive call in function(s)
