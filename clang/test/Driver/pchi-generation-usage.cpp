// This test generates a PCH header file with .pchi file extension and
// verifies that the driver does an implicit include of the .pchi file
// via '-include-pch foo.h.pchi' when using '-include foo.h'

// RUN: %clang -x c-header %S/../Modules/Inputs/codegen-flags/foo.h
// RUN: %clang -c -include %S/../Modules/Inputs/codegen-flags/foo.h  %S/../Modules/Inputs/codegen-flags/use.cpp -### 2> %t 
// RUN: FileCheck %s -input-file=%t
// CHECK: -include-pch
// CHECK: foo.h.pchi
