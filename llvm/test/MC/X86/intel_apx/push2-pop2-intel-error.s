# REQUIRES: intel_feature_isa_apx_f
# RUN: not llvm-mc -triple x86_64 -show-encoding -x86-asm-syntax=intel -output-asm-variant=1 %s 2>&1 | FileCheck %s

# CHECK: error: immediate must be an integer in range [0, 1]
         push2	0, rcx, rax, 2

# CHECK: error: immediate must be an integer in range [0, 3]
         push2	4, rcx, rax, 0

# CHECK: error: immediate must be an integer in range [0, 1]
         pop2	2, rcx, rax, 0

# CHECK: error: immediate must be an integer in range [0, 3]
         pop2	0, rcx, rax, 4
