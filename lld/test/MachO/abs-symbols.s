# IF INTEL_CUSTOMIZATION
# This test case has been marked as unsupported for OCL compiler and Windows
# until the issue with CMPLRLLVM-10268 is fixed.
# UNSUPPORTED: intel_opencl && i686-pc-windows
# END INTEL_CUSTOMIZATION
# REQUIRES: x86
# RUN: llvm-mc -filetype=obj -triple=x86_64-apple-darwin %s -o %t.o
# RUN: %lld -lSystem %t.o -o %t
# RUN: llvm-objdump --macho --syms --exports-trie %t | FileCheck %s

# CHECK-LABEL: SYMBOL TABLE:
# CHECK-DAG:   000000000000dead g       *ABS* _foo
# CHECK-DAG:   000000000000beef g       *ABS* _weakfoo

# CHECK-LABEL: Exports trie:
# CHECK-DAG:   0x0000DEAD  _foo [absolute]
# CHECK-DAG:   0x0000BEEF  _weakfoo [absolute]

.globl _foo, _weakfoo, _main
.weak_definition _weakfoo
_foo = 0xdead
_weakfoo = 0xbeef

.text
_main:
  ret

## TODO: once we support emitting local symbols in the symtab, test local
## absolute symbols too
