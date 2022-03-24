# REQUIRES: intel_feature_xucc
# RUN: llvm-mc -triple x86_64_xucc-unknown-unknown -x86-asm-syntax=intel -output-asm-variant=1  --show-encoding < %s  | FileCheck %s

# CHECK: acquireepoch rax, xmmword ptr [rbx]
# check: encoding: [0x0f,0x00,0x13]
acquireepoch rax, xmmword ptr [rbx]

# CHECK: asidswitch_tlbflush rbx, rax
# CHECK: encoding: [0x66,0x0f,0x03,0xd8]
asidswitch_tlbflush rbx, rax

# CHECK: asidswitch_tlbflush rbx, qword ptr [rax]
# CHECK: encoding: [0x66,0x0f,0x03,0x18]
asidswitch_tlbflush rbx, qword ptr [rax]

# CHECK: cmodemov rbx, rax, rcx
# CHECK: encoding: [0x0f,0x03,0xd8]
cmodemov rbx, rax, rcx

# CHECK: cmodemov rbx, qword ptr [rax], rcx
# CHECK: encoding: [0x0f,0x03,0x18]
cmodemov rbx, qword ptr [rax], rcx

# CHECK: getbasekey rdx
# CHECK: encoding: [0x0f,0x00,0xe2]
getbasekey rdx

# CHECK: gmovlin ax, word ptr [rbx]
# CHECK: encoding: [0x66,0xf3,0x0f,0x02,0x03]
gmovlin ax, word ptr [rbx]

# CHECK: gmovlin eax, dword ptr [rbx]
# CHECK: encoding: [0xf3,0x0f,0x02,0x03]
gmovlin eax, dword ptr [rbx]

# CHECK: gmovlin rax, qword ptr [rbx]
# CHECK: encoding: [0xf3,0x48,0x0f,0x02,0x03]
gmovlin rax, qword ptr [rbx]

# CHECK: gmovpphys cx, word ptr [rdx]
# CHECK: encoding: [0x66,0xf3,0x0f,0xb4,0x0a]
gmovpphys cx, word ptr [rdx]

# CHECK: gmovpphys ecx, dword ptr [rdx]
# CHECK: encoding: [0xf3,0x0f,0xb4,0x0a]
gmovpphys ecx, dword ptr [rdx]

# CHECK: gmovpphys rcx, qword ptr [rdx]
# CHECK: encoding: [0xf3,0x48,0x0f,0xb4,0x0a]
gmovpphys rcx, qword ptr [rdx]

# CHECK: gmovpphys word ptr [rdx], cx
# CHECK: encoding: [0x66,0xf2,0x0f,0xb4,0x0a]
gmovpphys word ptr [rdx], cx

# CHECK: gmovpphys  dword ptr [rdx], ecx
# CHECK: encoding: [0xf2,0x0f,0xb4,0x0a]
gmovpphys dword ptr [rdx], ecx

# CHECK: gmovpphys  qword ptr [rdx], rcx
# CHECK: encoding: [0xf2,0x48,0x0f,0xb4,0x0a]
gmovpphys qword ptr [rdx], rcx

# CHECK: gtranslaterd_noepc qword ptr [rax]
# CHECK: encoding: [0xf3,0x0f,0x00,0x28]
gtranslaterd_noepc qword ptr [rax]

# CHECK: gtranslatewr_noepc qword ptr [rdx]
# CHECK: encoding: [0xf2,0x0f,0x00,0x2a]
gtranslatewr_noepc qword ptr [rdx]

# CHECK: gtranslaterd_tioprm  qword ptr [rax]
# CHECK: encoding: [0x66,0xf3,0x0f,0x00,0x28]
gtranslaterd_tioprm qword ptr [rax]

# CHECK: gtranslatewr_tioprm  qword ptr [rcx]
# CHECK: encoding: [0x66,0xf2,0x0f,0x00,0x29]
gtranslatewr_tioprm qword ptr [rcx]

# CHECK: gtranslaterd_epc qword ptr [rbx]
# CHECK: encoding: [0xf3,0x0f,0x00,0x23]
gtranslaterd_epc qword ptr [rbx]

# CHECK: gtranslatewr_epc qword ptr [r14]
# CHECK: encoding: [0xf2,0x41,0x0f,0x00,0x26]
gtranslatewr_epc qword ptr [r14]

# CHECK: rsworld dword ptr [rbx], rax
# CHECK: encoding: [0xf3,0x66,0x0f,0xb5,0x03]
rsworld [rbx], rax

# CHECK: spbusmsg rax, qword ptr [rcx]
# CHECK:# encoding: [0x66,0xf2,0x0f,0x03,0x01]
spbusmsg rax, qword ptr [rcx]

# CHECK: spbusmsg rdx, rcx
# CHECK: encoding: [0x66,0xf2,0x0f,0x03,0xca]
spbusmsg rdx, rcx

# CHECK: svworld dword ptr [rdx], rcx
# CHECK: encoding: [0xf2,0x66,0x0f,0xb5,0x0a]
svworld dword ptr [rdx], rcx

# CHECK: loadseg xmmword ptr cs:[rdx]
# CHECK: encoding: [0x2e,0x0f,0xb2,0x02]
loadseg xmmword ptr cs:[rdx]

# CHECK: loadseg xmmword ptr ss:[rdx]
# CHECK: encoding: [0x36,0x0f,0xb2,0x02]
loadseg xmmword ptr ss:[rdx]

# CHECK: loadseg xmmword ptr ds:[rdx]
# CHECK: encoding: [0x3e,0x0f,0xb2,0x02]
loadseg xmmword ptr ds:[rdx]

# CHECK: loadseg xmmword ptr es:[rdx]
# CHECK: encoding: [0x26,0x0f,0xb2,0x02]
loadseg xmmword ptr es:[rdx]

# CHECK: loadseg xmmword ptr fs:[rdx]
# CHECK: encoding: [0x64,0x0f,0xb2,0x02]
loadseg xmmword ptr fs:[rdx]

# CHECK: loadseg xmmword ptr gs:[rdx]
# CHECK: encoding: [0x65,0x0f,0xb2,0x02]
loadseg xmmword ptr gs:[rdx]

# CHECK: storeseg xmmword ptr cs:[rdx]
# CHECK: encoding: [0x2e,0x0f,0xb4,0x02]
storeseg xmmword ptr cs:[rdx]

# CHECK: storeseg xmmword ptr ss:[rdx]
# CHECK: encoding: [0x36,0x0f,0xb4,0x02]
storeseg xmmword ptr ss:[rdx]

# CHECK: storeseg xmmword ptr ds:[rdx]
# CHECK: encoding: [0x3e,0x0f,0xb4,0x02]
storeseg xmmword ptr ds:[rdx]

# CHECK: storeseg xmmword ptr es:[rdx]
# CHECK: encoding: [0x26,0x0f,0xb4,0x02]
storeseg xmmword ptr es:[rdx]

# CHECK: storeseg xmmword ptr fs:[rdx]
# CHECK: encoding: [0x64,0x0f,0xb4,0x02]
storeseg xmmword ptr fs:[rdx]

# CHECK: storeseg xmmword ptr gs:[rdx]
# CHECK: encoding: [0x65,0x0f,0xb4,0x02]
storeseg xmmword ptr gs:[rdx]

# CHECK: int1
# CHECK: encoding: [0xf1]
int1
