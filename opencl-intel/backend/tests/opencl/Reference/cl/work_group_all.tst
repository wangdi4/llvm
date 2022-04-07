; XFAIL: *
; RUN: SATest -REF -config=%s.cfg -neat=0 >%t
; RUN: FileCheck %s <%t
; CHECK: 0
; CHECK: 0
; CHECK: 1
; CHECK: 1
; CHECK: 0
; CHECK: 0
; CHECK: 1
; CHECK: 1
