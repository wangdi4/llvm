; RUN: SATest -OCL -BUILD -dump-time-passes=%t1 -config=%s.cfg
; RUN: FileCheck %s --input-file=%t1
; CHECK:                      Instruction Selection and Scheduling
