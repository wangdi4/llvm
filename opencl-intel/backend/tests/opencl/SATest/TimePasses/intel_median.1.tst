; XFAIL: linux
; see status of CSSD100013459
; RUN: SATest -OCL -BUILD -dump-time-passes=%t2 -config=%s.cfg
; RUN: FileCheck %s --input-file=%t2
; CHECK:                      ... Pass execution timing report ...
