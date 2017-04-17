; XFAILX: i686-pc-win32
; RUNX: SATest -BUILD -config=%s.cfg -cpuarch=core-avx2 --dump-JIT=%t_no_bmi.s -cpufeatures=-bmi -tsize=8
; RUNX: grep bsf %t_no_bmi.s
; RUNX: SATest -BUILD -config=%s.cfg -cpuarch=core-avx2 --dump-JIT=%t_bmi.s -tsize=8
; RUNX: grep tzcntl %t_bmi.s

