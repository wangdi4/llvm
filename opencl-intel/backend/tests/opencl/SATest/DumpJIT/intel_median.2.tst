; RUN: SATest -OCL -PERF -dump-JIT=%t3 -config=%s.cfg
; RUN: FileCheck %s --input-file=%t3
; CHECK: .def {{_*intel_median_separated_args}}
