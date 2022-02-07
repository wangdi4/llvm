# This test case checks that the module definition parser works with files
# encoded in UTF-8 byte order mark (BOM). The input file contains Japanese
# characters to enforce the use of UTF.

# Check that the file is encoded in UTF-8 BOM:
# RUN: python %S/Inputs/utf-encoding-check.py %S/Inputs/utf-8-bom.def utf-8-sig | FileCheck %s --check-prefix=CHECK-UTF-8
CHECK-UTF-8: Pass

# Create the library

# RUN: lld-link /machine:x64 /def:%S/Inputs/utf-8-bom.def  /out:%t.lib | FileCheck %s --allow-empty --check-prefix=CHECK-LIB
CHECK-LIB-NOT: unknown directive

# Check that the library was created correctly.

# RUN: llvm-nm %t.lib | FileCheck %s --check-prefix=CHECK-NM
CHECK-NM: 00000000 I __IMPORT_DESCRIPTOR

