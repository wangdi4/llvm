; RUN: SATest -VAL -config=%s.cfg -force_ref
;attempt to generate pointer within the structure. does not supported by OCLKernelGenerator
; XFAIL: *
