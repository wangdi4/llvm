; RUN: SATest -BUILD --vectorizer-type=vpo --native-subgroups -cpuarch=corei7-avx -build-log -config=%s.cfg | FileCheck %s
; CHECK: Warning in kernel <test> specified intel_vec_len_hint can't be satisfied. Fall back to autovectorization mode.
; CHECK: Kernel <test> was successfully vectorized (4)
