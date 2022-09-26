// REQUIRES: intel_feature_isa_avx_ne_convert
// RUN: llvm-mc -triple x86_64-unknown-unknown -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK:      {evex} vcvtne2ps2ph zmm22, zmm23, zmm24
// CHECK: encoding: [0x62,0x82,0x45,0x40,0x67,0xf0]
               {evex} vcvtne2ps2ph zmm22, zmm23, zmm24

// CHECK:      {evex} vcvtne2ps2ph zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x45,0x40,0x67,0xb4,0xf5,0x00,0x00,0x00,0x10]
               {evex} vcvtne2ps2ph zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      {evex} vcvtne2ps2ph zmm22, zmm23, dword ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe2,0x45,0x50,0x67,0x35,0x00,0x00,0x00,0x00]
               {evex} vcvtne2ps2ph zmm22, zmm23, dword ptr [rip]{1to16}

// CHECK:      {evex} vcvtne2ps2ph zmm22, zmm23, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe2,0x45,0x40,0x67,0x34,0x6d,0x00,0xf8,0xff,0xff]
               {evex} vcvtne2ps2ph zmm22, zmm23, zmmword ptr [2*rbp - 2048]

// CHECK:      {evex} vcvtne2ps2ph zmm22, zmm23, zmm24, {rn-sae}
// CHECK: encoding: [0x62,0x82,0x45,0x10,0x67,0xf0]
               {evex} vcvtne2ps2ph zmm22, zmm23, zmm24, {rn-sae}

// CHECK:      {evex} vcvtne2ps2ph zmm22, zmm23, zmm24, {rz-sae}
// CHECK: encoding: [0x62,0x82,0x45,0x70,0x67,0xf0]
               {evex} vcvtne2ps2ph zmm22, zmm23, zmm24, {rz-sae}

// CHECK:      {evex} vbcstnebf162ps zmm22, word ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x7e,0x48,0xb1,0xb4,0xf5,0x00,0x00,0x00,0x10]
               {evex} vbcstnebf162ps zmm22, word ptr [rbp + 8*r14 + 268435456]

// CHECK:      {evex} vbcstnebf162ps zmm22 {k7}, word ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x7e,0x4f,0xb1,0xb4,0x80,0x23,0x01,0x00,0x00]
               {evex} vbcstnebf162ps zmm22 {k7}, word ptr [r8 + 4*rax + 291]

// CHECK:      {evex} vbcstnebf162ps zmm22, word ptr [rip]
// CHECK: encoding: [0x62,0xe2,0x7e,0x48,0xb1,0x35,0x00,0x00,0x00,0x00]
               {evex} vbcstnebf162ps zmm22, word ptr [rip]

// CHECK:      {evex} vbcstnebf162ps zmm22, word ptr [2*rbp - 64]
// CHECK: encoding: [0x62,0xe2,0x7e,0x48,0xb1,0x34,0x6d,0xc0,0xff,0xff,0xff]
               {evex} vbcstnebf162ps zmm22, word ptr [2*rbp - 64]

// CHECK:      {evex} vbcstnesh2ps zmm22, word ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x7d,0x48,0xb1,0xb4,0xf5,0x00,0x00,0x00,0x10]
               {evex} vbcstnesh2ps zmm22, word ptr [rbp + 8*r14 + 268435456]

// CHECK:      {evex} vbcstnesh2ps zmm22 {k7}, word ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x7d,0x4f,0xb1,0xb4,0x80,0x23,0x01,0x00,0x00]
               {evex} vbcstnesh2ps zmm22 {k7}, word ptr [r8 + 4*rax + 291]

// CHECK:      {evex} vbcstnesh2ps zmm22, word ptr [rip]
// CHECK: encoding: [0x62,0xe2,0x7d,0x48,0xb1,0x35,0x00,0x00,0x00,0x00]
               {evex} vbcstnesh2ps zmm22, word ptr [rip]

// CHECK:      {evex} vbcstnesh2ps zmm22, word ptr [2*rbp - 64]
// CHECK: encoding: [0x62,0xe2,0x7d,0x48,0xb1,0x34,0x6d,0xc0,0xff,0xff,0xff]
               {evex} vbcstnesh2ps zmm22, word ptr [2*rbp - 64]

// CHECK:      {evex} vcvtneebf162ps zmm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x7e,0x48,0xb0,0xb4,0xf5,0x00,0x00,0x00,0x10]
               {evex} vcvtneebf162ps zmm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      {evex} vcvtneebf162ps zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x7e,0x4f,0xb0,0xb4,0x80,0x23,0x01,0x00,0x00]
               {evex} vcvtneebf162ps zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]

// CHECK:      {evex} vcvtneebf162ps zmm22, zmmword ptr [rip]
// CHECK: encoding: [0x62,0xe2,0x7e,0x48,0xb0,0x35,0x00,0x00,0x00,0x00]
               {evex} vcvtneebf162ps zmm22, zmmword ptr [rip]

// CHECK:      {evex} vcvtneebf162ps zmm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe2,0x7e,0x48,0xb0,0x34,0x6d,0x00,0xf8,0xff,0xff]
               {evex} vcvtneebf162ps zmm22, zmmword ptr [2*rbp - 2048]

// CHECK:      {evex} vcvtneebf162ps zmm22 {k7} {z}, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe2,0x7e,0xcf,0xb0,0x71,0x7f]
               {evex} vcvtneebf162ps zmm22 {k7} {z}, zmmword ptr [rcx + 8128]

// CHECK:      {evex} vcvtneebf162ps zmm22 {k7} {z}, zmmword ptr [rdx - 8192]
// CHECK: encoding: [0x62,0xe2,0x7e,0xcf,0xb0,0x72,0x80]
               {evex} vcvtneebf162ps zmm22 {k7} {z}, zmmword ptr [rdx - 8192]

// CHECK:      {evex} vcvtneeph2ps zmm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x7d,0x48,0xb0,0xb4,0xf5,0x00,0x00,0x00,0x10]
               {evex} vcvtneeph2ps zmm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      {evex} vcvtneeph2ps zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x7d,0x4f,0xb0,0xb4,0x80,0x23,0x01,0x00,0x00]
               {evex} vcvtneeph2ps zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]

// CHECK:      {evex} vcvtneeph2ps zmm22, zmmword ptr [rip]
// CHECK: encoding: [0x62,0xe2,0x7d,0x48,0xb0,0x35,0x00,0x00,0x00,0x00]
               {evex} vcvtneeph2ps zmm22, zmmword ptr [rip]

// CHECK:      {evex} vcvtneeph2ps zmm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe2,0x7d,0x48,0xb0,0x34,0x6d,0x00,0xf8,0xff,0xff]
               {evex} vcvtneeph2ps zmm22, zmmword ptr [2*rbp - 2048]

// CHECK:      {evex} vcvtneeph2ps zmm22 {k7} {z}, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe2,0x7d,0xcf,0xb0,0x71,0x7f]
               {evex} vcvtneeph2ps zmm22 {k7} {z}, zmmword ptr [rcx + 8128]

// CHECK:      {evex} vcvtneeph2ps zmm22 {k7} {z}, zmmword ptr [rdx - 8192]
// CHECK: encoding: [0x62,0xe2,0x7d,0xcf,0xb0,0x72,0x80]
               {evex} vcvtneeph2ps zmm22 {k7} {z}, zmmword ptr [rdx - 8192]

// CHECK:      {evex} vcvtneobf162ps zmm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x7f,0x48,0xb0,0xb4,0xf5,0x00,0x00,0x00,0x10]
               {evex} vcvtneobf162ps zmm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      {evex} vcvtneobf162ps zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x7f,0x4f,0xb0,0xb4,0x80,0x23,0x01,0x00,0x00]
               {evex} vcvtneobf162ps zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]

// CHECK:      {evex} vcvtneobf162ps zmm22, zmmword ptr [rip]
// CHECK: encoding: [0x62,0xe2,0x7f,0x48,0xb0,0x35,0x00,0x00,0x00,0x00]
               {evex} vcvtneobf162ps zmm22, zmmword ptr [rip]

// CHECK:      {evex} vcvtneobf162ps zmm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe2,0x7f,0x48,0xb0,0x34,0x6d,0x00,0xf8,0xff,0xff]
               {evex} vcvtneobf162ps zmm22, zmmword ptr [2*rbp - 2048]

// CHECK:      {evex} vcvtneobf162ps zmm22 {k7} {z}, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe2,0x7f,0xcf,0xb0,0x71,0x7f]
               {evex} vcvtneobf162ps zmm22 {k7} {z}, zmmword ptr [rcx + 8128]

// CHECK:      {evex} vcvtneobf162ps zmm22 {k7} {z}, zmmword ptr [rdx - 8192]
// CHECK: encoding: [0x62,0xe2,0x7f,0xcf,0xb0,0x72,0x80]
               {evex} vcvtneobf162ps zmm22 {k7} {z}, zmmword ptr [rdx - 8192]

// CHECK:      {evex} vcvtneoph2ps zmm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x7c,0x48,0xb0,0xb4,0xf5,0x00,0x00,0x00,0x10]
               {evex} vcvtneoph2ps zmm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      {evex} vcvtneoph2ps zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x7c,0x4f,0xb0,0xb4,0x80,0x23,0x01,0x00,0x00]
               {evex} vcvtneoph2ps zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]

// CHECK:      {evex} vcvtneoph2ps zmm22, zmmword ptr [rip]
// CHECK: encoding: [0x62,0xe2,0x7c,0x48,0xb0,0x35,0x00,0x00,0x00,0x00]
               {evex} vcvtneoph2ps zmm22, zmmword ptr [rip]

// CHECK:      {evex} vcvtneoph2ps zmm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe2,0x7c,0x48,0xb0,0x34,0x6d,0x00,0xf8,0xff,0xff]
               {evex} vcvtneoph2ps zmm22, zmmword ptr [2*rbp - 2048]

// CHECK:      {evex} vcvtneoph2ps zmm22 {k7} {z}, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe2,0x7c,0xcf,0xb0,0x71,0x7f]
               {evex} vcvtneoph2ps zmm22 {k7} {z}, zmmword ptr [rcx + 8128]

// CHECK:      {evex} vcvtneoph2ps zmm22 {k7} {z}, zmmword ptr [rdx - 8192]
// CHECK: encoding: [0x62,0xe2,0x7c,0xcf,0xb0,0x72,0x80]
               {evex} vcvtneoph2ps zmm22 {k7} {z}, zmmword ptr [rdx - 8192]

