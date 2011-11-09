RUN: SATest -OCL -BUILD -config=%s.cfg -dump-llvm-file=%t
RUN: llvm-as %t
