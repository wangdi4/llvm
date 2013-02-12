; RUN: SATest -OCL -REF -config=%s.cfg -neat=1
; RUN: NEATChecker -r %s -a %s.neat -t 0

;CHECKNEAT: ACCURATE 3
;CHECKNEAT: ACCURATE 3
;CHECKNEAT: ACCURATE 3
;CHECKNEAT: ACCURATE 3
;CHECKNEAT: ACCURATE 3
; XFAIL: linux
; expected to fail till CSSD100015632 will be fixed