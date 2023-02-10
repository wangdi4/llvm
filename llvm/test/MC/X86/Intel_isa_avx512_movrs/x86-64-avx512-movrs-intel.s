// REQUIRES: intel_feature_isa_avx512_movrs
// RUN: llvm-mc -triple x86_64-unknown-unknown -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK:      {evex} vmovadvisew zmmword ptr [rbp + 8*r14 + 268435456], zmm30, 123
// CHECK: encoding: [0x62,0x23,0x7e,0x48,0x11,0xb4,0xf5,0x00,0x00,0x00,0x10,0x7b]
               vmovadvisew zmmword ptr [rbp + 8*r14 + 268435456], zmm30, 123

// CHECK:      {evex} vmovadvisew zmmword ptr [r9], zmm30, 123
// CHECK: encoding: [0x62,0x43,0x7e,0x48,0x11,0x31,0x7b]
               vmovadvisew zmmword ptr [r9], zmm30, 123

// CHECK:      {evex} vmovadvisew zmmword ptr [rcx + 8128], zmm30, 123
// CHECK: encoding: [0x62,0x63,0x7e,0x48,0x11,0x71,0x7f,0x7b]
               vmovadvisew zmmword ptr [rcx + 8128], zmm30, 123

// CHECK:      {evex} vmovadvisew zmmword ptr [rdx - 8192], zmm30, 123
// CHECK: encoding: [0x62,0x63,0x7e,0x48,0x11,0x72,0x80,0x7b]
               vmovadvisew zmmword ptr [rdx - 8192], zmm30, 123

// CHECK:      {evex} vmovadvisew zmm30, zmmword ptr [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0x62,0x23,0x7e,0x48,0x10,0xb4,0xf5,0x00,0x00,0x00,0x10,0x7b]
               vmovadvisew zmm30, zmmword ptr [rbp + 8*r14 + 268435456], 123

// CHECK:      {evex} vmovadvisew zmm30, zmmword ptr [r9], 123
// CHECK: encoding: [0x62,0x43,0x7e,0x48,0x10,0x31,0x7b]
               vmovadvisew zmm30, zmmword ptr [r9], 123

// CHECK:      {evex} vmovadvisew zmm30, zmmword ptr [rcx + 8128], 123
// CHECK: encoding: [0x62,0x63,0x7e,0x48,0x10,0x71,0x7f,0x7b]
               vmovadvisew zmm30, zmmword ptr [rcx + 8128], 123

// CHECK:      {evex} vmovadvisew zmm30, zmmword ptr [rdx - 8192], 123
// CHECK: encoding: [0x62,0x63,0x7e,0x48,0x10,0x72,0x80,0x7b]
               vmovadvisew zmm30, zmmword ptr [rdx - 8192], 123

// CHECK:      {evex} vmovadvisew zmmword ptr [rbp + 8*r14 + 268435456], zmm30, 123
// CHECK: encoding: [0x62,0x23,0x7e,0x48,0x11,0xb4,0xf5,0x00,0x00,0x00,0x10,0x7b]
               vmovadvisew zmmword ptr [rbp + 8*r14 + 268435456], zmm30, 123

// CHECK:      {evex} vmovadvisew zmmword ptr [r9], zmm30, 123
// CHECK: encoding: [0x62,0x43,0x7e,0x48,0x11,0x31,0x7b]
               vmovadvisew zmmword ptr [r9], zmm30, 123

// CHECK:      {evex} vmovadvisew zmmword ptr [rcx + 8128], zmm30, 123
// CHECK: encoding: [0x62,0x63,0x7e,0x48,0x11,0x71,0x7f,0x7b]
               vmovadvisew zmmword ptr [rcx + 8128], zmm30, 123

// CHECK:      {evex} vmovadvisew zmmword ptr [rdx - 8192], zmm30, 123
// CHECK: encoding: [0x62,0x63,0x7e,0x48,0x11,0x72,0x80,0x7b]
               vmovadvisew zmmword ptr [rdx - 8192], zmm30, 123

// CHECK:      {evex} vmovadvisew zmm30, zmmword ptr [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0x62,0x23,0x7e,0x48,0x10,0xb4,0xf5,0x00,0x00,0x00,0x10,0x7b]
               vmovadvisew zmm30, zmmword ptr [rbp + 8*r14 + 268435456], 123

// CHECK:      {evex} vmovadvisew zmm30, zmmword ptr [r9], 123
// CHECK: encoding: [0x62,0x43,0x7e,0x48,0x10,0x31,0x7b]
               vmovadvisew zmm30, zmmword ptr [r9], 123

// CHECK:      {evex} vmovadvisew zmm30, zmmword ptr [rcx + 8128], 123
// CHECK: encoding: [0x62,0x63,0x7e,0x48,0x10,0x71,0x7f,0x7b]
               vmovadvisew zmm30, zmmword ptr [rcx + 8128], 123

// CHECK:      {evex} vmovadvisew zmm30, zmmword ptr [rdx - 8192], 123
// CHECK: encoding: [0x62,0x63,0x7e,0x48,0x10,0x72,0x80,0x7b]
               vmovadvisew zmm30, zmmword ptr [rdx - 8192], 123

// CHECK:      {evex} vmovadvisew xmm12, xmmword ptr [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0x62,0x33,0x7e,0x08,0x10,0xa4,0xf5,0x00,0x00,0x00,0x10,0x7b]
               {evex} vmovadvisew xmm12, xmmword ptr [rbp + 8*r14 + 268435456], 123

// CHECK:      {evex} vmovadvisew xmm30, xmmword ptr [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0x62,0x23,0x7e,0x08,0x10,0xb4,0xf5,0x00,0x00,0x00,0x10,0x7b]
               vmovadvisew xmm30, xmmword ptr [rbp + 8*r14 + 268435456], 123

// CHECK:      {evex} vmovadvisew xmm30, xmmword ptr [r9], 123
// CHECK: encoding: [0x62,0x43,0x7e,0x08,0x10,0x31,0x7b]
               vmovadvisew xmm30, xmmword ptr [r9], 123

// CHECK:      {evex} vmovadvisew ymm12, ymmword ptr [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0x62,0x33,0x7e,0x28,0x10,0xa4,0xf5,0x00,0x00,0x00,0x10,0x7b]
               {evex} vmovadvisew ymm12, ymmword ptr [rbp + 8*r14 + 268435456], 123

// CHECK:      {evex} vmovadvisew ymm30, ymmword ptr [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0x62,0x23,0x7e,0x28,0x10,0xb4,0xf5,0x00,0x00,0x00,0x10,0x7b]
               vmovadvisew ymm30, ymmword ptr [rbp + 8*r14 + 268435456], 123

// CHECK:      {evex} vmovadvisew ymm30, ymmword ptr [r9], 123
// CHECK: encoding: [0x62,0x43,0x7e,0x28,0x10,0x31,0x7b]
               vmovadvisew ymm30, ymmword ptr [r9], 123

// CHECK:      {evex} vmovadvisew xmmword ptr [rbp + 8*r14 + 268435456], xmm30, 123
// CHECK: encoding: [0x62,0x23,0x7e,0x08,0x11,0xb4,0xf5,0x00,0x00,0x00,0x10,0x7b]
               vmovadvisew xmmword ptr [rbp + 8*r14 + 268435456], xmm30, 123

// CHECK:      {evex} vmovadvisew xmmword ptr [r9], xmm30, 123
// CHECK: encoding: [0x62,0x43,0x7e,0x08,0x11,0x31,0x7b]
               vmovadvisew xmmword ptr [r9], xmm30, 123

// CHECK:      {evex} vmovadvisew ymmword ptr [rbp + 8*r14 + 268435456], ymm30, 123
// CHECK: encoding: [0x62,0x23,0x7e,0x28,0x11,0xb4,0xf5,0x00,0x00,0x00,0x10,0x7b]
               vmovadvisew ymmword ptr [rbp + 8*r14 + 268435456], ymm30, 123

// CHECK:      {evex} vmovadvisew ymmword ptr [r9], ymm30, 123
// CHECK: encoding: [0x62,0x43,0x7e,0x28,0x11,0x31,0x7b]
               vmovadvisew ymmword ptr [r9], ymm30, 123

// CHECK:      {evex} vmemadvise zmmword ptr [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0x62,0xb1,0x7f,0x48,0x71,0x84,0xf5,0x00,0x00,0x00,0x10,0x7b]
               vmemadvise zmmword ptr [rbp + 8*r14 + 268435456], 123

// CHECK:      {evex} vmemadvise zmmword ptr [r8 + 4*rax + 291], 123
// CHECK: encoding: [0x62,0xd1,0x7f,0x48,0x71,0x84,0x80,0x23,0x01,0x00,0x00,0x7b]
               vmemadvise zmmword ptr [r8 + 4*rax + 291], 123

// CHECK:      {evex} vmemadvise zmmword ptr [rip], 123
// CHECK: encoding: [0x62,0xf1,0x7f,0x48,0x71,0x05,0x00,0x00,0x00,0x00,0x7b]
               vmemadvise zmmword ptr [rip], 123

// CHECK:      {evex} vmemadvise zmmword ptr [2*rbp - 2048], 123
// CHECK: encoding: [0x62,0xf1,0x7f,0x48,0x71,0x04,0x6d,0x00,0xf8,0xff,0xff,0x7b]
               vmemadvise zmmword ptr [2*rbp - 2048], 123

// CHECK:      {evex} vmemadvise zmmword ptr [rcx + 8128], 123
// CHECK: encoding: [0x62,0xf1,0x7f,0x48,0x71,0x41,0x7f,0x7b]
               vmemadvise zmmword ptr [rcx + 8128], 123

// CHECK:      {evex} vmemadvise zmmword ptr [rdx - 8192], 123
// CHECK: encoding: [0x62,0xf1,0x7f,0x48,0x71,0x42,0x80,0x7b]
               vmemadvise zmmword ptr [rdx - 8192], 123

// CHECK:      {evex} vmemadvise xmmword ptr [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0x62,0xb1,0x7f,0x08,0x71,0x84,0xf5,0x00,0x00,0x00,0x10,0x7b]
               {evex} vmemadvise xmmword ptr [rbp + 8*r14 + 268435456], 123

// CHECK:      {evex} vmemadvise xmmword ptr [r8 + 4*rax + 291], 123
// CHECK: encoding: [0x62,0xd1,0x7f,0x08,0x71,0x84,0x80,0x23,0x01,0x00,0x00,0x7b]
               {evex} vmemadvise xmmword ptr [r8 + 4*rax + 291], 123

// CHECK:      {evex} vmemadvise xmmword ptr [rip], 123
// CHECK: encoding: [0x62,0xf1,0x7f,0x08,0x71,0x05,0x00,0x00,0x00,0x00,0x7b]
               {evex} vmemadvise xmmword ptr [rip], 123

// CHECK:      {evex} vmemadvise xmmword ptr [2*rbp - 512], 123
// CHECK: encoding: [0x62,0xf1,0x7f,0x08,0x71,0x04,0x6d,0x00,0xfe,0xff,0xff,0x7b]
               {evex} vmemadvise xmmword ptr [2*rbp - 512], 123

// CHECK:      {evex} vmemadvise xmmword ptr [rcx + 2032], 123
// CHECK: encoding: [0x62,0xf1,0x7f,0x08,0x71,0x41,0x7f,0x7b]
               {evex} vmemadvise xmmword ptr [rcx + 2032], 123

// CHECK:      {evex} vmemadvise xmmword ptr [rdx - 2048], 123
// CHECK: encoding: [0x62,0xf1,0x7f,0x08,0x71,0x42,0x80,0x7b]
               {evex} vmemadvise xmmword ptr [rdx - 2048], 123

// CHECK:      {evex} vmemadvise ymmword ptr [2*rbp - 1024], 123
// CHECK: encoding: [0x62,0xf1,0x7f,0x28,0x71,0x04,0x6d,0x00,0xfc,0xff,0xff,0x7b]
               {evex} vmemadvise ymmword ptr [2*rbp - 1024], 123

// CHECK:      {evex} vmemadvise ymmword ptr [rcx + 4064], 123
// CHECK: encoding: [0x62,0xf1,0x7f,0x28,0x71,0x41,0x7f,0x7b]
               {evex} vmemadvise ymmword ptr [rcx + 4064], 123

// CHECK:      {evex} vmemadvise ymmword ptr [rdx - 4096], 123
// CHECK: encoding: [0x62,0xf1,0x7f,0x28,0x71,0x42,0x80,0x7b]
               {evex} vmemadvise ymmword ptr [rdx - 4096], 123

// CHECK: vmovrsb zmm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7f,0x48,0x6f,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vmovrsb zmm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vmovrsb zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7f,0x4f,0x6f,0xb4,0x80,0x23,0x01,0x00,0x00]
          vmovrsb zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vmovrsb zmm22, zmmword ptr [rip]
// CHECK: encoding: [0x62,0xe5,0x7f,0x48,0x6f,0x35,0x00,0x00,0x00,0x00]
          vmovrsb zmm22, zmmword ptr [rip]

// CHECK: vmovrsb zmm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0x7f,0x48,0x6f,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vmovrsb zmm22, zmmword ptr [2*rbp - 2048]

// CHECK: vmovrsb zmm22 {k7} {z}, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0x7f,0xcf,0x6f,0x71,0x7f]
          vmovrsb zmm22 {k7} {z}, zmmword ptr [rcx + 8128]

// CHECK: vmovrsb zmm22 {k7} {z}, zmmword ptr [rdx - 8192]
// CHECK: encoding: [0x62,0xe5,0x7f,0xcf,0x6f,0x72,0x80]
          vmovrsb zmm22 {k7} {z}, zmmword ptr [rdx - 8192]

// CHECK: vmovrsd zmm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7e,0x48,0x6f,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vmovrsd zmm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vmovrsd zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7e,0x4f,0x6f,0xb4,0x80,0x23,0x01,0x00,0x00]
          vmovrsd zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vmovrsd zmm22, zmmword ptr [rip]
// CHECK: encoding: [0x62,0xe5,0x7e,0x48,0x6f,0x35,0x00,0x00,0x00,0x00]
          vmovrsd zmm22, zmmword ptr [rip]

// CHECK: vmovrsd zmm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0x7e,0x48,0x6f,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vmovrsd zmm22, zmmword ptr [2*rbp - 2048]

// CHECK: vmovrsd zmm22 {k7} {z}, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0x7e,0xcf,0x6f,0x71,0x7f]
          vmovrsd zmm22 {k7} {z}, zmmword ptr [rcx + 8128]

// CHECK: vmovrsd zmm22 {k7} {z}, zmmword ptr [rdx - 8192]
// CHECK: encoding: [0x62,0xe5,0x7e,0xcf,0x6f,0x72,0x80]
          vmovrsd zmm22 {k7} {z}, zmmword ptr [rdx - 8192]

// CHECK: vmovrsq zmm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0xfe,0x48,0x6f,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vmovrsq zmm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vmovrsq zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0xfe,0x4f,0x6f,0xb4,0x80,0x23,0x01,0x00,0x00]
          vmovrsq zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vmovrsq zmm22, zmmword ptr [rip]
// CHECK: encoding: [0x62,0xe5,0xfe,0x48,0x6f,0x35,0x00,0x00,0x00,0x00]
          vmovrsq zmm22, zmmword ptr [rip]

// CHECK: vmovrsq zmm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0xfe,0x48,0x6f,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vmovrsq zmm22, zmmword ptr [2*rbp - 2048]

// CHECK: vmovrsq zmm22 {k7} {z}, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0xfe,0xcf,0x6f,0x71,0x7f]
          vmovrsq zmm22 {k7} {z}, zmmword ptr [rcx + 8128]

// CHECK: vmovrsq zmm22 {k7} {z}, zmmword ptr [rdx - 8192]
// CHECK: encoding: [0x62,0xe5,0xfe,0xcf,0x6f,0x72,0x80]
          vmovrsq zmm22 {k7} {z}, zmmword ptr [rdx - 8192]

// CHECK: vmovrsw zmm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0xff,0x48,0x6f,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vmovrsw zmm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vmovrsw zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0xff,0x4f,0x6f,0xb4,0x80,0x23,0x01,0x00,0x00]
          vmovrsw zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vmovrsw zmm22, zmmword ptr [rip]
// CHECK: encoding: [0x62,0xe5,0xff,0x48,0x6f,0x35,0x00,0x00,0x00,0x00]
          vmovrsw zmm22, zmmword ptr [rip]

// CHECK: vmovrsw zmm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0xff,0x48,0x6f,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vmovrsw zmm22, zmmword ptr [2*rbp - 2048]

// CHECK: vmovrsw zmm22 {k7} {z}, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0xff,0xcf,0x6f,0x71,0x7f]
          vmovrsw zmm22 {k7} {z}, zmmword ptr [rcx + 8128]

// CHECK: vmovrsw zmm22 {k7} {z}, zmmword ptr [rdx - 8192]
// CHECK: encoding: [0x62,0xe5,0xff,0xcf,0x6f,0x72,0x80]
          vmovrsw zmm22 {k7} {z}, zmmword ptr [rdx - 8192]
