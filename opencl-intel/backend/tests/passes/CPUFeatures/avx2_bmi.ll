; XFAIL: *

; XFAIL: i686-pc-win32
; RUN: SATest -BUILD -config=%s.cfg -cpuarch=core-avx2 --dump-JIT=%t_no_bmi.s -cpufeatures=-bmi -tsize=8
; RUN: grep bsf %t_no_bmi.s
; RUN: SATest -BUILD -config=%s.cfg -cpuarch=core-avx2 --dump-JIT=%t_bmi.s -tsize=8
; RUN: grep tzcntl %t_bmi.s

; INTEL_CUSTOMIZATION
; Force fail to avoid unexpected passes.
; See CORC-7259
; RUN: FileCheck %s < %t_bmi.s
; CHECK: Force fail
; end INTEL_CUSTOMIZATION
