; This test checks that the following floating-point conversion symbols are
; resolved and build is successful.

; RUN: SATest -BUILD --config=%s.cfg -cpuarch="corei7" --dump-JIT=%t
; RUN: FileCheck %s --input-file=%t

; CHECK-DAG: __trunctfdf2
; CHECK-DAG: __trunctfsf2
; CHECK-DAG: __trunctfhf2
; CHECK-DAG: __truncdfhf2
; CHECK-DAG: __truncsfhf2
; CHECK-DAG: __extendhfsf2
; CHECK-DAG: __extendhftf2
; CHECK-DAG: __extendsftf2
; CHECK-DAG: __extenddftf2
