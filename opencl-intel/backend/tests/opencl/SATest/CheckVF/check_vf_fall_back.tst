; RUN: SATest -BUILD --vectorizer-type=vpo --native-subgroups -cpuarch=corei7-avx -build-log -config=%s.cfg | FileCheck %s
; CHECK: Kernel <test> was successfully vectorized (4)
