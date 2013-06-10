; RUN: SATest -OCL -REF -config=%s.cfg -neat=0 >%t
; RUN: FileCheck %s <%t
; CHECK: -453
; CHECK: -453
; CHECK: 4
; CHECK: 4
; CHECK: 0
; CHECK: 0
; CHECK: -333
; CHECK: -333
