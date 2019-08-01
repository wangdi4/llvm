// This tests that our repo url is not stored in AST/PCH files.
// RUN: echo 'int foo = 0;' > %t_header1.h
// RUN: %clang_cc1 -x c-header %t_header1.h -emit-pch -o %t.pch
// RUN: ! grep -a -e 'ssh:.*intel.com' %t.pch > /dev/null
// REQUIRES: shell
