// INTEL_CUSTOMIZATION
// Disable this test until we can replace homegrown printf.EXE
// XFAIL: system-windows
// END_INTEL_CUSTOMIZATION
// Test UTF8 BOM at start of file
// RUN: printf '\xef\xbb\xbf' > %t.c
﻿// RUN: echo '#ifdef TEST\n' >> %t.c
// RUN: echo '#include <string>' >> %t.c
// RUN: echo '#endif' >> %t.c
// RUN: %clang_cc1 -DTEST -print-dependency-directives-minimized-source %t.c 2>&1 | FileCheck %s

﻿// CHECK:      #ifdef TEST
// CHECK-NEXT: #include <string>
// CHECK-NEXT: #endif
