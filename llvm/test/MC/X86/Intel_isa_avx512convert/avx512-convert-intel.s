// REQUIRES: intel_feature_isa_avx512_convert
// RUN: llvm-mc -triple i686-unknown-unknown -mattr=+avx512convert -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK:      vcvt2ps2ph zmm2, zmm3, zmm4
// CHECK: encoding: [0x62,0xf2,0x65,0x48,0x67,0xd4]
               vcvt2ps2ph zmm2, zmm3, zmm4

// CHECK:      vcvt2ps2ph zmm2, zmm3, zmm4, {rn-sae}
// CHECK: encoding: [0x62,0xf2,0x65,0x18,0x67,0xd4]
               vcvt2ps2ph zmm2, zmm3, zmm4, {rn-sae}

// CHECK:      vcvt2ps2ph zmm2 {k7}, zmm3, zmm4
// CHECK: encoding: [0x62,0xf2,0x65,0x4f,0x67,0xd4]
               vcvt2ps2ph zmm2 {k7}, zmm3, zmm4

// CHECK:      vcvt2ps2ph zmm2 {k7} {z}, zmm3, zmm4, {rz-sae}
// CHECK: encoding: [0x62,0xf2,0x65,0xff,0x67,0xd4]
               vcvt2ps2ph zmm2 {k7} {z}, zmm3, zmm4, {rz-sae}

// CHECK:      vcvt2ps2ph zmm2, zmm3, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf2,0x65,0x48,0x67,0x94,0xf4,0x00,0x00,0x00,0x10]
               vcvt2ps2ph zmm2, zmm3, zmmword ptr [esp + 8*esi + 268435456]

// CHECK:      vcvt2ps2ph zmm2 {k7}, zmm3, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x65,0x4f,0x67,0x94,0x87,0x23,0x01,0x00,0x00]
               vcvt2ps2ph zmm2 {k7}, zmm3, zmmword ptr [edi + 4*eax + 291]

// CHECK:      vcvt2ps2ph zmm2, zmm3, dword ptr [eax]{1to16}
// CHECK: encoding: [0x62,0xf2,0x65,0x58,0x67,0x10]
               vcvt2ps2ph zmm2, zmm3, dword ptr [eax]{1to16}

// CHECK:      vcvt2ps2ph zmm2, zmm3, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf2,0x65,0x48,0x67,0x14,0x6d,0x00,0xf8,0xff,0xff]
               vcvt2ps2ph zmm2, zmm3, zmmword ptr [2*ebp - 2048]

// CHECK:      vcvt2ps2ph zmm2 {k7} {z}, zmm3, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf2,0x65,0xcf,0x67,0x51,0x7f]
               vcvt2ps2ph zmm2 {k7} {z}, zmm3, zmmword ptr [ecx + 8128]

// CHECK:      vcvt2ps2ph zmm2 {k7} {z}, zmm3, dword ptr [edx - 512]{1to16}
// CHECK: encoding: [0x62,0xf2,0x65,0xdf,0x67,0x52,0x80]
               vcvt2ps2ph zmm2 {k7} {z}, zmm3, dword ptr [edx - 512]{1to16}

// CHECK:      vcvtbf162ph zmm2, zmm3
// CHECK: encoding: [0x62,0xf2,0x7e,0x48,0x67,0xd3]
               vcvtbf162ph zmm2, zmm3

// CHECK:      vcvtbf162ph zmm2, zmm3, {rn-sae}
// CHECK: encoding: [0x62,0xf2,0x7e,0x18,0x67,0xd3]
               vcvtbf162ph zmm2, zmm3, {rn-sae}

// CHECK:      vcvtbf162ph zmm2 {k7}, zmm3
// CHECK: encoding: [0x62,0xf2,0x7e,0x4f,0x67,0xd3]
               vcvtbf162ph zmm2 {k7}, zmm3

// CHECK:      vcvtbf162ph zmm2 {k7} {z}, zmm3, {rz-sae}
// CHECK: encoding: [0x62,0xf2,0x7e,0xff,0x67,0xd3]
               vcvtbf162ph zmm2 {k7} {z}, zmm3, {rz-sae}

// CHECK:      vcvtbf162ph zmm2, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf2,0x7e,0x48,0x67,0x94,0xf4,0x00,0x00,0x00,0x10]
               vcvtbf162ph zmm2, zmmword ptr [esp + 8*esi + 268435456]

// CHECK:      vcvtbf162ph zmm2 {k7}, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x7e,0x4f,0x67,0x94,0x87,0x23,0x01,0x00,0x00]
               vcvtbf162ph zmm2 {k7}, zmmword ptr [edi + 4*eax + 291]

// CHECK:      vcvtbf162ph zmm2, word ptr [eax]{1to32}
// CHECK: encoding: [0x62,0xf2,0x7e,0x58,0x67,0x10]
               vcvtbf162ph zmm2, word ptr [eax]{1to32}

// CHECK:      vcvtbf162ph zmm2, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf2,0x7e,0x48,0x67,0x14,0x6d,0x00,0xf8,0xff,0xff]
               vcvtbf162ph zmm2, zmmword ptr [2*ebp - 2048]

// CHECK:      vcvtbf162ph zmm2 {k7} {z}, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf2,0x7e,0xcf,0x67,0x51,0x7f]
               vcvtbf162ph zmm2 {k7} {z}, zmmword ptr [ecx + 8128]

// CHECK:      vcvtbf162ph zmm2 {k7} {z}, word ptr [edx - 256]{1to32}
// CHECK: encoding: [0x62,0xf2,0x7e,0xdf,0x67,0x52,0x80]
               vcvtbf162ph zmm2 {k7} {z}, word ptr [edx - 256]{1to32}

// CHECK:      vcvtneph2bf16 zmm2, zmm3
// CHECK: encoding: [0x62,0xf2,0x7f,0x48,0x67,0xd3]
               vcvtneph2bf16 zmm2, zmm3

// CHECK:      vcvtneph2bf16 zmm2 {k7}, zmm3
// CHECK: encoding: [0x62,0xf2,0x7f,0x4f,0x67,0xd3]
               vcvtneph2bf16 zmm2 {k7}, zmm3

// CHECK:      vcvtneph2bf16 zmm2 {k7} {z}, zmm3
// CHECK: encoding: [0x62,0xf2,0x7f,0xcf,0x67,0xd3]
               vcvtneph2bf16 zmm2 {k7} {z}, zmm3

// CHECK:      vcvtneph2bf16 zmm2, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf2,0x7f,0x48,0x67,0x94,0xf4,0x00,0x00,0x00,0x10]
               vcvtneph2bf16 zmm2, zmmword ptr [esp + 8*esi + 268435456]

// CHECK:      vcvtneph2bf16 zmm2 {k7}, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x7f,0x4f,0x67,0x94,0x87,0x23,0x01,0x00,0x00]
               vcvtneph2bf16 zmm2 {k7}, zmmword ptr [edi + 4*eax + 291]

// CHECK:      vcvtneph2bf16 zmm2, word ptr [eax]{1to32}
// CHECK: encoding: [0x62,0xf2,0x7f,0x58,0x67,0x10]
               vcvtneph2bf16 zmm2, word ptr [eax]{1to32}

// CHECK:      vcvtneph2bf16 zmm2, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf2,0x7f,0x48,0x67,0x14,0x6d,0x00,0xf8,0xff,0xff]
               vcvtneph2bf16 zmm2, zmmword ptr [2*ebp - 2048]

// CHECK:      vcvtneph2bf16 zmm2 {k7} {z}, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf2,0x7f,0xcf,0x67,0x51,0x7f]
               vcvtneph2bf16 zmm2 {k7} {z}, zmmword ptr [ecx + 8128]

// CHECK:      vcvtneph2bf16 zmm2 {k7} {z}, word ptr [edx - 256]{1to32}
// CHECK: encoding: [0x62,0xf2,0x7f,0xdf,0x67,0x52,0x80]
               vcvtneph2bf16 zmm2 {k7} {z}, word ptr [edx - 256]{1to32}

