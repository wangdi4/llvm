// These tests check if "-fwhole-program-vtables" is not added in the command
// when -march=core-avx2 and -qopt-mem-layout-trans=4 are enabled, but
// -fwhole-program-vtables is not enabled.

// Check that -fwhole-program-vtables is turned off when -march=core-avx2 is
// enabled and -fwhole-program-vtables is not added by the user
// RUN: %clang -### -qopt-mem-layout-trans=4 -march=core-avx2 -flto -c %s 2>&1 | FileCheck --check-prefix=CHECK-VTABLE-OFF %s
// RUN: %clang_cl -### /Qopt-mem-layout-trans:4 -march=core-avx2 -Qipo -c %s 2>&1 | FileCheck --check-prefix=CHECK-VTABLE-OFF %s
// CHECK-VTABLE-OFF-NOT: "-fwhole-program-vtables"

// Check that -fwhole-program-vtables is turned on when -march=core-avx2 and
// -fwhole-program-vtables are added by the user
// RUN: %clang -### -qopt-mem-layout-trans=4 -march=core-avx2 -fwhole-program-vtables -flto -c %s 2>&1 | FileCheck --check-prefix=CHECK-VTABLE-ON %s
// RUN: %clang_cl -### /Qopt-mem-layout-trans:4 -march=core-avx2 -fwhole-program-vtables -Qipo -c %s 2>&1 | FileCheck --check-prefix=CHECK-VTABLE-ON %s
// CHECK-VTABLE-ON: "-fwhole-program-vtables"

// Check that -fwhole-program-vtables is turned on when -xCORE-AVX2 is enabled
// and -fwhole-program-vtables is not added by the user
// RUN: %clang -### -qopt-mem-layout-trans=4 -xCORE-AVX2 -flto -c %s 2>&1 | FileCheck --check-prefix=CHECK-AVX2 %s
// RUN: %clang_cl -### /Qopt-mem-layout-trans:4 -xCORE-AVX2 -Qipo -c %s 2>&1 | FileCheck --check-prefix=CHECK-AVX2 %s
// CHECK-AVX2: "-fwhole-program-vtables"