; Checks that local/special buffer will use heap memory as expected when the kernel requires a large local and barrier buffer.

; RUN: SATest --VAL --config=%s.cfg -noref -llvm-option='-debug-only=sycl-kernel-prepare-args,sycl-kernel-barrier' 2>&1 | FileCheck %s

; CHECK-DAG: LOCAL OR SPECIAL BUFFER USE HEAP MEMORY:  [CMP]{{.*}}: 0 [ALLOCA_SIZE]{{.*}}: 0
; CHECK-DAG: Test Passed.
