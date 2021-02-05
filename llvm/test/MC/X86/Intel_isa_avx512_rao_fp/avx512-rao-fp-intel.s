// REQUIRES: intel_feature_isa_avx_rao_fp
// RUN: llvm-mc -triple i686-unknown-unknown -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK:      vaaddpbf16 zmmword ptr [esp + 8*esi + 268435456], zmm2
// CHECK: encoding: [0x62,0xf2,0x7d,0x48,0x94,0x94,0xf4,0x00,0x00,0x00,0x10]
               vaaddpbf16 zmmword ptr [esp + 8*esi + 268435456], zmm2

// CHECK:      vaaddpbf16 zmmword ptr [edi + 4*eax + 291] {k7}, zmm2
// CHECK: encoding: [0x62,0xf2,0x7d,0x4f,0x94,0x94,0x87,0x23,0x01,0x00,0x00]
               vaaddpbf16 zmmword ptr [edi + 4*eax + 291] {k7}, zmm2

// CHECK:      vaaddpbf16 zmmword ptr [eax], zmm2
// CHECK: encoding: [0x62,0xf2,0x7d,0x48,0x94,0x10]
               vaaddpbf16 zmmword ptr [eax], zmm2

// CHECK:      vaaddpbf16 zmmword ptr [2*ebp - 2048], zmm2
// CHECK: encoding: [0x62,0xf2,0x7d,0x48,0x94,0x14,0x6d,0x00,0xf8,0xff,0xff]
               vaaddpbf16 zmmword ptr [2*ebp - 2048], zmm2

// CHECK:      vaaddpbf16 zmmword ptr [ecx + 8128] {k7}, zmm2
// CHECK: encoding: [0x62,0xf2,0x7d,0x4f,0x94,0x51,0x7f]
               vaaddpbf16 zmmword ptr [ecx + 8128] {k7}, zmm2

// CHECK:      vaaddpbf16 zmmword ptr [edx - 8192] {k7}, zmm2
// CHECK: encoding: [0x62,0xf2,0x7d,0x4f,0x94,0x52,0x80]
               vaaddpbf16 zmmword ptr [edx - 8192] {k7}, zmm2

// CHECK:      vaaddpd zmmword ptr [esp + 8*esi + 268435456], zmm2
// CHECK: encoding: [0x62,0xf2,0xfd,0x48,0x84,0x94,0xf4,0x00,0x00,0x00,0x10]
               vaaddpd zmmword ptr [esp + 8*esi + 268435456], zmm2

// CHECK:      vaaddpd zmmword ptr [edi + 4*eax + 291] {k7}, zmm2
// CHECK: encoding: [0x62,0xf2,0xfd,0x4f,0x84,0x94,0x87,0x23,0x01,0x00,0x00]
               vaaddpd zmmword ptr [edi + 4*eax + 291] {k7}, zmm2

// CHECK:      vaaddpd zmmword ptr [eax], zmm2
// CHECK: encoding: [0x62,0xf2,0xfd,0x48,0x84,0x10]
               vaaddpd zmmword ptr [eax], zmm2

// CHECK:      vaaddpd zmmword ptr [2*ebp - 2048], zmm2
// CHECK: encoding: [0x62,0xf2,0xfd,0x48,0x84,0x14,0x6d,0x00,0xf8,0xff,0xff]
               vaaddpd zmmword ptr [2*ebp - 2048], zmm2

// CHECK:      vaaddpd zmmword ptr [ecx + 8128] {k7}, zmm2
// CHECK: encoding: [0x62,0xf2,0xfd,0x4f,0x84,0x51,0x7f]
               vaaddpd zmmword ptr [ecx + 8128] {k7}, zmm2

// CHECK:      vaaddpd zmmword ptr [edx - 8192] {k7}, zmm2
// CHECK: encoding: [0x62,0xf2,0xfd,0x4f,0x84,0x52,0x80]
               vaaddpd zmmword ptr [edx - 8192] {k7}, zmm2

// CHECK:      vaaddph zmmword ptr [esp + 8*esi + 268435456], zmm2
// CHECK: encoding: [0x62,0xf2,0x7c,0x48,0x94,0x94,0xf4,0x00,0x00,0x00,0x10]
               vaaddph zmmword ptr [esp + 8*esi + 268435456], zmm2

// CHECK:      vaaddph zmmword ptr [edi + 4*eax + 291] {k7}, zmm2
// CHECK: encoding: [0x62,0xf2,0x7c,0x4f,0x94,0x94,0x87,0x23,0x01,0x00,0x00]
               vaaddph zmmword ptr [edi + 4*eax + 291] {k7}, zmm2

// CHECK:      vaaddph zmmword ptr [eax], zmm2
// CHECK: encoding: [0x62,0xf2,0x7c,0x48,0x94,0x10]
               vaaddph zmmword ptr [eax], zmm2

// CHECK:      vaaddph zmmword ptr [2*ebp - 2048], zmm2
// CHECK: encoding: [0x62,0xf2,0x7c,0x48,0x94,0x14,0x6d,0x00,0xf8,0xff,0xff]
               vaaddph zmmword ptr [2*ebp - 2048], zmm2

// CHECK:      vaaddph zmmword ptr [ecx + 8128] {k7}, zmm2
// CHECK: encoding: [0x62,0xf2,0x7c,0x4f,0x94,0x51,0x7f]
               vaaddph zmmword ptr [ecx + 8128] {k7}, zmm2

// CHECK:      vaaddph zmmword ptr [edx - 8192] {k7}, zmm2
// CHECK: encoding: [0x62,0xf2,0x7c,0x4f,0x94,0x52,0x80]
               vaaddph zmmword ptr [edx - 8192] {k7}, zmm2

// CHECK:      vaaddps zmmword ptr [esp + 8*esi + 268435456], zmm2
// CHECK: encoding: [0x62,0xf2,0x7c,0x48,0x84,0x94,0xf4,0x00,0x00,0x00,0x10]
               vaaddps zmmword ptr [esp + 8*esi + 268435456], zmm2

// CHECK:      vaaddps zmmword ptr [edi + 4*eax + 291] {k7}, zmm2
// CHECK: encoding: [0x62,0xf2,0x7c,0x4f,0x84,0x94,0x87,0x23,0x01,0x00,0x00]
               vaaddps zmmword ptr [edi + 4*eax + 291] {k7}, zmm2

// CHECK:      vaaddps zmmword ptr [eax], zmm2
// CHECK: encoding: [0x62,0xf2,0x7c,0x48,0x84,0x10]
               vaaddps zmmword ptr [eax], zmm2

// CHECK:      vaaddps zmmword ptr [2*ebp - 2048], zmm2
// CHECK: encoding: [0x62,0xf2,0x7c,0x48,0x84,0x14,0x6d,0x00,0xf8,0xff,0xff]
               vaaddps zmmword ptr [2*ebp - 2048], zmm2

// CHECK:      vaaddps zmmword ptr [ecx + 8128] {k7}, zmm2
// CHECK: encoding: [0x62,0xf2,0x7c,0x4f,0x84,0x51,0x7f]
               vaaddps zmmword ptr [ecx + 8128] {k7}, zmm2

// CHECK:      vaaddps zmmword ptr [edx - 8192] {k7}, zmm2
// CHECK: encoding: [0x62,0xf2,0x7c,0x4f,0x84,0x52,0x80]
               vaaddps zmmword ptr [edx - 8192] {k7}, zmm2

// CHECK:      {evex} vaaddsbf16 word ptr [esp + 8*esi + 268435456], xmm2
// CHECK: encoding: [0x62,0xf2,0x7f,0x08,0x94,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vaaddsbf16 word ptr [esp + 8*esi + 268435456], xmm2

// CHECK:      {evex} vaaddsbf16 word ptr [edi + 4*eax + 291] {k7}, xmm2
// CHECK: encoding: [0x62,0xf2,0x7f,0x0f,0x94,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vaaddsbf16 word ptr [edi + 4*eax + 291] {k7}, xmm2

// CHECK:      {evex} vaaddsbf16 word ptr [eax], xmm2
// CHECK: encoding: [0x62,0xf2,0x7f,0x08,0x94,0x10]
               {evex} vaaddsbf16 word ptr [eax], xmm2

// CHECK:      {evex} vaaddsbf16 word ptr [2*ebp - 64], xmm2
// CHECK: encoding: [0x62,0xf2,0x7f,0x08,0x94,0x14,0x6d,0xc0,0xff,0xff,0xff]
               {evex} vaaddsbf16 word ptr [2*ebp - 64], xmm2

// CHECK:      {evex} vaaddsbf16 word ptr [ecx + 254] {k7}, xmm2
// CHECK: encoding: [0x62,0xf2,0x7f,0x0f,0x94,0x51,0x7f]
               {evex} vaaddsbf16 word ptr [ecx + 254] {k7}, xmm2

// CHECK:      {evex} vaaddsbf16 word ptr [edx - 256] {k7}, xmm2
// CHECK: encoding: [0x62,0xf2,0x7f,0x0f,0x94,0x52,0x80]
               {evex} vaaddsbf16 word ptr [edx - 256] {k7}, xmm2

// CHECK:      {evex} vaaddsd qword ptr [esp + 8*esi + 268435456], xmm2
// CHECK: encoding: [0x62,0xf2,0xff,0x08,0x84,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vaaddsd qword ptr [esp + 8*esi + 268435456], xmm2

// CHECK:      {evex} vaaddsd qword ptr [edi + 4*eax + 291] {k7}, xmm2
// CHECK: encoding: [0x62,0xf2,0xff,0x0f,0x84,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vaaddsd qword ptr [edi + 4*eax + 291] {k7}, xmm2

// CHECK:      {evex} vaaddsd qword ptr [eax], xmm2
// CHECK: encoding: [0x62,0xf2,0xff,0x08,0x84,0x10]
               {evex} vaaddsd qword ptr [eax], xmm2

// CHECK:      {evex} vaaddsd qword ptr [2*ebp - 256], xmm2
// CHECK: encoding: [0x62,0xf2,0xff,0x08,0x84,0x14,0x6d,0x00,0xff,0xff,0xff]
               {evex} vaaddsd qword ptr [2*ebp - 256], xmm2

// CHECK:      {evex} vaaddsd qword ptr [ecx + 1016] {k7}, xmm2
// CHECK: encoding: [0x62,0xf2,0xff,0x0f,0x84,0x51,0x7f]
               {evex} vaaddsd qword ptr [ecx + 1016] {k7}, xmm2

// CHECK:      {evex} vaaddsd qword ptr [edx - 1024] {k7}, xmm2
// CHECK: encoding: [0x62,0xf2,0xff,0x0f,0x84,0x52,0x80]
               {evex} vaaddsd qword ptr [edx - 1024] {k7}, xmm2

// CHECK:      {evex} vaaddsh word ptr [esp + 8*esi + 268435456], xmm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x08,0x94,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vaaddsh word ptr [esp + 8*esi + 268435456], xmm2

// CHECK:      {evex} vaaddsh word ptr [edi + 4*eax + 291] {k7}, xmm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x0f,0x94,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vaaddsh word ptr [edi + 4*eax + 291] {k7}, xmm2

// CHECK:      {evex} vaaddsh word ptr [eax], xmm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x08,0x94,0x10]
               {evex} vaaddsh word ptr [eax], xmm2

// CHECK:      {evex} vaaddsh word ptr [2*ebp - 64], xmm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x08,0x94,0x14,0x6d,0xc0,0xff,0xff,0xff]
               {evex} vaaddsh word ptr [2*ebp - 64], xmm2

// CHECK:      {evex} vaaddsh word ptr [ecx + 254] {k7}, xmm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x0f,0x94,0x51,0x7f]
               {evex} vaaddsh word ptr [ecx + 254] {k7}, xmm2

// CHECK:      {evex} vaaddsh word ptr [edx - 256] {k7}, xmm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x0f,0x94,0x52,0x80]
               {evex} vaaddsh word ptr [edx - 256] {k7}, xmm2

// CHECK:      {evex} vaaddss dword ptr [esp + 8*esi + 268435456], xmm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x08,0x84,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vaaddss dword ptr [esp + 8*esi + 268435456], xmm2

// CHECK:      {evex} vaaddss dword ptr [edi + 4*eax + 291] {k7}, xmm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x0f,0x84,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vaaddss dword ptr [edi + 4*eax + 291] {k7}, xmm2

// CHECK:      {evex} vaaddss dword ptr [eax], xmm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x08,0x84,0x10]
               {evex} vaaddss dword ptr [eax], xmm2

// CHECK:      {evex} vaaddss dword ptr [2*ebp - 128], xmm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x08,0x84,0x14,0x6d,0x80,0xff,0xff,0xff]
               {evex} vaaddss dword ptr [2*ebp - 128], xmm2

// CHECK:      {evex} vaaddss dword ptr [ecx + 508] {k7}, xmm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x0f,0x84,0x51,0x7f]
               {evex} vaaddss dword ptr [ecx + 508] {k7}, xmm2

// CHECK:      {evex} vaaddss dword ptr [edx - 512] {k7}, xmm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x0f,0x84,0x52,0x80]
               {evex} vaaddss dword ptr [edx - 512] {k7}, xmm2

