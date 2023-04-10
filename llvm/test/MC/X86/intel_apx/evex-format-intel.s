# REQUIRES: intel_feature_isa_apx_f
## NOTE: This file needs to be updated after promoted instruction is supported
# RUN: llvm-mc -triple x86_64 -show-encoding -x86-asm-syntax=intel -output-asm-variant=1 %s | FileCheck %s

## MRMDestMem

# CHECK: vextractf32x4	xmmword ptr [r16 + r17], zmm0, 1
# CHECK: encoding: [0x62,0xfb,0x79,0x48,0x19,0x04,0x08,0x01]
         vextractf32x4	xmmword ptr [r16 + r17], zmm0, 1

# CHECK: shld	r18d, dword ptr [r17d + 127], r16d, cl
# CHECK: encoding: [0x67,0x62,0xec,0x6c,0x10,0xa5,0x41,0x7f]
         shld	r18d, dword ptr [r17d + 127], r16d, cl

## MRMSrcMem

# CHECK: vbroadcasti32x4	zmm0, xmmword ptr [r16 + r17]
# CHECK: encoding: [0x62,0xfa,0x79,0x48,0x5a,0x04,0x08]
         vbroadcasti32x4	zmm0, xmmword ptr [r16 + r17]

## MRM0m

# CHECK: vprorq	zmm0, zmmword ptr [r16 + r17], 0
# CHECK: encoding: [0x62,0xf9,0xf9,0x48,0x72,0x04,0x08,0x00]
         vprorq	zmm0, zmmword ptr [r16 + r17], 0

# CHECK: rol	r17d, dword ptr [r16d + 127], cl
# CHECK: encoding: [0x67,0x62,0xfc,0x74,0x10,0xd3,0x40,0x7f]
         rol	r17d, dword ptr [r16d + 127], cl

## MRM1m

# CHECK: vprolq	zmm0, zmmword ptr [r16 + r17], 0
# CHECK: encoding: [0x62,0xf9,0xf9,0x48,0x72,0x0c,0x08,0x00]
         vprolq	zmm0, zmmword ptr [r16 + r17], 0

# CHECK: ror	r17d, dword ptr [r16d + 127], cl
# CHECK: encoding: [0x67,0x62,0xfc,0x74,0x10,0xd3,0x48,0x7f]
         ror	r17d, dword ptr [r16d + 127], cl

## MRM2m

# CHECK: vpsrlq	zmm0, zmmword ptr [r16 + r17], 0
# CHECK: encoding: [0x62,0xf9,0xf9,0x48,0x73,0x14,0x08,0x00]
         vpsrlq	zmm0, zmmword ptr [r16 + r17], 0

# CHECK: rcl	r17d, dword ptr [r16d + 127], cl
# CHECK: encoding: [0x67,0x62,0xfc,0x74,0x10,0xd3,0x50,0x7f]
         rcl	r17d, dword ptr [r16d + 127], cl

## MRM3m

# CHECK: vpsrldq	zmm0, zmmword ptr [r16 + r17], 0
# CHECK: encoding: [0x62,0xf9,0x79,0x48,0x73,0x1c,0x08,0x00]
         vpsrldq	zmm0, zmmword ptr [r16 + r17], 0

# CHECK: rcr	r17d, dword ptr [r16d + 127], cl
# CHECK: encoding: [0x67,0x62,0xfc,0x74,0x10,0xd3,0x58,0x7f]
         rcr	r17d, dword ptr [r16d + 127], cl

## MRM4m

# CHECK: vpsraq	zmm0, zmmword ptr [r16 + r17], 0
# CHECK: encoding: [0x62,0xf9,0xf9,0x48,0x72,0x24,0x08,0x00]
         vpsraq	zmm0, zmmword ptr [r16 + r17], 0

# CHECK: shl	r17d, dword ptr [r16d + 127], cl
# CHECK: encoding: [0x67,0x62,0xfc,0x74,0x10,0xd3,0x60,0x7f]
         shl	r17d, dword ptr [r16d + 127], cl

## MRM5m
## AsmParser is buggy for this KNC instruction
# C;HECK: vscatterpf0dps	{k1}, zmmword ptr [r16 + zmm0]
# C;HECK: encoding: [0x62,0xfa,0x7d,0x49,0xc6,0x2c,0x00]
#         vscatterpf0dps	{k1}, zmmword ptr [r16 + zmm0]

# CHECK: shr	r17d, dword ptr [r16d + 127], cl
# CHECK: encoding: [0x67,0x62,0xfc,0x74,0x10,0xd3,0x68,0x7f]
         shr	r17d, dword ptr [r16d + 127], cl

## MRM6m

# CHECK: vpsllq	zmm0, zmmword ptr [r16 + r17], 0
# CHECK: encoding: [0x62,0xf9,0xf9,0x48,0x73,0x34,0x08,0x00]
         vpsllq	zmm0, zmmword ptr [r16 + r17], 0

## MRM7m

# CHECK: vpslldq	zmm0, zmmword ptr [r16 + r17], 0
# CHECK: encoding: [0x62,0xf9,0x79,0x48,0x73,0x3c,0x08,0x00]
         vpslldq	zmm0, zmmword ptr [r16 + r17], 0

# CHECK: sar	r17d, dword ptr [r16d + 127], cl
# CHECK: encoding: [0x67,0x62,0xfc,0x74,0x10,0xd3,0x78,0x7f]
         sar	r17d, dword ptr [r16d + 127], cl

## MRMDestReg

# CHECK: vextractps	r16d, xmm16, 1
# CHECK: encoding: [0x62,0xeb,0x7d,0x08,0x17,0xc0,0x01]
         vextractps	r16d, xmm16, 1

# CHECK: shld	r18d, r17d, r16d, cl
# CHECK: encoding: [0x62,0xec,0x6c,0x10,0xa5,0xc1]
         shld	r18d, r17d, r16d, cl

## MRM0r

# CHECK: rol	r17d, r16d, cl
# CHECK: encoding: [0x62,0xfc,0x74,0x10,0xd3,0xc0]
         rol	r17d, r16d, cl

## MRM1r

# CHECK: ror	r17d, r16d, cl
# CHECK: encoding: [0x62,0xfc,0x74,0x10,0xd3,0xc8]
         ror	r17d, r16d, cl

## MRM2r

# CHECK: rcl	r17d, r16d, cl
# CHECK: encoding: [0x62,0xfc,0x74,0x10,0xd3,0xd0]
         rcl	r17d, r16d, cl

## MRM3r

# CHECK: rcr	r17d, r16d, cl
# CHECK: encoding: [0x62,0xfc,0x74,0x10,0xd3,0xd8]
         rcr	r17d, r16d, cl

## MRM4r

# CHECK: shl	r17d, r16d
# CHECK: encoding: [0x62,0xfc,0x74,0x10,0xd1,0xe0]
         shl	r17d, r16d

## MRM5r

# CHECK: shr	r17d, r16d, cl
# CHECK: encoding: [0x62,0xfc,0x74,0x10,0xd3,0xe8]
         shr	r17d, r16d, cl

## MRM7r

# CHECK: sar	r17d, r16d, cl
# CHECK: encoding: [0x62,0xfc,0x74,0x10,0xd3,0xf8]
         sar	r17d, r16d, cl
