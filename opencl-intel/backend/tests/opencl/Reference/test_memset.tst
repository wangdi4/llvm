; XFAIL: *
; RUN: llvm-as %s.ll -o %s.bin
; RUN: SATest -REF -config=%s.cfg -neat=0 > %t
; RUN: FileCheck %s <%t
; CHECK: 50529027 50529027 50529027 50529027 50529027 50529027 50529027 50529027
