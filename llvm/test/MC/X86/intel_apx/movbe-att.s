# RUN: llvm-mc -triple x86_64 -show-encoding %s | FileCheck %s
# RUN: not llvm-mc -triple i386 -show-encoding %s 2>&1 | FileCheck %s --check-prefix=ERROR

# ERROR-COUNT-9: error:
# ERROR-NOT: error:
# CHECK: movbew	(%r16,%r17), %r18w
# CHECK: encoding: [0x62,0xec,0x79,0x08,0x60,0x14,0x08]
         movbew	(%r16,%r17), %r18w
# CHECK: movbel	(%r16,%r17), %r18d
# CHECK: encoding: [0x62,0xec,0x78,0x08,0x60,0x14,0x08]
         movbel	(%r16,%r17), %r18d
# CHECK: movbeq	(%r16,%r17), %r18
# CHECK: encoding: [0x62,0xec,0xf8,0x08,0x60,0x14,0x08]
         movbeq	(%r16,%r17), %r18

# CHECK: movbew	%r16w, (%r17,%r18)
# CHECK: encoding: [0x62,0xec,0x79,0x08,0x61,0x04,0x11]
         movbew	%r16w, (%r17,%r18)
# CHECK: movbel	%r16d, (%r17,%r18)
# CHECK: encoding: [0x62,0xec,0x78,0x08,0x61,0x04,0x11]
         movbel	%r16d, (%r17,%r18)
# CHECK: movbeq	%r16, (%r17,%r18)
# CHECK: encoding: [0x62,0xec,0xf8,0x08,0x61,0x04,0x11]
         movbeq	%r16, (%r17,%r18)

# CHECK: movbew	 %r16w, %r17w
# CHECK: encoding: [0x62,0xec,0x7d,0x08,0x61,0xc1]
         movbew	 %r16w, %r17w
# CHECK: movbel	 %r16d, %r17d
# CHECK: encoding: [0x62,0xec,0x7c,0x08,0x61,0xc1]
         movbel	 %r16d, %r17d
# CHECK: movbeq	 %r16, %r17
# CHECK: encoding: [0x62,0xec,0xfc,0x08,0x61,0xc1]
         movbeq	 %r16, %r17
