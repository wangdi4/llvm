; RUN: SATest -BUILD -tsize=4 -pass-manager-type=lto-legacy -cpuarch=skx -build-log -config=%s.cfg | FileCheck %s

; CHECK: Kernel "test" was successfully vectorized (4)
