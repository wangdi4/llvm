# REQUIRES: intel_feature_isa_apx_f
# RUN: not llvm-mc -triple x86_64 -show-encoding %s 2>&1 | FileCheck %s

# CHECK: error: immediate must be an integer in range [0, 1]
         push2	$2, %rax, %rcx, $0

# CHECK: error: immediate must be an integer in range [0, 3]
         push2	$0, %rax, %rcx, $4

# CHECK: error: immediate must be an integer in range [0, 1]
         pop2	$0, %rax, %rcx, $2

# CHECK: error: immediate must be an integer in range [0, 3]
         pop2	$4, %rax, %rcx, $0
