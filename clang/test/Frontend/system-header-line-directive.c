// RUN: %clang_cc1 -Wall %s -I %S/Inputs -isystem %S/Inputs/SystemHeaderPrefix -verify
// RUN: %clang_cc1 %s -E -o - -I %S/Inputs -isystem %S/Inputs/SystemHeaderPrefix | FileCheck %s
#include <noline.h>
#include <line-directive-in-system.h>

// expected-warning@line-directive.h:* {{type specifier missing, defaults to 'int'}}
#include "line-directive.h"

// This tests that "#line" directives in system headers preserve system
// header-ness just like GNU line markers that don't have filenames.  This was
// PR30752.

// INTEL
// Fix is under review https://reviews.llvm.org/D37079
// XFAIL: win32, cygwin

<<<<<<< HEAD
// expected-no-diagnostics

=======
>>>>>>> f1bf7e37ffcdaf7c93ef74e602c96d0d0c01e3a4
// CHECK: # {{[0-9]+}} "{{.*}}system-header-line-directive.c" 2
// CHECK: # 1 "{{.*}}noline.h" 1 3
// CHECK: foo();
// CHECK: # 4 "{{.*}}system-header-line-directive.c" 2
// CHECK: # 1 "{{.*}}line-directive-in-system.h" 1 3
//      The "3" below indicates that "foo.h" is considered a system header.
// CHECK: # 1 "foo.h" 3
// CHECK: foo();
// CHECK: # {{[0-9]+}} "{{.*}}system-header-line-directive.c" 2
// CHECK: # 1 "{{.*}}line-directive.h" 1
// CHECK: # 10 "foo.h"{{$}}
