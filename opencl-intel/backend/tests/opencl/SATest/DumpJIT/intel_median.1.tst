; RUN: SATest -OCL -BUILD -dump-JIT=%t2 -config=%s.cfg
; RUN: FileCheck %s --input-file=%t2
; CHECK: .def	 {{_*intel_median_scalar}}

