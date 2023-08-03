;XFAIL: i686-pc-win32
;RUN: SATest -BUILD -config=%s.cfg -llvm-option=-print-before=sycl-kernel-target-ext-type-lower 2>&1 | FileCheck %s
;CHECK: = call {{.*}}read_image{{.*}}{{%opencl.sampler_t|spirv.Sampler}}
