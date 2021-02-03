// REQUIRES: intel_feature_isa_avx_rao_int
// RUN: llvm-mc -triple i686-unknown-unknown -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK:      vpaaddd zmmword ptr [esp + 8*esi + 268435456], zmm2
// CHECK: encoding: [0x62,0xf2,0x7c,0x48,0xfc,0x94,0xf4,0x00,0x00,0x00,0x10]
               vpaaddd zmmword ptr [esp + 8*esi + 268435456], zmm2

// CHECK:      vpaaddd zmmword ptr [edi + 4*eax + 291] {k7}, zmm2
// CHECK: encoding: [0x62,0xf2,0x7c,0x4f,0xfc,0x94,0x87,0x23,0x01,0x00,0x00]
               vpaaddd zmmword ptr [edi + 4*eax + 291] {k7}, zmm2

// CHECK:      vpaaddd zmmword ptr [eax], zmm2
// CHECK: encoding: [0x62,0xf2,0x7c,0x48,0xfc,0x10]
               vpaaddd zmmword ptr [eax], zmm2

// CHECK:      vpaaddd zmmword ptr [2*ebp - 2048], zmm2
// CHECK: encoding: [0x62,0xf2,0x7c,0x48,0xfc,0x14,0x6d,0x00,0xf8,0xff,0xff]
               vpaaddd zmmword ptr [2*ebp - 2048], zmm2

// CHECK:      vpaaddd zmmword ptr [ecx + 8128] {k7}, zmm2
// CHECK: encoding: [0x62,0xf2,0x7c,0x4f,0xfc,0x51,0x7f]
               vpaaddd zmmword ptr [ecx + 8128] {k7}, zmm2

// CHECK:      vpaaddd zmmword ptr [edx - 8192] {k7}, zmm2
// CHECK: encoding: [0x62,0xf2,0x7c,0x4f,0xfc,0x52,0x80]
               vpaaddd zmmword ptr [edx - 8192] {k7}, zmm2

// CHECK:      vpaaddq zmmword ptr [esp + 8*esi + 268435456], zmm2
// CHECK: encoding: [0x62,0xf2,0xfc,0x48,0xfc,0x94,0xf4,0x00,0x00,0x00,0x10]
               vpaaddq zmmword ptr [esp + 8*esi + 268435456], zmm2

// CHECK:      vpaaddq zmmword ptr [edi + 4*eax + 291] {k7}, zmm2
// CHECK: encoding: [0x62,0xf2,0xfc,0x4f,0xfc,0x94,0x87,0x23,0x01,0x00,0x00]
               vpaaddq zmmword ptr [edi + 4*eax + 291] {k7}, zmm2

// CHECK:      vpaaddq zmmword ptr [eax], zmm2
// CHECK: encoding: [0x62,0xf2,0xfc,0x48,0xfc,0x10]
               vpaaddq zmmword ptr [eax], zmm2

// CHECK:      vpaaddq zmmword ptr [2*ebp - 2048], zmm2
// CHECK: encoding: [0x62,0xf2,0xfc,0x48,0xfc,0x14,0x6d,0x00,0xf8,0xff,0xff]
               vpaaddq zmmword ptr [2*ebp - 2048], zmm2

// CHECK:      vpaaddq zmmword ptr [ecx + 8128] {k7}, zmm2
// CHECK: encoding: [0x62,0xf2,0xfc,0x4f,0xfc,0x51,0x7f]
               vpaaddq zmmword ptr [ecx + 8128] {k7}, zmm2

// CHECK:      vpaaddq zmmword ptr [edx - 8192] {k7}, zmm2
// CHECK: encoding: [0x62,0xf2,0xfc,0x4f,0xfc,0x52,0x80]
               vpaaddq zmmword ptr [edx - 8192] {k7}, zmm2

// CHECK:      vpaandd zmmword ptr [esp + 8*esi + 268435456], zmm2
// CHECK: encoding: [0x62,0xf2,0x7d,0x48,0xfc,0x94,0xf4,0x00,0x00,0x00,0x10]
               vpaandd zmmword ptr [esp + 8*esi + 268435456], zmm2

// CHECK:      vpaandd zmmword ptr [edi + 4*eax + 291] {k7}, zmm2
// CHECK: encoding: [0x62,0xf2,0x7d,0x4f,0xfc,0x94,0x87,0x23,0x01,0x00,0x00]
               vpaandd zmmword ptr [edi + 4*eax + 291] {k7}, zmm2

// CHECK:      vpaandd zmmword ptr [eax], zmm2
// CHECK: encoding: [0x62,0xf2,0x7d,0x48,0xfc,0x10]
               vpaandd zmmword ptr [eax], zmm2

// CHECK:      vpaandd zmmword ptr [2*ebp - 2048], zmm2
// CHECK: encoding: [0x62,0xf2,0x7d,0x48,0xfc,0x14,0x6d,0x00,0xf8,0xff,0xff]
               vpaandd zmmword ptr [2*ebp - 2048], zmm2

// CHECK:      vpaandd zmmword ptr [ecx + 8128] {k7}, zmm2
// CHECK: encoding: [0x62,0xf2,0x7d,0x4f,0xfc,0x51,0x7f]
               vpaandd zmmword ptr [ecx + 8128] {k7}, zmm2

// CHECK:      vpaandd zmmword ptr [edx - 8192] {k7}, zmm2
// CHECK: encoding: [0x62,0xf2,0x7d,0x4f,0xfc,0x52,0x80]
               vpaandd zmmword ptr [edx - 8192] {k7}, zmm2

// CHECK:      vpaandq zmmword ptr [esp + 8*esi + 268435456], zmm2
// CHECK: encoding: [0x62,0xf2,0xfd,0x48,0xfc,0x94,0xf4,0x00,0x00,0x00,0x10]
               vpaandq zmmword ptr [esp + 8*esi + 268435456], zmm2

// CHECK:      vpaandq zmmword ptr [edi + 4*eax + 291] {k7}, zmm2
// CHECK: encoding: [0x62,0xf2,0xfd,0x4f,0xfc,0x94,0x87,0x23,0x01,0x00,0x00]
               vpaandq zmmword ptr [edi + 4*eax + 291] {k7}, zmm2

// CHECK:      vpaandq zmmword ptr [eax], zmm2
// CHECK: encoding: [0x62,0xf2,0xfd,0x48,0xfc,0x10]
               vpaandq zmmword ptr [eax], zmm2

// CHECK:      vpaandq zmmword ptr [2*ebp - 2048], zmm2
// CHECK: encoding: [0x62,0xf2,0xfd,0x48,0xfc,0x14,0x6d,0x00,0xf8,0xff,0xff]
               vpaandq zmmword ptr [2*ebp - 2048], zmm2

// CHECK:      vpaandq zmmword ptr [ecx + 8128] {k7}, zmm2
// CHECK: encoding: [0x62,0xf2,0xfd,0x4f,0xfc,0x51,0x7f]
               vpaandq zmmword ptr [ecx + 8128] {k7}, zmm2

// CHECK:      vpaandq zmmword ptr [edx - 8192] {k7}, zmm2
// CHECK: encoding: [0x62,0xf2,0xfd,0x4f,0xfc,0x52,0x80]
               vpaandq zmmword ptr [edx - 8192] {k7}, zmm2

// CHECK:      vpaord zmmword ptr [esp + 8*esi + 268435456], zmm2
// CHECK: encoding: [0x62,0xf2,0x7f,0x48,0xfc,0x94,0xf4,0x00,0x00,0x00,0x10]
               vpaord zmmword ptr [esp + 8*esi + 268435456], zmm2

// CHECK:      vpaord zmmword ptr [edi + 4*eax + 291] {k7}, zmm2
// CHECK: encoding: [0x62,0xf2,0x7f,0x4f,0xfc,0x94,0x87,0x23,0x01,0x00,0x00]
               vpaord zmmword ptr [edi + 4*eax + 291] {k7}, zmm2

// CHECK:      vpaord zmmword ptr [eax], zmm2
// CHECK: encoding: [0x62,0xf2,0x7f,0x48,0xfc,0x10]
               vpaord zmmword ptr [eax], zmm2

// CHECK:      vpaord zmmword ptr [2*ebp - 2048], zmm2
// CHECK: encoding: [0x62,0xf2,0x7f,0x48,0xfc,0x14,0x6d,0x00,0xf8,0xff,0xff]
               vpaord zmmword ptr [2*ebp - 2048], zmm2

// CHECK:      vpaord zmmword ptr [ecx + 8128] {k7}, zmm2
// CHECK: encoding: [0x62,0xf2,0x7f,0x4f,0xfc,0x51,0x7f]
               vpaord zmmword ptr [ecx + 8128] {k7}, zmm2

// CHECK:      vpaord zmmword ptr [edx - 8192] {k7}, zmm2
// CHECK: encoding: [0x62,0xf2,0x7f,0x4f,0xfc,0x52,0x80]
               vpaord zmmword ptr [edx - 8192] {k7}, zmm2

// CHECK:      vpaorq zmmword ptr [esp + 8*esi + 268435456], zmm2
// CHECK: encoding: [0x62,0xf2,0xff,0x48,0xfc,0x94,0xf4,0x00,0x00,0x00,0x10]
               vpaorq zmmword ptr [esp + 8*esi + 268435456], zmm2

// CHECK:      vpaorq zmmword ptr [edi + 4*eax + 291] {k7}, zmm2
// CHECK: encoding: [0x62,0xf2,0xff,0x4f,0xfc,0x94,0x87,0x23,0x01,0x00,0x00]
               vpaorq zmmword ptr [edi + 4*eax + 291] {k7}, zmm2

// CHECK:      vpaorq zmmword ptr [eax], zmm2
// CHECK: encoding: [0x62,0xf2,0xff,0x48,0xfc,0x10]
               vpaorq zmmword ptr [eax], zmm2

// CHECK:      vpaorq zmmword ptr [2*ebp - 2048], zmm2
// CHECK: encoding: [0x62,0xf2,0xff,0x48,0xfc,0x14,0x6d,0x00,0xf8,0xff,0xff]
               vpaorq zmmword ptr [2*ebp - 2048], zmm2

// CHECK:      vpaorq zmmword ptr [ecx + 8128] {k7}, zmm2
// CHECK: encoding: [0x62,0xf2,0xff,0x4f,0xfc,0x51,0x7f]
               vpaorq zmmword ptr [ecx + 8128] {k7}, zmm2

// CHECK:      vpaorq zmmword ptr [edx - 8192] {k7}, zmm2
// CHECK: encoding: [0x62,0xf2,0xff,0x4f,0xfc,0x52,0x80]
               vpaorq zmmword ptr [edx - 8192] {k7}, zmm2

// CHECK:      vpaxord zmmword ptr [esp + 8*esi + 268435456], zmm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x48,0xfc,0x94,0xf4,0x00,0x00,0x00,0x10]
               vpaxord zmmword ptr [esp + 8*esi + 268435456], zmm2

// CHECK:      vpaxord zmmword ptr [edi + 4*eax + 291] {k7}, zmm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x4f,0xfc,0x94,0x87,0x23,0x01,0x00,0x00]
               vpaxord zmmword ptr [edi + 4*eax + 291] {k7}, zmm2

// CHECK:      vpaxord zmmword ptr [eax], zmm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x48,0xfc,0x10]
               vpaxord zmmword ptr [eax], zmm2

// CHECK:      vpaxord zmmword ptr [2*ebp - 2048], zmm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x48,0xfc,0x14,0x6d,0x00,0xf8,0xff,0xff]
               vpaxord zmmword ptr [2*ebp - 2048], zmm2

// CHECK:      vpaxord zmmword ptr [ecx + 8128] {k7}, zmm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x4f,0xfc,0x51,0x7f]
               vpaxord zmmword ptr [ecx + 8128] {k7}, zmm2

// CHECK:      vpaxord zmmword ptr [edx - 8192] {k7}, zmm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x4f,0xfc,0x52,0x80]
               vpaxord zmmword ptr [edx - 8192] {k7}, zmm2

// CHECK:      vpaxorq zmmword ptr [esp + 8*esi + 268435456], zmm2
// CHECK: encoding: [0x62,0xf2,0xfe,0x48,0xfc,0x94,0xf4,0x00,0x00,0x00,0x10]
               vpaxorq zmmword ptr [esp + 8*esi + 268435456], zmm2

// CHECK:      vpaxorq zmmword ptr [edi + 4*eax + 291] {k7}, zmm2
// CHECK: encoding: [0x62,0xf2,0xfe,0x4f,0xfc,0x94,0x87,0x23,0x01,0x00,0x00]
               vpaxorq zmmword ptr [edi + 4*eax + 291] {k7}, zmm2

// CHECK:      vpaxorq zmmword ptr [eax], zmm2
// CHECK: encoding: [0x62,0xf2,0xfe,0x48,0xfc,0x10]
               vpaxorq zmmword ptr [eax], zmm2

// CHECK:      vpaxorq zmmword ptr [2*ebp - 2048], zmm2
// CHECK: encoding: [0x62,0xf2,0xfe,0x48,0xfc,0x14,0x6d,0x00,0xf8,0xff,0xff]
               vpaxorq zmmword ptr [2*ebp - 2048], zmm2

// CHECK:      vpaxorq zmmword ptr [ecx + 8128] {k7}, zmm2
// CHECK: encoding: [0x62,0xf2,0xfe,0x4f,0xfc,0x51,0x7f]
               vpaxorq zmmword ptr [ecx + 8128] {k7}, zmm2

// CHECK:      vpaxorq zmmword ptr [edx - 8192] {k7}, zmm2
// CHECK: encoding: [0x62,0xf2,0xfe,0x4f,0xfc,0x52,0x80]
               vpaxorq zmmword ptr [edx - 8192] {k7}, zmm2

