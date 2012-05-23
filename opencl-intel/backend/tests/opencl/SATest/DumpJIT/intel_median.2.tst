; RUN: SATest -OCL -BUILD -dump-JIT=%t3 -config=%s.cfg
; RUN: FileCheck %s --input-file=%t3
; CHECK: {{_*intel_median_separated_args}}
