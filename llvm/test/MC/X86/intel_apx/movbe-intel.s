# RUN: llvm-mc -triple x86_64 -show-encoding -x86-asm-syntax=intel -output-asm-variant=1 %s | FileCheck %s

# CHECK: movbe	r18w, word ptr [r16 + r17]
# CHECK: encoding: [0x62,0xec,0x79,0x08,0x60,0x14,0x08]
         movbe	r18w, word ptr [r16 + r17]
# CHECK: movbe	r18d, dword ptr [r16 + r17]
# CHECK: encoding: [0x62,0xec,0x78,0x08,0x60,0x14,0x08]
         movbe	r18d, dword ptr [r16 + r17]
# CHECK: movbe	r18, qword ptr [r16 + r17]
# CHECK: encoding: [0x62,0xec,0xf8,0x08,0x60,0x14,0x08]
         movbe	r18, qword ptr [r16 + r17]

# CHECK: movbe	word ptr [r17 + r18], r16w
# CHECK: encoding: [0x62,0xec,0x79,0x08,0x61,0x04,0x11]
         movbe	word ptr [r17 + r18], r16w
# CHECK: movbe	dword ptr [r17 + r18], r16d
# CHECK: encoding: [0x62,0xec,0x78,0x08,0x61,0x04,0x11]
         movbe	dword ptr [r17 + r18], r16d
# CHECK: movbe	qword ptr [r17 + r18], r16
# CHECK: encoding: [0x62,0xec,0xf8,0x08,0x61,0x04,0x11]
         movbe	qword ptr [r17 + r18], r16

# CHECK: movbe	r17w, r16w
# CHECK: encoding: [0x62,0xec,0x7d,0x08,0x61,0xc1]
         movbe	r17w, r16w
# CHECK: movbe	r17d, r16d
# CHECK: encoding: [0x62,0xec,0x7c,0x08,0x61,0xc1]
         movbe	r17d, r16d
# CHECK: movbe	r17, r16
# CHECK: encoding: [0x62,0xec,0xfc,0x08,0x61,0xc1]
         movbe	r17, r16
