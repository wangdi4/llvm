; RUN: SATest -BUILD -dump-JIT=%t2 -config=%s.cfg -cpuarch=core-avx2
; RUN: FileCheck %s --input-file=%t2
; CHECK: {{[a-zA-Z]*}}fmadd{{[a-zA-Z0-9]*}}
