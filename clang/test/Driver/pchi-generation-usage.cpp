// This test generates a PCH header file with .pchi file extension and
// verifies that the driver does an implicit include of the .pchi file
// via '-include-pch %t.h.pchi' when using '-include %t.h'

// RUN: touch %t.h
// RUN: touch %t.h.pchi

// Check if the deafult PCH file generated is a .pchi file.
// RUN: %clang -x c++-header %t.h -###  %s -### 2> %t1.txt
// RUN: FileCheck %s -input-file=%t1.txt
// CHECK: -emit-pch
// CHECK: pchi-generation-usage.cpp.tmp.h.pchi

// Check that the PCH file is implicitly included via -include-pch
// RUN: %clang -c -include %t.h  %s -### 2> %t2.txt
// RUN: FileCheck %s -check-prefix=CHECK-INCLUDE-PCH -input-file=%t2.txt
// CHECK-INCLUDE-PCH: -include-pch
