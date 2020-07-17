; RUN: SATest -BUILD -config=%s.cfg -cpuarch=core-avx2 --dump-JIT=%t_no_bmi.s -cpufeatures=-bmi -tsize=8
; RUN: grep bsf %t_no_bmi.s
; RUN: SATest -BUILD -config=%s.cfg -cpuarch=core-avx2 --dump-JIT=%t_bmi.s -tsize=8
; RUN: grep tzcnt %t_bmi.s
