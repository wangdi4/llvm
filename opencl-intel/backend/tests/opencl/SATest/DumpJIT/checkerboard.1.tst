; RUN: SATest -OCL -BUILD -dump-JIT=%t1 -config=%s.cfg
; RUN: FileCheck %s --input-file=%t1
; CHECK: .def	 _____Vectorized_.checkerboard2D_separated_args;
