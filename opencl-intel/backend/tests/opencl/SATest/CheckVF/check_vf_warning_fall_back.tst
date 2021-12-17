; RUN: SATest -BUILD --enable-subgroup-emu=false --vectorizer-type=vpo --native-subgroups -cpuarch=corei7-avx -build-log -config=%s.cfg | FileCheck %s
; CHECK: warning: kernel "test": Fall back vectorization width to 4 due to unsupported vec_len_hint value for workgroup/subgroup builtins
; CHECK: Kernel "test" was successfully vectorized (4)
