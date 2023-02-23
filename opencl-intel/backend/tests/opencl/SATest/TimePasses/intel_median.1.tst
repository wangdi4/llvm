; XFAIL: linux
; RUN: SATest -BUILD -dump-time-passes=- -config=%s.cfg | FileCheck %s
; CHECK:                      Pass execution timing report
