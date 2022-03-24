// REQUIRES: intel_feature_xucc
// RUN: llvm-mc -triple x86_64_xucc-unknown-unknown --show-encoding < %s  | FileCheck %s

# CHECK: acquireepoch (%rbx), %rax
# check: encoding: [0x0f,0x00,0x13]
acquireepoch (%rbx), %rax

# CHECK: asidswitch_tlbflush %rax, %rbx
# CHECK: encoding: [0x66,0x0f,0x03,0xd8]
asidswitch_tlbflush %rax, %rbx

# CHECK: asidswitch_tlbflush (%rax), %rbx
# CHECK: encoding: [0x66,0x0f,0x03,0x18]
asidswitch_tlbflush (%rax), %rbx

# CHECK: cmodemov %rcx, %rax, %rbx
# CHECK: encoding: [0x0f,0x03,0xd8]
cmodemov %rcx, %rax, %rbx

# CHECK: cmodemov %rcx, (%rax), %rbx
# CHECK: encoding: [0x0f,0x03,0x18]
cmodemov %rcx, (%rax), %rbx

# CHECK: getbasekey %rdx
# CHECK: encoding: [0x0f,0x00,0xe2]
getbasekey %rdx

# CHECK: gmovlin (%rbx), %ax
# CHECK: encoding: [0x66,0xf3,0x0f,0x02,0x03]
gmovlin (%rbx), %ax

# CHECK: gmovlin (%rbx), %eax
# CHECK: encoding: [0xf3,0x0f,0x02,0x03]
gmovlin (%rbx), %eax

# CHECK: gmovlin (%rbx), %rax
# CHECK: encoding: [0xf3,0x48,0x0f,0x02,0x03]
gmovlin (%rbx), %rax

# CHECK: gmovpphys (%rdx), %cx
# CHECK: encoding: [0x66,0xf3,0x0f,0xb4,0x0a]
gmovpphys (%rdx), %cx

# CHECK: gmovpphys (%rdx), %ecx
# CHECK: encoding: [0xf3,0x0f,0xb4,0x0a]
gmovpphys (%rdx), %ecx

# CHECK: gmovpphys (%rdx), %rcx
# CHECK: encoding: [0xf3,0x48,0x0f,0xb4,0x0a]
gmovpphys (%rdx), %rcx

# CHECK: gmovpphys %cx, (%rdx)
# CHECK: encoding: [0x66,0xf2,0x0f,0xb4,0x0a]
gmovpphys %cx, (%rdx)

# CHECK: gmovpphys  %ecx, (%rdx)
# CHECK: encoding: [0xf2,0x0f,0xb4,0x0a]
gmovpphys %ecx, (%rdx)

# CHECK: gmovpphys  %rcx, (%rdx)
# CHECK: encoding: [0xf2,0x48,0x0f,0xb4,0x0a]
gmovpphys %rcx, (%rdx)

# CHECK: gtranslaterd_noepc (%rax)
# CHECK: encoding: [0xf3,0x0f,0x00,0x28]
gtranslaterd_noepc (%rax)

# CHECK: gtranslatewr_noepc (%rdx)
# CHECK: encoding: [0xf2,0x0f,0x00,0x2a]
gtranslatewr_noepc (%rdx)

# CHECK: gtranslaterd_tioprm  (%rax)
# CHECK: encoding: [0x66,0xf3,0x0f,0x00,0x28]
gtranslaterd_tioprm (%rax)

# CHECK: gtranslatewr_tioprm  (%rcx)
# CHECK: encoding: [0x66,0xf2,0x0f,0x00,0x29]
gtranslatewr_tioprm (%rcx)

# CHECK: gtranslaterd_epc (%rbx)
# CHECK: encoding: [0xf3,0x0f,0x00,0x23]
gtranslaterd_epc (%rbx)

# CHECK: gtranslatewr_epc (%r14)
# CHECK: encoding: [0xf2,0x41,0x0f,0x00,0x26]
gtranslatewr_epc (%r14)

# CHECK: rsworld %rax, (%rbx)
# CHECK: encoding: [0xf3,0x66,0x0f,0xb5,0x03]
rsworld %rax, (%rbx)

# CHECK: spbusmsg (%rcx), %rax
# CHECK:# encoding: [0x66,0xf2,0x0f,0x03,0x01]
spbusmsg (%rcx), %rax

# CHECK: spbusmsg %rcx, %rdx
# CHECK: encoding: [0x66,0xf2,0x0f,0x03,0xca]
spbusmsg %rcx, %rdx

# CHECK: svworld %rcx, (%rdx)
# CHECK: encoding: [0xf2,0x66,0x0f,0xb5,0x0a]
svworld %rcx, (%rdx)

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

// CHECK: int1
// CHECK: encoding: [0xf1]
int1
