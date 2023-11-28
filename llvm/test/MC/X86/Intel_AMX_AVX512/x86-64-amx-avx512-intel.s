# REQUIRES: intel_feature_isa_amx_avx512
# RUN: llvm-mc -triple x86_64 -show-encoding -x86-asm-syntax=intel -output-asm-variant=1 %s | FileCheck %s

# CHECK: tilemovrow	zmm22, tmm3, ecx
# CHECK: encoding: [0x62,0xe2,0x75,0x48,0x4a,0xf3]
         tilemovrow	zmm22, tmm3, ecx

# CHECK: tilemovrow	zmm22, tmm2, ecx
# CHECK: encoding: [0x62,0xe2,0x75,0x48,0x4a,0xf2]
         tilemovrow	zmm22, tmm2, ecx

# CHECK: tilemovrow	zmm22, tmm3, 123
# CHECK: encoding: [0x62,0xe3,0x7d,0x48,0x07,0xf3,0x7b]
         tilemovrow	zmm22, tmm3, 123

# CHECK: tilemovrow	zmm22, tmm2, 123
# CHECK: encoding: [0x62,0xe3,0x7d,0x48,0x07,0xf2,0x7b]
         tilemovrow	zmm22, tmm2, 123

# CHECK: tilemovrow	zmm22, tmm0, edx
# CHECK: encoding: [0x62,0xe2,0x6d,0x48,0x4a,0xf0]
         tilemovrow	zmm22, tmm0, edx

# CHECK: tilemovrow	zmm22, tmm0, 123
# CHECK: encoding: [0x62,0xe3,0x7d,0x48,0x07,0xf0,0x7b]
         tilemovrow	zmm22, tmm0, 123
