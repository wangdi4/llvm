; RUN: llvm-dis %s.bin -o %s.ll
; RUN: SATest -OCL -BUILD -config=%s.cfg
; RUN: diff intel_median.1.tst.llvm_ir %s.bin