; Checks that auto-memory is working as expected when the kernel requires a large barrier buffer.

; RUN: SATest --VAL --config=%s.cfg -noref -llvm-option='-debug-only=dpcpp-kernel-prepare-args,dpcpp-kernel-barrier' 2>&1 | FileCheck %s

; The local sizes for kernel execution are {1, 1, 1}
; but the kernel is vectorized to VF=64.
; So the actual barrier buffer size is rounded-up to BUFFER_SIZE_PER_WI * 64 * 1 * 1.

; The barrier buffer size per WI is around 130KB
; --> total barrier buffer size is larger than 8MB (130KB * 64)
; CHECK: Set metadata for kernel k1: BarrierBufferSize=[[#BUFFER_SIZE_PER_WI:]]

; Dump the allocated barrier buffer size when running the kernel.
; CHECK: PRINT SPECIAL BUFFER SIZE:{{.*}} [[#mul(BUFFER_SIZE_PER_WI, 64)]]

; Auto-memory should opt-in because of the large stack usage,
; checks that kernel execution doesn't fail with a stack overflow (segfault).
; CHECK: Test Passed.
