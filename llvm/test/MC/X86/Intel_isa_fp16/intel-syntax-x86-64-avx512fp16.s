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

