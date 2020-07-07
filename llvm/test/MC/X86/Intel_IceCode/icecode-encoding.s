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
// CHECK: encoding: [0x2e,0x66,0x0f,0xb2,0x02]
          storeseg %cs:(%rdx)

// CHECK: storeseg %ss:(%rdx)
// CHECK: encoding: [0x36,0x66,0x0f,0xb2,0x02]
          storeseg %ss:(%rdx)

// CHECK: storeseg %ds:(%rdx)
// CHECK: encoding: [0x3e,0x66,0x0f,0xb2,0x02]
          storeseg %ds:(%rdx)

// CHECK: storeseg %es:(%rdx)
// CHECK: encoding: [0x26,0x66,0x0f,0xb2,0x02]
          storeseg %es:(%rdx)

// CHECK: storeseg %fs:(%rdx)
// CHECK: encoding: [0x64,0x66,0x0f,0xb2,0x02]
          storeseg %fs:(%rdx)

// CHECK: storeseg %gs:(%rdx)
// CHECK: encoding: [0x65,0x66,0x0f,0xb2,0x02]
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

// CHECK: creg_xchgl %eax, %ebx
// CHECK: encoding: [0xf3,0x01,0xc3]
          creg_xchg %eax, %ebx

// CHECK: creg_xchgq %eax, %rbx
// CHECK: encoding: [0xf3,0x48,0x01,0xc3]
          creg_xchg %eax, %rbx

// CHECK: creg_readl %eax, %ebx
// CHECK: encoding: [0xf3,0x29,0xc3]
          creg_read %eax, %ebx

// CHECK: creg_readq %eax, %rbx
// CHECK: encoding: [0xf3,0x48,0x29,0xc3]
          creg_read %eax, %rbx

// CHECK: fscp_xchgl %eax, %ebx
// CHECK: encoding: [0xf3,0x21,0xc3]
          fscp_xchg %eax, %ebx

// CHECK: fscp_xchgq %eax, %rbx
// CHECK: encoding: [0xf3,0x48,0x21,0xc3]
          fscp_xchg %eax, %rbx

// CHECK: fscp_readl %eax, %ebx
// CHECK: encoding: [0xf3,0x09,0xc3]
          fscp_read %eax, %ebx

// CHECK: fscp_readq %eax, %rbx
// CHECK: encoding: [0xf3,0x48,0x09,0xc3]
          fscp_read %eax, %rbx

// CHECK: creg_xchgl $16909060, %ebx
// CHECK: encoding: [0xf3,0x81,0xc3,0x04,0x03,0x02,0x01]
          creg_xchg $0x01020304, %ebx

// CHECK: creg_xchgq $16909060, %rbx
// CHECK: encoding: [0xf3,0x48,0x81,0xc3,0x04,0x03,0x02,0x01]
          creg_xchg $0x01020304, %rbx

// CHECK: creg_readl $16909060, %ebx
// CHECK: encoding: [0xf3,0x81,0xeb,0x04,0x03,0x02,0x01]
          creg_read $0x01020304, %ebx

// CHECK: creg_readq $16909060, %rbx
// CHECK: encoding: [0xf3,0x48,0x81,0xeb,0x04,0x03,0x02,0x01]
          creg_read $0x01020304, %rbx

// CHECK: fscp_xchgl $16909060, %ebx
// CHECK: encoding: [0xf3,0x81,0xe3,0x04,0x03,0x02,0x01]
          fscp_xchg $0x01020304, %ebx

// CHECK: fscp_xchgq $16909060, %rbx
// CHECK: encoding: [0xf3,0x48,0x81,0xe3,0x04,0x03,0x02,0x01]
          fscp_xchg $0x01020304, %rbx

// CHECK: fscp_readl $16909060, %ebx
// CHECK: encoding: [0xf3,0x81,0xcb,0x04,0x03,0x02,0x01]
          fscp_read $0x01020304, %ebx

// CHECK: fscp_readq $16909060, %rbx
// CHECK: encoding: [0xf3,0x48,0x81,0xcb,0x04,0x03,0x02,0x01]
          fscp_read $0x01020304, %rbx

// CHECK: portinb (%ebx), %al
// CHECK: encoding: [0xf3,0x8a,0x03]
          portin (%ebx), %al

// CHECK: portinw (%ebx), %ax
// CHECK: encoding: [0x66,0xf3,0x8b,0x03]
          portin (%ebx), %ax

// CHECK: portinl (%ebx), %eax
// CHECK: encoding: [0xf3,0x8b,0x03]
          portin (%ebx), %eax

// CHECK: portinq (%ebx), %rax
// CHECK: encoding: [0xf3,0x48,0x8b,0x03]
          portin (%ebx), %rax

// CHECK: portoutb %al, (%ebx)
// CHECK: encoding: [0xf3,0x88,0x03]
          portout %al, (%ebx)

// CHECK: portoutw %ax, (%ebx)
// CHECK: encoding: [0x66,0xf3,0x89,0x03]
          portout %ax, (%ebx)

// CHECK: portoutl %eax, (%ebx)
// CHECK: encoding: [0xf3,0x89,0x03]
          portout %eax, (%ebx)

// CHECK: portoutq %rax, (%ebx)
// CHECK: encoding: [0xf3,0x48,0x89,0x03]
          portout %rax, (%ebx)

// CHECK: sta_special (%eax)
// CHECK: encoding: [0xf3,0x8f,0x00]
          sta_special (%eax)

// CHECK: nr_read $6, %eax
// CHECK: encoding: [0xf3,0x83,0xf0,0x06]
          nr_read $6, %eax

// CHECK: sigeventjump $33, %ebx
// CHECK: encoding: [0xf3,0x81,0xd3,0x21,0x00,0x00,0x00]
          sigeventjump $0x21, %ebx

// CHECK: sserialize
// CHECK: encoding: [0x0f,0x01,0xd4]
          sserialize

// CHECK: ucodecall %eax
// CHECK: encoding: [0x0f,0x22,0xc0]
          ucodecall %eax

// CHECK: nop_set_sb
// CHECK: encoding: [0x27]
          nop_set_sb

// CHECK: nop_read_sb
// CHECK: encoding: [0x2f]
          nop_read_sb

// CHECK: get_excl_acc
// CHECK: encoding: [0xfc]
          get_excl_acc

// CHECK: release_excl_acc
// CHECK: encoding: [0xfd]
          release_excl_acc

// CHECK: virt_nuke_point
// CHECK: encoding: [0x0f,0x01,0xd5]
          virt_nuke_point

// CHECK: int_trap_point
// CHECK: encoding: [0x0f,0x01,0xd0]
          int_trap_point

// CHECK: iceret %eax
// CHECK: encoding: [0x0f,0xaa]
          iceret %eax

// CHECK: iceret_indirect %ebx, %eax
// CHECK: encoding: [0xf3,0x0f,0xaa]
          iceret_indirect %ebx, %eax

// CHECK: cmodemov $0, %rax, %rbx
// CHECK: encoding: [0xf3,0x0f,0x40,0xd8]
          cmodemov $0, %rax, %rbx

// CHECK: cmodemov $1, %rax, %rbx
// CHECK: encoding: [0xf3,0x0f,0x41,0xd8]
          cmodemov $1, %rax, %rbx

// CHECK: cmodemov $3, %rax, %rbx
// CHECK: encoding: [0xf3,0x0f,0x43,0xd8]
          cmodemov $3, %rax, %rbx

// CHECK: cmodemov $4, %rax, %rbx
// CHECK: encoding: [0xf3,0x0f,0x44,0xd8]
          cmodemov $4, %rax, %rbx

// CHECK: cmodemov $5, %rax, %rbx
// CHECK: encoding: [0xf3,0x0f,0x45,0xd8]
          cmodemov $5, %rax, %rbx

// CHECK: cmodemov $6, %rax, %rbx
// CHECK: encoding: [0xf3,0x0f,0x46,0xd8]
          cmodemov $6, %rax, %rbx

// CHECK: cmodemov $7, %rax, %rbx
// CHECK: encoding: [0xf3,0x0f,0x47,0xd8]
          cmodemov $7, %rax, %rbx

// CHECK: cmodemov $8, %rax, %rbx
// CHECK: encoding: [0xf3,0x0f,0x48,0xd8]
          cmodemov $8, %rax, %rbx

// CHECK: cmodemov $9, %rax, %rbx
// CHECK: encoding: [0xf3,0x0f,0x49,0xd8]
          cmodemov $9, %rax, %rbx

// CHECK: cmodemov $10, %rax, %rbx
// CHECK: encoding: [0xf3,0x0f,0x4a,0xd8]
          cmodemov $0xa, %rax, %rbx

// CHECK: cmodemov $11, %rax, %rbx
// CHECK: encoding: [0xf3,0x0f,0x4b,0xd8]
          cmodemov $0xb, %rax, %rbx

// CHECK: cmodemov $12, %rax, %rbx
// CHECK: encoding: [0xf3,0x0f,0x4c,0xd8]
          cmodemov $0xc, %rax, %rbx

// CHECK: cmodemov $13, %rax, %rbx
// CHECK: encoding: [0xf3,0x0f,0x4d,0xd8]
          cmodemov $0xd, %rax, %rbx

// CHECK: cmodemov $14, %rax, %rbx
// CHECK: encoding: [0xf3,0x0f,0x4e,0xd8]
          cmodemov $0xe, %rax, %rbx

// CHECK: cmodemov $15, %rax, %rbx
// CHECK: encoding: [0xf3,0x0f,0x4f,0xd8]
          cmodemov $0xf, %rax, %rbx

// CHECK: settracker $1
// CHECK: encoding: [0xf3,0x6a,0x01]
          settracker $1

// CHECK: portin24
// CHECK: encoding: [0xf2,0xad]
          portin24

// CHECK: portout24
// CHECK: encoding: [0xf2,0xab]
          portout24
