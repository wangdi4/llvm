; XFAIL: linux
; see status of CSSD100013459
; RUN: SATest -BUILD -dump-time-passes=- -config=%s.cfg | FileCheck %s
; CHECK:                      Instruction Selection and Scheduling
