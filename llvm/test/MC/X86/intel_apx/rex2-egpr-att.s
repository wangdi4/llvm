# REQUIRES: intel_feature_isa_apx_f
# RUN: llvm-mc -triple x86_64 -show-encoding %s | FileCheck %s

# CHECK: incl	%r16d
# CHECK: encoding: [0xd5,0x10,0xff,0xc0]
         incl	%r16d
