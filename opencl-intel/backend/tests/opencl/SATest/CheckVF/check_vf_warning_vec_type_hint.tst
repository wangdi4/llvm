; RUN: SATest -BUILD --vectorizer-type=vpo -cpuarch=corei7-avx -build-log -config=%s.cfg > %t.build_log
; RUN: FileCheck %s --input-file=%t.build_log
; CHECK: warning: kernel "test_2": Kernel can't be vectorized due to unsupported vec type hint
; CHECK: Kernel "test" was successfully vectorized (4)
; CHECK: Kernel "test_1" was successfully vectorized (4)
; CHECK: Kernel "test_2" was not vectorized
