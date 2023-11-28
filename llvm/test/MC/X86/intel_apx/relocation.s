# RUN: llvm-mc -triple x86_64 -filetype=obj <%s | llvm-readelf --relocs - | FileCheck %s

# CHECK: Relocation section '.rela.text' at offset 0x78 contains 1 entries:
# CHECK:    Offset             Info             Type               Symbol's Value  Symbol's Name + Addend
# CHECK:    0000000000000004  0000000100000002 R_X86_64_PC32          0000000000000000 ABC - 4

movq    ABC(%rip), %r16
