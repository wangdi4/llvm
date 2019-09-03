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
# DISASM:      0040124c func1:
# DISASM-NEXT:   40124c: e8 2f 00 00 00                calll   47 <func2+0x401280>
# DISASM-NEXT:   401251: c3                            retl

# DISASM:      Disassembly of section .plt:
# DISASM:      00401260 .plt:
# DISASM-NEXT:   401260: ff 35 fc 32 40 00             pushl   4207356
# DISASM-NEXT:   401266: ff 25 00 33 40 00             jmpl    *4207360
# DISASM-NEXT:   40126c: 90                            nop
# DISASM-NEXT:   40126d: 90                            nop
# DISASM-NEXT:   40126e: 90                            nop
# DISASM-NEXT:   40126f: 90                            nop
# DISASM-NEXT:   401270: f3 0f 1e fb                   endbr32
# DISASM-NEXT:   401274: 68 00 00 00 00                pushl   $0
# DISASM-NEXT:   401279: e9 e2 ff ff ff                jmp     -30 <.plt>
# DISASM-NEXT:   40127e: 66 90                         nop

# DISASM:      Disassembly of section .plt.sec:
# DISASM:      00401280 .plt.sec:
# DISASM-NEXT:   401280: f3 0f 1e fb                   endbr32
# DISASM-NEXT:   401284: ff 25 04 33 40 00             jmpl    *4207364
# DISASM-NEXT:   40128a: 66 0f 1f 44 00 00             nopw    (%eax,%eax)

# DISASM:      Contents of section .got.plt:
# DISASM-NEXT:   4032f8 90224000 00000000 00000000 70124000  ."@.........p.@.
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
