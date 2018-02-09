; RUN: SATest -REF -config=%s.cfg -neat=1
;; Mark as XFAIL because NEAT does not support cast from Ty** to Ty*
;; and fails with an assertion in Debug.
; XFAIL: *
