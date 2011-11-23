; RUN: SATest -OCL -BUILD -dump-JIT=%t2 -config=%s.cfg
; RUN: FileCheck %s --input-file=%t2
; CHECK: .def	 _____Vectorized_.intel_median_separated_args;
