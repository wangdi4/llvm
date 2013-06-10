; RUN: SATest -OCL -REF -config=%s.cfg -neat=0 >%t
; RUN: FileCheck %s <%t
; CHECK: 3
; CHECK: 3
; CHECK: 3
; CHECK: 3
; CHECK: 2343
; CHECK: 2343
; CHECK: 2343
; CHECK: 2343
; CHECK: 433
; CHECK: 433
; CHECK: 433
; CHECK: 433
; CHECK: 4533
; CHECK: 4533
; CHECK: 4533
; CHECK: 4533
