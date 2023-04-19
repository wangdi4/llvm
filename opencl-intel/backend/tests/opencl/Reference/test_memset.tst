; XFAIL: *
; RUN: llvm-as %s.ll -o %s.bin
; RUN: SATest -REF -config=%s.cfg -neat=0 2>&1 | FileCheck %s

; CHECK: 50529027 50529027 50529027 50529027 50529027 50529027 50529027 50529027
