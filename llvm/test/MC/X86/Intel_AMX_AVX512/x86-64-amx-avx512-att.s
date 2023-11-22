# REQUIRES: intel_feature_isa_amx_avx512
# RUN: llvm-mc -triple x86_64 -show-encoding %s | FileCheck %s

# CHECK: tilemovrow	%ecx, %tmm3, %zmm22
# CHECK: encoding: [0x62,0xe2,0x75,0x48,0x4a,0xf3]
         tilemovrow	%ecx, %tmm3, %zmm22

# CHECK: tilemovrow	%ecx, %tmm2, %zmm22
# CHECK: encoding: [0x62,0xe2,0x75,0x48,0x4a,0xf2]
         tilemovrow	%ecx, %tmm2, %zmm22

# CHECK: tilemovrow	$123, %tmm3, %zmm22
# CHECK: encoding: [0x62,0xe3,0x7d,0x48,0x07,0xf3,0x7b]
         tilemovrow	$123, %tmm3, %zmm22

# CHECK: tilemovrow	$123, %tmm2, %zmm22
# CHECK: encoding: [0x62,0xe3,0x7d,0x48,0x07,0xf2,0x7b]
         tilemovrow	$123, %tmm2, %zmm22

# CHECK: tilemovrow	%edx, %tmm0, %zmm22
# CHECK: encoding: [0x62,0xe2,0x6d,0x48,0x4a,0xf0]
         tilemovrow	%edx, %tmm0, %zmm22

# CHECK: tilemovrow	$123, %tmm0, %zmm22
# CHECK: encoding: [0x62,0xe3,0x7d,0x48,0x07,0xf0,0x7b]
         tilemovrow	$123, %tmm0, %zmm22
