; RUN: SATest -REF -config=%s.cfg -neat=0 >%t
; RUN: FileCheck %s <%t
; CHECK: -453
; CHECK: -453
; CHECK: -453
; CHECK: -453
; CHECK: 0
; CHECK: 0
; CHECK: 7
; CHECK: 7
; CHECK: 3
; CHECK: 3
; CHECK: 4
; CHECK: 4
; CHECK: -123
; CHECK: -123
; CHECK: -123
; CHECK: -123
; CHECK: 0
; CHECK: 0
; CHECK: -678
; CHECK: -678
; CHECK: -345
; CHECK: -345
; CHECK: -333
; CHECK: -333
