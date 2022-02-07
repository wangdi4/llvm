# This test case checks that the module definition parser works with files
# encoded in UTF-16 byte order mark big endian (BOM BE). The input file
# contains Japanese characters to enforce the use of UTF.

# Check that the file is encoded in UTF-16 BOM BE:
# RUN: python %S/Inputs/utf-encoding-check.py %S/Inputs/utf-16-bom-be.def utf-16be | FileCheck %s --check-prefix=CHECK-UTF-16-BE
CHECK-UTF-16-BE: Pass

# The pattern for little endian should not be in the input file. The python
# script will return error code 1 since the encoding mismatch with the input
# file. We use 'not' to convert the error code into 0.
# RUN: not python %S/Inputs/utf-encoding-check.py %S/Inputs/utf-16-bom-be.def utf-16le 2>&1 FileCheck %s --allow-empty --check-prefix=CHECK-UTF-16-LE
CHECK-UTF-16-LE: Encoding mismatch

# Create the library

# RUN: lld-link /machine:x64 /def:%S/Inputs/utf-16-bom-be.def  /out:%t.lib | FileCheck %s --allow-empty --check-prefix=CHECK-LIB
CHECK-LIB-NOT: unknown directive

# Check that the library was created correctly.

# RUN: llvm-nm %t.lib | FileCheck %s --check-prefix=CHECK-NM
CHECK-NM: 00000000 I __IMPORT_DESCRIPTOR

