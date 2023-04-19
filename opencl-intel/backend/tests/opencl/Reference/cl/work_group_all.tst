; RUN: SATest -REF -config=%s.cfg -neat=0 2>&1 | FileCheck %s

; CHECK: 0
; CHECK: 0
; CHECK: 1
; CHECK: 1
; CHECK: 0
; CHECK: 0
; CHECK: 1
; CHECK: 1
