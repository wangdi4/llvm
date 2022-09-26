; RUN: SATest -BUILD -config=%s.cfg -cpuarch=core-avx2 --dump-JIT=%t_no_bmi.s -cpufeatures=-bmi -tsize=8
; FileCheck %s --input-file=%t_no_bmi.s -check-prefix=CHECK-NOBMI
; RUN: SATest -BUILD -config=%s.cfg -cpuarch=core-avx2 --dump-JIT=%t_bmi.s -tsize=8
; FileCheck %s --input-file=%t_bmi.s -check-prefix=CHECK-BMI

; CHECK-NOBMI-NOT: andn
; CHECK-BMI: andn
