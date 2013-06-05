; RUN: SATest -OCL -BUILD -dump-JIT=%t1 -config=%s.cfg
; RUN: FileCheck %s --input-file=%t1
; CHECK-NOT: {{[a-zA-Z]*}}fmadd{{[a-zA-Z0-9]*}}
