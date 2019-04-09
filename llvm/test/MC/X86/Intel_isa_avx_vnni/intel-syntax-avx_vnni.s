// REQUIRES: intel_feature_isa_avx_vnni
// RUN: llvm-mc -triple i686-unknown-unknown -mattr=+avxvnni -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK: vpdpbusd ymm6, ymm5, ymm4
// CHECK: encoding: [0xc4,0xe2,0x55,0x50,0xf4]
          vpdpbusd ymm6, ymm5, ymm4

// CHECK: vpdpbusd xmm6, xmm5, xmm4
// CHECK: encoding: [0xc4,0xe2,0x51,0x50,0xf4]
          vpdpbusd xmm6, xmm5, xmm4

// CHECK: vpdpbusd ymm6, ymm5, ymmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe2,0x55,0x50,0xb4,0xf4,0x00,0x00,0x00,0x10]
          vpdpbusd ymm6, ymm5, ymmword ptr [esp + 8*esi + 268435456]

// CHECK: vpdpbusd ymm6, ymm5, ymmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe2,0x55,0x50,0xb4,0x87,0x23,0x01,0x00,0x00]
          vpdpbusd ymm6, ymm5, ymmword ptr [edi + 4*eax + 291]

// CHECK: vpdpbusd ymm6, ymm5, ymmword ptr [eax]
// CHECK: encoding: [0xc4,0xe2,0x55,0x50,0x30]
          vpdpbusd ymm6, ymm5, ymmword ptr [eax]

// CHECK: vpdpbusd ymm6, ymm5, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0xc4,0xe2,0x55,0x50,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vpdpbusd ymm6, ymm5, ymmword ptr [2*ebp - 1024]

// CHECK: vpdpbusd ymm6, ymm5, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0xc4,0xe2,0x55,0x50,0xb1,0xe0,0x0f,0x00,0x00]
          vpdpbusd ymm6, ymm5, ymmword ptr [ecx + 4064]

// CHECK: vpdpbusd ymm6, ymm5, ymmword ptr [edx - 4096]
// CHECK: encoding: [0xc4,0xe2,0x55,0x50,0xb2,0x00,0xf0,0xff,0xff]
          vpdpbusd ymm6, ymm5, ymmword ptr [edx - 4096]

// CHECK: vpdpbusd xmm6, xmm5, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe2,0x51,0x50,0xb4,0xf4,0x00,0x00,0x00,0x10]
          vpdpbusd xmm6, xmm5, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vpdpbusd xmm6, xmm5, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe2,0x51,0x50,0xb4,0x87,0x23,0x01,0x00,0x00]
          vpdpbusd xmm6, xmm5, xmmword ptr [edi + 4*eax + 291]

// CHECK: vpdpbusd xmm6, xmm5, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe2,0x51,0x50,0x30]
          vpdpbusd xmm6, xmm5, xmmword ptr [eax]

// CHECK: vpdpbusd xmm6, xmm5, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe2,0x51,0x50,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vpdpbusd xmm6, xmm5, xmmword ptr [2*ebp - 512]

// CHECK: vpdpbusd xmm6, xmm5, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe2,0x51,0x50,0xb1,0xf0,0x07,0x00,0x00]
          vpdpbusd xmm6, xmm5, xmmword ptr [ecx + 2032]

// CHECK: vpdpbusd xmm6, xmm5, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe2,0x51,0x50,0xb2,0x00,0xf8,0xff,0xff]
          vpdpbusd xmm6, xmm5, xmmword ptr [edx - 2048]

// CHECK: vpdpbusds ymm6, ymm5, ymm4
// CHECK: encoding: [0xc4,0xe2,0x55,0x51,0xf4]
          vpdpbusds ymm6, ymm5, ymm4

// CHECK: vpdpbusds xmm6, xmm5, xmm4
// CHECK: encoding: [0xc4,0xe2,0x51,0x51,0xf4]
          vpdpbusds xmm6, xmm5, xmm4

// CHECK: vpdpbusds ymm6, ymm5, ymmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe2,0x55,0x51,0xb4,0xf4,0x00,0x00,0x00,0x10]
          vpdpbusds ymm6, ymm5, ymmword ptr [esp + 8*esi + 268435456]

// CHECK: vpdpbusds ymm6, ymm5, ymmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe2,0x55,0x51,0xb4,0x87,0x23,0x01,0x00,0x00]
          vpdpbusds ymm6, ymm5, ymmword ptr [edi + 4*eax + 291]

// CHECK: vpdpbusds ymm6, ymm5, ymmword ptr [eax]
// CHECK: encoding: [0xc4,0xe2,0x55,0x51,0x30]
          vpdpbusds ymm6, ymm5, ymmword ptr [eax]

// CHECK: vpdpbusds ymm6, ymm5, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0xc4,0xe2,0x55,0x51,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vpdpbusds ymm6, ymm5, ymmword ptr [2*ebp - 1024]

// CHECK: vpdpbusds ymm6, ymm5, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0xc4,0xe2,0x55,0x51,0xb1,0xe0,0x0f,0x00,0x00]
          vpdpbusds ymm6, ymm5, ymmword ptr [ecx + 4064]

// CHECK: vpdpbusds ymm6, ymm5, ymmword ptr [edx - 4096]
// CHECK: encoding: [0xc4,0xe2,0x55,0x51,0xb2,0x00,0xf0,0xff,0xff]
          vpdpbusds ymm6, ymm5, ymmword ptr [edx - 4096]

// CHECK: vpdpbusds xmm6, xmm5, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe2,0x51,0x51,0xb4,0xf4,0x00,0x00,0x00,0x10]
          vpdpbusds xmm6, xmm5, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vpdpbusds xmm6, xmm5, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe2,0x51,0x51,0xb4,0x87,0x23,0x01,0x00,0x00]
          vpdpbusds xmm6, xmm5, xmmword ptr [edi + 4*eax + 291]

// CHECK: vpdpbusds xmm6, xmm5, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe2,0x51,0x51,0x30]
          vpdpbusds xmm6, xmm5, xmmword ptr [eax]

// CHECK: vpdpbusds xmm6, xmm5, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe2,0x51,0x51,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vpdpbusds xmm6, xmm5, xmmword ptr [2*ebp - 512]

// CHECK: vpdpbusds xmm6, xmm5, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe2,0x51,0x51,0xb1,0xf0,0x07,0x00,0x00]
          vpdpbusds xmm6, xmm5, xmmword ptr [ecx + 2032]

// CHECK: vpdpbusds xmm6, xmm5, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe2,0x51,0x51,0xb2,0x00,0xf8,0xff,0xff]
          vpdpbusds xmm6, xmm5, xmmword ptr [edx - 2048]

// CHECK: vpdpwssd ymm6, ymm5, ymm4
// CHECK: encoding: [0xc4,0xe2,0x55,0x52,0xf4]
          vpdpwssd ymm6, ymm5, ymm4

// CHECK: vpdpwssd xmm6, xmm5, xmm4
// CHECK: encoding: [0xc4,0xe2,0x51,0x52,0xf4]
          vpdpwssd xmm6, xmm5, xmm4

// CHECK: vpdpwssd ymm6, ymm5, ymmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe2,0x55,0x52,0xb4,0xf4,0x00,0x00,0x00,0x10]
          vpdpwssd ymm6, ymm5, ymmword ptr [esp + 8*esi + 268435456]

// CHECK: vpdpwssd ymm6, ymm5, ymmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe2,0x55,0x52,0xb4,0x87,0x23,0x01,0x00,0x00]
          vpdpwssd ymm6, ymm5, ymmword ptr [edi + 4*eax + 291]

// CHECK: vpdpwssd ymm6, ymm5, ymmword ptr [eax]
// CHECK: encoding: [0xc4,0xe2,0x55,0x52,0x30]
          vpdpwssd ymm6, ymm5, ymmword ptr [eax]

// CHECK: vpdpwssd ymm6, ymm5, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0xc4,0xe2,0x55,0x52,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vpdpwssd ymm6, ymm5, ymmword ptr [2*ebp - 1024]

// CHECK: vpdpwssd ymm6, ymm5, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0xc4,0xe2,0x55,0x52,0xb1,0xe0,0x0f,0x00,0x00]
          vpdpwssd ymm6, ymm5, ymmword ptr [ecx + 4064]

// CHECK: vpdpwssd ymm6, ymm5, ymmword ptr [edx - 4096]
// CHECK: encoding: [0xc4,0xe2,0x55,0x52,0xb2,0x00,0xf0,0xff,0xff]
          vpdpwssd ymm6, ymm5, ymmword ptr [edx - 4096]

// CHECK: vpdpwssd xmm6, xmm5, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe2,0x51,0x52,0xb4,0xf4,0x00,0x00,0x00,0x10]
          vpdpwssd xmm6, xmm5, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vpdpwssd xmm6, xmm5, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe2,0x51,0x52,0xb4,0x87,0x23,0x01,0x00,0x00]
          vpdpwssd xmm6, xmm5, xmmword ptr [edi + 4*eax + 291]

// CHECK: vpdpwssd xmm6, xmm5, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe2,0x51,0x52,0x30]
          vpdpwssd xmm6, xmm5, xmmword ptr [eax]

// CHECK: vpdpwssd xmm6, xmm5, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe2,0x51,0x52,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vpdpwssd xmm6, xmm5, xmmword ptr [2*ebp - 512]

// CHECK: vpdpwssd xmm6, xmm5, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe2,0x51,0x52,0xb1,0xf0,0x07,0x00,0x00]
          vpdpwssd xmm6, xmm5, xmmword ptr [ecx + 2032]

// CHECK: vpdpwssd xmm6, xmm5, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe2,0x51,0x52,0xb2,0x00,0xf8,0xff,0xff]
          vpdpwssd xmm6, xmm5, xmmword ptr [edx - 2048]

// CHECK: vpdpwssds ymm6, ymm5, ymm4
// CHECK: encoding: [0xc4,0xe2,0x55,0x53,0xf4]
          vpdpwssds ymm6, ymm5, ymm4

// CHECK: vpdpwssds xmm6, xmm5, xmm4
// CHECK: encoding: [0xc4,0xe2,0x51,0x53,0xf4]
          vpdpwssds xmm6, xmm5, xmm4

// CHECK: vpdpwssds ymm6, ymm5, ymmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe2,0x55,0x53,0xb4,0xf4,0x00,0x00,0x00,0x10]
          vpdpwssds ymm6, ymm5, ymmword ptr [esp + 8*esi + 268435456]

// CHECK: vpdpwssds ymm6, ymm5, ymmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe2,0x55,0x53,0xb4,0x87,0x23,0x01,0x00,0x00]
          vpdpwssds ymm6, ymm5, ymmword ptr [edi + 4*eax + 291]

// CHECK: vpdpwssds ymm6, ymm5, ymmword ptr [eax]
// CHECK: encoding: [0xc4,0xe2,0x55,0x53,0x30]
          vpdpwssds ymm6, ymm5, ymmword ptr [eax]

// CHECK: vpdpwssds ymm6, ymm5, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0xc4,0xe2,0x55,0x53,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vpdpwssds ymm6, ymm5, ymmword ptr [2*ebp - 1024]

// CHECK: vpdpwssds ymm6, ymm5, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0xc4,0xe2,0x55,0x53,0xb1,0xe0,0x0f,0x00,0x00]
          vpdpwssds ymm6, ymm5, ymmword ptr [ecx + 4064]

// CHECK: vpdpwssds ymm6, ymm5, ymmword ptr [edx - 4096]
// CHECK: encoding: [0xc4,0xe2,0x55,0x53,0xb2,0x00,0xf0,0xff,0xff]
          vpdpwssds ymm6, ymm5, ymmword ptr [edx - 4096]

// CHECK: vpdpwssds xmm6, xmm5, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe2,0x51,0x53,0xb4,0xf4,0x00,0x00,0x00,0x10]
          vpdpwssds xmm6, xmm5, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vpdpwssds xmm6, xmm5, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe2,0x51,0x53,0xb4,0x87,0x23,0x01,0x00,0x00]
          vpdpwssds xmm6, xmm5, xmmword ptr [edi + 4*eax + 291]

// CHECK: vpdpwssds xmm6, xmm5, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe2,0x51,0x53,0x30]
          vpdpwssds xmm6, xmm5, xmmword ptr [eax]

// CHECK: vpdpwssds xmm6, xmm5, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe2,0x51,0x53,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vpdpwssds xmm6, xmm5, xmmword ptr [2*ebp - 512]

// CHECK: vpdpwssds xmm6, xmm5, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe2,0x51,0x53,0xb1,0xf0,0x07,0x00,0x00]
          vpdpwssds xmm6, xmm5, xmmword ptr [ecx + 2032]

// CHECK: vpdpwssds xmm6, xmm5, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe2,0x51,0x53,0xb2,0x00,0xf8,0xff,0xff]
          vpdpwssds xmm6, xmm5, xmmword ptr [edx - 2048]

