# REQUIRES: x86
# RUN: llvm-mc -filetype=obj -triple=i386-unknown-linux %s -o %t.o
# RUN: llvm-mc -filetype=obj -triple=i386-unknown-linux %p/Inputs/i386-cet1.s -o %t1.o
# RUN: llvm-mc -filetype=obj -triple=i386-unknown-linux %p/Inputs/i386-cet2.s -o %t2.o
# RUN: llvm-mc -filetype=obj -triple=i386-unknown-linux %p/Inputs/i386-cet3.s -o %t3.o
# RUN: llvm-mc -filetype=obj -triple=i386-unknown-linux %p/Inputs/i386-cet4.s -o %t4.o

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
# ERROR: i386-cet.s.tmp3.o: --require-cet: file is not compatible with CET

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
# DISASM:      {{.*}} func1:
# DISASM-NEXT: {{.*}}              calll {{.*}}
# DISASM-NEXT: {{.*}}              retl

# DISASM:      Disassembly of section .plt:
# DISASM:      {{.*}}.plt:
# DISASM-NEXT: {{.*}} pushl {{.*}}
# DISASM-NEXT: {{.*}} jmpl  {{.*}}
# DISASM-NEXT: {{.*}} nop
# DISASM-NEXT: {{.*}} nop
# DISASM-NEXT: {{.*}} nop
# DISASM-NEXT: {{.*}} nop
# DISASM-NEXT: {{.*}} endbr32
# DISASM-NEXT: {{.*}} pushl   $0
# DISASM-NEXT: {{.*}} jmp     -30 <.plt>
# DISASM-NEXT: {{.*}} nop

# DISASM:      Disassembly of section .plt.sec:
# DISASM:      {{.*}}.plt.sec:
# DISASM-NEXT: {{.*}} endbr32
# DISASM-NEXT: {{.*}} jmpl  {{.*}}
# DISASM-NEXT: {{.*}} nopw  {{.*}}
# end INTEL_CUSTOMIZATION

.section ".note.gnu.property", "a"
.long 4
.long 0xc
.long 0x5
.asciz "GNU"

.long 0xc0000002
.long 4
.long 3

.text
.globl func1
.type func1,@function
func1:
  call func2
  ret
