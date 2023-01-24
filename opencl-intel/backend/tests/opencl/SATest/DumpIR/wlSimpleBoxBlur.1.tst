; RUN: SATest -BUILD -llvm-option=-print-before=sycl-kernel-equalizer -config=%s.cfg 2>&1 | FileCheck %s
; CHECK: define{{.*}}void @wlSimpleBoxBlur_GPU
