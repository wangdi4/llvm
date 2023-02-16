# REQUIRES: intel_feature_isa_apx_f
# RUN: llvm-mc -triple x86_64 -show-encoding -x86-asm-syntax=intel -output-asm-variant=1 %s | FileCheck %s

# CHECK: inc	r16d
# CHECK: encoding: [0xd5,0x10,0xff,0xc0]
         inc	r16d
