; RUN: SATest -REF -config=%s.cfg -neat=1
; XFAIL: *
; TODO: add NEATChecker -r %s.ll -a %s.neat -t 0
; TODO: Rewrite test to eliminate pointer bitcast instruction which is not supported by NEAT.
