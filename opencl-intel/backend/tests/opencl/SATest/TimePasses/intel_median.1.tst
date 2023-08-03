; XFAIL: linux
; RUN: SATest -BUILD -dump-time-passes=- -config=%s.cfg 2>&1 | FileCheck %s
; CHECK: Pass execution timing report
