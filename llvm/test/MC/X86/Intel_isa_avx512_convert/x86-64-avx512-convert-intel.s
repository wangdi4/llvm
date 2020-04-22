// REQUIRES: intel_feature_isa_avx512_convert
// RUN: llvm-mc -triple x86_64-unknown-unknown -mattr=+avx512convert -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK:      vcvt2ps2ph zmm22, zmm23, zmm24
// CHECK: encoding: [0x62,0x82,0x45,0x40,0x67,0xf0]
               vcvt2ps2ph zmm22, zmm23, zmm24

// CHECK:      vcvt2ps2ph zmm22, zmm23, zmm24, {rn-sae}
// CHECK: encoding: [0x62,0x82,0x45,0x10,0x67,0xf0]
               vcvt2ps2ph zmm22, zmm23, zmm24, {rn-sae}

// CHECK:      vcvt2ps2ph zmm22 {k7}, zmm23, zmm24
// CHECK: encoding: [0x62,0x82,0x45,0x47,0x67,0xf0]
               vcvt2ps2ph zmm22 {k7}, zmm23, zmm24

// CHECK:      vcvt2ps2ph zmm22 {k7} {z}, zmm23, zmm24, {rz-sae}
// CHECK: encoding: [0x62,0x82,0x45,0xf7,0x67,0xf0]
               vcvt2ps2ph zmm22 {k7} {z}, zmm23, zmm24, {rz-sae}

// CHECK:      vcvt2ps2ph zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x45,0x40,0x67,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vcvt2ps2ph zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      vcvt2ps2ph zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x45,0x47,0x67,0xb4,0x80,0x23,0x01,0x00,0x00]
               vcvt2ps2ph zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]

// CHECK:      vcvt2ps2ph zmm22, zmm23, dword ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe2,0x45,0x50,0x67,0x35,0x00,0x00,0x00,0x00]
               vcvt2ps2ph zmm22, zmm23, dword ptr [rip]{1to16}

// CHECK:      vcvt2ps2ph zmm22, zmm23, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe2,0x45,0x40,0x67,0x34,0x6d,0x00,0xf8,0xff,0xff]
               vcvt2ps2ph zmm22, zmm23, zmmword ptr [2*rbp - 2048]

// CHECK:      vcvt2ps2ph zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe2,0x45,0xc7,0x67,0x71,0x7f]
               vcvt2ps2ph zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]

// CHECK:      vcvt2ps2ph zmm22 {k7} {z}, zmm23, dword ptr [rdx - 512]{1to16}
// CHECK: encoding: [0x62,0xe2,0x45,0xd7,0x67,0x72,0x80]
               vcvt2ps2ph zmm22 {k7} {z}, zmm23, dword ptr [rdx - 512]{1to16}

// CHECK:      vcvtbf162ph zmm22, zmm23
// CHECK: encoding: [0x62,0xa2,0x7e,0x48,0x67,0xf7]
               vcvtbf162ph zmm22, zmm23

// CHECK:      vcvtbf162ph zmm22, zmm23, {rn-sae}
// CHECK: encoding: [0x62,0xa2,0x7e,0x18,0x67,0xf7]
               vcvtbf162ph zmm22, zmm23, {rn-sae}

// CHECK:      vcvtbf162ph zmm22 {k7}, zmm23
// CHECK: encoding: [0x62,0xa2,0x7e,0x4f,0x67,0xf7]
               vcvtbf162ph zmm22 {k7}, zmm23

// CHECK:      vcvtbf162ph zmm22 {k7} {z}, zmm23, {rz-sae}
// CHECK: encoding: [0x62,0xa2,0x7e,0xff,0x67,0xf7]
               vcvtbf162ph zmm22 {k7} {z}, zmm23, {rz-sae}

// CHECK:      vcvtbf162ph zmm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x7e,0x48,0x67,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vcvtbf162ph zmm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      vcvtbf162ph zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x7e,0x4f,0x67,0xb4,0x80,0x23,0x01,0x00,0x00]
               vcvtbf162ph zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]

// CHECK:      vcvtbf162ph zmm22, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0xe2,0x7e,0x58,0x67,0x35,0x00,0x00,0x00,0x00]
               vcvtbf162ph zmm22, word ptr [rip]{1to32}

// CHECK:      vcvtbf162ph zmm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe2,0x7e,0x48,0x67,0x34,0x6d,0x00,0xf8,0xff,0xff]
               vcvtbf162ph zmm22, zmmword ptr [2*rbp - 2048]

// CHECK:      vcvtbf162ph zmm22 {k7} {z}, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe2,0x7e,0xcf,0x67,0x71,0x7f]
               vcvtbf162ph zmm22 {k7} {z}, zmmword ptr [rcx + 8128]

// CHECK:      vcvtbf162ph zmm22 {k7} {z}, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0xe2,0x7e,0xdf,0x67,0x72,0x80]
               vcvtbf162ph zmm22 {k7} {z}, word ptr [rdx - 256]{1to32}

// CHECK:      vcvtneph2bf16 zmm22, zmm23
// CHECK: encoding: [0x62,0xa2,0x7f,0x48,0x67,0xf7]
               vcvtneph2bf16 zmm22, zmm23

// CHECK:      vcvtneph2bf16 zmm22 {k7}, zmm23
// CHECK: encoding: [0x62,0xa2,0x7f,0x4f,0x67,0xf7]
               vcvtneph2bf16 zmm22 {k7}, zmm23

// CHECK:      vcvtneph2bf16 zmm22 {k7} {z}, zmm23
// CHECK: encoding: [0x62,0xa2,0x7f,0xcf,0x67,0xf7]
               vcvtneph2bf16 zmm22 {k7} {z}, zmm23

// CHECK:      vcvtneph2bf16 zmm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x7f,0x48,0x67,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vcvtneph2bf16 zmm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      vcvtneph2bf16 zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x7f,0x4f,0x67,0xb4,0x80,0x23,0x01,0x00,0x00]
               vcvtneph2bf16 zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]

// CHECK:      vcvtneph2bf16 zmm22, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0xe2,0x7f,0x58,0x67,0x35,0x00,0x00,0x00,0x00]
               vcvtneph2bf16 zmm22, word ptr [rip]{1to32}

// CHECK:      vcvtneph2bf16 zmm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe2,0x7f,0x48,0x67,0x34,0x6d,0x00,0xf8,0xff,0xff]
               vcvtneph2bf16 zmm22, zmmword ptr [2*rbp - 2048]

// CHECK:      vcvtneph2bf16 zmm22 {k7} {z}, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe2,0x7f,0xcf,0x67,0x71,0x7f]
               vcvtneph2bf16 zmm22 {k7} {z}, zmmword ptr [rcx + 8128]

// CHECK:      vcvtneph2bf16 zmm22 {k7} {z}, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0xe2,0x7f,0xdf,0x67,0x72,0x80]
               vcvtneph2bf16 zmm22 {k7} {z}, word ptr [rdx - 256]{1to32}

