/// -fintel-pragma-prefetch
// RUN: %clang -c -### -fintel-pragma-prefetch %s 2>&1 \
// RUN:  | FileCheck %s -check-prefix PREFETCH
// RUN: %clang -c -### %s 2>&1 \
// RUN:  | FileCheck %s -check-prefix PREFETCH
// PREFETCH-NOT: "-fintel-pragma-prefetch"

/// -fno-intel-pragma-prefetch
// RUN: %clang -c -### -fno-intel-pragma-prefetch %s 2>&1 \
// RUN:  | FileCheck %s -check-prefix NO_PREFETCH
// NO_PREFETCH: "-fno-intel-pragma-prefetch"
