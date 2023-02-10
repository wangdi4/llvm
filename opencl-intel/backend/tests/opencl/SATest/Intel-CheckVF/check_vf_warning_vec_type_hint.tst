; RUN: SATest -BUILD -pass-manager-type=none -cpuarch=corei7-avx -build-log -config=%s.cfg 2>&1 | FileCheck %s
; RUN: SATest -BUILD -pass-manager-type=ocl -cpuarch=corei7-avx -build-log -config=%s.cfg 2>&1 | FileCheck %s
; RUN: SATest -BUILD -pass-manager-type=lto -cpuarch=corei7-avx -build-log -config=%s.cfg 2>&1 | FileCheck %s

; CHECK: warning: kernel "test_2": Kernel can't be vectorized due to unsupported vec type hint
; CHECK: Kernel "test" was successfully vectorized (4)
; CHECK: Kernel "test_1" was successfully vectorized (4)
; CHECK: Kernel "test_2" was not vectorized
