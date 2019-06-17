# REQUIRES: x86
# RUN: llvm-mc -filetype=obj -triple=x86_64-unknown-linux %s -o %t.o
# RUN: llvm-mc -filetype=obj -triple=x86_64-unknown-linux %p/Inputs/x86-64-cet1.s -o %t1.o
# RUN: llvm-mc -filetype=obj -triple=x86_64-unknown-linux %p/Inputs/x86-64-cet2.s -o %t2.o
# RUN: llvm-mc -filetype=obj -triple=x86_64-unknown-linux %p/Inputs/x86-64-cet3.s -o %t3.o
# RUN: llvm-mc -filetype=obj -triple=x86_64-unknown-linux %p/Inputs/x86-64-cet4.s -o %t4.o

# RUN: ld.lld -e func1 %t.o %t1.o -o %t
# RUN: llvm-readelf -n %t | FileCheck -check-prefix=CET -match-full-lines %s

# RUN: ld.lld -e func1 %t.o %t2.o -o %t
# RUN: llvm-readelf -n %t | FileCheck -check-prefix=CET -match-full-lines %s

# CET: Properties: x86 feature: IBT, SHSTK

# RUN: ld.lld -e func1 %t.o %t3.o -o %t
# RUN: llvm-readelf -S %t | FileCheck -check-prefix=NOCET %s

# NOCET:     Section Headers
# NOCET-NOT: .note.gnu.property

# RUN: not ld.lld -e func1 %t.o %t3.o -o %t --require-cet 2>&1 \
# RUN:   | FileCheck -check-prefix=ERROR %s
# ERROR: x86-64-cet.s.tmp3.o: --require-cet: file is not compatible with CET

# RUN: ld.lld -e func1 %t.o %t4.o -o %t
# RUN: llvm-readelf -n %t | FileCheck -check-prefix=NOSHSTK -match-full-lines %s

# Check .note.gnu.protery without property SHSTK.
# NOSHSTK: Properties: x86 feature: IBT

# INTEL_CUSTOMIZATION
# RUN: ld.lld -shared %t1.o -o %t1.so
# RUN: ld.lld -e func1 %t.o %t1.so -o %t
# RUN: llvm-readelf -n %t | FileCheck -check-prefix=CET -match-full-lines %s
# RUN: llvm-objdump -s -d %t | FileCheck -check-prefix=DISASM %s

# DISASM:      Disassembly of section .text:
# DISASM:      0000000000201000 func1:
# DISASM-NEXT: 201000:       e8 2b 00 00 00  callq   43 <func2+0x201030>
# DISASM-NEXT: 201005:       c3      retq

# DISASM:      Disassembly of section .plt:
# DISASM:      0000000000201010 .plt:
# DISASM-NEXT: 201010:       ff 35 f2 1f 00 00       pushq   8178(%rip)
# DISASM-NEXT: 201016:       ff 25 f4 1f 00 00       jmpq    *8180(%rip)
# DISASM-NEXT: 20101c:       0f 1f 40 00     nopl    (%rax)
# DISASM-NEXT: 201020:       f3 0f 1e fa     endbr64
# DISASM-NEXT: 201024:       68 00 00 00 00  pushq   $0
# DISASM-NEXT: 201029:       e9 e2 ff ff ff  jmp     -30 <.plt>
# DISASM-NEXT: 20102e:       66 90   nop

# DISASM:      Disassembly of section .plt.sec:
# DISASM:      0000000000201030 .plt.sec:
# DISASM-NEXT: 201030:       f3 0f 1e fa     endbr64
# DISASM-NEXT: 201034:       ff 25 de 1f 00 00       jmpq    *8158(%rip)
# DISASM-NEXT: 20103a:       66 0f 1f 44 00 00       nopw    (%rax,%rax)

# DISASM:      Contents of section .got.plt:
# DISASM-NEXT: 203000 00202000 00000000 00000000 00000000
# DISASM-NEXT: 203010 00000000 00000000 20102000 00000000
# end INTEL_CUSTOMIZATION
.section ".note.gnu.property", "a"
.long 4
.long 0x10
.long 0x5
.asciz "GNU"

.long 0xc0000002
.long 4
.long 3
.long 0

.text
.globl func1
.type func1,@function
func1:
  call func2
  ret
