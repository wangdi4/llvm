# This test case checks that the module definition parser works with files
# encoded in UTF-16 byte order mark big endian (BOM BE). The input file
# contains Japanese characters to enforce the use of UTF.

# Check that the file is encoded in UTF-16 BOM BE by doing a grep for the
# sequence '\xfe\xff' in the input definition file. Grep sees UTF-16 as
# a binary object file:

# RUN: grep -P '\xfe\xff' %S/Inputs/utf-16-bom-be.def | FileCheck %s --check-prefix=CHECK-UTF-16-BE
CHECK-UTF-16-BE: Binary file
CHECK-UTF-16-BE-SAME: utf-16-bom-be.def matches

# The pattern for little endian should not be in the input file. Grep returns
# error code 1 when pattern is not found. We use 'not' to convert the error
# code into 0.
# RUN: not grep -P '\xff\xfe' %S/Inputs/utf-16-bom-be.def | FileCheck %s --allow-empty --check-prefix=CHECK-UTF-16-LE
CHECK-UTF-16-LE-NOT: Binary file

# Create the library

# RUN: lld-link /machine:x64 /def:%S/Inputs/utf-16-bom-be.def  /out:%t.lib | FileCheck %s --allow-empty --check-prefix=CHECK-LIB
CHECK-LIB-NOT: unknown directive

# Check that the library was created correctly.

# RUN: llvm-nm %t.lib | FileCheck %s --check-prefix=CHECK-NM
CHECK-NM: 00000000 I __IMPORT_DESCRIPTOR

