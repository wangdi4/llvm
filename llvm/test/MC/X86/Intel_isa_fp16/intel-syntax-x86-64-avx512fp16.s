// REQUIRES: intel_feature_isa_fp16
// RUN: llvm-mc -triple x86_64-unknown-unknown -mcpu=knl -mattr=+avx512fp16 -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK: vaddph zmm19 {k2}, zmm19, zmm19, {rn-sae}
// CHECK: # encoding: [0x62,0xa5,0x64,0x12,0x58,0xdb]
          vaddph zmm19 {k2}, zmm19, zmm19, {rn-sae}

// CHECK: vaddph zmm19 {k2} {z}, zmm19, zmm19, {rn-sae}
// CHECK: # encoding: [0x62,0xa5,0x64,0x92,0x58,0xdb]
          vaddph zmm19 {k2} {z}, zmm19, zmm19, {rn-sae}

// CHECK: vaddph zmm19 {k2}, zmm19, zmm19, {rd-sae}
// CHECK: # encoding: [0x62,0xa5,0x64,0x32,0x58,0xdb]
          vaddph zmm19 {k2}, zmm19, zmm19, {rd-sae}

// CHECK: vaddph zmm19 {k2} {z}, zmm19, zmm19, {rd-sae}
// CHECK: # encoding: [0x62,0xa5,0x64,0xb2,0x58,0xdb]
          vaddph zmm19 {k2} {z}, zmm19, zmm19, {rd-sae}

// CHECK: vaddph zmm19 {k2}, zmm19, zmm19, {ru-sae}
// CHECK: # encoding: [0x62,0xa5,0x64,0x52,0x58,0xdb]
          vaddph zmm19 {k2}, zmm19, zmm19, {ru-sae}

// CHECK: vaddph zmm19 {k2} {z}, zmm19, zmm19, {ru-sae}
// CHECK: # encoding: [0x62,0xa5,0x64,0xd2,0x58,0xdb]
          vaddph zmm19 {k2} {z}, zmm19, zmm19, {ru-sae}

// CHECK: vaddph zmm19 {k2}, zmm19, zmm19, {rz-sae}
// CHECK: # encoding: [0x62,0xa5,0x64,0x72,0x58,0xdb]
          vaddph zmm19 {k2}, zmm19, zmm19, {rz-sae}

// CHECK: vaddph zmm19 {k2} {z}, zmm19, zmm19, {rz-sae}
// CHECK: # encoding: [0x62,0xa5,0x64,0xf2,0x58,0xdb]
          vaddph zmm19 {k2} {z}, zmm19, zmm19, {rz-sae}

// CHECK: vaddph zmm19 {k2}, zmm19, zmm19
// CHECK: # encoding: [0x62,0xa5,0x64,0x42,0x58,0xdb]
          vaddph zmm19 {k2}, zmm19, zmm19

// CHECK: vaddph zmm19 {k2} {z}, zmm19, zmm19
// CHECK: # encoding: [0x62,0xa5,0x64,0xc2,0x58,0xdb]
          vaddph zmm19 {k2} {z}, zmm19, zmm19

// CHECK: vaddph zmm19 {k2}, zmm19, zmmword ptr [485498096]
// CHECK: # encoding: [0x62,0xe5,0x64,0x42,0x58,0x1c,0x25,0xf0,0x1c,0xf0,0x1c]
          vaddph zmm19 {k2}, zmm19, zmmword ptr [485498096]

// CHECK: vaddph zmm19 {k2}, zmm19, word ptr [485498096]{1to32}
// CHECK: # encoding: [0x62,0xe5,0x64,0x52,0x58,0x1c,0x25,0xf0,0x1c,0xf0,0x1c]
          vaddph zmm19 {k2}, zmm19, word ptr [485498096]{1to32}

// CHECK: vaddph zmm19 {k2}, zmm19, zmmword ptr [rdx]
// CHECK: # encoding: [0x62,0xe5,0x64,0x42,0x58,0x1a]
          vaddph zmm19 {k2}, zmm19, zmmword ptr [rdx]

// CHECK: vaddph zmm19 {k2}, zmm19, word ptr [rdx]{1to32}
// CHECK: # encoding: [0x62,0xe5,0x64,0x52,0x58,0x1a]
          vaddph zmm19 {k2}, zmm19, word ptr [rdx]{1to32}

// CHECK: vaddph zmm19 {k2}, zmm19, zmmword ptr [rdx + 4096]
// CHECK: # encoding: [0x62,0xe5,0x64,0x42,0x58,0x5a,0x40]
          vaddph zmm19 {k2}, zmm19, zmmword ptr [rdx + 4096]

// CHECK: vaddph zmm19 {k2}, zmm19, word ptr [rdx + 128]{1to32}
// CHECK: # encoding: [0x62,0xe5,0x64,0x52,0x58,0x5a,0x40]
          vaddph zmm19 {k2}, zmm19, word ptr [rdx + 128]{1to32}

// CHECK: vaddph zmm19 {k2}, zmm19, zmmword ptr [rdx + rax + 4096]
// CHECK: # encoding: [0x62,0xe5,0x64,0x42,0x58,0x5c,0x02,0x40]
          vaddph zmm19 {k2}, zmm19, zmmword ptr [rdx + rax + 4096]

// CHECK: vaddph zmm19 {k2}, zmm19, word ptr [rdx + rax + 128]{1to32}
// CHECK: # encoding: [0x62,0xe5,0x64,0x52,0x58,0x5c,0x02,0x40]
          vaddph zmm19 {k2}, zmm19, word ptr [rdx + rax + 128]{1to32}

// CHECK: vaddph zmm19 {k2}, zmm19, zmmword ptr [rdx + 4*rax + 4096]
// CHECK: # encoding: [0x62,0xe5,0x64,0x42,0x58,0x5c,0x82,0x40]
          vaddph zmm19 {k2}, zmm19, zmmword ptr [rdx + 4*rax + 4096]

// CHECK: vaddph zmm19 {k2}, zmm19, word ptr [rdx + 4*rax + 128]{1to32}
// CHECK: # encoding: [0x62,0xe5,0x64,0x52,0x58,0x5c,0x82,0x40]
          vaddph zmm19 {k2}, zmm19, word ptr [rdx + 4*rax + 128]{1to32}

// CHECK: vaddph zmm19 {k2}, zmm19, zmmword ptr [rdx + 4*rax - 4096]
// CHECK: # encoding: [0x62,0xe5,0x64,0x42,0x58,0x5c,0x82,0xc0]
          vaddph zmm19 {k2}, zmm19, zmmword ptr [rdx + 4*rax - 4096]

// CHECK: vaddph zmm19 {k2}, zmm19, word ptr [rdx + 4*rax - 128]{1to32}
// CHECK: # encoding: [0x62,0xe5,0x64,0x52,0x58,0x5c,0x82,0xc0]
          vaddph zmm19 {k2}, zmm19, word ptr [rdx + 4*rax - 128]{1to32}

// CHECK: vaddph zmm19 {k2} {z}, zmm19, zmmword ptr [485498096]
// CHECK: # encoding: [0x62,0xe5,0x64,0xc2,0x58,0x1c,0x25,0xf0,0x1c,0xf0,0x1c]
          vaddph zmm19 {k2} {z}, zmm19, zmmword ptr [485498096]

// CHECK: vaddph zmm19 {k2} {z}, zmm19, word ptr [485498096]{1to32}
// CHECK: # encoding: [0x62,0xe5,0x64,0xd2,0x58,0x1c,0x25,0xf0,0x1c,0xf0,0x1c]
          vaddph zmm19 {k2} {z}, zmm19, word ptr [485498096]{1to32}

// CHECK: vaddph zmm19 {k2} {z}, zmm19, zmmword ptr [rdx]
// CHECK: # encoding: [0x62,0xe5,0x64,0xc2,0x58,0x1a]
          vaddph zmm19 {k2} {z}, zmm19, zmmword ptr [rdx]

// CHECK: vaddph zmm19 {k2} {z}, zmm19, word ptr [rdx]{1to32}
// CHECK: # encoding: [0x62,0xe5,0x64,0xd2,0x58,0x1a]
          vaddph zmm19 {k2} {z}, zmm19, word ptr [rdx]{1to32}

// CHECK: vaddph zmm19 {k2} {z}, zmm19, zmmword ptr [rdx + 4096]
// CHECK: # encoding: [0x62,0xe5,0x64,0xc2,0x58,0x5a,0x40]
          vaddph zmm19 {k2} {z}, zmm19, zmmword ptr [rdx + 4096]

// CHECK: vaddph zmm19 {k2} {z}, zmm19, word ptr [rdx + 128]{1to32}
// CHECK: # encoding: [0x62,0xe5,0x64,0xd2,0x58,0x5a,0x40]
          vaddph zmm19 {k2} {z}, zmm19, word ptr [rdx + 128]{1to32}

// CHECK: vaddph zmm19 {k2} {z}, zmm19, zmmword ptr [rdx + rax + 4096]
// CHECK: # encoding: [0x62,0xe5,0x64,0xc2,0x58,0x5c,0x02,0x40]
          vaddph zmm19 {k2} {z}, zmm19, zmmword ptr [rdx + rax + 4096]

// CHECK: vaddph zmm19 {k2} {z}, zmm19, word ptr [rdx + rax + 128]{1to32}
// CHECK: # encoding: [0x62,0xe5,0x64,0xd2,0x58,0x5c,0x02,0x40]
          vaddph zmm19 {k2} {z}, zmm19, word ptr [rdx + rax + 128]{1to32}

// CHECK: vaddph zmm19 {k2} {z}, zmm19, zmmword ptr [rdx + 4*rax + 4096]
// CHECK: # encoding: [0x62,0xe5,0x64,0xc2,0x58,0x5c,0x82,0x40]
          vaddph zmm19 {k2} {z}, zmm19, zmmword ptr [rdx + 4*rax + 4096]

// CHECK: vaddph zmm19 {k2} {z}, zmm19, word ptr [rdx + 4*rax + 128]{1to32}
// CHECK: # encoding: [0x62,0xe5,0x64,0xd2,0x58,0x5c,0x82,0x40]
          vaddph zmm19 {k2} {z}, zmm19, word ptr [rdx + 4*rax + 128]{1to32}

// CHECK: vaddph zmm19 {k2} {z}, zmm19, zmmword ptr [rdx + 4*rax - 4096]
// CHECK: # encoding: [0x62,0xe5,0x64,0xc2,0x58,0x5c,0x82,0xc0]
          vaddph zmm19 {k2} {z}, zmm19, zmmword ptr [rdx + 4*rax - 4096]

// CHECK: vaddph zmm19 {k2} {z}, zmm19, word ptr [rdx + 4*rax - 128]{1to32}
// CHECK: # encoding: [0x62,0xe5,0x64,0xd2,0x58,0x5c,0x82,0xc0]
          vaddph zmm19 {k2} {z}, zmm19, word ptr [rdx + 4*rax - 128]{1to32}

// CHECK: vaddsh xmm30, xmm29, xmm28
// CHECK: encoding: [0x62,0x05,0x16,0x00,0x58,0xf4]
          vaddsh xmm30, xmm29, xmm28

// CHECK: vaddsh xmm30, xmm29, xmm28, {rn-sae}
// CHECK: encoding: [0x62,0x05,0x16,0x10,0x58,0xf4]
          vaddsh xmm30, xmm29, xmm28, {rn-sae}

// CHECK: vaddsh xmm30 {k7}, xmm29, xmm28
// CHECK: encoding: [0x62,0x05,0x16,0x07,0x58,0xf4]
          vaddsh xmm30 {k7}, xmm29, xmm28

// CHECK: vaddsh xmm30 {k7} {z}, xmm29, xmm28, {rz-sae}
// CHECK: encoding: [0x62,0x05,0x16,0xf7,0x58,0xf4]
          vaddsh xmm30 {k7} {z}, xmm29, xmm28, {rz-sae}

// CHECK: vaddsh xmm30, xmm29, word ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0x25,0x16,0x00,0x58,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vaddsh xmm30, xmm29, word ptr [rbp + 8*r14 + 268435456]

// CHECK: vaddsh xmm30 {k7}, xmm29, word ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0x45,0x16,0x07,0x58,0xb4,0x80,0x23,0x01,0x00,0x00]
          vaddsh xmm30 {k7}, xmm29, word ptr [r8 + 4*rax + 291]

// CHECK: vaddsh xmm30, xmm29, word ptr [rip]
// CHECK: encoding: [0x62,0x65,0x16,0x00,0x58,0x35,0x00,0x00,0x00,0x00]
          vaddsh xmm30, xmm29, word ptr [rip]

// CHECK: vaddsh xmm30, xmm29, word ptr [2*rbp - 64]
// CHECK: encoding: [0x62,0x65,0x16,0x00,0x58,0x34,0x6d,0xc0,0xff,0xff,0xff]
          vaddsh xmm30, xmm29, word ptr [2*rbp - 64]

// CHECK: vaddsh xmm30 {k7} {z}, xmm29, word ptr [rcx + 254]
// CHECK: encoding: [0x62,0x65,0x16,0x87,0x58,0x71,0x7f]
          vaddsh xmm30 {k7} {z}, xmm29, word ptr [rcx + 254]

// CHECK: vaddsh xmm30 {k7} {z}, xmm29, word ptr [rdx - 256]
// CHECK: encoding: [0x62,0x65,0x16,0x87,0x58,0x72,0x80]
          vaddsh xmm30 {k7} {z}, xmm29, word ptr [rdx - 256]

// CHECK: vcmpsh k5, xmm29, xmm28, 123
// CHECK: encoding: [0x62,0x93,0x16,0x00,0xc2,0xec,0x7b]
          vcmpsh k5, xmm29, xmm28, 123

// CHECK: vcmpsh k5, xmm29, xmm28, {sae}, 123
// CHECK: encoding: [0x62,0x93,0x16,0x10,0xc2,0xec,0x7b]
          vcmpsh k5, xmm29, xmm28, {sae}, 123

// CHECK: vcmpsh k5 {k7}, xmm29, xmm28, 123
// CHECK: encoding: [0x62,0x93,0x16,0x07,0xc2,0xec,0x7b]
          vcmpsh k5 {k7}, xmm29, xmm28, 123

// CHECK: vcmpsh k5 {k7}, xmm29, xmm28, {sae}, 123
// CHECK: encoding: [0x62,0x93,0x16,0x17,0xc2,0xec,0x7b]
          vcmpsh k5 {k7}, xmm29, xmm28, {sae}, 123

// CHECK: vcmpsh k5, xmm29, word ptr [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0x62,0xb3,0x16,0x00,0xc2,0xac,0xf5,0x00,0x00,0x00,0x10,0x7b]
          vcmpsh k5, xmm29, word ptr [rbp + 8*r14 + 268435456], 123

// CHECK: vcmpsh k5 {k7}, xmm29, word ptr [r8 + 4*rax + 291], 123
// CHECK: encoding: [0x62,0xd3,0x16,0x07,0xc2,0xac,0x80,0x23,0x01,0x00,0x00,0x7b]
          vcmpsh k5 {k7}, xmm29, word ptr [r8 + 4*rax + 291], 123

// CHECK: vcmpsh k5, xmm29, word ptr [rip], 123
// CHECK: encoding: [0x62,0xf3,0x16,0x00,0xc2,0x2d,0x00,0x00,0x00,0x00,0x7b]
          vcmpsh k5, xmm29, word ptr [rip], 123

// CHECK: vcmpsh k5, xmm29, word ptr [2*rbp - 64], 123
// CHECK: encoding: [0x62,0xf3,0x16,0x00,0xc2,0x2c,0x6d,0xc0,0xff,0xff,0xff,0x7b]
          vcmpsh k5, xmm29, word ptr [2*rbp - 64], 123

// CHECK: vcmpsh k5 {k7}, xmm29, word ptr [rcx + 254], 123
// CHECK: encoding: [0x62,0xf3,0x16,0x07,0xc2,0x69,0x7f,0x7b]
          vcmpsh k5 {k7}, xmm29, word ptr [rcx + 254], 123

// CHECK: vcmpsh k5 {k7}, xmm29, word ptr [rdx - 256], 123
// CHECK: encoding: [0x62,0xf3,0x16,0x07,0xc2,0x6a,0x80,0x7b]
          vcmpsh k5 {k7}, xmm29, word ptr [rdx - 256], 123

// CHECK: vdivsh xmm30, xmm29, xmm28
// CHECK: encoding: [0x62,0x05,0x16,0x00,0x5e,0xf4]
          vdivsh xmm30, xmm29, xmm28

// CHECK: vdivsh xmm30, xmm29, xmm28, {rn-sae}
// CHECK: encoding: [0x62,0x05,0x16,0x10,0x5e,0xf4]
          vdivsh xmm30, xmm29, xmm28, {rn-sae}

// CHECK: vdivsh xmm30 {k7}, xmm29, xmm28
// CHECK: encoding: [0x62,0x05,0x16,0x07,0x5e,0xf4]
          vdivsh xmm30 {k7}, xmm29, xmm28

// CHECK: vdivsh xmm30 {k7} {z}, xmm29, xmm28, {rz-sae}
// CHECK: encoding: [0x62,0x05,0x16,0xf7,0x5e,0xf4]
          vdivsh xmm30 {k7} {z}, xmm29, xmm28, {rz-sae}

// CHECK: vdivsh xmm30, xmm29, word ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0x25,0x16,0x00,0x5e,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vdivsh xmm30, xmm29, word ptr [rbp + 8*r14 + 268435456]

// CHECK: vdivsh xmm30 {k7}, xmm29, word ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0x45,0x16,0x07,0x5e,0xb4,0x80,0x23,0x01,0x00,0x00]
          vdivsh xmm30 {k7}, xmm29, word ptr [r8 + 4*rax + 291]

// CHECK: vdivsh xmm30, xmm29, word ptr [rip]
// CHECK: encoding: [0x62,0x65,0x16,0x00,0x5e,0x35,0x00,0x00,0x00,0x00]
          vdivsh xmm30, xmm29, word ptr [rip]

// CHECK: vdivsh xmm30, xmm29, word ptr [2*rbp - 64]
// CHECK: encoding: [0x62,0x65,0x16,0x00,0x5e,0x34,0x6d,0xc0,0xff,0xff,0xff]
          vdivsh xmm30, xmm29, word ptr [2*rbp - 64]

// CHECK: vdivsh xmm30 {k7} {z}, xmm29, word ptr [rcx + 254]
// CHECK: encoding: [0x62,0x65,0x16,0x87,0x5e,0x71,0x7f]
          vdivsh xmm30 {k7} {z}, xmm29, word ptr [rcx + 254]

// CHECK: vdivsh xmm30 {k7} {z}, xmm29, word ptr [rdx - 256]
// CHECK: encoding: [0x62,0x65,0x16,0x87,0x5e,0x72,0x80]
          vdivsh xmm30 {k7} {z}, xmm29, word ptr [rdx - 256]

// CHECK: vmaxsh xmm30, xmm29, xmm28
// CHECK: encoding: [0x62,0x05,0x16,0x00,0x5f,0xf4]
          vmaxsh xmm30, xmm29, xmm28

// CHECK: vmaxsh xmm30, xmm29, xmm28, {sae}
// CHECK: encoding: [0x62,0x05,0x16,0x10,0x5f,0xf4]
          vmaxsh xmm30, xmm29, xmm28, {sae}

// CHECK: vmaxsh xmm30 {k7}, xmm29, xmm28
// CHECK: encoding: [0x62,0x05,0x16,0x07,0x5f,0xf4]
          vmaxsh xmm30 {k7}, xmm29, xmm28

// CHECK: vmaxsh xmm30 {k7} {z}, xmm29, xmm28, {sae}
// CHECK: encoding: [0x62,0x05,0x16,0x97,0x5f,0xf4]
          vmaxsh xmm30 {k7} {z}, xmm29, xmm28, {sae}

// CHECK: vmaxsh xmm30, xmm29, word ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0x25,0x16,0x00,0x5f,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vmaxsh xmm30, xmm29, word ptr [rbp + 8*r14 + 268435456]

// CHECK: vmaxsh xmm30 {k7}, xmm29, word ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0x45,0x16,0x07,0x5f,0xb4,0x80,0x23,0x01,0x00,0x00]
          vmaxsh xmm30 {k7}, xmm29, word ptr [r8 + 4*rax + 291]

// CHECK: vmaxsh xmm30, xmm29, word ptr [rip]
// CHECK: encoding: [0x62,0x65,0x16,0x00,0x5f,0x35,0x00,0x00,0x00,0x00]
          vmaxsh xmm30, xmm29, word ptr [rip]

// CHECK: vmaxsh xmm30, xmm29, word ptr [2*rbp - 64]
// CHECK: encoding: [0x62,0x65,0x16,0x00,0x5f,0x34,0x6d,0xc0,0xff,0xff,0xff]
          vmaxsh xmm30, xmm29, word ptr [2*rbp - 64]

// CHECK: vmaxsh xmm30 {k7} {z}, xmm29, word ptr [rcx + 254]
// CHECK: encoding: [0x62,0x65,0x16,0x87,0x5f,0x71,0x7f]
          vmaxsh xmm30 {k7} {z}, xmm29, word ptr [rcx + 254]

// CHECK: vmaxsh xmm30 {k7} {z}, xmm29, word ptr [rdx - 256]
// CHECK: encoding: [0x62,0x65,0x16,0x87,0x5f,0x72,0x80]
          vmaxsh xmm30 {k7} {z}, xmm29, word ptr [rdx - 256]

// CHECK: vminsh xmm30, xmm29, xmm28
// CHECK: encoding: [0x62,0x05,0x16,0x00,0x5d,0xf4]
          vminsh xmm30, xmm29, xmm28

// CHECK: vminsh xmm30, xmm29, xmm28, {sae}
// CHECK: encoding: [0x62,0x05,0x16,0x10,0x5d,0xf4]
          vminsh xmm30, xmm29, xmm28, {sae}

// CHECK: vminsh xmm30 {k7}, xmm29, xmm28
// CHECK: encoding: [0x62,0x05,0x16,0x07,0x5d,0xf4]
          vminsh xmm30 {k7}, xmm29, xmm28

// CHECK: vminsh xmm30 {k7} {z}, xmm29, xmm28, {sae}
// CHECK: encoding: [0x62,0x05,0x16,0x97,0x5d,0xf4]
          vminsh xmm30 {k7} {z}, xmm29, xmm28, {sae}

// CHECK: vminsh xmm30, xmm29, word ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0x25,0x16,0x00,0x5d,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vminsh xmm30, xmm29, word ptr [rbp + 8*r14 + 268435456]

// CHECK: vminsh xmm30 {k7}, xmm29, word ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0x45,0x16,0x07,0x5d,0xb4,0x80,0x23,0x01,0x00,0x00]
          vminsh xmm30 {k7}, xmm29, word ptr [r8 + 4*rax + 291]

// CHECK: vminsh xmm30, xmm29, word ptr [rip]
// CHECK: encoding: [0x62,0x65,0x16,0x00,0x5d,0x35,0x00,0x00,0x00,0x00]
          vminsh xmm30, xmm29, word ptr [rip]

// CHECK: vminsh xmm30, xmm29, word ptr [2*rbp - 64]
// CHECK: encoding: [0x62,0x65,0x16,0x00,0x5d,0x34,0x6d,0xc0,0xff,0xff,0xff]
          vminsh xmm30, xmm29, word ptr [2*rbp - 64]

// CHECK: vminsh xmm30 {k7} {z}, xmm29, word ptr [rcx + 254]
// CHECK: encoding: [0x62,0x65,0x16,0x87,0x5d,0x71,0x7f]
          vminsh xmm30 {k7} {z}, xmm29, word ptr [rcx + 254]

// CHECK: vminsh xmm30 {k7} {z}, xmm29, word ptr [rdx - 256]
// CHECK: encoding: [0x62,0x65,0x16,0x87,0x5d,0x72,0x80]
          vminsh xmm30 {k7} {z}, xmm29, word ptr [rdx - 256]

// CHECK: vmulsh xmm30, xmm29, xmm28
// CHECK: encoding: [0x62,0x05,0x16,0x00,0x59,0xf4]
          vmulsh xmm30, xmm29, xmm28

// CHECK: vmulsh xmm30, xmm29, xmm28, {rn-sae}
// CHECK: encoding: [0x62,0x05,0x16,0x10,0x59,0xf4]
          vmulsh xmm30, xmm29, xmm28, {rn-sae}

// CHECK: vmulsh xmm30 {k7}, xmm29, xmm28
// CHECK: encoding: [0x62,0x05,0x16,0x07,0x59,0xf4]
          vmulsh xmm30 {k7}, xmm29, xmm28

// CHECK: vmulsh xmm30 {k7} {z}, xmm29, xmm28, {rz-sae}
// CHECK: encoding: [0x62,0x05,0x16,0xf7,0x59,0xf4]
          vmulsh xmm30 {k7} {z}, xmm29, xmm28, {rz-sae}

// CHECK: vmulsh xmm30, xmm29, word ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0x25,0x16,0x00,0x59,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vmulsh xmm30, xmm29, word ptr [rbp + 8*r14 + 268435456]

// CHECK: vmulsh xmm30 {k7}, xmm29, word ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0x45,0x16,0x07,0x59,0xb4,0x80,0x23,0x01,0x00,0x00]
          vmulsh xmm30 {k7}, xmm29, word ptr [r8 + 4*rax + 291]

// CHECK: vmulsh xmm30, xmm29, word ptr [rip]
// CHECK: encoding: [0x62,0x65,0x16,0x00,0x59,0x35,0x00,0x00,0x00,0x00]
          vmulsh xmm30, xmm29, word ptr [rip]

// CHECK: vmulsh xmm30, xmm29, word ptr [2*rbp - 64]
// CHECK: encoding: [0x62,0x65,0x16,0x00,0x59,0x34,0x6d,0xc0,0xff,0xff,0xff]
          vmulsh xmm30, xmm29, word ptr [2*rbp - 64]

// CHECK: vmulsh xmm30 {k7} {z}, xmm29, word ptr [rcx + 254]
// CHECK: encoding: [0x62,0x65,0x16,0x87,0x59,0x71,0x7f]
          vmulsh xmm30 {k7} {z}, xmm29, word ptr [rcx + 254]

// CHECK: vmulsh xmm30 {k7} {z}, xmm29, word ptr [rdx - 256]
// CHECK: encoding: [0x62,0x65,0x16,0x87,0x59,0x72,0x80]
          vmulsh xmm30 {k7} {z}, xmm29, word ptr [rdx - 256]

// CHECK: vsubsh xmm30, xmm29, xmm28
// CHECK: encoding: [0x62,0x05,0x16,0x00,0x5c,0xf4]
          vsubsh xmm30, xmm29, xmm28

// CHECK: vsubsh xmm30, xmm29, xmm28, {rn-sae}
// CHECK: encoding: [0x62,0x05,0x16,0x10,0x5c,0xf4]
          vsubsh xmm30, xmm29, xmm28, {rn-sae}

// CHECK: vsubsh xmm30 {k7}, xmm29, xmm28
// CHECK: encoding: [0x62,0x05,0x16,0x07,0x5c,0xf4]
          vsubsh xmm30 {k7}, xmm29, xmm28

// CHECK: vsubsh xmm30 {k7} {z}, xmm29, xmm28, {rz-sae}
// CHECK: encoding: [0x62,0x05,0x16,0xf7,0x5c,0xf4]
          vsubsh xmm30 {k7} {z}, xmm29, xmm28, {rz-sae}

// CHECK: vsubsh xmm30, xmm29, word ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0x25,0x16,0x00,0x5c,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vsubsh xmm30, xmm29, word ptr [rbp + 8*r14 + 268435456]

// CHECK: vsubsh xmm30 {k7}, xmm29, word ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0x45,0x16,0x07,0x5c,0xb4,0x80,0x23,0x01,0x00,0x00]
          vsubsh xmm30 {k7}, xmm29, word ptr [r8 + 4*rax + 291]

// CHECK: vsubsh xmm30, xmm29, word ptr [rip]
// CHECK: encoding: [0x62,0x65,0x16,0x00,0x5c,0x35,0x00,0x00,0x00,0x00]
          vsubsh xmm30, xmm29, word ptr [rip]

// CHECK: vsubsh xmm30, xmm29, word ptr [2*rbp - 64]
// CHECK: encoding: [0x62,0x65,0x16,0x00,0x5c,0x34,0x6d,0xc0,0xff,0xff,0xff]
          vsubsh xmm30, xmm29, word ptr [2*rbp - 64]

// CHECK: vsubsh xmm30 {k7} {z}, xmm29, word ptr [rcx + 254]
// CHECK: encoding: [0x62,0x65,0x16,0x87,0x5c,0x71,0x7f]
          vsubsh xmm30 {k7} {z}, xmm29, word ptr [rcx + 254]

// CHECK: vsubsh xmm30 {k7} {z}, xmm29, word ptr [rdx - 256]
// CHECK: encoding: [0x62,0x65,0x16,0x87,0x5c,0x72,0x80]
          vsubsh xmm30 {k7} {z}, xmm29, word ptr [rdx - 256]

// CHECK: vcmpph k5, zmm29, zmm28, 123
// CHECK: encoding: [0x62,0x93,0x14,0x40,0xc2,0xec,0x7b]
          vcmpph k5, zmm29, zmm28, 123

// CHECK: vcmpph k5, zmm29, zmm28, {sae}, 123
// CHECK: encoding: [0x62,0x93,0x14,0x10,0xc2,0xec,0x7b]
          vcmpph k5, zmm29, zmm28, {sae}, 123

// CHECK: vcmpph k5 {k7}, zmm29, zmm28, 123
// CHECK: encoding: [0x62,0x93,0x14,0x47,0xc2,0xec,0x7b]
          vcmpph k5 {k7}, zmm29, zmm28, 123

// CHECK: vcmpph k5 {k7}, zmm29, zmm28, {sae}, 123
// CHECK: encoding: [0x62,0x93,0x14,0x17,0xc2,0xec,0x7b]
          vcmpph k5 {k7}, zmm29, zmm28, {sae}, 123

// CHECK: vcmpph k5, zmm29, zmmword ptr [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0x62,0xb3,0x14,0x40,0xc2,0xac,0xf5,0x00,0x00,0x00,0x10,0x7b]
          vcmpph k5, zmm29, zmmword ptr [rbp + 8*r14 + 268435456], 123

// CHECK: vcmpph k5 {k7}, zmm29, zmmword ptr [r8 + 4*rax + 291], 123
// CHECK: encoding: [0x62,0xd3,0x14,0x47,0xc2,0xac,0x80,0x23,0x01,0x00,0x00,0x7b]
          vcmpph k5 {k7}, zmm29, zmmword ptr [r8 + 4*rax + 291], 123

// CHECK: vcmpph k5, zmm29, word ptr [rip]{1to32}, 123
// CHECK: encoding: [0x62,0xf3,0x14,0x50,0xc2,0x2d,0x00,0x00,0x00,0x00,0x7b]
          vcmpph k5, zmm29, word ptr [rip]{1to32}, 123

// CHECK: vcmpph k5, zmm29, zmmword ptr [2*rbp - 2048], 123
// CHECK: encoding: [0x62,0xf3,0x14,0x40,0xc2,0x2c,0x6d,0x00,0xf8,0xff,0xff,0x7b]
          vcmpph k5, zmm29, zmmword ptr [2*rbp - 2048], 123

// CHECK: vcmpph k5 {k7}, zmm29, zmmword ptr [rcx + 8128], 123
// CHECK: encoding: [0x62,0xf3,0x14,0x47,0xc2,0x69,0x7f,0x7b]
          vcmpph k5 {k7}, zmm29, zmmword ptr [rcx + 8128], 123

// CHECK: vcmpph k5 {k7}, zmm29, word ptr [rdx - 256]{1to32}, 123
// CHECK: encoding: [0x62,0xf3,0x14,0x57,0xc2,0x6a,0x80,0x7b]
          vcmpph k5 {k7}, zmm29, word ptr [rdx - 256]{1to32}, 123

// CHECK: vdivph zmm30, zmm29, zmm28
// CHECK: encoding: [0x62,0x05,0x14,0x40,0x5e,0xf4]
          vdivph zmm30, zmm29, zmm28

// CHECK: vdivph zmm30, zmm29, zmm28, {rn-sae}
// CHECK: encoding: [0x62,0x05,0x14,0x10,0x5e,0xf4]
          vdivph zmm30, zmm29, zmm28, {rn-sae}

// CHECK: vdivph zmm30 {k7}, zmm29, zmm28
// CHECK: encoding: [0x62,0x05,0x14,0x47,0x5e,0xf4]
          vdivph zmm30 {k7}, zmm29, zmm28

// CHECK: vdivph zmm30 {k7} {z}, zmm29, zmm28, {rz-sae}
// CHECK: encoding: [0x62,0x05,0x14,0xf7,0x5e,0xf4]
          vdivph zmm30 {k7} {z}, zmm29, zmm28, {rz-sae}

// CHECK: vdivph zmm30, zmm29, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0x25,0x14,0x40,0x5e,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vdivph zmm30, zmm29, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vdivph zmm30 {k7}, zmm29, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0x45,0x14,0x47,0x5e,0xb4,0x80,0x23,0x01,0x00,0x00]
          vdivph zmm30 {k7}, zmm29, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vdivph zmm30, zmm29, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0x65,0x14,0x50,0x5e,0x35,0x00,0x00,0x00,0x00]
          vdivph zmm30, zmm29, word ptr [rip]{1to32}

// CHECK: vdivph zmm30, zmm29, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0x65,0x14,0x40,0x5e,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vdivph zmm30, zmm29, zmmword ptr [2*rbp - 2048]

// CHECK: vdivph zmm30 {k7} {z}, zmm29, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0x65,0x14,0xc7,0x5e,0x71,0x7f]
          vdivph zmm30 {k7} {z}, zmm29, zmmword ptr [rcx + 8128]

// CHECK: vdivph zmm30 {k7} {z}, zmm29, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0x65,0x14,0xd7,0x5e,0x72,0x80]
          vdivph zmm30 {k7} {z}, zmm29, word ptr [rdx - 256]{1to32}

// CHECK: vmaxph zmm30, zmm29, zmm28
// CHECK: encoding: [0x62,0x05,0x14,0x40,0x5f,0xf4]
          vmaxph zmm30, zmm29, zmm28

// CHECK: vmaxph zmm30, zmm29, zmm28, {sae}
// CHECK: encoding: [0x62,0x05,0x14,0x10,0x5f,0xf4]
          vmaxph zmm30, zmm29, zmm28, {sae}

// CHECK: vmaxph zmm30 {k7}, zmm29, zmm28
// CHECK: encoding: [0x62,0x05,0x14,0x47,0x5f,0xf4]
          vmaxph zmm30 {k7}, zmm29, zmm28

// CHECK: vmaxph zmm30 {k7} {z}, zmm29, zmm28, {sae}
// CHECK: encoding: [0x62,0x05,0x14,0x97,0x5f,0xf4]
          vmaxph zmm30 {k7} {z}, zmm29, zmm28, {sae}

// CHECK: vmaxph zmm30, zmm29, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0x25,0x14,0x40,0x5f,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vmaxph zmm30, zmm29, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vmaxph zmm30 {k7}, zmm29, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0x45,0x14,0x47,0x5f,0xb4,0x80,0x23,0x01,0x00,0x00]
          vmaxph zmm30 {k7}, zmm29, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vmaxph zmm30, zmm29, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0x65,0x14,0x50,0x5f,0x35,0x00,0x00,0x00,0x00]
          vmaxph zmm30, zmm29, word ptr [rip]{1to32}

// CHECK: vmaxph zmm30, zmm29, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0x65,0x14,0x40,0x5f,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vmaxph zmm30, zmm29, zmmword ptr [2*rbp - 2048]

// CHECK: vmaxph zmm30 {k7} {z}, zmm29, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0x65,0x14,0xc7,0x5f,0x71,0x7f]
          vmaxph zmm30 {k7} {z}, zmm29, zmmword ptr [rcx + 8128]

// CHECK: vmaxph zmm30 {k7} {z}, zmm29, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0x65,0x14,0xd7,0x5f,0x72,0x80]
          vmaxph zmm30 {k7} {z}, zmm29, word ptr [rdx - 256]{1to32}

// CHECK: vminph zmm30, zmm29, zmm28
// CHECK: encoding: [0x62,0x05,0x14,0x40,0x5d,0xf4]
          vminph zmm30, zmm29, zmm28

// CHECK: vminph zmm30, zmm29, zmm28, {sae}
// CHECK: encoding: [0x62,0x05,0x14,0x10,0x5d,0xf4]
          vminph zmm30, zmm29, zmm28, {sae}

// CHECK: vminph zmm30 {k7}, zmm29, zmm28
// CHECK: encoding: [0x62,0x05,0x14,0x47,0x5d,0xf4]
          vminph zmm30 {k7}, zmm29, zmm28

// CHECK: vminph zmm30 {k7} {z}, zmm29, zmm28, {sae}
// CHECK: encoding: [0x62,0x05,0x14,0x97,0x5d,0xf4]
          vminph zmm30 {k7} {z}, zmm29, zmm28, {sae}

// CHECK: vminph zmm30, zmm29, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0x25,0x14,0x40,0x5d,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vminph zmm30, zmm29, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vminph zmm30 {k7}, zmm29, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0x45,0x14,0x47,0x5d,0xb4,0x80,0x23,0x01,0x00,0x00]
          vminph zmm30 {k7}, zmm29, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vminph zmm30, zmm29, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0x65,0x14,0x50,0x5d,0x35,0x00,0x00,0x00,0x00]
          vminph zmm30, zmm29, word ptr [rip]{1to32}

// CHECK: vminph zmm30, zmm29, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0x65,0x14,0x40,0x5d,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vminph zmm30, zmm29, zmmword ptr [2*rbp - 2048]

// CHECK: vminph zmm30 {k7} {z}, zmm29, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0x65,0x14,0xc7,0x5d,0x71,0x7f]
          vminph zmm30 {k7} {z}, zmm29, zmmword ptr [rcx + 8128]

// CHECK: vminph zmm30 {k7} {z}, zmm29, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0x65,0x14,0xd7,0x5d,0x72,0x80]
          vminph zmm30 {k7} {z}, zmm29, word ptr [rdx - 256]{1to32}

// CHECK: vmulph zmm30, zmm29, zmm28
// CHECK: encoding: [0x62,0x05,0x14,0x40,0x59,0xf4]
          vmulph zmm30, zmm29, zmm28

// CHECK: vmulph zmm30, zmm29, zmm28, {rn-sae}
// CHECK: encoding: [0x62,0x05,0x14,0x10,0x59,0xf4]
          vmulph zmm30, zmm29, zmm28, {rn-sae}

// CHECK: vmulph zmm30 {k7}, zmm29, zmm28
// CHECK: encoding: [0x62,0x05,0x14,0x47,0x59,0xf4]
          vmulph zmm30 {k7}, zmm29, zmm28

// CHECK: vmulph zmm30 {k7} {z}, zmm29, zmm28, {rz-sae}
// CHECK: encoding: [0x62,0x05,0x14,0xf7,0x59,0xf4]
          vmulph zmm30 {k7} {z}, zmm29, zmm28, {rz-sae}

// CHECK: vmulph zmm30, zmm29, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0x25,0x14,0x40,0x59,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vmulph zmm30, zmm29, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vmulph zmm30 {k7}, zmm29, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0x45,0x14,0x47,0x59,0xb4,0x80,0x23,0x01,0x00,0x00]
          vmulph zmm30 {k7}, zmm29, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vmulph zmm30, zmm29, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0x65,0x14,0x50,0x59,0x35,0x00,0x00,0x00,0x00]
          vmulph zmm30, zmm29, word ptr [rip]{1to32}

// CHECK: vmulph zmm30, zmm29, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0x65,0x14,0x40,0x59,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vmulph zmm30, zmm29, zmmword ptr [2*rbp - 2048]

// CHECK: vmulph zmm30 {k7} {z}, zmm29, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0x65,0x14,0xc7,0x59,0x71,0x7f]
          vmulph zmm30 {k7} {z}, zmm29, zmmword ptr [rcx + 8128]

// CHECK: vmulph zmm30 {k7} {z}, zmm29, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0x65,0x14,0xd7,0x59,0x72,0x80]
          vmulph zmm30 {k7} {z}, zmm29, word ptr [rdx - 256]{1to32}

// CHECK: vsubph zmm30, zmm29, zmm28
// CHECK: encoding: [0x62,0x05,0x14,0x40,0x5c,0xf4]
          vsubph zmm30, zmm29, zmm28

// CHECK: vsubph zmm30, zmm29, zmm28, {rn-sae}
// CHECK: encoding: [0x62,0x05,0x14,0x10,0x5c,0xf4]
          vsubph zmm30, zmm29, zmm28, {rn-sae}

// CHECK: vsubph zmm30 {k7}, zmm29, zmm28
// CHECK: encoding: [0x62,0x05,0x14,0x47,0x5c,0xf4]
          vsubph zmm30 {k7}, zmm29, zmm28

// CHECK: vsubph zmm30 {k7} {z}, zmm29, zmm28, {rz-sae}
// CHECK: encoding: [0x62,0x05,0x14,0xf7,0x5c,0xf4]
          vsubph zmm30 {k7} {z}, zmm29, zmm28, {rz-sae}

// CHECK: vsubph zmm30, zmm29, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0x25,0x14,0x40,0x5c,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vsubph zmm30, zmm29, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vsubph zmm30 {k7}, zmm29, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0x45,0x14,0x47,0x5c,0xb4,0x80,0x23,0x01,0x00,0x00]
          vsubph zmm30 {k7}, zmm29, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vsubph zmm30, zmm29, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0x65,0x14,0x50,0x5c,0x35,0x00,0x00,0x00,0x00]
          vsubph zmm30, zmm29, word ptr [rip]{1to32}

// CHECK: vsubph zmm30, zmm29, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0x65,0x14,0x40,0x5c,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vsubph zmm30, zmm29, zmmword ptr [2*rbp - 2048]

// CHECK: vsubph zmm30 {k7} {z}, zmm29, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0x65,0x14,0xc7,0x5c,0x71,0x7f]
          vsubph zmm30 {k7} {z}, zmm29, zmmword ptr [rcx + 8128]

// CHECK: vsubph zmm30 {k7} {z}, zmm29, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0x65,0x14,0xd7,0x5c,0x72,0x80]
          vsubph zmm30 {k7} {z}, zmm29, word ptr [rdx - 256]{1to32}

// CHECK: vcomish xmm30, xmm29
// CHECK: encoding: [0x62,0x05,0x7c,0x08,0x2f,0xf5]
          vcomish xmm30, xmm29

// CHECK: vcomish xmm30, xmm29, {sae}
// CHECK: encoding: [0x62,0x05,0x7c,0x18,0x2f,0xf5]
          vcomish xmm30, xmm29, {sae}

// CHECK: vcomish xmm30, word ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0x25,0x7c,0x08,0x2f,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcomish xmm30, word ptr [rbp + 8*r14 + 268435456]

// CHECK: vcomish xmm30, word ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0x45,0x7c,0x08,0x2f,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcomish xmm30, word ptr [r8 + 4*rax + 291]

// CHECK: vcomish xmm30, word ptr [rip]
// CHECK: encoding: [0x62,0x65,0x7c,0x08,0x2f,0x35,0x00,0x00,0x00,0x00]
          vcomish xmm30, word ptr [rip]

// CHECK: vcomish xmm30, word ptr [2*rbp - 64]
// CHECK: encoding: [0x62,0x65,0x7c,0x08,0x2f,0x34,0x6d,0xc0,0xff,0xff,0xff]
          vcomish xmm30, word ptr [2*rbp - 64]

// CHECK: vcomish xmm30, word ptr [rcx + 254]
// CHECK: encoding: [0x62,0x65,0x7c,0x08,0x2f,0x71,0x7f]
          vcomish xmm30, word ptr [rcx + 254]

// CHECK: vcomish xmm30, word ptr [rdx - 256]
// CHECK: encoding: [0x62,0x65,0x7c,0x08,0x2f,0x72,0x80]
          vcomish xmm30, word ptr [rdx - 256]

// CHECK: vucomish xmm30, xmm29
// CHECK: encoding: [0x62,0x05,0x7c,0x08,0x2e,0xf5]
          vucomish xmm30, xmm29

// CHECK: vucomish xmm30, xmm29, {sae}
// CHECK: encoding: [0x62,0x05,0x7c,0x18,0x2e,0xf5]
          vucomish xmm30, xmm29, {sae}

// CHECK: vucomish xmm30, word ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0x25,0x7c,0x08,0x2e,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vucomish xmm30, word ptr [rbp + 8*r14 + 268435456]

// CHECK: vucomish xmm30, word ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0x45,0x7c,0x08,0x2e,0xb4,0x80,0x23,0x01,0x00,0x00]
          vucomish xmm30, word ptr [r8 + 4*rax + 291]

// CHECK: vucomish xmm30, word ptr [rip]
// CHECK: encoding: [0x62,0x65,0x7c,0x08,0x2e,0x35,0x00,0x00,0x00,0x00]
          vucomish xmm30, word ptr [rip]

// CHECK: vucomish xmm30, word ptr [2*rbp - 64]
// CHECK: encoding: [0x62,0x65,0x7c,0x08,0x2e,0x34,0x6d,0xc0,0xff,0xff,0xff]
          vucomish xmm30, word ptr [2*rbp - 64]

// CHECK: vucomish xmm30, word ptr [rcx + 254]
// CHECK: encoding: [0x62,0x65,0x7c,0x08,0x2e,0x71,0x7f]
          vucomish xmm30, word ptr [rcx + 254]

// CHECK: vucomish xmm30, word ptr [rdx - 256]
// CHECK: encoding: [0x62,0x65,0x7c,0x08,0x2e,0x72,0x80]
          vucomish xmm30, word ptr [rdx - 256]

// CHECK: vmovsh xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x85,0x46,0x00,0x10,0xf0]
          vmovsh xmm22, xmm23, xmm24

// CHECK: vmovsh xmm22 {k7}, xmm23, xmm24
// CHECK: encoding: [0x62,0x85,0x46,0x07,0x10,0xf0]
          vmovsh xmm22 {k7}, xmm23, xmm24

// CHECK: vmovsh xmm22 {k7} {z}, xmm23, xmm24
// CHECK: encoding: [0x62,0x85,0x46,0x87,0x10,0xf0]
          vmovsh xmm22 {k7} {z}, xmm23, xmm24

// CHECK: vmovsh xmm22, word ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7e,0x08,0x10,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vmovsh xmm22, word ptr [rbp + 8*r14 + 268435456]

// CHECK: vmovsh xmm22 {k7}, word ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7e,0x0f,0x10,0xb4,0x80,0x23,0x01,0x00,0x00]
          vmovsh xmm22 {k7}, word ptr [r8 + 4*rax + 291]

// CHECK: vmovsh xmm22, word ptr [rip]
// CHECK: encoding: [0x62,0xe5,0x7e,0x08,0x10,0x35,0x00,0x00,0x00,0x00]
          vmovsh xmm22, word ptr [rip]

// CHECK: vmovsh xmm22, word ptr [2*rbp - 64]
// CHECK: encoding: [0x62,0xe5,0x7e,0x08,0x10,0x34,0x6d,0xc0,0xff,0xff,0xff]
          vmovsh xmm22, word ptr [2*rbp - 64]

// CHECK: vmovsh xmm22 {k7} {z}, word ptr [rcx + 254]
// CHECK: encoding: [0x62,0xe5,0x7e,0x8f,0x10,0x71,0x7f]
          vmovsh xmm22 {k7} {z}, word ptr [rcx + 254]

// CHECK: vmovsh xmm22 {k7} {z}, word ptr [rdx - 256]
// CHECK: encoding: [0x62,0xe5,0x7e,0x8f,0x10,0x72,0x80]
          vmovsh xmm22 {k7} {z}, word ptr [rdx - 256]

// CHECK: vmovsh word ptr [rbp + 8*r14 + 268435456], xmm22
// CHECK: encoding: [0x62,0xa5,0x7e,0x08,0x11,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vmovsh word ptr [rbp + 8*r14 + 268435456], xmm22

// CHECK: vmovsh word ptr [r8 + 4*rax + 291] {k7}, xmm22
// CHECK: encoding: [0x62,0xc5,0x7e,0x0f,0x11,0xb4,0x80,0x23,0x01,0x00,0x00]
          vmovsh word ptr [r8 + 4*rax + 291] {k7}, xmm22

// CHECK: vmovsh word ptr [rip], xmm22
// CHECK: encoding: [0x62,0xe5,0x7e,0x08,0x11,0x35,0x00,0x00,0x00,0x00]
          vmovsh word ptr [rip], xmm22

// CHECK: vmovsh word ptr [2*rbp - 64], xmm22
// CHECK: encoding: [0x62,0xe5,0x7e,0x08,0x11,0x34,0x6d,0xc0,0xff,0xff,0xff]
          vmovsh word ptr [2*rbp - 64], xmm22

// CHECK: vmovsh word ptr [rcx + 254] {k7}, xmm22
// CHECK: encoding: [0x62,0xe5,0x7e,0x0f,0x11,0x71,0x7f]
          vmovsh word ptr [rcx + 254] {k7}, xmm22

// CHECK: vmovsh word ptr [rdx - 256] {k7}, xmm22
// CHECK: encoding: [0x62,0xe5,0x7e,0x0f,0x11,0x72,0x80]
          vmovsh word ptr [rdx - 256] {k7}, xmm22

// CHECK: vmovw xmm22, r12
// CHECK: encoding: [0x62,0xc5,0xfd,0x08,0x6e,0xf4]
          vmovw xmm22, r12

// CHECK: vmovw r12, xmm22
// CHECK: encoding: [0x62,0xc5,0xfd,0x08,0x7e,0xf4]
          vmovw r12, xmm22

// CHECK: vmovw xmm22, r12d
// CHECK: encoding: [0x62,0xc5,0x7d,0x08,0x6e,0xf4]
          vmovw xmm22, r12d

// CHECK: vmovw r12d, xmm22
// CHECK: encoding: [0x62,0xc5,0x7d,0x08,0x7e,0xf4]
          vmovw r12d, xmm22

// CHECK: vmovw xmm22, word ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7d,0x08,0x6e,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vmovw xmm22, word ptr [rbp + 8*r14 + 268435456]

// CHECK: vmovw xmm22, word ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7d,0x08,0x6e,0xb4,0x80,0x23,0x01,0x00,0x00]
          vmovw xmm22, word ptr [r8 + 4*rax + 291]

// CHECK: vmovw xmm22, word ptr [rip]
// CHECK: encoding: [0x62,0xe5,0x7d,0x08,0x6e,0x35,0x00,0x00,0x00,0x00]
          vmovw xmm22, word ptr [rip]

// CHECK: vmovw xmm22, word ptr [2*rbp - 64]
// CHECK: encoding: [0x62,0xe5,0x7d,0x08,0x6e,0x34,0x6d,0xc0,0xff,0xff,0xff]
          vmovw xmm22, word ptr [2*rbp - 64]

// CHECK: vmovw xmm22, word ptr [rcx + 254]
// CHECK: encoding: [0x62,0xe5,0x7d,0x08,0x6e,0x71,0x7f]
          vmovw xmm22, word ptr [rcx + 254]

// CHECK: vmovw xmm22, word ptr [rdx - 256]
// CHECK: encoding: [0x62,0xe5,0x7d,0x08,0x6e,0x72,0x80]
          vmovw xmm22, word ptr [rdx - 256]

// CHECK: vmovw word ptr [rbp + 8*r14 + 268435456], xmm22
// CHECK: encoding: [0x62,0xa5,0x7d,0x08,0x7e,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vmovw word ptr [rbp + 8*r14 + 268435456], xmm22

// CHECK: vmovw word ptr [r8 + 4*rax + 291], xmm22
// CHECK: encoding: [0x62,0xc5,0x7d,0x08,0x7e,0xb4,0x80,0x23,0x01,0x00,0x00]
          vmovw word ptr [r8 + 4*rax + 291], xmm22

// CHECK: vmovw word ptr [rip], xmm22
// CHECK: encoding: [0x62,0xe5,0x7d,0x08,0x7e,0x35,0x00,0x00,0x00,0x00]
          vmovw word ptr [rip], xmm22

// CHECK: vmovw word ptr [2*rbp - 64], xmm22
// CHECK: encoding: [0x62,0xe5,0x7d,0x08,0x7e,0x34,0x6d,0xc0,0xff,0xff,0xff]
          vmovw word ptr [2*rbp - 64], xmm22

// CHECK: vmovw word ptr [rcx + 254], xmm22
// CHECK: encoding: [0x62,0xe5,0x7d,0x08,0x7e,0x71,0x7f]
          vmovw word ptr [rcx + 254], xmm22

// CHECK: vmovw word ptr [rdx - 256], xmm22
// CHECK: encoding: [0x62,0xe5,0x7d,0x08,0x7e,0x72,0x80]
          vmovw word ptr [rdx - 256], xmm22

// CHECK: vcvtph2psx zmm22, ymm23
// CHECK: encoding: [0x62,0xa6,0x7d,0x48,0x13,0xf7]
          vcvtph2psx zmm22, ymm23

// CHECK: vcvtph2psx zmm22, ymm23, {sae}
// CHECK: encoding: [0x62,0xa6,0x7d,0x18,0x13,0xf7]
          vcvtph2psx zmm22, ymm23, {sae}

// CHECK: vcvtph2psx zmm22 {k7}, ymm23
// CHECK: encoding: [0x62,0xa6,0x7d,0x4f,0x13,0xf7]
          vcvtph2psx zmm22 {k7}, ymm23

// CHECK: vcvtph2psx zmm22 {k7} {z}, ymm23, {sae}
// CHECK: encoding: [0x62,0xa6,0x7d,0x9f,0x13,0xf7]
          vcvtph2psx zmm22 {k7} {z}, ymm23, {sae}

// CHECK: vcvtph2psx zmm22, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0x7d,0x48,0x13,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtph2psx zmm22, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtph2psx zmm22 {k7}, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0x7d,0x4f,0x13,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtph2psx zmm22 {k7}, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtph2psx zmm22, word ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe6,0x7d,0x58,0x13,0x35,0x00,0x00,0x00,0x00]
          vcvtph2psx zmm22, word ptr [rip]{1to16}

// CHECK: vcvtph2psx zmm22, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe6,0x7d,0x48,0x13,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vcvtph2psx zmm22, ymmword ptr [2*rbp - 1024]

// CHECK: vcvtph2psx zmm22 {k7} {z}, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe6,0x7d,0xcf,0x13,0x71,0x7f]
          vcvtph2psx zmm22 {k7} {z}, ymmword ptr [rcx + 4064]

// CHECK: vcvtph2psx zmm22 {k7} {z}, word ptr [rdx - 256]{1to16}
// CHECK: encoding: [0x62,0xe6,0x7d,0xdf,0x13,0x72,0x80]
          vcvtph2psx zmm22 {k7} {z}, word ptr [rdx - 256]{1to16}

// CHECK: vcvtps2phx ymm22, zmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x48,0x1d,0xf7]
          vcvtps2phx ymm22, zmm23

// CHECK: vcvtps2phx ymm22, zmm23, {rn-sae}
// CHECK: encoding: [0x62,0xa5,0x7d,0x18,0x1d,0xf7]
          vcvtps2phx ymm22, zmm23, {rn-sae}

// CHECK: vcvtps2phx ymm22 {k7}, zmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x4f,0x1d,0xf7]
          vcvtps2phx ymm22 {k7}, zmm23

// CHECK: vcvtps2phx ymm22 {k7} {z}, zmm23, {rz-sae}
// CHECK: encoding: [0x62,0xa5,0x7d,0xff,0x1d,0xf7]
          vcvtps2phx ymm22 {k7} {z}, zmm23, {rz-sae}

// CHECK: vcvtps2phx ymm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7d,0x48,0x1d,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtps2phx ymm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtps2phx ymm22 {k7}, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7d,0x4f,0x1d,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtps2phx ymm22 {k7}, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtps2phx ymm22, dword ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe5,0x7d,0x58,0x1d,0x35,0x00,0x00,0x00,0x00]
          vcvtps2phx ymm22, dword ptr [rip]{1to16}

// CHECK: vcvtps2phx ymm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0x7d,0x48,0x1d,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vcvtps2phx ymm22, zmmword ptr [2*rbp - 2048]

// CHECK: vcvtps2phx ymm22 {k7} {z}, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0x7d,0xcf,0x1d,0x71,0x7f]
          vcvtps2phx ymm22 {k7} {z}, zmmword ptr [rcx + 8128]

// CHECK: vcvtps2phx ymm22 {k7} {z}, dword ptr [rdx - 512]{1to16}
// CHECK: encoding: [0x62,0xe5,0x7d,0xdf,0x1d,0x72,0x80]
          vcvtps2phx ymm22 {k7} {z}, dword ptr [rdx - 512]{1to16}

// CHECK: vcvtpd2ph xmm22, zmm23
// CHECK: encoding: [0x62,0xa5,0xfd,0x48,0x5a,0xf7]
          vcvtpd2ph xmm22, zmm23

// CHECK: vcvtpd2ph xmm22, zmm23, {rn-sae}
// CHECK: encoding: [0x62,0xa5,0xfd,0x18,0x5a,0xf7]
          vcvtpd2ph xmm22, zmm23, {rn-sae}

// CHECK: vcvtpd2ph xmm22 {k7}, zmm23
// CHECK: encoding: [0x62,0xa5,0xfd,0x4f,0x5a,0xf7]
          vcvtpd2ph xmm22 {k7}, zmm23

// CHECK: vcvtpd2ph xmm22 {k7} {z}, zmm23, {rz-sae}
// CHECK: encoding: [0x62,0xa5,0xfd,0xff,0x5a,0xf7]
          vcvtpd2ph xmm22 {k7} {z}, zmm23, {rz-sae}

// CHECK: vcvtpd2ph xmm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0xfd,0x48,0x5a,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtpd2ph xmm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtpd2ph xmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0xfd,0x4f,0x5a,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtpd2ph xmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtpd2ph xmm22, qword ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe5,0xfd,0x58,0x5a,0x35,0x00,0x00,0x00,0x00]
          vcvtpd2ph xmm22, qword ptr [rip]{1to8}

// CHECK: vcvtpd2ph xmm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0xfd,0x48,0x5a,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vcvtpd2ph xmm22, zmmword ptr [2*rbp - 2048]

// CHECK: vcvtpd2ph xmm22 {k7} {z}, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0xfd,0xcf,0x5a,0x71,0x7f]
          vcvtpd2ph xmm22 {k7} {z}, zmmword ptr [rcx + 8128]

// CHECK: vcvtpd2ph xmm22 {k7} {z}, qword ptr [rdx - 1024]{1to8}
// CHECK: encoding: [0x62,0xe5,0xfd,0xdf,0x5a,0x72,0x80]
          vcvtpd2ph xmm22 {k7} {z}, qword ptr [rdx - 1024]{1to8}

// CHECK: vcvtph2pd zmm22, xmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x48,0x5a,0xf7]
          vcvtph2pd zmm22, xmm23

// CHECK: vcvtph2pd zmm22, xmm23, {sae}
// CHECK: encoding: [0x62,0xa5,0x7c,0x18,0x5a,0xf7]
          vcvtph2pd zmm22, xmm23, {sae}

// CHECK: vcvtph2pd zmm22 {k7}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x4f,0x5a,0xf7]
          vcvtph2pd zmm22 {k7}, xmm23

// CHECK: vcvtph2pd zmm22 {k7} {z}, xmm23, {sae}
// CHECK: encoding: [0x62,0xa5,0x7c,0x9f,0x5a,0xf7]
          vcvtph2pd zmm22 {k7} {z}, xmm23, {sae}

// CHECK: vcvtph2pd zmm22, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7c,0x48,0x5a,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtph2pd zmm22, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtph2pd zmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7c,0x4f,0x5a,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtph2pd zmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtph2pd zmm22, word ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe5,0x7c,0x58,0x5a,0x35,0x00,0x00,0x00,0x00]
          vcvtph2pd zmm22, word ptr [rip]{1to8}

// CHECK: vcvtph2pd zmm22, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe5,0x7c,0x48,0x5a,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvtph2pd zmm22, xmmword ptr [2*rbp - 512]

// CHECK: vcvtph2pd zmm22 {k7} {z}, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe5,0x7c,0xcf,0x5a,0x71,0x7f]
          vcvtph2pd zmm22 {k7} {z}, xmmword ptr [rcx + 2032]

// CHECK: vcvtph2pd zmm22 {k7} {z}, word ptr [rdx - 256]{1to8}
// CHECK: encoding: [0x62,0xe5,0x7c,0xdf,0x5a,0x72,0x80]
          vcvtph2pd zmm22 {k7} {z}, word ptr [rdx - 256]{1to8}

// CHECK: vcvtph2uw zmm22, zmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x48,0x7d,0xf7]
          vcvtph2uw zmm22, zmm23

// CHECK: vcvtph2uw zmm22, zmm23, {rn-sae}
// CHECK: encoding: [0x62,0xa5,0x7c,0x18,0x7d,0xf7]
          vcvtph2uw zmm22, zmm23, {rn-sae}

// CHECK: vcvtph2uw zmm22 {k7}, zmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x4f,0x7d,0xf7]
          vcvtph2uw zmm22 {k7}, zmm23

// CHECK: vcvtph2uw zmm22 {k7} {z}, zmm23, {rz-sae}
// CHECK: encoding: [0x62,0xa5,0x7c,0xff,0x7d,0xf7]
          vcvtph2uw zmm22 {k7} {z}, zmm23, {rz-sae}

// CHECK: vcvtph2uw zmm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7c,0x48,0x7d,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtph2uw zmm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtph2uw zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7c,0x4f,0x7d,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtph2uw zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtph2uw zmm22, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0xe5,0x7c,0x58,0x7d,0x35,0x00,0x00,0x00,0x00]
          vcvtph2uw zmm22, word ptr [rip]{1to32}

// CHECK: vcvtph2uw zmm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0x7c,0x48,0x7d,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vcvtph2uw zmm22, zmmword ptr [2*rbp - 2048]

// CHECK: vcvtph2uw zmm22 {k7} {z}, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0x7c,0xcf,0x7d,0x71,0x7f]
          vcvtph2uw zmm22 {k7} {z}, zmmword ptr [rcx + 8128]

// CHECK: vcvtph2uw zmm22 {k7} {z}, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0xe5,0x7c,0xdf,0x7d,0x72,0x80]
          vcvtph2uw zmm22 {k7} {z}, word ptr [rdx - 256]{1to32}

// CHECK: vcvtph2w zmm22, zmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x48,0x7d,0xf7]
          vcvtph2w zmm22, zmm23

// CHECK: vcvtph2w zmm22, zmm23, {rn-sae}
// CHECK: encoding: [0x62,0xa5,0x7d,0x18,0x7d,0xf7]
          vcvtph2w zmm22, zmm23, {rn-sae}

// CHECK: vcvtph2w zmm22 {k7}, zmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x4f,0x7d,0xf7]
          vcvtph2w zmm22 {k7}, zmm23

// CHECK: vcvtph2w zmm22 {k7} {z}, zmm23, {rz-sae}
// CHECK: encoding: [0x62,0xa5,0x7d,0xff,0x7d,0xf7]
          vcvtph2w zmm22 {k7} {z}, zmm23, {rz-sae}

// CHECK: vcvtph2w zmm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7d,0x48,0x7d,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtph2w zmm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtph2w zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7d,0x4f,0x7d,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtph2w zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtph2w zmm22, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0xe5,0x7d,0x58,0x7d,0x35,0x00,0x00,0x00,0x00]
          vcvtph2w zmm22, word ptr [rip]{1to32}

// CHECK: vcvtph2w zmm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0x7d,0x48,0x7d,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vcvtph2w zmm22, zmmword ptr [2*rbp - 2048]

// CHECK: vcvtph2w zmm22 {k7} {z}, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0x7d,0xcf,0x7d,0x71,0x7f]
          vcvtph2w zmm22 {k7} {z}, zmmword ptr [rcx + 8128]

// CHECK: vcvtph2w zmm22 {k7} {z}, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0xe5,0x7d,0xdf,0x7d,0x72,0x80]
          vcvtph2w zmm22 {k7} {z}, word ptr [rdx - 256]{1to32}

// CHECK: vcvttph2uw zmm22, zmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x48,0x7c,0xf7]
          vcvttph2uw zmm22, zmm23

// CHECK: vcvttph2uw zmm22, zmm23, {sae}
// CHECK: encoding: [0x62,0xa5,0x7c,0x18,0x7c,0xf7]
          vcvttph2uw zmm22, zmm23, {sae}

// CHECK: vcvttph2uw zmm22 {k7}, zmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x4f,0x7c,0xf7]
          vcvttph2uw zmm22 {k7}, zmm23

// CHECK: vcvttph2uw zmm22 {k7} {z}, zmm23, {sae}
// CHECK: encoding: [0x62,0xa5,0x7c,0x9f,0x7c,0xf7]
          vcvttph2uw zmm22 {k7} {z}, zmm23, {sae}

// CHECK: vcvttph2uw zmm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7c,0x48,0x7c,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvttph2uw zmm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvttph2uw zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7c,0x4f,0x7c,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvttph2uw zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvttph2uw zmm22, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0xe5,0x7c,0x58,0x7c,0x35,0x00,0x00,0x00,0x00]
          vcvttph2uw zmm22, word ptr [rip]{1to32}

// CHECK: vcvttph2uw zmm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0x7c,0x48,0x7c,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vcvttph2uw zmm22, zmmword ptr [2*rbp - 2048]

// CHECK: vcvttph2uw zmm22 {k7} {z}, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0x7c,0xcf,0x7c,0x71,0x7f]
          vcvttph2uw zmm22 {k7} {z}, zmmword ptr [rcx + 8128]

// CHECK: vcvttph2uw zmm22 {k7} {z}, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0xe5,0x7c,0xdf,0x7c,0x72,0x80]
          vcvttph2uw zmm22 {k7} {z}, word ptr [rdx - 256]{1to32}

// CHECK: vcvttph2w zmm22, zmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x48,0x7c,0xf7]
          vcvttph2w zmm22, zmm23

// CHECK: vcvttph2w zmm22, zmm23, {sae}
// CHECK: encoding: [0x62,0xa5,0x7d,0x18,0x7c,0xf7]
          vcvttph2w zmm22, zmm23, {sae}

// CHECK: vcvttph2w zmm22 {k7}, zmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x4f,0x7c,0xf7]
          vcvttph2w zmm22 {k7}, zmm23

// CHECK: vcvttph2w zmm22 {k7} {z}, zmm23, {sae}
// CHECK: encoding: [0x62,0xa5,0x7d,0x9f,0x7c,0xf7]
          vcvttph2w zmm22 {k7} {z}, zmm23, {sae}

// CHECK: vcvttph2w zmm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7d,0x48,0x7c,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvttph2w zmm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvttph2w zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7d,0x4f,0x7c,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvttph2w zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvttph2w zmm22, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0xe5,0x7d,0x58,0x7c,0x35,0x00,0x00,0x00,0x00]
          vcvttph2w zmm22, word ptr [rip]{1to32}

// CHECK: vcvttph2w zmm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0x7d,0x48,0x7c,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vcvttph2w zmm22, zmmword ptr [2*rbp - 2048]

// CHECK: vcvttph2w zmm22 {k7} {z}, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0x7d,0xcf,0x7c,0x71,0x7f]
          vcvttph2w zmm22 {k7} {z}, zmmword ptr [rcx + 8128]

// CHECK: vcvttph2w zmm22 {k7} {z}, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0xe5,0x7d,0xdf,0x7c,0x72,0x80]
          vcvttph2w zmm22 {k7} {z}, word ptr [rdx - 256]{1to32}

// CHECK: vcvtuw2ph zmm22, zmm23
// CHECK: encoding: [0x62,0xa5,0x7f,0x48,0x7d,0xf7]
          vcvtuw2ph zmm22, zmm23

// CHECK: vcvtuw2ph zmm22, zmm23, {rn-sae}
// CHECK: encoding: [0x62,0xa5,0x7f,0x18,0x7d,0xf7]
          vcvtuw2ph zmm22, zmm23, {rn-sae}

// CHECK: vcvtuw2ph zmm22 {k7}, zmm23
// CHECK: encoding: [0x62,0xa5,0x7f,0x4f,0x7d,0xf7]
          vcvtuw2ph zmm22 {k7}, zmm23

// CHECK: vcvtuw2ph zmm22 {k7} {z}, zmm23, {rz-sae}
// CHECK: encoding: [0x62,0xa5,0x7f,0xff,0x7d,0xf7]
          vcvtuw2ph zmm22 {k7} {z}, zmm23, {rz-sae}

// CHECK: vcvtuw2ph zmm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7f,0x48,0x7d,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtuw2ph zmm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtuw2ph zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7f,0x4f,0x7d,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtuw2ph zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtuw2ph zmm22, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0xe5,0x7f,0x58,0x7d,0x35,0x00,0x00,0x00,0x00]
          vcvtuw2ph zmm22, word ptr [rip]{1to32}

// CHECK: vcvtuw2ph zmm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0x7f,0x48,0x7d,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vcvtuw2ph zmm22, zmmword ptr [2*rbp - 2048]

// CHECK: vcvtuw2ph zmm22 {k7} {z}, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0x7f,0xcf,0x7d,0x71,0x7f]
          vcvtuw2ph zmm22 {k7} {z}, zmmword ptr [rcx + 8128]

// CHECK: vcvtuw2ph zmm22 {k7} {z}, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0xe5,0x7f,0xdf,0x7d,0x72,0x80]
          vcvtuw2ph zmm22 {k7} {z}, word ptr [rdx - 256]{1to32}

// CHECK: vcvtw2ph zmm22, zmm23
// CHECK: encoding: [0x62,0xa5,0x7e,0x48,0x7d,0xf7]
          vcvtw2ph zmm22, zmm23

// CHECK: vcvtw2ph zmm22, zmm23, {rn-sae}
// CHECK: encoding: [0x62,0xa5,0x7e,0x18,0x7d,0xf7]
          vcvtw2ph zmm22, zmm23, {rn-sae}

// CHECK: vcvtw2ph zmm22 {k7}, zmm23
// CHECK: encoding: [0x62,0xa5,0x7e,0x4f,0x7d,0xf7]
          vcvtw2ph zmm22 {k7}, zmm23

// CHECK: vcvtw2ph zmm22 {k7} {z}, zmm23, {rz-sae}
// CHECK: encoding: [0x62,0xa5,0x7e,0xff,0x7d,0xf7]
          vcvtw2ph zmm22 {k7} {z}, zmm23, {rz-sae}

// CHECK: vcvtw2ph zmm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7e,0x48,0x7d,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtw2ph zmm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtw2ph zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7e,0x4f,0x7d,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtw2ph zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtw2ph zmm22, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0xe5,0x7e,0x58,0x7d,0x35,0x00,0x00,0x00,0x00]
          vcvtw2ph zmm22, word ptr [rip]{1to32}

// CHECK: vcvtw2ph zmm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0x7e,0x48,0x7d,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vcvtw2ph zmm22, zmmword ptr [2*rbp - 2048]

// CHECK: vcvtw2ph zmm22 {k7} {z}, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0x7e,0xcf,0x7d,0x71,0x7f]
          vcvtw2ph zmm22 {k7} {z}, zmmword ptr [rcx + 8128]

// CHECK: vcvtw2ph zmm22 {k7} {z}, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0xe5,0x7e,0xdf,0x7d,0x72,0x80]
          vcvtw2ph zmm22 {k7} {z}, word ptr [rdx - 256]{1to32}

// CHECK: vcvtdq2ph ymm22, zmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x48,0x5b,0xf7]
          vcvtdq2ph ymm22, zmm23

// CHECK: vcvtdq2ph ymm22, zmm23, {rn-sae}
// CHECK: encoding: [0x62,0xa5,0x7c,0x18,0x5b,0xf7]
          vcvtdq2ph ymm22, zmm23, {rn-sae}

// CHECK: vcvtdq2ph ymm22 {k7}, zmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x4f,0x5b,0xf7]
          vcvtdq2ph ymm22 {k7}, zmm23

// CHECK: vcvtdq2ph ymm22 {k7} {z}, zmm23, {rz-sae}
// CHECK: encoding: [0x62,0xa5,0x7c,0xff,0x5b,0xf7]
          vcvtdq2ph ymm22 {k7} {z}, zmm23, {rz-sae}

// CHECK: vcvtdq2ph ymm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7c,0x48,0x5b,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtdq2ph ymm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtdq2ph ymm22 {k7}, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7c,0x4f,0x5b,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtdq2ph ymm22 {k7}, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtdq2ph ymm22, dword ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe5,0x7c,0x58,0x5b,0x35,0x00,0x00,0x00,0x00]
          vcvtdq2ph ymm22, dword ptr [rip]{1to16}

// CHECK: vcvtdq2ph ymm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0x7c,0x48,0x5b,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vcvtdq2ph ymm22, zmmword ptr [2*rbp - 2048]

// CHECK: vcvtdq2ph ymm22 {k7} {z}, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0x7c,0xcf,0x5b,0x71,0x7f]
          vcvtdq2ph ymm22 {k7} {z}, zmmword ptr [rcx + 8128]

// CHECK: vcvtdq2ph ymm22 {k7} {z}, dword ptr [rdx - 512]{1to16}
// CHECK: encoding: [0x62,0xe5,0x7c,0xdf,0x5b,0x72,0x80]
          vcvtdq2ph ymm22 {k7} {z}, dword ptr [rdx - 512]{1to16}

// CHECK: vcvtph2dq zmm22, ymm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x48,0x5b,0xf7]
          vcvtph2dq zmm22, ymm23

// CHECK: vcvtph2dq zmm22, ymm23, {rn-sae}
// CHECK: encoding: [0x62,0xa5,0x7d,0x18,0x5b,0xf7]
          vcvtph2dq zmm22, ymm23, {rn-sae}

// CHECK: vcvtph2dq zmm22 {k7}, ymm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x4f,0x5b,0xf7]
          vcvtph2dq zmm22 {k7}, ymm23

// CHECK: vcvtph2dq zmm22 {k7} {z}, ymm23, {rz-sae}
// CHECK: encoding: [0x62,0xa5,0x7d,0xff,0x5b,0xf7]
          vcvtph2dq zmm22 {k7} {z}, ymm23, {rz-sae}

// CHECK: vcvtph2dq zmm22, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7d,0x48,0x5b,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtph2dq zmm22, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtph2dq zmm22 {k7}, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7d,0x4f,0x5b,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtph2dq zmm22 {k7}, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtph2dq zmm22, word ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe5,0x7d,0x58,0x5b,0x35,0x00,0x00,0x00,0x00]
          vcvtph2dq zmm22, word ptr [rip]{1to16}

// CHECK: vcvtph2dq zmm22, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe5,0x7d,0x48,0x5b,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vcvtph2dq zmm22, ymmword ptr [2*rbp - 1024]

// CHECK: vcvtph2dq zmm22 {k7} {z}, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe5,0x7d,0xcf,0x5b,0x71,0x7f]
          vcvtph2dq zmm22 {k7} {z}, ymmword ptr [rcx + 4064]

// CHECK: vcvtph2dq zmm22 {k7} {z}, word ptr [rdx - 256]{1to16}
// CHECK: encoding: [0x62,0xe5,0x7d,0xdf,0x5b,0x72,0x80]
          vcvtph2dq zmm22 {k7} {z}, word ptr [rdx - 256]{1to16}

// CHECK: vcvtph2udq zmm22, ymm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x48,0x79,0xf7]
          vcvtph2udq zmm22, ymm23

// CHECK: vcvtph2udq zmm22, ymm23, {rn-sae}
// CHECK: encoding: [0x62,0xa5,0x7c,0x18,0x79,0xf7]
          vcvtph2udq zmm22, ymm23, {rn-sae}

// CHECK: vcvtph2udq zmm22 {k7}, ymm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x4f,0x79,0xf7]
          vcvtph2udq zmm22 {k7}, ymm23

// CHECK: vcvtph2udq zmm22 {k7} {z}, ymm23, {rz-sae}
// CHECK: encoding: [0x62,0xa5,0x7c,0xff,0x79,0xf7]
          vcvtph2udq zmm22 {k7} {z}, ymm23, {rz-sae}

// CHECK: vcvtph2udq zmm22, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7c,0x48,0x79,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtph2udq zmm22, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtph2udq zmm22 {k7}, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7c,0x4f,0x79,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtph2udq zmm22 {k7}, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtph2udq zmm22, word ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe5,0x7c,0x58,0x79,0x35,0x00,0x00,0x00,0x00]
          vcvtph2udq zmm22, word ptr [rip]{1to16}

// CHECK: vcvtph2udq zmm22, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe5,0x7c,0x48,0x79,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vcvtph2udq zmm22, ymmword ptr [2*rbp - 1024]

// CHECK: vcvtph2udq zmm22 {k7} {z}, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe5,0x7c,0xcf,0x79,0x71,0x7f]
          vcvtph2udq zmm22 {k7} {z}, ymmword ptr [rcx + 4064]

// CHECK: vcvtph2udq zmm22 {k7} {z}, word ptr [rdx - 256]{1to16}
// CHECK: encoding: [0x62,0xe5,0x7c,0xdf,0x79,0x72,0x80]
          vcvtph2udq zmm22 {k7} {z}, word ptr [rdx - 256]{1to16}

// CHECK: vcvttph2dq zmm22, ymm23
// CHECK: encoding: [0x62,0xa5,0x7e,0x48,0x5b,0xf7]
          vcvttph2dq zmm22, ymm23

// CHECK: vcvttph2dq zmm22, ymm23, {sae}
// CHECK: encoding: [0x62,0xa5,0x7e,0x18,0x5b,0xf7]
          vcvttph2dq zmm22, ymm23, {sae}

// CHECK: vcvttph2dq zmm22 {k7}, ymm23
// CHECK: encoding: [0x62,0xa5,0x7e,0x4f,0x5b,0xf7]
          vcvttph2dq zmm22 {k7}, ymm23

// CHECK: vcvttph2dq zmm22 {k7} {z}, ymm23, {sae}
// CHECK: encoding: [0x62,0xa5,0x7e,0x9f,0x5b,0xf7]
          vcvttph2dq zmm22 {k7} {z}, ymm23, {sae}

// CHECK: vcvttph2dq zmm22, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7e,0x48,0x5b,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvttph2dq zmm22, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvttph2dq zmm22 {k7}, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7e,0x4f,0x5b,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvttph2dq zmm22 {k7}, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vcvttph2dq zmm22, word ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe5,0x7e,0x58,0x5b,0x35,0x00,0x00,0x00,0x00]
          vcvttph2dq zmm22, word ptr [rip]{1to16}

// CHECK: vcvttph2dq zmm22, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe5,0x7e,0x48,0x5b,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vcvttph2dq zmm22, ymmword ptr [2*rbp - 1024]

// CHECK: vcvttph2dq zmm22 {k7} {z}, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe5,0x7e,0xcf,0x5b,0x71,0x7f]
          vcvttph2dq zmm22 {k7} {z}, ymmword ptr [rcx + 4064]

// CHECK: vcvttph2dq zmm22 {k7} {z}, word ptr [rdx - 256]{1to16}
// CHECK: encoding: [0x62,0xe5,0x7e,0xdf,0x5b,0x72,0x80]
          vcvttph2dq zmm22 {k7} {z}, word ptr [rdx - 256]{1to16}

// CHECK: vcvttph2udq zmm22, ymm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x48,0x78,0xf7]
          vcvttph2udq zmm22, ymm23

// CHECK: vcvttph2udq zmm22, ymm23, {sae}
// CHECK: encoding: [0x62,0xa5,0x7c,0x18,0x78,0xf7]
          vcvttph2udq zmm22, ymm23, {sae}

// CHECK: vcvttph2udq zmm22 {k7}, ymm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x4f,0x78,0xf7]
          vcvttph2udq zmm22 {k7}, ymm23

// CHECK: vcvttph2udq zmm22 {k7} {z}, ymm23, {sae}
// CHECK: encoding: [0x62,0xa5,0x7c,0x9f,0x78,0xf7]
          vcvttph2udq zmm22 {k7} {z}, ymm23, {sae}

// CHECK: vcvttph2udq zmm22, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7c,0x48,0x78,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvttph2udq zmm22, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvttph2udq zmm22 {k7}, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7c,0x4f,0x78,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvttph2udq zmm22 {k7}, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vcvttph2udq zmm22, word ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe5,0x7c,0x58,0x78,0x35,0x00,0x00,0x00,0x00]
          vcvttph2udq zmm22, word ptr [rip]{1to16}

// CHECK: vcvttph2udq zmm22, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe5,0x7c,0x48,0x78,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vcvttph2udq zmm22, ymmword ptr [2*rbp - 1024]

// CHECK: vcvttph2udq zmm22 {k7} {z}, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe5,0x7c,0xcf,0x78,0x71,0x7f]
          vcvttph2udq zmm22 {k7} {z}, ymmword ptr [rcx + 4064]

// CHECK: vcvttph2udq zmm22 {k7} {z}, word ptr [rdx - 256]{1to16}
// CHECK: encoding: [0x62,0xe5,0x7c,0xdf,0x78,0x72,0x80]
          vcvttph2udq zmm22 {k7} {z}, word ptr [rdx - 256]{1to16}

// CHECK: vcvtudq2ph ymm22, zmm23
// CHECK: encoding: [0x62,0xa5,0x7f,0x48,0x7a,0xf7]
          vcvtudq2ph ymm22, zmm23

// CHECK: vcvtudq2ph ymm22, zmm23, {rn-sae}
// CHECK: encoding: [0x62,0xa5,0x7f,0x18,0x7a,0xf7]
          vcvtudq2ph ymm22, zmm23, {rn-sae}

// CHECK: vcvtudq2ph ymm22 {k7}, zmm23
// CHECK: encoding: [0x62,0xa5,0x7f,0x4f,0x7a,0xf7]
          vcvtudq2ph ymm22 {k7}, zmm23

// CHECK: vcvtudq2ph ymm22 {k7} {z}, zmm23, {rz-sae}
// CHECK: encoding: [0x62,0xa5,0x7f,0xff,0x7a,0xf7]
          vcvtudq2ph ymm22 {k7} {z}, zmm23, {rz-sae}

// CHECK: vcvtudq2ph ymm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7f,0x48,0x7a,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtudq2ph ymm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtudq2ph ymm22 {k7}, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7f,0x4f,0x7a,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtudq2ph ymm22 {k7}, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtudq2ph ymm22, dword ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe5,0x7f,0x58,0x7a,0x35,0x00,0x00,0x00,0x00]
          vcvtudq2ph ymm22, dword ptr [rip]{1to16}

// CHECK: vcvtudq2ph ymm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0x7f,0x48,0x7a,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vcvtudq2ph ymm22, zmmword ptr [2*rbp - 2048]

// CHECK: vcvtudq2ph ymm22 {k7} {z}, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0x7f,0xcf,0x7a,0x71,0x7f]
          vcvtudq2ph ymm22 {k7} {z}, zmmword ptr [rcx + 8128]

// CHECK: vcvtudq2ph ymm22 {k7} {z}, dword ptr [rdx - 512]{1to16}
// CHECK: encoding: [0x62,0xe5,0x7f,0xdf,0x7a,0x72,0x80]
          vcvtudq2ph ymm22 {k7} {z}, dword ptr [rdx - 512]{1to16}

// CHECK: vcvtph2qq zmm22, xmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x48,0x7b,0xf7]
          vcvtph2qq zmm22, xmm23

// CHECK: vcvtph2qq zmm22, xmm23, {rn-sae}
// CHECK: encoding: [0x62,0xa5,0x7d,0x18,0x7b,0xf7]
          vcvtph2qq zmm22, xmm23, {rn-sae}

// CHECK: vcvtph2qq zmm22 {k7}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x4f,0x7b,0xf7]
          vcvtph2qq zmm22 {k7}, xmm23

// CHECK: vcvtph2qq zmm22 {k7} {z}, xmm23, {rz-sae}
// CHECK: encoding: [0x62,0xa5,0x7d,0xff,0x7b,0xf7]
          vcvtph2qq zmm22 {k7} {z}, xmm23, {rz-sae}

// CHECK: vcvtph2qq zmm22, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7d,0x48,0x7b,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtph2qq zmm22, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtph2qq zmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7d,0x4f,0x7b,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtph2qq zmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtph2qq zmm22, word ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe5,0x7d,0x58,0x7b,0x35,0x00,0x00,0x00,0x00]
          vcvtph2qq zmm22, word ptr [rip]{1to8}

// CHECK: vcvtph2qq zmm22, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe5,0x7d,0x48,0x7b,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvtph2qq zmm22, xmmword ptr [2*rbp - 512]

// CHECK: vcvtph2qq zmm22 {k7} {z}, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe5,0x7d,0xcf,0x7b,0x71,0x7f]
          vcvtph2qq zmm22 {k7} {z}, xmmword ptr [rcx + 2032]

// CHECK: vcvtph2qq zmm22 {k7} {z}, word ptr [rdx - 256]{1to8}
// CHECK: encoding: [0x62,0xe5,0x7d,0xdf,0x7b,0x72,0x80]
          vcvtph2qq zmm22 {k7} {z}, word ptr [rdx - 256]{1to8}

// CHECK: vcvtph2uqq zmm22, xmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x48,0x79,0xf7]
          vcvtph2uqq zmm22, xmm23

// CHECK: vcvtph2uqq zmm22, xmm23, {rn-sae}
// CHECK: encoding: [0x62,0xa5,0x7d,0x18,0x79,0xf7]
          vcvtph2uqq zmm22, xmm23, {rn-sae}

// CHECK: vcvtph2uqq zmm22 {k7}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x4f,0x79,0xf7]
          vcvtph2uqq zmm22 {k7}, xmm23

// CHECK: vcvtph2uqq zmm22 {k7} {z}, xmm23, {rz-sae}
// CHECK: encoding: [0x62,0xa5,0x7d,0xff,0x79,0xf7]
          vcvtph2uqq zmm22 {k7} {z}, xmm23, {rz-sae}

// CHECK: vcvtph2uqq zmm22, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7d,0x48,0x79,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtph2uqq zmm22, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtph2uqq zmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7d,0x4f,0x79,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtph2uqq zmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtph2uqq zmm22, word ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe5,0x7d,0x58,0x79,0x35,0x00,0x00,0x00,0x00]
          vcvtph2uqq zmm22, word ptr [rip]{1to8}

// CHECK: vcvtph2uqq zmm22, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe5,0x7d,0x48,0x79,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvtph2uqq zmm22, xmmword ptr [2*rbp - 512]

// CHECK: vcvtph2uqq zmm22 {k7} {z}, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe5,0x7d,0xcf,0x79,0x71,0x7f]
          vcvtph2uqq zmm22 {k7} {z}, xmmword ptr [rcx + 2032]

// CHECK: vcvtph2uqq zmm22 {k7} {z}, word ptr [rdx - 256]{1to8}
// CHECK: encoding: [0x62,0xe5,0x7d,0xdf,0x79,0x72,0x80]
          vcvtph2uqq zmm22 {k7} {z}, word ptr [rdx - 256]{1to8}

// CHECK: vcvtqq2ph xmm22, zmm23
// CHECK: encoding: [0x62,0xa5,0xfc,0x48,0x5b,0xf7]
          vcvtqq2ph xmm22, zmm23

// CHECK: vcvtqq2ph xmm22, zmm23, {rn-sae}
// CHECK: encoding: [0x62,0xa5,0xfc,0x18,0x5b,0xf7]
          vcvtqq2ph xmm22, zmm23, {rn-sae}

// CHECK: vcvtqq2ph xmm22 {k7}, zmm23
// CHECK: encoding: [0x62,0xa5,0xfc,0x4f,0x5b,0xf7]
          vcvtqq2ph xmm22 {k7}, zmm23

// CHECK: vcvtqq2ph xmm22 {k7} {z}, zmm23, {rz-sae}
// CHECK: encoding: [0x62,0xa5,0xfc,0xff,0x5b,0xf7]
          vcvtqq2ph xmm22 {k7} {z}, zmm23, {rz-sae}

// CHECK: vcvtqq2ph xmm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0xfc,0x48,0x5b,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtqq2ph xmm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtqq2ph xmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0xfc,0x4f,0x5b,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtqq2ph xmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtqq2ph xmm22, qword ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe5,0xfc,0x58,0x5b,0x35,0x00,0x00,0x00,0x00]
          vcvtqq2ph xmm22, qword ptr [rip]{1to8}

// CHECK: vcvtqq2ph xmm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0xfc,0x48,0x5b,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vcvtqq2ph xmm22, zmmword ptr [2*rbp - 2048]

// CHECK: vcvtqq2ph xmm22 {k7} {z}, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0xfc,0xcf,0x5b,0x71,0x7f]
          vcvtqq2ph xmm22 {k7} {z}, zmmword ptr [rcx + 8128]

// CHECK: vcvtqq2ph xmm22 {k7} {z}, qword ptr [rdx - 1024]{1to8}
// CHECK: encoding: [0x62,0xe5,0xfc,0xdf,0x5b,0x72,0x80]
          vcvtqq2ph xmm22 {k7} {z}, qword ptr [rdx - 1024]{1to8}

// CHECK: vcvttph2qq zmm22, xmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x48,0x7a,0xf7]
          vcvttph2qq zmm22, xmm23

// CHECK: vcvttph2qq zmm22, xmm23, {sae}
// CHECK: encoding: [0x62,0xa5,0x7d,0x18,0x7a,0xf7]
          vcvttph2qq zmm22, xmm23, {sae}

// CHECK: vcvttph2qq zmm22 {k7}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x4f,0x7a,0xf7]
          vcvttph2qq zmm22 {k7}, xmm23

// CHECK: vcvttph2qq zmm22 {k7} {z}, xmm23, {sae}
// CHECK: encoding: [0x62,0xa5,0x7d,0x9f,0x7a,0xf7]
          vcvttph2qq zmm22 {k7} {z}, xmm23, {sae}

// CHECK: vcvttph2qq zmm22, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7d,0x48,0x7a,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvttph2qq zmm22, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvttph2qq zmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7d,0x4f,0x7a,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvttph2qq zmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvttph2qq zmm22, word ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe5,0x7d,0x58,0x7a,0x35,0x00,0x00,0x00,0x00]
          vcvttph2qq zmm22, word ptr [rip]{1to8}

// CHECK: vcvttph2qq zmm22, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe5,0x7d,0x48,0x7a,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvttph2qq zmm22, xmmword ptr [2*rbp - 512]

// CHECK: vcvttph2qq zmm22 {k7} {z}, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe5,0x7d,0xcf,0x7a,0x71,0x7f]
          vcvttph2qq zmm22 {k7} {z}, xmmword ptr [rcx + 2032]

// CHECK: vcvttph2qq zmm22 {k7} {z}, word ptr [rdx - 256]{1to8}
// CHECK: encoding: [0x62,0xe5,0x7d,0xdf,0x7a,0x72,0x80]
          vcvttph2qq zmm22 {k7} {z}, word ptr [rdx - 256]{1to8}

// CHECK: vcvttph2uqq zmm22, xmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x48,0x78,0xf7]
          vcvttph2uqq zmm22, xmm23

// CHECK: vcvttph2uqq zmm22, xmm23, {sae}
// CHECK: encoding: [0x62,0xa5,0x7d,0x18,0x78,0xf7]
          vcvttph2uqq zmm22, xmm23, {sae}

// CHECK: vcvttph2uqq zmm22 {k7}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x4f,0x78,0xf7]
          vcvttph2uqq zmm22 {k7}, xmm23

// CHECK: vcvttph2uqq zmm22 {k7} {z}, xmm23, {sae}
// CHECK: encoding: [0x62,0xa5,0x7d,0x9f,0x78,0xf7]
          vcvttph2uqq zmm22 {k7} {z}, xmm23, {sae}

// CHECK: vcvttph2uqq zmm22, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7d,0x48,0x78,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvttph2uqq zmm22, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvttph2uqq zmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7d,0x4f,0x78,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvttph2uqq zmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvttph2uqq zmm22, word ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe5,0x7d,0x58,0x78,0x35,0x00,0x00,0x00,0x00]
          vcvttph2uqq zmm22, word ptr [rip]{1to8}

// CHECK: vcvttph2uqq zmm22, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe5,0x7d,0x48,0x78,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvttph2uqq zmm22, xmmword ptr [2*rbp - 512]

// CHECK: vcvttph2uqq zmm22 {k7} {z}, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe5,0x7d,0xcf,0x78,0x71,0x7f]
          vcvttph2uqq zmm22 {k7} {z}, xmmword ptr [rcx + 2032]

// CHECK: vcvttph2uqq zmm22 {k7} {z}, word ptr [rdx - 256]{1to8}
// CHECK: encoding: [0x62,0xe5,0x7d,0xdf,0x78,0x72,0x80]
          vcvttph2uqq zmm22 {k7} {z}, word ptr [rdx - 256]{1to8}

// CHECK: vcvtuqq2ph xmm22, zmm23
// CHECK: encoding: [0x62,0xa5,0xff,0x48,0x7a,0xf7]
          vcvtuqq2ph xmm22, zmm23

// CHECK: vcvtuqq2ph xmm22, zmm23, {rn-sae}
// CHECK: encoding: [0x62,0xa5,0xff,0x18,0x7a,0xf7]
          vcvtuqq2ph xmm22, zmm23, {rn-sae}

// CHECK: vcvtuqq2ph xmm22 {k7}, zmm23
// CHECK: encoding: [0x62,0xa5,0xff,0x4f,0x7a,0xf7]
          vcvtuqq2ph xmm22 {k7}, zmm23

// CHECK: vcvtuqq2ph xmm22 {k7} {z}, zmm23, {rz-sae}
// CHECK: encoding: [0x62,0xa5,0xff,0xff,0x7a,0xf7]
          vcvtuqq2ph xmm22 {k7} {z}, zmm23, {rz-sae}

// CHECK: vcvtuqq2ph xmm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0xff,0x48,0x7a,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtuqq2ph xmm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtuqq2ph xmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0xff,0x4f,0x7a,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtuqq2ph xmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtuqq2ph xmm22, qword ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe5,0xff,0x58,0x7a,0x35,0x00,0x00,0x00,0x00]
          vcvtuqq2ph xmm22, qword ptr [rip]{1to8}

// CHECK: vcvtuqq2ph xmm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0xff,0x48,0x7a,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vcvtuqq2ph xmm22, zmmword ptr [2*rbp - 2048]

// CHECK: vcvtuqq2ph xmm22 {k7} {z}, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0xff,0xcf,0x7a,0x71,0x7f]
          vcvtuqq2ph xmm22 {k7} {z}, zmmword ptr [rcx + 8128]

// CHECK: vcvtuqq2ph xmm22 {k7} {z}, qword ptr [rdx - 1024]{1to8}
// CHECK: encoding: [0x62,0xe5,0xff,0xdf,0x7a,0x72,0x80]
          vcvtuqq2ph xmm22 {k7} {z}, qword ptr [rdx - 1024]{1to8}

// CHECK: vcvtsh2si ecx, xmm22
// CHECK: encoding: [0x62,0xb5,0x7e,0x08,0x2d,0xce]
          vcvtsh2si ecx, xmm22

// CHECK: vcvtsh2si ecx, xmm22, {rn-sae}
// CHECK: encoding: [0x62,0xb5,0x7e,0x18,0x2d,0xce]
          vcvtsh2si ecx, xmm22, {rn-sae}

// CHECK: vcvtsh2si ecx, xmm22, {rz-sae}
// CHECK: encoding: [0x62,0xb5,0x7e,0x78,0x2d,0xce]
          vcvtsh2si ecx, xmm22, {rz-sae}

// CHECK: vcvtsh2si r9, xmm22
// CHECK: encoding: [0x62,0x35,0xfe,0x08,0x2d,0xce]
          vcvtsh2si r9, xmm22

// CHECK: vcvtsh2si r9, xmm22, {rn-sae}
// CHECK: encoding: [0x62,0x35,0xfe,0x18,0x2d,0xce]
          vcvtsh2si r9, xmm22, {rn-sae}

// CHECK: vcvtsh2si r9, xmm22, {rz-sae}
// CHECK: encoding: [0x62,0x35,0xfe,0x78,0x2d,0xce]
          vcvtsh2si r9, xmm22, {rz-sae}

// CHECK: vcvtsh2si ecx, word ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xb5,0x7e,0x08,0x2d,0x8c,0xf5,0x00,0x00,0x00,0x10]
          vcvtsh2si ecx, word ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtsh2si ecx, word ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xd5,0x7e,0x08,0x2d,0x8c,0x80,0x23,0x01,0x00,0x00]
          vcvtsh2si ecx, word ptr [r8 + 4*rax + 291]

// CHECK: vcvtsh2si ecx, word ptr [rip]
// CHECK: encoding: [0x62,0xf5,0x7e,0x08,0x2d,0x0d,0x00,0x00,0x00,0x00]
          vcvtsh2si ecx, word ptr [rip]

// CHECK: vcvtsh2si ecx, word ptr [2*rbp - 64]
// CHECK: encoding: [0x62,0xf5,0x7e,0x08,0x2d,0x0c,0x6d,0xc0,0xff,0xff,0xff]
          vcvtsh2si ecx, word ptr [2*rbp - 64]

// CHECK: vcvtsh2si ecx, word ptr [rcx + 254]
// CHECK: encoding: [0x62,0xf5,0x7e,0x08,0x2d,0x49,0x7f]
          vcvtsh2si ecx, word ptr [rcx + 254]

// CHECK: vcvtsh2si ecx, word ptr [rdx - 256]
// CHECK: encoding: [0x62,0xf5,0x7e,0x08,0x2d,0x4a,0x80]
          vcvtsh2si ecx, word ptr [rdx - 256]

// CHECK: vcvtsh2si r9, word ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0x35,0xfe,0x08,0x2d,0x8c,0xf5,0x00,0x00,0x00,0x10]
          vcvtsh2si r9, word ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtsh2si r9, word ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0x55,0xfe,0x08,0x2d,0x8c,0x80,0x23,0x01,0x00,0x00]
          vcvtsh2si r9, word ptr [r8 + 4*rax + 291]

// CHECK: vcvtsh2si r9, word ptr [rip]
// CHECK: encoding: [0x62,0x75,0xfe,0x08,0x2d,0x0d,0x00,0x00,0x00,0x00]
          vcvtsh2si r9, word ptr [rip]

// CHECK: vcvtsh2si r9, word ptr [2*rbp - 64]
// CHECK: encoding: [0x62,0x75,0xfe,0x08,0x2d,0x0c,0x6d,0xc0,0xff,0xff,0xff]
          vcvtsh2si r9, word ptr [2*rbp - 64]

// CHECK: vcvtsh2si r9, word ptr [rcx + 254]
// CHECK: encoding: [0x62,0x75,0xfe,0x08,0x2d,0x49,0x7f]
          vcvtsh2si r9, word ptr [rcx + 254]

// CHECK: vcvtsh2si r9, word ptr [rdx - 256]
// CHECK: encoding: [0x62,0x75,0xfe,0x08,0x2d,0x4a,0x80]
          vcvtsh2si r9, word ptr [rdx - 256]

// CHECK: vcvtsh2usi ecx, xmm22
// CHECK: encoding: [0x62,0xb5,0x7e,0x08,0x79,0xce]
          vcvtsh2usi ecx, xmm22

// CHECK: vcvtsh2usi ecx, xmm22, {rn-sae}
// CHECK: encoding: [0x62,0xb5,0x7e,0x18,0x79,0xce]
          vcvtsh2usi ecx, xmm22, {rn-sae}

// CHECK: vcvtsh2usi ecx, xmm22, {rz-sae}
// CHECK: encoding: [0x62,0xb5,0x7e,0x78,0x79,0xce]
          vcvtsh2usi ecx, xmm22, {rz-sae}

// CHECK: vcvtsh2usi r9, xmm22
// CHECK: encoding: [0x62,0x35,0xfe,0x08,0x79,0xce]
          vcvtsh2usi r9, xmm22

// CHECK: vcvtsh2usi r9, xmm22, {rn-sae}
// CHECK: encoding: [0x62,0x35,0xfe,0x18,0x79,0xce]
          vcvtsh2usi r9, xmm22, {rn-sae}

// CHECK: vcvtsh2usi r9, xmm22, {rz-sae}
// CHECK: encoding: [0x62,0x35,0xfe,0x78,0x79,0xce]
          vcvtsh2usi r9, xmm22, {rz-sae}

// CHECK: vcvtsh2usi ecx, word ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xb5,0x7e,0x08,0x79,0x8c,0xf5,0x00,0x00,0x00,0x10]
          vcvtsh2usi ecx, word ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtsh2usi ecx, word ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xd5,0x7e,0x08,0x79,0x8c,0x80,0x23,0x01,0x00,0x00]
          vcvtsh2usi ecx, word ptr [r8 + 4*rax + 291]

// CHECK: vcvtsh2usi ecx, word ptr [rip]
// CHECK: encoding: [0x62,0xf5,0x7e,0x08,0x79,0x0d,0x00,0x00,0x00,0x00]
          vcvtsh2usi ecx, word ptr [rip]

// CHECK: vcvtsh2usi ecx, word ptr [2*rbp - 64]
// CHECK: encoding: [0x62,0xf5,0x7e,0x08,0x79,0x0c,0x6d,0xc0,0xff,0xff,0xff]
          vcvtsh2usi ecx, word ptr [2*rbp - 64]

// CHECK: vcvtsh2usi ecx, word ptr [rcx + 254]
// CHECK: encoding: [0x62,0xf5,0x7e,0x08,0x79,0x49,0x7f]
          vcvtsh2usi ecx, word ptr [rcx + 254]

// CHECK: vcvtsh2usi ecx, word ptr [rdx - 256]
// CHECK: encoding: [0x62,0xf5,0x7e,0x08,0x79,0x4a,0x80]
          vcvtsh2usi ecx, word ptr [rdx - 256]

// CHECK: vcvtsh2usi r9, word ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0x35,0xfe,0x08,0x79,0x8c,0xf5,0x00,0x00,0x00,0x10]
          vcvtsh2usi r9, word ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtsh2usi r9, word ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0x55,0xfe,0x08,0x79,0x8c,0x80,0x23,0x01,0x00,0x00]
          vcvtsh2usi r9, word ptr [r8 + 4*rax + 291]

// CHECK: vcvtsh2usi r9, word ptr [rip]
// CHECK: encoding: [0x62,0x75,0xfe,0x08,0x79,0x0d,0x00,0x00,0x00,0x00]
          vcvtsh2usi r9, word ptr [rip]

// CHECK: vcvtsh2usi r9, word ptr [2*rbp - 64]
// CHECK: encoding: [0x62,0x75,0xfe,0x08,0x79,0x0c,0x6d,0xc0,0xff,0xff,0xff]
          vcvtsh2usi r9, word ptr [2*rbp - 64]

// CHECK: vcvtsh2usi r9, word ptr [rcx + 254]
// CHECK: encoding: [0x62,0x75,0xfe,0x08,0x79,0x49,0x7f]
          vcvtsh2usi r9, word ptr [rcx + 254]

// CHECK: vcvtsh2usi r9, word ptr [rdx - 256]
// CHECK: encoding: [0x62,0x75,0xfe,0x08,0x79,0x4a,0x80]
          vcvtsh2usi r9, word ptr [rdx - 256]

// CHECK: vcvtsi2sh xmm22, xmm23, r9
// CHECK: encoding: [0x62,0xc5,0xc6,0x00,0x2a,0xf1]
          vcvtsi2sh xmm22, xmm23, r9

// CHECK: vcvtsi2sh xmm22, xmm23, {rn-sae}, r9
// CHECK: encoding: [0x62,0xc5,0xc6,0x10,0x2a,0xf1]
          vcvtsi2sh xmm22, xmm23, {rn-sae}, r9

// CHECK: vcvtsi2sh xmm22, xmm23, {rz-sae}, r9
// CHECK: encoding: [0x62,0xc5,0xc6,0x70,0x2a,0xf1]
          vcvtsi2sh xmm22, xmm23, {rz-sae}, r9

// CHECK: vcvtsi2sh xmm22, xmm23, ecx
// CHECK: encoding: [0x62,0xe5,0x46,0x00,0x2a,0xf1]
          vcvtsi2sh xmm22, xmm23, ecx

// CHECK: vcvtsi2sh xmm22, xmm23, {rn-sae}, ecx
// CHECK: encoding: [0x62,0xe5,0x46,0x10,0x2a,0xf1]
          vcvtsi2sh xmm22, xmm23, {rn-sae}, ecx

// CHECK: vcvtsi2sh xmm22, xmm23, {rz-sae}, ecx
// CHECK: encoding: [0x62,0xe5,0x46,0x70,0x2a,0xf1]
          vcvtsi2sh xmm22, xmm23, {rz-sae}, ecx

// CHECK: vcvtsi2sh xmm22, xmm23, dword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x46,0x00,0x2a,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtsi2sh xmm22, xmm23, dword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtsi2sh xmm22, xmm23, dword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x46,0x00,0x2a,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtsi2sh xmm22, xmm23, dword ptr [r8 + 4*rax + 291]

// CHECK: vcvtsi2sh xmm22, xmm23, dword ptr [rip]
// CHECK: encoding: [0x62,0xe5,0x46,0x00,0x2a,0x35,0x00,0x00,0x00,0x00]
          vcvtsi2sh xmm22, xmm23, dword ptr [rip]

// CHECK: vcvtsi2sh xmm22, xmm23, dword ptr [2*rbp - 128]
// CHECK: encoding: [0x62,0xe5,0x46,0x00,0x2a,0x34,0x6d,0x80,0xff,0xff,0xff]
          vcvtsi2sh xmm22, xmm23, dword ptr [2*rbp - 128]

// CHECK: vcvtsi2sh xmm22, xmm23, dword ptr [rcx + 508]
// CHECK: encoding: [0x62,0xe5,0x46,0x00,0x2a,0x71,0x7f]
          vcvtsi2sh xmm22, xmm23, dword ptr [rcx + 508]

// CHECK: vcvtsi2sh xmm22, xmm23, dword ptr [rdx - 512]
// CHECK: encoding: [0x62,0xe5,0x46,0x00,0x2a,0x72,0x80]
          vcvtsi2sh xmm22, xmm23, dword ptr [rdx - 512]

// CHECK: vcvtsi2sh xmm22, xmm23, qword ptr [2*rbp - 256]
// CHECK: encoding: [0x62,0xe5,0xc6,0x00,0x2a,0x34,0x6d,0x00,0xff,0xff,0xff]
          vcvtsi2sh xmm22, xmm23, qword ptr [2*rbp - 256]

// CHECK: vcvtsi2sh xmm22, xmm23, qword ptr [rcx + 1016]
// CHECK: encoding: [0x62,0xe5,0xc6,0x00,0x2a,0x71,0x7f]
          vcvtsi2sh xmm22, xmm23, qword ptr [rcx + 1016]

// CHECK: vcvtsi2sh xmm22, xmm23, qword ptr [rdx - 1024]
// CHECK: encoding: [0x62,0xe5,0xc6,0x00,0x2a,0x72,0x80]
          vcvtsi2sh xmm22, xmm23, qword ptr [rdx - 1024]

// CHECK: vcvttsh2si ecx, xmm22
// CHECK: encoding: [0x62,0xb5,0x7e,0x08,0x2c,0xce]
          vcvttsh2si ecx, xmm22

// CHECK: vcvttsh2si ecx, xmm22, {sae}
// CHECK: encoding: [0x62,0xb5,0x7e,0x18,0x2c,0xce]
          vcvttsh2si ecx, xmm22, {sae}

// CHECK: vcvttsh2si r9, xmm22
// CHECK: encoding: [0x62,0x35,0xfe,0x08,0x2c,0xce]
          vcvttsh2si r9, xmm22

// CHECK: vcvttsh2si r9, xmm22, {sae}
// CHECK: encoding: [0x62,0x35,0xfe,0x18,0x2c,0xce]
          vcvttsh2si r9, xmm22, {sae}

// CHECK: vcvttsh2si ecx, word ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xb5,0x7e,0x08,0x2c,0x8c,0xf5,0x00,0x00,0x00,0x10]
          vcvttsh2si ecx, word ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvttsh2si ecx, word ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xd5,0x7e,0x08,0x2c,0x8c,0x80,0x23,0x01,0x00,0x00]
          vcvttsh2si ecx, word ptr [r8 + 4*rax + 291]

// CHECK: vcvttsh2si ecx, word ptr [rip]
// CHECK: encoding: [0x62,0xf5,0x7e,0x08,0x2c,0x0d,0x00,0x00,0x00,0x00]
          vcvttsh2si ecx, word ptr [rip]

// CHECK: vcvttsh2si ecx, word ptr [2*rbp - 64]
// CHECK: encoding: [0x62,0xf5,0x7e,0x08,0x2c,0x0c,0x6d,0xc0,0xff,0xff,0xff]
          vcvttsh2si ecx, word ptr [2*rbp - 64]

// CHECK: vcvttsh2si ecx, word ptr [rcx + 254]
// CHECK: encoding: [0x62,0xf5,0x7e,0x08,0x2c,0x49,0x7f]
          vcvttsh2si ecx, word ptr [rcx + 254]

// CHECK: vcvttsh2si ecx, word ptr [rdx - 256]
// CHECK: encoding: [0x62,0xf5,0x7e,0x08,0x2c,0x4a,0x80]
          vcvttsh2si ecx, word ptr [rdx - 256]

// CHECK: vcvttsh2si r9, word ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0x35,0xfe,0x08,0x2c,0x8c,0xf5,0x00,0x00,0x00,0x10]
          vcvttsh2si r9, word ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvttsh2si r9, word ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0x55,0xfe,0x08,0x2c,0x8c,0x80,0x23,0x01,0x00,0x00]
          vcvttsh2si r9, word ptr [r8 + 4*rax + 291]

// CHECK: vcvttsh2si r9, word ptr [rip]
// CHECK: encoding: [0x62,0x75,0xfe,0x08,0x2c,0x0d,0x00,0x00,0x00,0x00]
          vcvttsh2si r9, word ptr [rip]

// CHECK: vcvttsh2si r9, word ptr [2*rbp - 64]
// CHECK: encoding: [0x62,0x75,0xfe,0x08,0x2c,0x0c,0x6d,0xc0,0xff,0xff,0xff]
          vcvttsh2si r9, word ptr [2*rbp - 64]

// CHECK: vcvttsh2si r9, word ptr [rcx + 254]
// CHECK: encoding: [0x62,0x75,0xfe,0x08,0x2c,0x49,0x7f]
          vcvttsh2si r9, word ptr [rcx + 254]

// CHECK: vcvttsh2si r9, word ptr [rdx - 256]
// CHECK: encoding: [0x62,0x75,0xfe,0x08,0x2c,0x4a,0x80]
          vcvttsh2si r9, word ptr [rdx - 256]

// CHECK: vcvttsh2usi ecx, xmm22
// CHECK: encoding: [0x62,0xb5,0x7e,0x08,0x78,0xce]
          vcvttsh2usi ecx, xmm22

// CHECK: vcvttsh2usi ecx, xmm22, {sae}
// CHECK: encoding: [0x62,0xb5,0x7e,0x18,0x78,0xce]
          vcvttsh2usi ecx, xmm22, {sae}

// CHECK: vcvttsh2usi r9, xmm22
// CHECK: encoding: [0x62,0x35,0xfe,0x08,0x78,0xce]
          vcvttsh2usi r9, xmm22

// CHECK: vcvttsh2usi r9, xmm22, {sae}
// CHECK: encoding: [0x62,0x35,0xfe,0x18,0x78,0xce]
          vcvttsh2usi r9, xmm22, {sae}

// CHECK: vcvttsh2usi ecx, word ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xb5,0x7e,0x08,0x78,0x8c,0xf5,0x00,0x00,0x00,0x10]
          vcvttsh2usi ecx, word ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvttsh2usi ecx, word ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xd5,0x7e,0x08,0x78,0x8c,0x80,0x23,0x01,0x00,0x00]
          vcvttsh2usi ecx, word ptr [r8 + 4*rax + 291]

// CHECK: vcvttsh2usi ecx, word ptr [rip]
// CHECK: encoding: [0x62,0xf5,0x7e,0x08,0x78,0x0d,0x00,0x00,0x00,0x00]
          vcvttsh2usi ecx, word ptr [rip]

// CHECK: vcvttsh2usi ecx, word ptr [2*rbp - 64]
// CHECK: encoding: [0x62,0xf5,0x7e,0x08,0x78,0x0c,0x6d,0xc0,0xff,0xff,0xff]
          vcvttsh2usi ecx, word ptr [2*rbp - 64]

// CHECK: vcvttsh2usi ecx, word ptr [rcx + 254]
// CHECK: encoding: [0x62,0xf5,0x7e,0x08,0x78,0x49,0x7f]
          vcvttsh2usi ecx, word ptr [rcx + 254]

// CHECK: vcvttsh2usi ecx, word ptr [rdx - 256]
// CHECK: encoding: [0x62,0xf5,0x7e,0x08,0x78,0x4a,0x80]
          vcvttsh2usi ecx, word ptr [rdx - 256]

// CHECK: vcvttsh2usi r9, word ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0x35,0xfe,0x08,0x78,0x8c,0xf5,0x00,0x00,0x00,0x10]
          vcvttsh2usi r9, word ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvttsh2usi r9, word ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0x55,0xfe,0x08,0x78,0x8c,0x80,0x23,0x01,0x00,0x00]
          vcvttsh2usi r9, word ptr [r8 + 4*rax + 291]

// CHECK: vcvttsh2usi r9, word ptr [rip]
// CHECK: encoding: [0x62,0x75,0xfe,0x08,0x78,0x0d,0x00,0x00,0x00,0x00]
          vcvttsh2usi r9, word ptr [rip]

// CHECK: vcvttsh2usi r9, word ptr [2*rbp - 64]
// CHECK: encoding: [0x62,0x75,0xfe,0x08,0x78,0x0c,0x6d,0xc0,0xff,0xff,0xff]
          vcvttsh2usi r9, word ptr [2*rbp - 64]

// CHECK: vcvttsh2usi r9, word ptr [rcx + 254]
// CHECK: encoding: [0x62,0x75,0xfe,0x08,0x78,0x49,0x7f]
          vcvttsh2usi r9, word ptr [rcx + 254]

// CHECK: vcvttsh2usi r9, word ptr [rdx - 256]
// CHECK: encoding: [0x62,0x75,0xfe,0x08,0x78,0x4a,0x80]
          vcvttsh2usi r9, word ptr [rdx - 256]

// CHECK: vcvtusi2sh xmm22, xmm23, r9
// CHECK: encoding: [0x62,0xc5,0xc6,0x00,0x7b,0xf1]
          vcvtusi2sh xmm22, xmm23, r9

// CHECK: vcvtusi2sh xmm22, xmm23, {rn-sae}, r9
// CHECK: encoding: [0x62,0xc5,0xc6,0x10,0x7b,0xf1]
          vcvtusi2sh xmm22, xmm23, {rn-sae}, r9

// CHECK: vcvtusi2sh xmm22, xmm23, {rz-sae}, r9
// CHECK: encoding: [0x62,0xc5,0xc6,0x70,0x7b,0xf1]
          vcvtusi2sh xmm22, xmm23, {rz-sae}, r9

// CHECK: vcvtusi2sh xmm22, xmm23, ecx
// CHECK: encoding: [0x62,0xe5,0x46,0x00,0x7b,0xf1]
          vcvtusi2sh xmm22, xmm23, ecx

// CHECK: vcvtusi2sh xmm22, xmm23, {rn-sae}, ecx
// CHECK: encoding: [0x62,0xe5,0x46,0x10,0x7b,0xf1]
          vcvtusi2sh xmm22, xmm23, {rn-sae}, ecx

// CHECK: vcvtusi2sh xmm22, xmm23, {rz-sae}, ecx
// CHECK: encoding: [0x62,0xe5,0x46,0x70,0x7b,0xf1]
          vcvtusi2sh xmm22, xmm23, {rz-sae}, ecx

// CHECK: vcvtusi2sh xmm22, xmm23, dword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x46,0x00,0x7b,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtusi2sh xmm22, xmm23, dword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtusi2sh xmm22, xmm23, dword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x46,0x00,0x7b,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtusi2sh xmm22, xmm23, dword ptr [r8 + 4*rax + 291]

// CHECK: vcvtusi2sh xmm22, xmm23, dword ptr [rip]
// CHECK: encoding: [0x62,0xe5,0x46,0x00,0x7b,0x35,0x00,0x00,0x00,0x00]
          vcvtusi2sh xmm22, xmm23, dword ptr [rip]

// CHECK: vcvtusi2sh xmm22, xmm23, dword ptr [2*rbp - 128]
// CHECK: encoding: [0x62,0xe5,0x46,0x00,0x7b,0x34,0x6d,0x80,0xff,0xff,0xff]
          vcvtusi2sh xmm22, xmm23, dword ptr [2*rbp - 128]

// CHECK: vcvtusi2sh xmm22, xmm23, dword ptr [rcx + 508]
// CHECK: encoding: [0x62,0xe5,0x46,0x00,0x7b,0x71,0x7f]
          vcvtusi2sh xmm22, xmm23, dword ptr [rcx + 508]

// CHECK: vcvtusi2sh xmm22, xmm23, dword ptr [rdx - 512]
// CHECK: encoding: [0x62,0xe5,0x46,0x00,0x7b,0x72,0x80]
          vcvtusi2sh xmm22, xmm23, dword ptr [rdx - 512]

// CHECK: vcvtusi2sh xmm22, xmm23, qword ptr [2*rbp - 256]
// CHECK: encoding: [0x62,0xe5,0xc6,0x00,0x7b,0x34,0x6d,0x00,0xff,0xff,0xff]
          vcvtusi2sh xmm22, xmm23, qword ptr [2*rbp - 256]

// CHECK: vcvtusi2sh xmm22, xmm23, qword ptr [rcx + 1016]
// CHECK: encoding: [0x62,0xe5,0xc6,0x00,0x7b,0x71,0x7f]
          vcvtusi2sh xmm22, xmm23, qword ptr [rcx + 1016]

// CHECK: vcvtusi2sh xmm22, xmm23, qword ptr [rdx - 1024]
// CHECK: encoding: [0x62,0xe5,0xc6,0x00,0x7b,0x72,0x80]
          vcvtusi2sh xmm22, xmm23, qword ptr [rdx - 1024]

