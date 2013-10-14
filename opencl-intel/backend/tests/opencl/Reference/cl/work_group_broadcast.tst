;XFAIL:
; R UN: SATest -OCL -REF -config=%s.cfg -neat=0 >%t
; R UN: FileCheck %s <%t
; C HECK: -453
; C HECK: -453
; C HECK: 4
; C HECK: 4
; C HECK: 0
; C HECK: 0
; C HECK: -333
; C HECK: -333
