;RUN: SATest -OCL -PERF -config=%s.cfg | FileCheck %s
;CHECK: 0x12345678,0xfedcba
