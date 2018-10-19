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
