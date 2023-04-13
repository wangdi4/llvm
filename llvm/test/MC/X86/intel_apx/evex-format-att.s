# REQUIRES: intel_feature_isa_apx_f
## NOTE: This file needs to be updated after promoted instruction is supported
# RUN: llvm-mc -triple x86_64 -show-encoding %s | FileCheck %s

## MRMDestMem

# CHECK: vextractf32x4	$1, %zmm0, (%r16,%r17)
# CHECK: encoding: [0x62,0xfb,0x79,0x48,0x19,0x04,0x08,0x01]
         vextractf32x4	$1, %zmm0, (%r16,%r17)

# CHECK: shldl	%cl, %r16d, 127(%r17d), %r18d
# CHECK: encoding: [0x67,0x62,0xec,0x6c,0x10,0xa5,0x41,0x7f]
         shldl	%cl, %r16d, 127(%r17d), %r18d

## MRMSrcMem

# CHECK: vbroadcasti32x4	(%r16,%r17), %zmm0
# CHECK: encoding: [0x62,0xfa,0x79,0x48,0x5a,0x04,0x08]
         vbroadcasti32x4	(%r16,%r17), %zmm0

## MRM0m

# CHECK: vprorq	$0, (%r16,%r17), %zmm0
# CHECK: encoding: [0x62,0xf9,0xf9,0x48,0x72,0x04,0x08,0x00]
         vprorq	$0, (%r16,%r17), %zmm0

# CHECK: roll	%cl, 127(%r16d), %r17d
# CHECK: encoding: [0x67,0x62,0xfc,0x74,0x10,0xd3,0x40,0x7f]
         roll	%cl, 127(%r16d), %r17d

## MRM1m

# CHECK: vprolq	$0, (%r16,%r17), %zmm0
# CHECK: encoding: [0x62,0xf9,0xf9,0x48,0x72,0x0c,0x08,0x00]
         vprolq	$0, (%r16,%r17), %zmm0

# CHECK: rorl	%cl, 127(%r16d), %r17d
# CHECK: encoding: [0x67,0x62,0xfc,0x74,0x10,0xd3,0x48,0x7f]
         rorl	%cl, 127(%r16d), %r17d

## MRM2m

# CHECK: vpsrlq	$0, (%r16,%r17), %zmm0
# CHECK: encoding: [0x62,0xf9,0xf9,0x48,0x73,0x14,0x08,0x00]
         vpsrlq	$0, (%r16,%r17), %zmm0

# CHECK: rcll	%cl, 127(%r16d), %r17d
# CHECK: encoding: [0x67,0x62,0xfc,0x74,0x10,0xd3,0x50,0x7f]
         rcll	%cl, 127(%r16d), %r17d

## MRM3m

# CHECK: vpsrldq	$0, (%r16,%r17), %zmm0
# CHECK: encoding: [0x62,0xf9,0x79,0x48,0x73,0x1c,0x08,0x00]
         vpsrldq	$0, (%r16,%r17), %zmm0

# CHECK: rcrl	%cl, 127(%r16d), %r17d
# CHECK: encoding: [0x67,0x62,0xfc,0x74,0x10,0xd3,0x58,0x7f]
         rcrl	%cl, 127(%r16d), %r17d

## MRM4m

# CHECK: vpsraq	$0, (%r16,%r17), %zmm0
# CHECK: encoding: [0x62,0xf9,0xf9,0x48,0x72,0x24,0x08,0x00]
         vpsraq	$0, (%r16,%r17), %zmm0

# CHECK: shll	%cl, 127(%r16d), %r17d
# CHECK: encoding: [0x67,0x62,0xfc,0x74,0x10,0xd3,0x60,0x7f]
         shll	%cl, 127(%r16d), %r17d

## MRM5m

# CHECK: vscatterpf0dps	(%r16,%zmm0) {%k1}
# CHECK: encoding: [0x62,0xfa,0x7d,0x49,0xc6,0x2c,0x00]
         vscatterpf0dps	(%r16,%zmm0) {%k1}

# CHECK: shrl	%cl, 127(%r16d), %r17d
# CHECK: encoding: [0x67,0x62,0xfc,0x74,0x10,0xd3,0x68,0x7f]
         shrl	%cl, 127(%r16d), %r17d

## MRM6m

# CHECK: vpsllq	$0, (%r16,%r17), %zmm0
# CHECK: encoding: [0x62,0xf9,0xf9,0x48,0x73,0x34,0x08,0x00]
         vpsllq	$0, (%r16,%r17), %zmm0

## MRM7m

# CHECK: vpslldq	$0, (%r16,%r17), %zmm0
# CHECK: encoding: [0x62,0xf9,0x79,0x48,0x73,0x3c,0x08,0x00]
         vpslldq	$0, (%r16,%r17), %zmm0

# CHECK: sarl	%cl, 127(%r16d), %r17d
# CHECK: encoding: [0x67,0x62,0xfc,0x74,0x10,0xd3,0x78,0x7f]
         sarl	%cl, 127(%r16d), %r17d

## MRMDestReg

# CHECK: vextractps	$1, %xmm16, %r16d
# CHECK: encoding: [0x62,0xeb,0x7d,0x08,0x17,0xc0,0x01]
         vextractps	$1, %xmm16, %r16d

# CHECK: shldl	%cl, %r16d, %r17d, %r18d
# CHECK: encoding: [0x62,0xec,0x6c,0x10,0xa5,0xc1]
         shldl	%cl, %r16d, %r17d, %r18d

## MRM0r

# CHECK: roll	%cl, %r16d, %r17d
# CHECK: encoding: [0x62,0xfc,0x74,0x10,0xd3,0xc0]
         roll	%cl, %r16d, %r17d

## MRM1r

# CHECK: rorl	%cl, %r16d, %r17d
# CHECK: encoding: [0x62,0xfc,0x74,0x10,0xd3,0xc8]
         rorl	%cl, %r16d, %r17d

## MRM2r

# CHECK: rcll	%cl, %r16d, %r17d
# CHECK: encoding: [0x62,0xfc,0x74,0x10,0xd3,0xd0]
         rcll	%cl, %r16d, %r17d

## MRM3r

# CHECK: rcrl	%cl, %r16d, %r17d
# CHECK: encoding: [0x62,0xfc,0x74,0x10,0xd3,0xd8]
         rcrl	%cl, %r16d, %r17d

## MRM4r

# CHECK: shll	%r16d, %r17d
# CHECK: encoding: [0x62,0xfc,0x74,0x10,0xd1,0xe0]
         shll	%r16d, %r17d

## MRM5r

# CHECK: shrl	%cl, %r16d, %r17d
# CHECK: encoding: [0x62,0xfc,0x74,0x10,0xd3,0xe8]
         shrl	%cl, %r16d, %r17d

## MRM7r

# CHECK: sarl	%cl, %r16d, %r17d
# CHECK: encoding: [0x62,0xfc,0x74,0x10,0xd3,0xf8]
         sarl	%cl, %r16d, %r17d
