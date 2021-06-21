// REQUIRES: intel_feature_isa_dspv1
// RUN: llvm-mc -triple i386-unknown-unknown --show-encoding %s | FileCheck %s

// CHECK:      dvpmacdhhq %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe2,0xe8,0xd4]
               dvpmacdhhq %xmm4, %xmm3, %xmm2

// CHECK:      dvpmacdhhq  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe2,0xe8,0x94,0xf4,0x00,0x00,0x00,0x10]
               dvpmacdhhq  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK:      dvpmacdhhq  291(%edi,%eax,4), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe2,0xe8,0x94,0x87,0x23,0x01,0x00,0x00]
               dvpmacdhhq  291(%edi,%eax,4), %xmm3, %xmm2

// CHECK:      dvpmacdhhq  (%eax), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe2,0xe8,0x10]
               dvpmacdhhq  (%eax), %xmm3, %xmm2

// CHECK:      dvpmacdhhq  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe2,0xe8,0x14,0x6d,0x00,0xfe,0xff,0xff]
               dvpmacdhhq  -512(,%ebp,2), %xmm3, %xmm2

// CHECK:      dvpmacdhhq  2032(%ecx), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe2,0xe8,0x91,0xf0,0x07,0x00,0x00]
               dvpmacdhhq  2032(%ecx), %xmm3, %xmm2

// CHECK:      dvpmacdhhq  -2048(%edx), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe2,0xe8,0x92,0x00,0xf8,0xff,0xff]
               dvpmacdhhq  -2048(%edx), %xmm3, %xmm2

// CHECK:      dvpmacdhhsq %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe2,0xe9,0xd4]
               dvpmacdhhsq %xmm4, %xmm3, %xmm2

// CHECK:      dvpmacdhhsq  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe2,0xe9,0x94,0xf4,0x00,0x00,0x00,0x10]
               dvpmacdhhsq  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK:      dvpmacdhhsq  291(%edi,%eax,4), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe2,0xe9,0x94,0x87,0x23,0x01,0x00,0x00]
               dvpmacdhhsq  291(%edi,%eax,4), %xmm3, %xmm2

// CHECK:      dvpmacdhhsq  (%eax), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe2,0xe9,0x10]
               dvpmacdhhsq  (%eax), %xmm3, %xmm2

// CHECK:      dvpmacdhhsq  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe2,0xe9,0x14,0x6d,0x00,0xfe,0xff,0xff]
               dvpmacdhhsq  -512(,%ebp,2), %xmm3, %xmm2

// CHECK:      dvpmacdhhsq  2032(%ecx), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe2,0xe9,0x91,0xf0,0x07,0x00,0x00]
               dvpmacdhhsq  2032(%ecx), %xmm3, %xmm2

// CHECK:      dvpmacdhhsq  -2048(%edx), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe2,0xe9,0x92,0x00,0xf8,0xff,0xff]
               dvpmacdhhsq  -2048(%edx), %xmm3, %xmm2

// CHECK:      dvpmacdllq %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe0,0xe8,0xd4]
               dvpmacdllq %xmm4, %xmm3, %xmm2

// CHECK:      dvpmacdllq  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe0,0xe8,0x94,0xf4,0x00,0x00,0x00,0x10]
               dvpmacdllq  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK:      dvpmacdllq  291(%edi,%eax,4), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe0,0xe8,0x94,0x87,0x23,0x01,0x00,0x00]
               dvpmacdllq  291(%edi,%eax,4), %xmm3, %xmm2

// CHECK:      dvpmacdllq  (%eax), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe0,0xe8,0x10]
               dvpmacdllq  (%eax), %xmm3, %xmm2

// CHECK:      dvpmacdllq  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe0,0xe8,0x14,0x6d,0x00,0xfe,0xff,0xff]
               dvpmacdllq  -512(,%ebp,2), %xmm3, %xmm2

// CHECK:      dvpmacdllq  2032(%ecx), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe0,0xe8,0x91,0xf0,0x07,0x00,0x00]
               dvpmacdllq  2032(%ecx), %xmm3, %xmm2

// CHECK:      dvpmacdllq  -2048(%edx), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe0,0xe8,0x92,0x00,0xf8,0xff,0xff]
               dvpmacdllq  -2048(%edx), %xmm3, %xmm2

// CHECK:      dvpmacdllsq %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe0,0xe9,0xd4]
               dvpmacdllsq %xmm4, %xmm3, %xmm2

// CHECK:      dvpmacdllsq  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe0,0xe9,0x94,0xf4,0x00,0x00,0x00,0x10]
               dvpmacdllsq  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK:      dvpmacdllsq  291(%edi,%eax,4), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe0,0xe9,0x94,0x87,0x23,0x01,0x00,0x00]
               dvpmacdllsq  291(%edi,%eax,4), %xmm3, %xmm2

// CHECK:      dvpmacdllsq  (%eax), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe0,0xe9,0x10]
               dvpmacdllsq  (%eax), %xmm3, %xmm2

// CHECK:      dvpmacdllsq  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe0,0xe9,0x14,0x6d,0x00,0xfe,0xff,0xff]
               dvpmacdllsq  -512(,%ebp,2), %xmm3, %xmm2

// CHECK:      dvpmacdllsq  2032(%ecx), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe0,0xe9,0x91,0xf0,0x07,0x00,0x00]
               dvpmacdllsq  2032(%ecx), %xmm3, %xmm2

// CHECK:      dvpmacdllsq  -2048(%edx), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe0,0xe9,0x92,0x00,0xf8,0xff,0xff]
               dvpmacdllsq  -2048(%edx), %xmm3, %xmm2

// CHECK:      dvpmacudhhq %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe2,0xea,0xd4]
               dvpmacudhhq %xmm4, %xmm3, %xmm2

// CHECK:      dvpmacudhhq  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe2,0xea,0x94,0xf4,0x00,0x00,0x00,0x10]
               dvpmacudhhq  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK:      dvpmacudhhq  291(%edi,%eax,4), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe2,0xea,0x94,0x87,0x23,0x01,0x00,0x00]
               dvpmacudhhq  291(%edi,%eax,4), %xmm3, %xmm2

// CHECK:      dvpmacudhhq  (%eax), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe2,0xea,0x10]
               dvpmacudhhq  (%eax), %xmm3, %xmm2

// CHECK:      dvpmacudhhq  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe2,0xea,0x14,0x6d,0x00,0xfe,0xff,0xff]
               dvpmacudhhq  -512(,%ebp,2), %xmm3, %xmm2

// CHECK:      dvpmacudhhq  2032(%ecx), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe2,0xea,0x91,0xf0,0x07,0x00,0x00]
               dvpmacudhhq  2032(%ecx), %xmm3, %xmm2

// CHECK:      dvpmacudhhq  -2048(%edx), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe2,0xea,0x92,0x00,0xf8,0xff,0xff]
               dvpmacudhhq  -2048(%edx), %xmm3, %xmm2

// CHECK:      dvpmacudhhsq %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe2,0xeb,0xd4]
               dvpmacudhhsq %xmm4, %xmm3, %xmm2

// CHECK:      dvpmacudhhsq  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe2,0xeb,0x94,0xf4,0x00,0x00,0x00,0x10]
               dvpmacudhhsq  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK:      dvpmacudhhsq  291(%edi,%eax,4), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe2,0xeb,0x94,0x87,0x23,0x01,0x00,0x00]
               dvpmacudhhsq  291(%edi,%eax,4), %xmm3, %xmm2

// CHECK:      dvpmacudhhsq  (%eax), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe2,0xeb,0x10]
               dvpmacudhhsq  (%eax), %xmm3, %xmm2

// CHECK:      dvpmacudhhsq  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe2,0xeb,0x14,0x6d,0x00,0xfe,0xff,0xff]
               dvpmacudhhsq  -512(,%ebp,2), %xmm3, %xmm2

// CHECK:      dvpmacudhhsq  2032(%ecx), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe2,0xeb,0x91,0xf0,0x07,0x00,0x00]
               dvpmacudhhsq  2032(%ecx), %xmm3, %xmm2

// CHECK:      dvpmacudhhsq  -2048(%edx), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe2,0xeb,0x92,0x00,0xf8,0xff,0xff]
               dvpmacudhhsq  -2048(%edx), %xmm3, %xmm2

// CHECK:      dvpmacudllq %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe0,0xea,0xd4]
               dvpmacudllq %xmm4, %xmm3, %xmm2

// CHECK:      dvpmacudllq  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe0,0xea,0x94,0xf4,0x00,0x00,0x00,0x10]
               dvpmacudllq  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK:      dvpmacudllq  291(%edi,%eax,4), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe0,0xea,0x94,0x87,0x23,0x01,0x00,0x00]
               dvpmacudllq  291(%edi,%eax,4), %xmm3, %xmm2

// CHECK:      dvpmacudllq  (%eax), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe0,0xea,0x10]
               dvpmacudllq  (%eax), %xmm3, %xmm2

// CHECK:      dvpmacudllq  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe0,0xea,0x14,0x6d,0x00,0xfe,0xff,0xff]
               dvpmacudllq  -512(,%ebp,2), %xmm3, %xmm2

// CHECK:      dvpmacudllq  2032(%ecx), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe0,0xea,0x91,0xf0,0x07,0x00,0x00]
               dvpmacudllq  2032(%ecx), %xmm3, %xmm2

// CHECK:      dvpmacudllq  -2048(%edx), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe0,0xea,0x92,0x00,0xf8,0xff,0xff]
               dvpmacudllq  -2048(%edx), %xmm3, %xmm2

// CHECK:      dvpmacudllsq %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe0,0xeb,0xd4]
               dvpmacudllsq %xmm4, %xmm3, %xmm2

// CHECK:      dvpmacudllsq  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe0,0xeb,0x94,0xf4,0x00,0x00,0x00,0x10]
               dvpmacudllsq  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK:      dvpmacudllsq  291(%edi,%eax,4), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe0,0xeb,0x94,0x87,0x23,0x01,0x00,0x00]
               dvpmacudllsq  291(%edi,%eax,4), %xmm3, %xmm2

// CHECK:      dvpmacudllsq  (%eax), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe0,0xeb,0x10]
               dvpmacudllsq  (%eax), %xmm3, %xmm2

// CHECK:      dvpmacudllsq  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe0,0xeb,0x14,0x6d,0x00,0xfe,0xff,0xff]
               dvpmacudllsq  -512(,%ebp,2), %xmm3, %xmm2

// CHECK:      dvpmacudllsq  2032(%ecx), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe0,0xeb,0x91,0xf0,0x07,0x00,0x00]
               dvpmacudllsq  2032(%ecx), %xmm3, %xmm2

// CHECK:      dvpmacudllsq  -2048(%edx), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe0,0xeb,0x92,0x00,0xf8,0xff,0xff]
               dvpmacudllsq  -2048(%edx), %xmm3, %xmm2

// CHECK:      dvpmsubadddllq %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe0,0xe7,0xd4]
               dvpmsubadddllq %xmm4, %xmm3, %xmm2

// CHECK:      dvpmsubadddllq  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe0,0xe7,0x94,0xf4,0x00,0x00,0x00,0x10]
               dvpmsubadddllq  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK:      dvpmsubadddllq  291(%edi,%eax,4), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe0,0xe7,0x94,0x87,0x23,0x01,0x00,0x00]
               dvpmsubadddllq  291(%edi,%eax,4), %xmm3, %xmm2

// CHECK:      dvpmsubadddllq  (%eax), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe0,0xe7,0x10]
               dvpmsubadddllq  (%eax), %xmm3, %xmm2

// CHECK:      dvpmsubadddllq  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe0,0xe7,0x14,0x6d,0x00,0xfe,0xff,0xff]
               dvpmsubadddllq  -512(,%ebp,2), %xmm3, %xmm2

// CHECK:      dvpmsubadddllq  2032(%ecx), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe0,0xe7,0x91,0xf0,0x07,0x00,0x00]
               dvpmsubadddllq  2032(%ecx), %xmm3, %xmm2

// CHECK:      dvpmsubadddllq  -2048(%edx), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe0,0xe7,0x92,0x00,0xf8,0xff,0xff]
               dvpmsubadddllq  -2048(%edx), %xmm3, %xmm2

// CHECK:      dvpmuldfrs %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0x63,0xca,0xd4]
               dvpmuldfrs %xmm4, %xmm3, %xmm2

// CHECK:      dvpmuldfrs  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0x63,0xca,0x94,0xf4,0x00,0x00,0x00,0x10]
               dvpmuldfrs  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK:      dvpmuldfrs  291(%edi,%eax,4), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0x63,0xca,0x94,0x87,0x23,0x01,0x00,0x00]
               dvpmuldfrs  291(%edi,%eax,4), %xmm3, %xmm2

// CHECK:      dvpmuldfrs  (%eax), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0x63,0xca,0x10]
               dvpmuldfrs  (%eax), %xmm3, %xmm2

// CHECK:      dvpmuldfrs  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0x63,0xca,0x14,0x6d,0x00,0xfe,0xff,0xff]
               dvpmuldfrs  -512(,%ebp,2), %xmm3, %xmm2

// CHECK:      dvpmuldfrs  2032(%ecx), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0x63,0xca,0x91,0xf0,0x07,0x00,0x00]
               dvpmuldfrs  2032(%ecx), %xmm3, %xmm2

// CHECK:      dvpmuldfrs  -2048(%edx), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0x63,0xca,0x92,0x00,0xf8,0xff,0xff]
               dvpmuldfrs  -2048(%edx), %xmm3, %xmm2

// CHECK:      dvpmuldhhq %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe2,0xcc,0xd4]
               dvpmuldhhq %xmm4, %xmm3, %xmm2

// CHECK:      dvpmuldhhq  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe2,0xcc,0x94,0xf4,0x00,0x00,0x00,0x10]
               dvpmuldhhq  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK:      dvpmuldhhq  291(%edi,%eax,4), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe2,0xcc,0x94,0x87,0x23,0x01,0x00,0x00]
               dvpmuldhhq  291(%edi,%eax,4), %xmm3, %xmm2

// CHECK:      dvpmuldhhq  (%eax), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe2,0xcc,0x10]
               dvpmuldhhq  (%eax), %xmm3, %xmm2

// CHECK:      dvpmuldhhq  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe2,0xcc,0x14,0x6d,0x00,0xfe,0xff,0xff]
               dvpmuldhhq  -512(,%ebp,2), %xmm3, %xmm2

// CHECK:      dvpmuldhhq  2032(%ecx), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe2,0xcc,0x91,0xf0,0x07,0x00,0x00]
               dvpmuldhhq  2032(%ecx), %xmm3, %xmm2

// CHECK:      dvpmuldhhq  -2048(%edx), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe2,0xcc,0x92,0x00,0xf8,0xff,0xff]
               dvpmuldhhq  -2048(%edx), %xmm3, %xmm2

// CHECK:      dvpmulds %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0x63,0xcd,0xd4]
               dvpmulds %xmm4, %xmm3, %xmm2

// CHECK:      dvpmulds  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0x63,0xcd,0x94,0xf4,0x00,0x00,0x00,0x10]
               dvpmulds  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK:      dvpmulds  291(%edi,%eax,4), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0x63,0xcd,0x94,0x87,0x23,0x01,0x00,0x00]
               dvpmulds  291(%edi,%eax,4), %xmm3, %xmm2

// CHECK:      dvpmulds  (%eax), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0x63,0xcd,0x10]
               dvpmulds  (%eax), %xmm3, %xmm2

// CHECK:      dvpmulds  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0x63,0xcd,0x14,0x6d,0x00,0xfe,0xff,0xff]
               dvpmulds  -512(,%ebp,2), %xmm3, %xmm2

// CHECK:      dvpmulds  2032(%ecx), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0x63,0xcd,0x91,0xf0,0x07,0x00,0x00]
               dvpmulds  2032(%ecx), %xmm3, %xmm2

// CHECK:      dvpmulds  -2048(%edx), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0x63,0xcd,0x92,0x00,0xf8,0xff,0xff]
               dvpmulds  -2048(%edx), %xmm3, %xmm2

// CHECK:      dvpmuludhhq %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe2,0xcb,0xd4]
               dvpmuludhhq %xmm4, %xmm3, %xmm2

// CHECK:      dvpmuludhhq  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe2,0xcb,0x94,0xf4,0x00,0x00,0x00,0x10]
               dvpmuludhhq  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK:      dvpmuludhhq  291(%edi,%eax,4), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe2,0xcb,0x94,0x87,0x23,0x01,0x00,0x00]
               dvpmuludhhq  291(%edi,%eax,4), %xmm3, %xmm2

// CHECK:      dvpmuludhhq  (%eax), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe2,0xcb,0x10]
               dvpmuludhhq  (%eax), %xmm3, %xmm2

// CHECK:      dvpmuludhhq  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe2,0xcb,0x14,0x6d,0x00,0xfe,0xff,0xff]
               dvpmuludhhq  -512(,%ebp,2), %xmm3, %xmm2

// CHECK:      dvpmuludhhq  2032(%ecx), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe2,0xcb,0x91,0xf0,0x07,0x00,0x00]
               dvpmuludhhq  2032(%ecx), %xmm3, %xmm2

// CHECK:      dvpmuludhhq  -2048(%edx), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe2,0xcb,0x92,0x00,0xf8,0xff,0xff]
               dvpmuludhhq  -2048(%edx), %xmm3, %xmm2

// CHECK:      dvpnmacdhhq %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe2,0xec,0xd4]
               dvpnmacdhhq %xmm4, %xmm3, %xmm2

// CHECK:      dvpnmacdhhq  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe2,0xec,0x94,0xf4,0x00,0x00,0x00,0x10]
               dvpnmacdhhq  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK:      dvpnmacdhhq  291(%edi,%eax,4), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe2,0xec,0x94,0x87,0x23,0x01,0x00,0x00]
               dvpnmacdhhq  291(%edi,%eax,4), %xmm3, %xmm2

// CHECK:      dvpnmacdhhq  (%eax), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe2,0xec,0x10]
               dvpnmacdhhq  (%eax), %xmm3, %xmm2

// CHECK:      dvpnmacdhhq  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe2,0xec,0x14,0x6d,0x00,0xfe,0xff,0xff]
               dvpnmacdhhq  -512(,%ebp,2), %xmm3, %xmm2

// CHECK:      dvpnmacdhhq  2032(%ecx), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe2,0xec,0x91,0xf0,0x07,0x00,0x00]
               dvpnmacdhhq  2032(%ecx), %xmm3, %xmm2

// CHECK:      dvpnmacdhhq  -2048(%edx), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe2,0xec,0x92,0x00,0xf8,0xff,0xff]
               dvpnmacdhhq  -2048(%edx), %xmm3, %xmm2

// CHECK:      dvpnmacdhhsq %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe2,0xed,0xd4]
               dvpnmacdhhsq %xmm4, %xmm3, %xmm2

// CHECK:      dvpnmacdhhsq  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe2,0xed,0x94,0xf4,0x00,0x00,0x00,0x10]
               dvpnmacdhhsq  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK:      dvpnmacdhhsq  291(%edi,%eax,4), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe2,0xed,0x94,0x87,0x23,0x01,0x00,0x00]
               dvpnmacdhhsq  291(%edi,%eax,4), %xmm3, %xmm2

// CHECK:      dvpnmacdhhsq  (%eax), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe2,0xed,0x10]
               dvpnmacdhhsq  (%eax), %xmm3, %xmm2

// CHECK:      dvpnmacdhhsq  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe2,0xed,0x14,0x6d,0x00,0xfe,0xff,0xff]
               dvpnmacdhhsq  -512(,%ebp,2), %xmm3, %xmm2

// CHECK:      dvpnmacdhhsq  2032(%ecx), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe2,0xed,0x91,0xf0,0x07,0x00,0x00]
               dvpnmacdhhsq  2032(%ecx), %xmm3, %xmm2

// CHECK:      dvpnmacdhhsq  -2048(%edx), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe2,0xed,0x92,0x00,0xf8,0xff,0xff]
               dvpnmacdhhsq  -2048(%edx), %xmm3, %xmm2

// CHECK:      dvpnmacdllq %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe0,0xec,0xd4]
               dvpnmacdllq %xmm4, %xmm3, %xmm2

// CHECK:      dvpnmacdllq  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe0,0xec,0x94,0xf4,0x00,0x00,0x00,0x10]
               dvpnmacdllq  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK:      dvpnmacdllq  291(%edi,%eax,4), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe0,0xec,0x94,0x87,0x23,0x01,0x00,0x00]
               dvpnmacdllq  291(%edi,%eax,4), %xmm3, %xmm2

// CHECK:      dvpnmacdllq  (%eax), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe0,0xec,0x10]
               dvpnmacdllq  (%eax), %xmm3, %xmm2

// CHECK:      dvpnmacdllq  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe0,0xec,0x14,0x6d,0x00,0xfe,0xff,0xff]
               dvpnmacdllq  -512(,%ebp,2), %xmm3, %xmm2

// CHECK:      dvpnmacdllq  2032(%ecx), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe0,0xec,0x91,0xf0,0x07,0x00,0x00]
               dvpnmacdllq  2032(%ecx), %xmm3, %xmm2

// CHECK:      dvpnmacdllq  -2048(%edx), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe0,0xec,0x92,0x00,0xf8,0xff,0xff]
               dvpnmacdllq  -2048(%edx), %xmm3, %xmm2

// CHECK:      dvpnmacdllsq %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe0,0xed,0xd4]
               dvpnmacdllsq %xmm4, %xmm3, %xmm2

// CHECK:      dvpnmacdllsq  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe0,0xed,0x94,0xf4,0x00,0x00,0x00,0x10]
               dvpnmacdllsq  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK:      dvpnmacdllsq  291(%edi,%eax,4), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe0,0xed,0x94,0x87,0x23,0x01,0x00,0x00]
               dvpnmacdllsq  291(%edi,%eax,4), %xmm3, %xmm2

// CHECK:      dvpnmacdllsq  (%eax), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe0,0xed,0x10]
               dvpnmacdllsq  (%eax), %xmm3, %xmm2

// CHECK:      dvpnmacdllsq  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe0,0xed,0x14,0x6d,0x00,0xfe,0xff,0xff]
               dvpnmacdllsq  -512(,%ebp,2), %xmm3, %xmm2

// CHECK:      dvpnmacdllsq  2032(%ecx), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe0,0xed,0x91,0xf0,0x07,0x00,0x00]
               dvpnmacdllsq  2032(%ecx), %xmm3, %xmm2

// CHECK:      dvpnmacdllsq  -2048(%edx), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe8,0xe0,0xed,0x92,0x00,0xf8,0xff,0xff]
               dvpnmacdllsq  -2048(%edx), %xmm3, %xmm2

