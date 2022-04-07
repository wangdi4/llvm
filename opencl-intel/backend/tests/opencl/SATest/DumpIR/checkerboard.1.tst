; RUN: SATest -BUILD -llvm-option=-print-before=dpcpp-kernel-equalizer -config=%s.cfg 2>&1 | FileCheck %s
; CHECK: define{{.*}}void @checkerboard2D
