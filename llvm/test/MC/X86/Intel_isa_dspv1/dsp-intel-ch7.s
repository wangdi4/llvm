// REQUIRES: intel_feature_isa_dspv1
// RUN: llvm-mc -triple i386-unknown-unknown -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK:      dvpmacdhhq xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe8,0xe2,0xe8,0xd4]
               dvpmacdhhq xmm2, xmm3, xmm4

// CHECK:      dvpmacdhhq xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xe8,0x94,0xf4,0x00,0x00,0x00,0x10]
               dvpmacdhhq xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK:      dvpmacdhhq xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xe8,0x94,0x87,0x23,0x01,0x00,0x00]
               dvpmacdhhq xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK:      dvpmacdhhq xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xe8,0x10]
               dvpmacdhhq xmm2, xmm3, xmmword ptr [eax]

// CHECK:      dvpmacdhhq xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xe8,0x14,0x6d,0x00,0xfe,0xff,0xff]
               dvpmacdhhq xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK:      dvpmacdhhq xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xe8,0x91,0xf0,0x07,0x00,0x00]
               dvpmacdhhq xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK:      dvpmacdhhq xmm2, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xe8,0x92,0x00,0xf8,0xff,0xff]
               dvpmacdhhq xmm2, xmm3, xmmword ptr [edx - 2048]

// CHECK:      dvpmacdhhsq xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe8,0xe2,0xe9,0xd4]
               dvpmacdhhsq xmm2, xmm3, xmm4

// CHECK:      dvpmacdhhsq xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xe9,0x94,0xf4,0x00,0x00,0x00,0x10]
               dvpmacdhhsq xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK:      dvpmacdhhsq xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xe9,0x94,0x87,0x23,0x01,0x00,0x00]
               dvpmacdhhsq xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK:      dvpmacdhhsq xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xe9,0x10]
               dvpmacdhhsq xmm2, xmm3, xmmword ptr [eax]

// CHECK:      dvpmacdhhsq xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xe9,0x14,0x6d,0x00,0xfe,0xff,0xff]
               dvpmacdhhsq xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK:      dvpmacdhhsq xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xe9,0x91,0xf0,0x07,0x00,0x00]
               dvpmacdhhsq xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK:      dvpmacdhhsq xmm2, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xe9,0x92,0x00,0xf8,0xff,0xff]
               dvpmacdhhsq xmm2, xmm3, xmmword ptr [edx - 2048]

// CHECK:      dvpmacdllq xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe8,0xe0,0xe8,0xd4]
               dvpmacdllq xmm2, xmm3, xmm4

// CHECK:      dvpmacdllq xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xe8,0x94,0xf4,0x00,0x00,0x00,0x10]
               dvpmacdllq xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK:      dvpmacdllq xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xe8,0x94,0x87,0x23,0x01,0x00,0x00]
               dvpmacdllq xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK:      dvpmacdllq xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xe8,0x10]
               dvpmacdllq xmm2, xmm3, xmmword ptr [eax]

// CHECK:      dvpmacdllq xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xe8,0x14,0x6d,0x00,0xfe,0xff,0xff]
               dvpmacdllq xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK:      dvpmacdllq xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xe8,0x91,0xf0,0x07,0x00,0x00]
               dvpmacdllq xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK:      dvpmacdllq xmm2, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xe8,0x92,0x00,0xf8,0xff,0xff]
               dvpmacdllq xmm2, xmm3, xmmword ptr [edx - 2048]

// CHECK:      dvpmacdllsq xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe8,0xe0,0xe9,0xd4]
               dvpmacdllsq xmm2, xmm3, xmm4

// CHECK:      dvpmacdllsq xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xe9,0x94,0xf4,0x00,0x00,0x00,0x10]
               dvpmacdllsq xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK:      dvpmacdllsq xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xe9,0x94,0x87,0x23,0x01,0x00,0x00]
               dvpmacdllsq xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK:      dvpmacdllsq xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xe9,0x10]
               dvpmacdllsq xmm2, xmm3, xmmword ptr [eax]

// CHECK:      dvpmacdllsq xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xe9,0x14,0x6d,0x00,0xfe,0xff,0xff]
               dvpmacdllsq xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK:      dvpmacdllsq xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xe9,0x91,0xf0,0x07,0x00,0x00]
               dvpmacdllsq xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK:      dvpmacdllsq xmm2, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xe9,0x92,0x00,0xf8,0xff,0xff]
               dvpmacdllsq xmm2, xmm3, xmmword ptr [edx - 2048]

// CHECK:      dvpmacudhhq xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe8,0xe2,0xea,0xd4]
               dvpmacudhhq xmm2, xmm3, xmm4

// CHECK:      dvpmacudhhq xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xea,0x94,0xf4,0x00,0x00,0x00,0x10]
               dvpmacudhhq xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK:      dvpmacudhhq xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xea,0x94,0x87,0x23,0x01,0x00,0x00]
               dvpmacudhhq xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK:      dvpmacudhhq xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xea,0x10]
               dvpmacudhhq xmm2, xmm3, xmmword ptr [eax]

// CHECK:      dvpmacudhhq xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xea,0x14,0x6d,0x00,0xfe,0xff,0xff]
               dvpmacudhhq xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK:      dvpmacudhhq xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xea,0x91,0xf0,0x07,0x00,0x00]
               dvpmacudhhq xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK:      dvpmacudhhq xmm2, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xea,0x92,0x00,0xf8,0xff,0xff]
               dvpmacudhhq xmm2, xmm3, xmmword ptr [edx - 2048]

// CHECK:      dvpmacudhhsq xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe8,0xe2,0xeb,0xd4]
               dvpmacudhhsq xmm2, xmm3, xmm4

// CHECK:      dvpmacudhhsq xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xeb,0x94,0xf4,0x00,0x00,0x00,0x10]
               dvpmacudhhsq xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK:      dvpmacudhhsq xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xeb,0x94,0x87,0x23,0x01,0x00,0x00]
               dvpmacudhhsq xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK:      dvpmacudhhsq xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xeb,0x10]
               dvpmacudhhsq xmm2, xmm3, xmmword ptr [eax]

// CHECK:      dvpmacudhhsq xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xeb,0x14,0x6d,0x00,0xfe,0xff,0xff]
               dvpmacudhhsq xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK:      dvpmacudhhsq xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xeb,0x91,0xf0,0x07,0x00,0x00]
               dvpmacudhhsq xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK:      dvpmacudhhsq xmm2, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xeb,0x92,0x00,0xf8,0xff,0xff]
               dvpmacudhhsq xmm2, xmm3, xmmword ptr [edx - 2048]

// CHECK:      dvpmacudllq xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe8,0xe0,0xea,0xd4]
               dvpmacudllq xmm2, xmm3, xmm4

// CHECK:      dvpmacudllq xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xea,0x94,0xf4,0x00,0x00,0x00,0x10]
               dvpmacudllq xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK:      dvpmacudllq xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xea,0x94,0x87,0x23,0x01,0x00,0x00]
               dvpmacudllq xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK:      dvpmacudllq xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xea,0x10]
               dvpmacudllq xmm2, xmm3, xmmword ptr [eax]

// CHECK:      dvpmacudllq xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xea,0x14,0x6d,0x00,0xfe,0xff,0xff]
               dvpmacudllq xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK:      dvpmacudllq xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xea,0x91,0xf0,0x07,0x00,0x00]
               dvpmacudllq xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK:      dvpmacudllq xmm2, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xea,0x92,0x00,0xf8,0xff,0xff]
               dvpmacudllq xmm2, xmm3, xmmword ptr [edx - 2048]

// CHECK:      dvpmacudllsq xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe8,0xe0,0xeb,0xd4]
               dvpmacudllsq xmm2, xmm3, xmm4

// CHECK:      dvpmacudllsq xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xeb,0x94,0xf4,0x00,0x00,0x00,0x10]
               dvpmacudllsq xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK:      dvpmacudllsq xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xeb,0x94,0x87,0x23,0x01,0x00,0x00]
               dvpmacudllsq xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK:      dvpmacudllsq xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xeb,0x10]
               dvpmacudllsq xmm2, xmm3, xmmword ptr [eax]

// CHECK:      dvpmacudllsq xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xeb,0x14,0x6d,0x00,0xfe,0xff,0xff]
               dvpmacudllsq xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK:      dvpmacudllsq xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xeb,0x91,0xf0,0x07,0x00,0x00]
               dvpmacudllsq xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK:      dvpmacudllsq xmm2, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xeb,0x92,0x00,0xf8,0xff,0xff]
               dvpmacudllsq xmm2, xmm3, xmmword ptr [edx - 2048]

// CHECK:      dvpmsubadddllq xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe8,0xe0,0xe7,0xd4]
               dvpmsubadddllq xmm2, xmm3, xmm4

// CHECK:      dvpmsubadddllq xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xe7,0x94,0xf4,0x00,0x00,0x00,0x10]
               dvpmsubadddllq xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK:      dvpmsubadddllq xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xe7,0x94,0x87,0x23,0x01,0x00,0x00]
               dvpmsubadddllq xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK:      dvpmsubadddllq xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xe7,0x10]
               dvpmsubadddllq xmm2, xmm3, xmmword ptr [eax]

// CHECK:      dvpmsubadddllq xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xe7,0x14,0x6d,0x00,0xfe,0xff,0xff]
               dvpmsubadddllq xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK:      dvpmsubadddllq xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xe7,0x91,0xf0,0x07,0x00,0x00]
               dvpmsubadddllq xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK:      dvpmsubadddllq xmm2, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xe7,0x92,0x00,0xf8,0xff,0xff]
               dvpmsubadddllq xmm2, xmm3, xmmword ptr [edx - 2048]

// CHECK:      dvpmuldfrs xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe8,0x63,0xca,0xd4]
               dvpmuldfrs xmm2, xmm3, xmm4

// CHECK:      dvpmuldfrs xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe8,0x63,0xca,0x94,0xf4,0x00,0x00,0x00,0x10]
               dvpmuldfrs xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK:      dvpmuldfrs xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe8,0x63,0xca,0x94,0x87,0x23,0x01,0x00,0x00]
               dvpmuldfrs xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK:      dvpmuldfrs xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe8,0x63,0xca,0x10]
               dvpmuldfrs xmm2, xmm3, xmmword ptr [eax]

// CHECK:      dvpmuldfrs xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe8,0x63,0xca,0x14,0x6d,0x00,0xfe,0xff,0xff]
               dvpmuldfrs xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK:      dvpmuldfrs xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe8,0x63,0xca,0x91,0xf0,0x07,0x00,0x00]
               dvpmuldfrs xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK:      dvpmuldfrs xmm2, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe8,0x63,0xca,0x92,0x00,0xf8,0xff,0xff]
               dvpmuldfrs xmm2, xmm3, xmmword ptr [edx - 2048]

// CHECK:      dvpmuldhhq xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe8,0xe2,0xcc,0xd4]
               dvpmuldhhq xmm2, xmm3, xmm4

// CHECK:      dvpmuldhhq xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xcc,0x94,0xf4,0x00,0x00,0x00,0x10]
               dvpmuldhhq xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK:      dvpmuldhhq xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xcc,0x94,0x87,0x23,0x01,0x00,0x00]
               dvpmuldhhq xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK:      dvpmuldhhq xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xcc,0x10]
               dvpmuldhhq xmm2, xmm3, xmmword ptr [eax]

// CHECK:      dvpmuldhhq xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xcc,0x14,0x6d,0x00,0xfe,0xff,0xff]
               dvpmuldhhq xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK:      dvpmuldhhq xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xcc,0x91,0xf0,0x07,0x00,0x00]
               dvpmuldhhq xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK:      dvpmuldhhq xmm2, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xcc,0x92,0x00,0xf8,0xff,0xff]
               dvpmuldhhq xmm2, xmm3, xmmword ptr [edx - 2048]

// CHECK:      dvpmulds xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe8,0x63,0xcd,0xd4]
               dvpmulds xmm2, xmm3, xmm4

// CHECK:      dvpmulds xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe8,0x63,0xcd,0x94,0xf4,0x00,0x00,0x00,0x10]
               dvpmulds xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK:      dvpmulds xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe8,0x63,0xcd,0x94,0x87,0x23,0x01,0x00,0x00]
               dvpmulds xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK:      dvpmulds xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe8,0x63,0xcd,0x10]
               dvpmulds xmm2, xmm3, xmmword ptr [eax]

// CHECK:      dvpmulds xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe8,0x63,0xcd,0x14,0x6d,0x00,0xfe,0xff,0xff]
               dvpmulds xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK:      dvpmulds xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe8,0x63,0xcd,0x91,0xf0,0x07,0x00,0x00]
               dvpmulds xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK:      dvpmulds xmm2, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe8,0x63,0xcd,0x92,0x00,0xf8,0xff,0xff]
               dvpmulds xmm2, xmm3, xmmword ptr [edx - 2048]

// CHECK:      dvpmuludhhq xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe8,0xe2,0xcb,0xd4]
               dvpmuludhhq xmm2, xmm3, xmm4

// CHECK:      dvpmuludhhq xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xcb,0x94,0xf4,0x00,0x00,0x00,0x10]
               dvpmuludhhq xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK:      dvpmuludhhq xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xcb,0x94,0x87,0x23,0x01,0x00,0x00]
               dvpmuludhhq xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK:      dvpmuludhhq xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xcb,0x10]
               dvpmuludhhq xmm2, xmm3, xmmword ptr [eax]

// CHECK:      dvpmuludhhq xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xcb,0x14,0x6d,0x00,0xfe,0xff,0xff]
               dvpmuludhhq xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK:      dvpmuludhhq xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xcb,0x91,0xf0,0x07,0x00,0x00]
               dvpmuludhhq xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK:      dvpmuludhhq xmm2, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xcb,0x92,0x00,0xf8,0xff,0xff]
               dvpmuludhhq xmm2, xmm3, xmmword ptr [edx - 2048]

// CHECK:      dvpnmacdhhq xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe8,0xe2,0xec,0xd4]
               dvpnmacdhhq xmm2, xmm3, xmm4

// CHECK:      dvpnmacdhhq xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xec,0x94,0xf4,0x00,0x00,0x00,0x10]
               dvpnmacdhhq xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK:      dvpnmacdhhq xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xec,0x94,0x87,0x23,0x01,0x00,0x00]
               dvpnmacdhhq xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK:      dvpnmacdhhq xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xec,0x10]
               dvpnmacdhhq xmm2, xmm3, xmmword ptr [eax]

// CHECK:      dvpnmacdhhq xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xec,0x14,0x6d,0x00,0xfe,0xff,0xff]
               dvpnmacdhhq xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK:      dvpnmacdhhq xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xec,0x91,0xf0,0x07,0x00,0x00]
               dvpnmacdhhq xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK:      dvpnmacdhhq xmm2, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xec,0x92,0x00,0xf8,0xff,0xff]
               dvpnmacdhhq xmm2, xmm3, xmmword ptr [edx - 2048]

// CHECK:      dvpnmacdhhsq xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe8,0xe2,0xed,0xd4]
               dvpnmacdhhsq xmm2, xmm3, xmm4

// CHECK:      dvpnmacdhhsq xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xed,0x94,0xf4,0x00,0x00,0x00,0x10]
               dvpnmacdhhsq xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK:      dvpnmacdhhsq xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xed,0x94,0x87,0x23,0x01,0x00,0x00]
               dvpnmacdhhsq xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK:      dvpnmacdhhsq xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xed,0x10]
               dvpnmacdhhsq xmm2, xmm3, xmmword ptr [eax]

// CHECK:      dvpnmacdhhsq xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xed,0x14,0x6d,0x00,0xfe,0xff,0xff]
               dvpnmacdhhsq xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK:      dvpnmacdhhsq xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xed,0x91,0xf0,0x07,0x00,0x00]
               dvpnmacdhhsq xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK:      dvpnmacdhhsq xmm2, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xed,0x92,0x00,0xf8,0xff,0xff]
               dvpnmacdhhsq xmm2, xmm3, xmmword ptr [edx - 2048]

// CHECK:      dvpnmacdllq xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe8,0xe0,0xec,0xd4]
               dvpnmacdllq xmm2, xmm3, xmm4

// CHECK:      dvpnmacdllq xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xec,0x94,0xf4,0x00,0x00,0x00,0x10]
               dvpnmacdllq xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK:      dvpnmacdllq xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xec,0x94,0x87,0x23,0x01,0x00,0x00]
               dvpnmacdllq xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK:      dvpnmacdllq xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xec,0x10]
               dvpnmacdllq xmm2, xmm3, xmmword ptr [eax]

// CHECK:      dvpnmacdllq xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xec,0x14,0x6d,0x00,0xfe,0xff,0xff]
               dvpnmacdllq xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK:      dvpnmacdllq xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xec,0x91,0xf0,0x07,0x00,0x00]
               dvpnmacdllq xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK:      dvpnmacdllq xmm2, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xec,0x92,0x00,0xf8,0xff,0xff]
               dvpnmacdllq xmm2, xmm3, xmmword ptr [edx - 2048]

// CHECK:      dvpnmacdllsq xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe8,0xe0,0xed,0xd4]
               dvpnmacdllsq xmm2, xmm3, xmm4

// CHECK:      dvpnmacdllsq xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xed,0x94,0xf4,0x00,0x00,0x00,0x10]
               dvpnmacdllsq xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK:      dvpnmacdllsq xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xed,0x94,0x87,0x23,0x01,0x00,0x00]
               dvpnmacdllsq xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK:      dvpnmacdllsq xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xed,0x10]
               dvpnmacdllsq xmm2, xmm3, xmmword ptr [eax]

// CHECK:      dvpnmacdllsq xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xed,0x14,0x6d,0x00,0xfe,0xff,0xff]
               dvpnmacdllsq xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK:      dvpnmacdllsq xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xed,0x91,0xf0,0x07,0x00,0x00]
               dvpnmacdllsq xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK:      dvpnmacdllsq xmm2, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xed,0x92,0x00,0xf8,0xff,0xff]
               dvpnmacdllsq xmm2, xmm3, xmmword ptr [edx - 2048]

