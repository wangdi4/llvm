# RUN: llvm-mc -triple x86_64 -show-encoding -x86-asm-syntax=intel -output-asm-variant=1 %s | FileCheck %s

# CHECK: pushp	rax
# CHECK: encoding: [0xd5,0x08,0x50]
         pushp	rax
# CHECK: pushp	r16
# CHECK: encoding: [0xd5,0x18,0x50]
         pushp	r16
# CHECK: popp	rax
# CHECK: encoding: [0xd5,0x08,0x58]
         popp	rax
# CHECK: popp	r16
# CHECK: encoding: [0xd5,0x18,0x58]
         popp	r16
