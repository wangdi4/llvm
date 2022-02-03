# This test case checks that the module definition parser works with files
# encoded in UTF-32 byte order mark little endian (BOM LE). The input file
# contains Japanese characters to enforce the use of UTF.

# NOTE: The input file is encoded in UTF-32, most of the text editors won't
# process the characters and will assume that it is opening a binary file.
# This command can convert the file to UTF-8:
# 'iconv -f UTF32LE -t UTF8 Inputs/utf-32-bom-be.def -o test.def'

# Check that the file is encoded in UTF-32 BOM LE by doing a grep for the
# sequence '\xff\xfe\x00\x00' in the input definition file. Grep sees UTF-32
# as a binary object file:

# RUN: grep -P '\xff\xfe\x00\x00' %S/Inputs/utf-32-bom-le.def | FileCheck %s --check-prefix=CHECK-UTF-32-LE
CHECK-UTF-32-LE: Binary file
CHECK-UTF-32-LE-SAME: utf-32-bom-le.def matches

# The pattern for big endian should not be in the input file. Grep returns
# error code 1 when pattern is not found. We use 'not' to convert the error
# code into 0.
# RUN: not grep -P '\x00\x00\xfe\xff' %S/Inputs/utf-32-bom-le.def | FileCheck %s --allow-empty --check-prefix=CHECK-UTF-32-BE
CHECK-UTF-32-BE-NOT: Binary file

# Create the library

# RUN: lld-link /machine:x64 /def:%S/Inputs/utf-32-bom-le.def  /out:%t.lib | FileCheck %s --allow-empty --check-prefix=CHECK-LIB
CHECK-LIB-NOT: unknown directive

# Check that the library was created correctly.

# RUN: llvm-nm %t.lib | FileCheck %s --check-prefix=CHECK-NM
CHECK-NM: 00000000 I __IMPORT_DESCRIPTOR

