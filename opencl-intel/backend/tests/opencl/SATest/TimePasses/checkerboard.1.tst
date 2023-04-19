; XFAIL: linux
; see status of CSSD100013459
; RUN: SATest -BUILD -dump-time-passes=- -config=%s.cfg 2>&1 | FileCheck %s
; CHECK: Instruction Selection and Scheduling
