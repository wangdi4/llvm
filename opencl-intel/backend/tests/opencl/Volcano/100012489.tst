;UNSUPPORTED: true
;RUN: SATest -PERF -config=%s.cfg 2>&1 | FileCheck %s
;CHECK: 0x12345678,0xfedcba
