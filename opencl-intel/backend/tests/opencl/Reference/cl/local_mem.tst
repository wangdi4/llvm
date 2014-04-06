; RUN: SATest -REF -config=%s.cfg -neat=0 >%t
; RUN: FileCheck %s <%t
; CHECK: 36
