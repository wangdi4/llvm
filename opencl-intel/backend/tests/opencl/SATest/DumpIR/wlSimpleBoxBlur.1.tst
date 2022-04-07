; RUN: SATest -BUILD -llvm-option=-print-before=dpcpp-kernel-vec-clone -config=%s.cfg 2>&1 | FileCheck %s
; CHECK: define{{.*}}void @wlSimpleBoxBlur_GPU
