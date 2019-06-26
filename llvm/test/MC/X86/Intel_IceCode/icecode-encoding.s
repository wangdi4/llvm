// REQUIRES: intel_feature_icecode
// RUN: llvm-mc -triple x86_icecode-unknown-unknown -mattr=icecode-mode --show-encoding < %s  | FileCheck %s

// CHECK: gmovpphysw (%rdx), %ax
// CHECK: encoding: [0x66,0xf3,0x0f,0xb4,0x02]
          gmovpphys (%rdx), %ax

// CHECK: gmovpphysl (%rdx), %eax
// CHECK: encoding: [0xf3,0x0f,0xb4,0x02]
          gmovpphys (%rdx), %eax

// CHECK: gmovpphysq (%rdx), %rax
// CHECK: encoding: [0xf3,0x48,0x0f,0xb4,0x02]
          gmovpphys (%rdx), %rax

// CHECK: gmovpphysw %ax, (%rdx)
// CHECK: encoding: [0x66,0xf2,0x0f,0xb4,0x02]
          gmovpphys %ax, (%rdx)

// CHECK: gmovpphysl %eax, (%rdx)
// CHECK: encoding: [0xf2,0x0f,0xb4,0x02]
          gmovpphys %eax, (%rdx)

// CHECK: gmovpphysq %rax, (%rdx)
// CHECK: encoding: [0xf2,0x48,0x0f,0xb4,0x02]
          gmovpphys %rax, (%rdx)

// CHECK: loadseg %cs:(%rdx)
// CHECK: encoding: [0x2e,0x0f,0xb2,0x02]
          loadseg %cs:(%rdx)

// CHECK: loadseg %ss:(%rdx)
// CHECK: encoding: [0x36,0x0f,0xb2,0x02]
          loadseg %ss:(%rdx)

// CHECK: loadseg %ds:(%rdx)
// CHECK: encoding: [0x3e,0x0f,0xb2,0x02]
          loadseg %ds:(%rdx)

// CHECK: loadseg %es:(%rdx)
// CHECK: encoding: [0x26,0x0f,0xb2,0x02]
          loadseg %es:(%rdx)

// CHECK: loadseg %fs:(%rdx)
// CHECK: encoding: [0x64,0x0f,0xb2,0x02]
          loadseg %fs:(%rdx)

// CHECK: loadseg %gs:(%rdx)
// CHECK: encoding: [0x65,0x0f,0xb2,0x02]
          loadseg %gs:(%rdx)

// CHECK: storeseg %cs:(%rdx)
// CHECK: encoding: [0x2e,0x0f,0xb4,0x02]
          storeseg %cs:(%rdx)

// CHECK: storeseg %ss:(%rdx)
// CHECK: encoding: [0x36,0x0f,0xb4,0x02]
          storeseg %ss:(%rdx)

// CHECK: storeseg %ds:(%rdx)
// CHECK: encoding: [0x3e,0x0f,0xb4,0x02]
          storeseg %ds:(%rdx)

// CHECK: storeseg %es:(%rdx)
// CHECK: encoding: [0x26,0x0f,0xb4,0x02]
          storeseg %es:(%rdx)

// CHECK: storeseg %fs:(%rdx)
// CHECK: encoding: [0x64,0x0f,0xb4,0x02]
          storeseg %fs:(%rdx)

// CHECK: storeseg %gs:(%rdx)
// CHECK: encoding: [0x65,0x0f,0xb4,0x02]
          storeseg %gs:(%rdx)

// CHECK: gtranslaterd_epc (%rdx)
// CHECK: encoding: [0xf3,0x0f,0x00,0x22]
          gtranslaterd_epc (%rdx)

// CHECK: gtranslatewr_epc (%rdx)
// CHECK: encoding: [0xf2,0x0f,0x00,0x22]
          gtranslatewr_epc (%rdx)

// CHECK: gtranslaterd_noepc (%rdx)
// CHECK: encoding: [0xf3,0x0f,0x00,0x2a]
          gtranslaterd_noepc (%rdx)

// CHECK: gtranslatewr_noepc (%rdx)
// CHECK: encoding: [0xf2,0x0f,0x00,0x2a]
          gtranslatewr_noepc (%rdx)

// CHECK: bcast_seg_state
// CHECK: encoding: [0xf3,0x0f,0x01,0xef]
          bcast_seg_state
