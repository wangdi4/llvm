; RUN: SATest -BUILD -config=%s.cfg -cpuarch=core-avx2 --dump-JIT=%s_no_bmi.s -cpufeatures=-bmi -tsize=8
; RUN: grep bsf %s_no_bmi.s
; RUN: SATest -BUILD -config=%s.cfg -cpuarch=core-avx2 --dump-JIT=%s_bmi.s -tsize=8
; RUN: grep tzcntl %s_bmi.s

