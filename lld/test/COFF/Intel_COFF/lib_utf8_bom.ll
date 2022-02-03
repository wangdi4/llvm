# This test case checks that the module definition parser works with files
# encoded in UTF-8 byte order mark (BOM). The input file contains Japanese
# characters to enforce the use of UTF.

# Check that the file is encoded in UTF-8 BOM by doing a grep for the
# sequence '\xef\xbb\xbf' in the input definition file. It should
# print the first line of the input file:

# RUN: grep -P '\xef\xbb\xbf' %S/Inputs/utf-8-bom.def | FileCheck %s --check-prefix=CHECK-UTF-8
CHECK-UTF-8: Input file for lib_utf8_bom.ll. This file is encoded in UTF-8 BOM

# Create the library

# RUN: lld-link /machine:x64 /def:%S/Inputs/utf-8-bom.def  /out:%t.lib | FileCheck %s --allow-empty --check-prefix=CHECK-LIB
CHECK-LIB-NOT: unknown directive

# Check that the library was created correctly.

# RUN: llvm-nm %t.lib | FileCheck %s --check-prefix=CHECK-NM
CHECK-NM: 00000000 I __IMPORT_DESCRIPTOR

